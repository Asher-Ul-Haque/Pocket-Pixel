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
#include "shader.h"


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

// - - - Store actual display dimensions (from nativeOnSurfaceChanged)
static GLint currentDisplayWidth = 0;
static GLint currentDisplayHeight = 0;

// - - - Uniform locations
static GLint positionLoc;
static GLint texCoordLoc;
static GLint samplerLoc;
static GLint resolutionLoc;
static GLint shaderIndexLoc;
static GLint timeLoc;

// - - - Flag to signal shader reload on the GL thread - - -
static bool shaderNeedsReload = false;


// - - - JNI Callback to Kotlin UI - - -

static JavaVM* cachedJvm             = nullptr;
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
    // Check for GL error after glCreateShader
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      FORGE_LOG_ERROR("glCreateShader GL Error: 0x%x", error);
    }
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

// - - - Helper function to compile and link shader program, and get uniform locations - - -
bool setupShaderProgram()
{
  GLuint VERTEX_SHADER;
  GLuint FRAGMENT_SHADER;
  GLint  LINKED;
  GLint  INFO_LEN = 0;
  char*  INFO_LOG = nullptr;

  FORGE_LOG_INFO("Attempting to setup shader program (current index: %d)", currentShaderIndex);

  // - - - Delete existing program if any
  if (gbProgramObject != 0)
  {
    glDeleteProgram(gbProgramObject);
    gbProgramObject = 0;
    FORGE_LOG_INFO("Deleted old shader program.");
  }

  // - - - Load shaders from the shader.h file
  VERTEX_SHADER   = loadShader(GL_VERTEX_SHADER, gVertexShader);
  FRAGMENT_SHADER = loadShader(GL_FRAGMENT_SHADER, gFragmentShader); // Use the single combined fragment shader

  if (VERTEX_SHADER == 0 || FRAGMENT_SHADER == 0)
  {
    FORGE_LOG_ERROR("Failed to load shaders (vertex or fragment failed compilation). VERTEX_SHADER=0x%x, FRAGMENT_SHADER=0x%x", VERTEX_SHADER, FRAGMENT_SHADER);
    return false;
  }
  FORGE_LOG_INFO("Shaders compiled successfully.");

  gbProgramObject = glCreateProgram();
  if (gbProgramObject == 0)
  {
    FORGE_LOG_ERROR("Failed to create shader program.");
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      FORGE_LOG_ERROR("glCreateProgram GL Error: 0x%x", error);
    }
    return false;
  }
  FORGE_LOG_INFO("Shader program created: %d", gbProgramObject);


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
    FORGE_LOG_ERROR("Shader program linking failed.");
    return false;
  }
  FORGE_LOG_INFO("Shader program linked successfully.");

  // D - - - etach and delete shaders after linking (they are now part of the program)
  glDetachShader(gbProgramObject, VERTEX_SHADER);
  glDetachShader(gbProgramObject, FRAGMENT_SHADER);
  glDeleteShader(VERTEX_SHADER);
  glDeleteShader(FRAGMENT_SHADER);
  FORGE_LOG_INFO("Shaders detached and deleted.");

  // - - - Get all uniform and attribute locations AFTER linking
  glUseProgram(gbProgramObject);

  positionLoc     = glGetAttribLocation(gbProgramObject, "a_position");
  texCoordLoc     = glGetAttribLocation(gbProgramObject, "a_texCoord");
  samplerLoc      = glGetUniformLocation(gbProgramObject, "s_texture");
  resolutionLoc   = glGetUniformLocation(gbProgramObject, "u_Resolution");
  shaderIndexLoc  = glGetUniformLocation(gbProgramObject, "u_ShaderIndex");
  timeLoc         = glGetUniformLocation(gbProgramObject, "u_Time");


  if (positionLoc == -1 || texCoordLoc == -1 || samplerLoc == -1 || resolutionLoc == -1 || shaderIndexLoc == -1 || timeLoc == -1)
  {
    FORGE_LOG_FATAL("Failed to get one or more uniform/attribute locations! This is critical.");
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      FORGE_LOG_FATAL("glGetUniformLocation/glGetAttribLocation GL Error: 0x%x", error);
    }
    return false;
  }

  FORGE_LOG_INFO("All uniform/attribute locations retrieved successfully.");
  return true;
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

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // - - - Check if shader needs reloading or if program is not initialized
  if (shaderNeedsReload || gbProgramObject == 0)
  {
    FORGE_LOG_INFO("renderFrameGl: Shader needs reload or program is uninitialized. Attempting setup.");
    if (setupShaderProgram())
    {
      shaderNeedsReload = false;
      FORGE_LOG_INFO("renderFrameGl: Shader program successfully setup/reloaded.");
    }
    else
    {
      FORGE_LOG_ERROR("renderFrameGl: Failed to setup shader program. Screen may remain black.");
      return;
    }
  }

  glUseProgram(gbProgramObject); // - - - Ensure the correct program is active before setting uniforms

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

  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glEnableVertexAttribArray(texCoordLoc);
  glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

  // - - - Set uniforms (these locations are global and should be valid if setupShaderProgram succeeded)
  glUniform1i(samplerLoc, 0);
  glUniform2f(resolutionLoc, (GLfloat)currentDisplayWidth, (GLfloat)currentDisplayHeight);
  glUniform1i(shaderIndexLoc, currentShaderIndex);
  glUniform1f(timeLoc, (GLfloat)emuGetContext()->ticks / (GLfloat)4194304.0f);


  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
}


// - - - Sound - - -

void playAudio()
{
  APUcontext* ctx = apuGetContext();

  if (ctx->bufferPtr <= 0)
  {
    return; // No samples to play
  }

  JNIEnv* ENV = getJniEnv();
  if (ENV && gameboyClassGlobalRef && playAudioMethodId)
  {
    jbyteArray audioBuffer = ENV->NewByteArray(ctx->bufferPtr);

    if (audioBuffer)
    {
      ENV->SetByteArrayRegion(audioBuffer, 0, ctx->bufferPtr, (const jbyte*)ctx->sampleBuffer);
      ENV->CallStaticVoidMethod(gameboyClassGlobalRef, playAudioMethodId, audioBuffer);
      ENV->DeleteLocalRef(audioBuffer);
    }
    else
    {
      FORGE_LOG_ERROR("Failed to create new Java byte array for audio.");
    }
    ctx->bufferPtr = 0;
  }
  else
  {
    FORGE_LOG_ERROR("Cannot call Java nativePlayAudio: JNIEnv or class/method ID not cached.");
  }
}

void sendSerialByte(u8 BYTE, u8 SC)
{
  JNIEnv* ENV = getJniEnv();
  if (ENV && gameboyClassGlobalRef && sendByteMethodId)
  {
    jbyte sb = static_cast<jbyte>(BYTE);
    jbyte sc = static_cast<jbyte>(SC);
    ENV->CallStaticVoidMethod(gameboyClassGlobalRef, sendByteMethodId, sb, sc);
  }
}


// - - - | EMULATOR THREAD | - - -


// - - - Puase and Play - - -

void pauseEmulator()
{}

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
{}

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
    JNIEnv* ENV,
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
Java_just_somebody_templates_domain_GameBoy_nativeFlushSave(JNIEnv* ENV, jobject THIS)
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
  FORGE_LOG_INFO("nativeOnSurfaceCreated: Initializing OpenGL ES");

  // Setup the shader program for the first time
  // This will now happen on the GL thread.
  if (!setupShaderProgram()) {
    FORGE_LOG_ERROR("Failed to setup initial shader program in nativeOnSurfaceCreated! Screen may be black.");
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

  // glUseProgram is already called inside setupShaderProgram()
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
  // Store the actual display dimensions for the shader
  currentDisplayWidth = WIDTH;
  currentDisplayHeight = HEIGHT;
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
      "(BB)V"
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

extern "C"
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

// - - - JNI function to set the active shader
extern "C"
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetShader(
    JNIEnv* ENV,
    jobject THIS,
    jint SHADER_INDEX)
{
  // Check if the index is within the valid range (0 to 4 for 5 shaders)
  // The size of gFragmentShaders is not directly available here since it's an extern array.
  // We'll hardcode 5 for now, assuming 5 shaders.
  if (SHADER_INDEX >= 0 && SHADER_INDEX <= 4) // Max index is 4 for 5 shaders
  {
    currentShaderIndex = SHADER_INDEX;
    shaderNeedsReload = true; // Signal that the shader needs to be reloaded on the GL thread
    FORGE_LOG_INFO("Shader index set to: %d. Flagging for reload on GL thread.", currentShaderIndex);

    // Request a render. The renderFrameGl will then pick up the shaderNeedsReload flag.
    callJavaRequestRender();
  }
  else
  {
    FORGE_LOG_ERROR("Invalid shader index: %d", SHADER_INDEX);
  }
}


extern "C"
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeChangePallete(
    JNIEnv* ENV,
    jobject THIS,
    jint INDEX)
{
  setColorScheme(INDEX);
}

#ifdef __cplusplus
#endif
#endif // __ANDROID__