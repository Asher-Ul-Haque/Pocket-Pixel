#ifdef __ANDROID__
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <ctime>
#include <atomic>
#include <chrono>

// - - - OpenGL ES Headers - - -

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// - - - Project Specific Headers - - -

#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoyCore.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/emu.h"
#include "GameBoy/include/apu.h"
#include "GameBoy/include/cartridge.h"
#include "GameBoy/include/ppu.h"
#include "GameBoy/include/serial.h"


#ifdef __cplusplus
extern "C"
{
#endif


// - - - | GLOBAL VARIABLES | - - -


// - - - Emulator Thread Management - - -

static pthread_t        emulatorThread;           // - - - main loop
static pthread_mutex_t  isRunningMutex;           // - - - mutex to protect the running flag
static bool             isRunning       = false;  // - - - flag indicating whether the thread should continue running
static pthread_mutex_t  pauseMutex      = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   pauseCond       = PTHREAD_COND_INITIALIZER;
static bool             isPaused        = false;



// - - - Rendering Globals - - -

const i32 GB_SCREEN_WIDTH  = 160;
const i32 GB_SCREEN_HEIGHT = 144;

static GLuint gbTextureId      = 0; // - - - openGL texture ID for the framebuffer
static GLuint gbProgramObject  = 0; // - - - openGL program ID for rendering


// - - - JNI Callback to Kotlin UI - - -

static JavaVM*    cachedJvm             = nullptr;
static jclass     gameboyClassGlobalRef = nullptr;
static jmethodID  requestRenderMethodId = nullptr;
static jmethodID  playAudioMethodId     = nullptr;
static jmethodID  stopAudioMethodId     = nullptr;
static jmethodID  sendByteMethodId      = nullptr;


// - - - JNI Method IDs for save/load callbacks to Kotlin - - -
static jmethodID  saveRamToFileMethodId       = nullptr;
static jmethodID  loadRamFromFileMethodId     = nullptr;
static jmethodID  getExpectedSaveSizeMethodId = nullptr;


// - - - FPS Control - - -

static const long long TARGET_FPS               = 60;
static const long long NANOSECONDS_PER_SECOND   = 1000000000LL;
static const long long TARGET_FRAME_DURATION_NS = NANOSECONDS_PER_SECOND / TARGET_FPS;
static struct timespec prevFrameTime = {0, 0};


// - - - Shader Source - - -

// - - - vertex shader
const char* gVertexShader =
  "attribute vec4 a_position;\n"
  "attribute vec2 a_texCoord;\n"
  "varying vec2 v_texCoord;\n"
  "void main() {\n"
  "  gl_Position = a_position;\n"
  "  v_texCoord = a_texCoord;\n"
  "}\n";


// - - - Fragment Shader Source
const char* gFragmentShader =
  "precision mediump float;\n"
  "varying vec2 v_texCoord;\n"
  "uniform sampler2D s_texture;\n"
  "void main() {\n"
  "  gl_FragColor = texture2D(s_texture, v_texCoord);\n"
  "}\n";


// - - - | HELPER FUNCTIONS | - - -


// - - - JNI Environment Retrieval - - -

// - - - Attaches the current native thread to the JVM and returns a JNIEnv pointer. Returns JNIEnv* on success, or nullptr on failure.
JNIEnv* getJniEnv()
{
  JNIEnv* ENV;
  i32     STATUS;

  // - - - GetEnv can return JNI_EDETACHED if the thread is not attached. In that case attach it
  STATUS = cachedJvm->GetEnv((void**)&ENV, JNI_VERSION_1_6);
  if (STATUS == JNI_EDETACHED)
  {
    FORGE_LOG_INFO("Attaching current thread to JVM");
    STATUS = cachedJvm->AttachCurrentThread(&ENV, NULL);
    if (STATUS != JNI_OK)
    {
      FORGE_LOG_ERROR("Failed to attach current thread to JVM: %d", STATUS);
      return nullptr;
    }
  }
  return ENV;
}


// - - - Java Render Request - - -

// - - - Calls the static requestRenderFromNative() method in GameBoy.kt, signaling GLSurfaceView to redraw.
void callJavaRequestRender()
{
  JNIEnv* ENV = getJniEnv();
  if (ENV && gameboyClassGlobalRef && requestRenderMethodId)
  {
    ENV->CallStaticVoidMethod(gameboyClassGlobalRef, requestRenderMethodId);
  }
  else  FORGE_LOG_ERROR("Cannot call Java requestRender: JNIEnv or class/method ID not cached.");
}


// - - - OpenGL Shader Compilation - - -

// - - - compile from source
GLuint loadShader(GLenum TYPE, const char* SOURCE)
{
  GLuint  SHADER   = glCreateShader(TYPE);
  GLint   COMPILED;
  GLint   INFO_LEN = 0;
  char* INFO_LOG = nullptr;

  if (SHADER == 0)
  {
    FORGE_LOG_ERROR("Failed to create shader of type %d", TYPE);
    return 0;
  }
  glShaderSource(SHADER, 1, &SOURCE, NULL);
  glCompileShader(SHADER);

  glGetShaderiv(SHADER, GL_COMPILE_STATUS, &COMPILED);
  if (!COMPILED)
  {
    glGetShaderiv(SHADER, GL_INFO_LOG_LENGTH, &INFO_LEN);
    if (INFO_LEN > 1)
    {
      INFO_LOG = (char*)malloc(sizeof(char) * INFO_LEN);
      glGetShaderInfoLog(SHADER, INFO_LEN, NULL, INFO_LOG);
      FORGE_LOG_ERROR("Error compiling shader (type %d):\n%s", TYPE, INFO_LOG);
      free(INFO_LOG);
    }
    glDeleteShader(SHADER);
    return 0;
  }
  return SHADER;
}


// - - - OpenGL Frame Rendering - - -

// - - - actually render the fram
void renderFrameGl()
{
  GLfloat vertices[] =
    {
      -1.0f,  1.0f, 0.0f, // - - - Top-left
      -1.0f, -1.0f, 0.0f, // - - - Bottom-left
       1.0f, -1.0f, 0.0f, // - - - Bottom-right
       1.0f,  1.0f, 0.0f  // - - - Top-right
    };

  GLfloat texCoords[] =
    {
      0.0f, 0.0f, // - - - Top-left of texture
      0.0f, 1.0f, // - - - Bottom-left of texture
      1.0f, 1.0f, // - - - Bottom-right of texture
      1.0f, 0.0f  // - - - Top-right of texture
    };

  PPUContext* ppuCTX       = nullptr;
  GLushort    indices[]    = { 0, 1, 2, 0, 2, 3 };
  GLint       positionLoc;
  GLint       texCoordLoc;
  GLint       samplerLoc;

  // - - - Wait for a new frame to be available from the emulator thread

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gbProgramObject);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gbTextureId);

  // - - - Directly use the emulator's frameBuffer
  ppuCTX = ppuGetContext();
  if (ppuCTX && ppuCTX->frameBuffer)
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, ppuCTX->frameBuffer);
  }
  else
  {
    FORGE_LOG_ERROR("Emulator frameBuffer is NULL in renderFrameGl!");
  }

  positionLoc = glGetAttribLocation(gbProgramObject, "a_position");
  texCoordLoc = glGetAttribLocation(gbProgramObject, "a_texCoord");
  samplerLoc  = glGetUniformLocation(gbProgramObject, "s_texture");

  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glEnableVertexAttribArray(texCoordLoc);
  glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

  glUniform1i(samplerLoc, 0);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
}


// - - - Sound - - -

void playAudio()
{
  APUcontext* ctx = apuGetContext();

  // Check if there are any samples to play.
  // ctx->bufferPtr holds the number of 8-bit bytes (stereo samples) accumulated.
  if (ctx->bufferPtr <= 0) {
      return; // No samples to play
  }

  JNIEnv* ENV = getJniEnv(); // Get JNIEnv for this thread
  if (ENV && gameboyClassGlobalRef && playAudioMethodId)
  {
    // Create a new Java byte array with the exact size of the accumulated stereo data
    jbyteArray audioBuffer = ENV->NewByteArray(ctx->bufferPtr);

    if (audioBuffer)
    {
      // Copy the raw 8-bit stereo samples from apuCtx.sampleBuffer into the Java byte array
      // ctx->sampleBuffer already contains interleaved L/R 8-bit samples.
      ENV->SetByteArrayRegion(audioBuffer, 0, ctx->bufferPtr, (const jbyte*)ctx->sampleBuffer);

      // Call the Kotlin method to play audio
      ENV->CallStaticVoidMethod(gameboyClassGlobalRef, playAudioMethodId, audioBuffer);

      // Delete the local reference to the Java byte array
      ENV->DeleteLocalRef(audioBuffer);
    }
    else
    {
      FORGE_LOG_ERROR("Failed to create new Java byte array for audio.");
    }
    ctx->bufferPtr = 0; // Reset APU buffer pointer after sending samples to Kotlin
  }
  else
  {
    FORGE_LOG_ERROR("Cannot call Java nativePlayAudio: JNIEnv or class/method ID not cached.");
  }
}

void sendSerialByte(u8 BYTE)
{
  JNIEnv* ENV = getJniEnv();
  if (ENV && gameboyClassGlobalRef && sendByteMethodId)
  {
    jbyte signedByte = static_cast<jbyte>(BYTE);  // Fix: Safe cast to signed
    ENV->CallStaticVoidMethod(gameboyClassGlobalRef, sendByteMethodId, signedByte);
  }
}


// - - - | EMULATOR THREAD | - - -


// - - - Puase and Play - - -

void pauseEmulator()
{

}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativePauseEmulator(
  JNIEnv* ENV,
  jobject THIZ)
{
  pthread_mutex_lock(&pauseMutex);
  isPaused = true;
  pthread_mutex_unlock(&pauseMutex);
}


void resumeEmulator()
{

}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeResumeEmulator(
  JNIEnv* ENV,
  jobject THIZ)
{
  pthread_mutex_lock(&pauseMutex);
  isPaused = false;
  pthread_cond_signal(&pauseCond);
  pthread_mutex_unlock(&pauseMutex);
}


// - - - Emulator Tick Loop - - -

// - - - The main emulation loop running on a separate thread.
void* tickLoop(void* ARG)
{
  u64 lastFrame = 0;

  while (true)
  {
    pthread_mutex_lock(&isRunningMutex);
    if (!isRunning)
    {
      pthread_mutex_unlock(&isRunningMutex);
      break;
    }
    pthread_mutex_unlock(&isRunningMutex);

    // - - - Check if paused
    pthread_mutex_lock(&pauseMutex);
    while (isPaused)
    {
      pthread_cond_wait(&pauseCond, &pauseMutex); // - - - Block here until resumed
    }
    pthread_mutex_unlock(&pauseMutex);

    cpuTick();
  }

  return nullptr;
}


void render()
{
  struct timespec currentTime;
  clock_gettime(CLOCK_MONOTONIC, &currentTime);

  long long elapsed_ns = (long long)(currentTime.tv_sec - prevFrameTime.tv_sec) * NANOSECONDS_PER_SECOND +
                         (currentTime.tv_nsec - prevFrameTime.tv_nsec);

  if (elapsed_ns < TARGET_FRAME_DURATION_NS)
  {
    long long sleep_ns = TARGET_FRAME_DURATION_NS - elapsed_ns;

    struct timespec req =
        {
            .tv_sec = (time_t)(sleep_ns / NANOSECONDS_PER_SECOND),
            .tv_nsec = (long)(sleep_ns % NANOSECONDS_PER_SECOND)
        };

    nanosleep(&req, NULL);
  }

  callJavaRequestRender();
  clock_gettime(CLOCK_MONOTONIC, &prevFrameTime);
}


// - - - | JNI EXPORTED FUNCTIONS | - - -


// - - - Loads the Game Boy ROM into the native core.

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
  JNIEnv*     ENV,
  jobject    THIZ,
  jbyteArray ROM,
  jint       SIZE)
{
  jbyte* BUFFER = ENV->GetByteArrayElements(ROM, nullptr);

  // - - - Define and initialize CartridgeFileIO for Android
  static CartridgeFileIO androidFileIO =
    {
      .saveRamToFile = [](const u8* ram_data, u32 ram_size) -> bool
        {
          JNIEnv* env = getJniEnv();
          if (!env || !gameboyClassGlobalRef || !saveRamToFileMethodId) {
              FORGE_LOG_ERROR("JNI: Cannot call saveRamToFile: JNIEnv or method ID not cached.");
              return false;
          }
          jbyteArray j_ram_data = env->NewByteArray(ram_size);
          env->SetByteArrayRegion(j_ram_data, 0, ram_size, (const jbyte*)ram_data);

          // Call Kotlin method without rom_filepath
          jboolean result = env->CallStaticBooleanMethod(gameboyClassGlobalRef, saveRamToFileMethodId, j_ram_data, ram_size);

          env->DeleteLocalRef(j_ram_data);
          return (bool)result;
        },
      .loadRamFromFile = [](u8* ram_data_buffer, u32 buffer_size) -> bool
        {
          JNIEnv* env = getJniEnv();
          if (!env || !gameboyClassGlobalRef || !loadRamFromFileMethodId)
          {
            FORGE_LOG_ERROR("JNI: Cannot call loadRamFromFile: JNIEnv or method ID not cached.");
            return false;
          }
          jbyteArray j_ram_data_buffer = env->NewByteArray(buffer_size); // Create a Java byte array for the data

          // - - - Call Kotlin method
          jboolean result = env->CallStaticBooleanMethod(gameboyClassGlobalRef, loadRamFromFileMethodId, j_ram_data_buffer, buffer_size);

          if ((bool)result) env->GetByteArrayRegion(j_ram_data_buffer, 0, buffer_size, (jbyte*)ram_data_buffer);
          else              memset(ram_data_buffer, 0, buffer_size);

          env->DeleteLocalRef(j_ram_data_buffer);
          return (bool)result;
        },
      .getExpectedSaveSize = []() -> u32
        {
          JNIEnv* env = getJniEnv();
          if (!env || !gameboyClassGlobalRef || !getExpectedSaveSizeMethodId)
          {
              FORGE_LOG_ERROR("JNI: Cannot call getExpectedSaveSize: JNIEnv or method ID not cached.");
              return 0;
          }
          jint result = env->CallStaticIntMethod(gameboyClassGlobalRef, getExpectedSaveSizeMethodId);
          return (u32)result;
        }
    };


  cartridgeLoad(reinterpret_cast<u8*>(BUFFER), SIZE, &androidFileIO);

  ENV->ReleaseByteArrayElements(ROM, BUFFER, JNI_ABORT);
}

// - - - Sets the state of a Game Boy button in the native core.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(
  JNIEnv* ENV,
  jobject   THIZ,
  jint      BUTTON,
  jboolean  PRESSED)
{
  setButton(static_cast<Buttons>(BUTTON), static_cast<bool>(PRESSED));
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeFlushSave(JNIEnv* ENV)
{
  if (isRunning) cartridgeFlushRAM();
}

// - - - Starts the Game Boy emulator thread.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
    JNIEnv* ENV,
    jobject THIZ,
    jfloatArray VOLUMES)
{
  pthread_mutex_lock(&isRunningMutex);
  if (isRunning)
  {
    pthread_mutex_unlock(&isRunningMutex);
    return; // - - - Emulator is already running
  }
  isRunning = true;
  pthread_mutex_unlock(&isRunningMutex);

  // - - - Initialize and allocate frameBuffer within the emulator core
  if (VOLUMES == nullptr || ENV->GetArrayLength(VOLUMES) != 5) return;

  jfloat nativeVolumes[5];
  ENV->GetFloatArrayRegion(VOLUMES, 0, 5, nativeVolumes);

  startEmulator(nativeVolumes);
  // - - - Create the emulator tick loop thread
  pthread_create(&emulatorThread, NULL, tickLoop, NULL);
}

// - - - Stops the Game Boy emulator thread and releases resources.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(
  JNIEnv* ENV,
  jobject THIZ)
{
  pthread_mutex_lock(&isRunningMutex);
  if (!isRunning)
  {
    pthread_mutex_unlock(&isRunningMutex);
    return; // - - - Emulator is not running
  }
  isRunning = false; // - - - Signal the tickLoop to stop
  pthread_mutex_unlock(&isRunningMutex);

  // - - - Wait for the emulator thread to finish its execution
  pthread_join(emulatorThread, NULL);

  // - - - Deallocate frameBuffer and clean up within the emulator core
  stopEmulator();
}


// - - - Initializes OpenGL ES resources when the rendering surface is created.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceCreated(JNIEnv* ENV, jobject THIZ)
{
  GLuint  VERTEX_SHADER;
  GLuint  FRAGMENT_SHADER;
  GLint   LINKED;
  GLint   INFO_LEN = 0;
  char* INFO_LOG = nullptr;

  FORGE_LOG_INFO("nativeOnSurfaceCreated: Initializing OpenGL ES");

  VERTEX_SHADER   = loadShader(GL_VERTEX_SHADER, gVertexShader);
  FRAGMENT_SHADER = loadShader(GL_FRAGMENT_SHADER, gFragmentShader);

  gbProgramObject = glCreateProgram();
  glAttachShader(gbProgramObject, VERTEX_SHADER);
  glAttachShader(gbProgramObject, FRAGMENT_SHADER);
  glLinkProgram(gbProgramObject);

  glGetProgramiv(gbProgramObject, GL_LINK_STATUS, &LINKED);
  if (!LINKED)
  {
    glGetProgramiv(gbProgramObject, GL_INFO_LOG_LENGTH, &INFO_LEN);
    if (INFO_LEN > 1)
    {
      INFO_LOG = (char*)malloc(sizeof(char) * INFO_LEN);
      glGetProgramInfoLog(gbProgramObject, INFO_LEN, NULL, INFO_LOG);
      FORGE_LOG_ERROR("Error linking program:\n%s", INFO_LOG);
      free(INFO_LOG);
    }
    glDeleteProgram(gbProgramObject);
    gbProgramObject = 0;
    return;
  }

  glGenTextures(1, &gbTextureId);
  glBindTexture(GL_TEXTURE_2D, gbTextureId);

  // - - - Set texture parameters for nearest filtering (pixelated look) and clamping
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // - - - Allocate texture memory on the GPU. Data will be updated with glTexSubImage2D.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glUseProgram(gbProgramObject);
}


// - - - Updates the OpenGL ES viewport when the rendering surface changes size.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceChanged(
  JNIEnv* ENV,
  jobject THIZ,
  jint    WIDTH,
  jint    HEIGHT)
{
  FORGE_LOG_INFO("nativeOnSurfaceChanged: %d x %d", WIDTH, HEIGHT);
  glViewport(0, 0, WIDTH, HEIGHT);
}


// - - - Triggers the OpenGL ES rendering for a new frame.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnDrawFrame(JNIEnv* ENV, jobject THIZ)
{
  renderFrameGl();
}


// - - - | JNI LIFECYCLE FUNCTIONS | - - -


// - - - Called when the native library is loaded by the JVM, caching essential JNI references.
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* VM, void* RESERVED)
{
  JNIEnv* ENV;
  jclass  LOCAL_GAMEBOY_CLASS;

  FORGE_LOG_INFO("JNI_OnLoad: Caching JVM and method IDs.");
  cachedJvm = VM;

  if (VM->GetEnv((void**)&ENV, JNI_VERSION_1_6) != JNI_OK)
  {
    FORGE_LOG_ERROR("Failed to get JNIEnv on JNI_OnLoad");
    return JNI_ERR;
  }

  // - - - Find the GameBoy Java class and create a global reference
  LOCAL_GAMEBOY_CLASS = ENV->FindClass("just/somebody/templates/domain/GameBoy");
  if (!LOCAL_GAMEBOY_CLASS)
  {
    FORGE_LOG_ERROR("Failed to find GameBoy class");
    return JNI_ERR;
  }
  gameboyClassGlobalRef = (jclass)ENV->NewGlobalRef(LOCAL_GAMEBOY_CLASS);
  ENV->DeleteLocalRef(LOCAL_GAMEBOY_CLASS); // - - - Release local reference

  // - - - Get the method ID for requestRenderFromNative
  requestRenderMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "requestRenderFromNative",
      "()V");
  if (!requestRenderMethodId)
  {
    FORGE_LOG_ERROR("Failed to find method ID for requestRenderFromNative");
    return JNI_ERR;
  }

  // - - - Get the method ID for sendingBytes
  sendByteMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "sendByte",
      "(B)V"
  );
  if (!sendByteMethodId)
  {
    FORGE_LOG_ERROR("Failed to find method ID for sendByte");
    return JNI_ERR;
  }

  // - - - Get the method ID for audio playing
  playAudioMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "nativePlayAudio",
      "([B)V"); // Signature: takes a byte array ([B) and returns void (V)
  if (!playAudioMethodId)
  {
    FORGE_LOG_ERROR("Failed to find method ID for nativePlayAudio");
    return JNI_ERR;
  }

  // - - - Get the method ID for stopping audio (for cleanup)
  stopAudioMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "nativeStopAudio",
      "()V"); // Signature: takes no arguments () and returns void (V)
  if (!stopAudioMethodId)
  {
    FORGE_LOG_WARNING("Failed to find method ID for nativeStopAudio. This is optional but recommended.");
  }

  // - - - Get Method IDs for save/load callbacks - - -
  saveRamToFileMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "saveRamToFile",
      "([BI)Z"); // (byte[] ram_data, int ram_size) -> boolean
  if (!saveRamToFileMethodId)
  {
    FORGE_LOG_ERROR("Failed to find method ID for saveRamToFile");
    return JNI_ERR;
  }

  loadRamFromFileMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "loadRamFromFile",
      "([BI)Z"); // (byte[] ram_data_buffer, int buffer_size) -> boolean
  if (!loadRamFromFileMethodId)
  {
      FORGE_LOG_ERROR("Failed to find method ID for loadRamFromFile");
      return JNI_ERR;
  }

  getExpectedSaveSizeMethodId = ENV->GetStaticMethodID(
      gameboyClassGlobalRef,
      "getExpectedSaveSize",
      "()I"); // () -> int
  if (!getExpectedSaveSizeMethodId)
  {
      FORGE_LOG_ERROR("Failed to find method ID for getExpectedSaveSize");
      return JNI_ERR;
  }


  // - - - Initialize pthread mutexes and condition variables
  pthread_mutex_init(&isRunningMutex, NULL);

  return JNI_VERSION_1_6;
}

// - - - Called when the native library is unloaded, cleaning up resources.
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* VM, void* RESERVED)
{
  JNIEnv* ENV;

  FORGE_LOG_INFO("JNI_OnUnload: Cleaning up resources.");

  // - - - Attempt to get JNIEnv, but proceed with cleanup even if it fails
  if (VM->GetEnv((void**)&ENV, JNI_VERSION_1_6) == JNI_OK)
  {
    if (gameboyClassGlobalRef)
    {
      ENV->DeleteGlobalRef(gameboyClassGlobalRef);
      gameboyClassGlobalRef = nullptr;
    }
  }

  // - - - Ensure emulator thread is stopped and joined before cleaning up
  pthread_mutex_lock(&isRunningMutex);
  if (isRunning)
  {
    isRunning = false; // - - - Signal the tickLoop to stop
    pthread_mutex_unlock(&isRunningMutex);
    stopEmulator(); // - - - Signal emulator core to stop and free its buffer
    pthread_join(emulatorThread, NULL); // - - - Wait for thread to terminate
  }
  else
  {
    pthread_mutex_unlock(&isRunningMutex);
  }

  // - - - OpenGL ES resource cleanup
  if (gbProgramObject != 0)
  {
    glDeleteProgram(gbProgramObject);
    gbProgramObject = 0;
  }
  if (gbTextureId != 0)
  {
    glDeleteTextures(1, &gbTextureId);
    gbTextureId = 0;
  }

  // - - - frameBuffer is managed by stopEmulator(), no need to delete here.

  // - - - Destroy pthread mutexes and condition variables
  pthread_mutex_destroy(&isRunningMutex);

  // - - - Clear cached JNI references
  cachedJvm                   = nullptr;
  requestRenderMethodId       = nullptr;
  playAudioMethodId           = nullptr;
  stopAudioMethodId           = nullptr;
  saveRamToFileMethodId       = nullptr;
  loadRamFromFileMethodId     = nullptr;
  getExpectedSaveSizeMethodId = nullptr;
  sendByteMethodId            = nullptr;

  pthread_mutex_destroy(&pauseMutex);
  pthread_cond_destroy(&pauseCond);

}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeRecieveByte(
    JNIEnv* ENV,
    jobject THIS,
    jbyte BYTE)
{ serialReceiveNetworkByte((u8)BYTE); }

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetVolumes(
  JNIEnv* ENV,
  jobject THIS,
  jfloatArray VOLUMES)
{
  if (VOLUMES == nullptr || ENV->GetArrayLength(VOLUMES) != 5) return;

  jfloat nativeVolumes[5];
  ENV->GetFloatArrayRegion(VOLUMES, 0, 5, nativeVolumes);

  apuSetVolume(nativeVolumes);}
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeChangePallete
(
  JNIEnv* ENV,
  jobject THIS,
  jint INDEX)
{
  setColorScheme(INDEX);
}

#ifdef __cplusplus
#endif
#endif // __ANDROID__

