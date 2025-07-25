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
#include "GameBoyCore.h" // - - - Assumed to contain startEmulator(), stopEmulator()
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/emu.h"
#include "GameBoy/include/apu.h"
#include "GameBoy/include/cartridge.h"
#include "GameBoy/include/ppu.h" // - - - Assuming ppuGetContext() and PPUcontext are defined here


#ifdef __cplusplus
extern "C"
{
#endif


// - - - | GLOBAL VARIABLES | - - -


// - - - Emulator Thread Management - - -

static pthread_t        emulatorThread;           // - - - main loop
static pthread_mutex_t  isRunningMutex;           // - - - mutex to protect the running flag
static bool             isRunning       = false;  // - - - flag indicating whether the thread should continue running


// - - - Rendering Globals - - -

const i32 GB_SCREEN_WIDTH  = 160;
const i32 GB_SCREEN_HEIGHT = 144;

static GLuint gbTextureId      = 0; // - - - openGL texture ID for the framebuffer
static GLuint gbProgramObject  = 0; // - - - openGL program ID for rendering


// - - - Thread Synchronization for Frame Buffer - - -
// TODO: Remember to actually find out how to signal from the gameboy that the frame is ready

static pthread_mutex_t frameMutex;
static pthread_cond_t  frameReadyCv;
static bool            newFrameAvailable = false;

// - - - JNI Callback to Kotlin UI - - -

static JavaVM* cachedJvm             = nullptr;
static jclass     gameboyClassGlobalRef = nullptr;
static jmethodID  requestRenderMethodId = nullptr;
static jmethodID  playAudioMethodId     = nullptr;
static jmethodID  stopAudioMethodId     = nullptr; // Added for completeness, though not used in this flow yet


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

  PPUcontext* ppuCTX       = nullptr;
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
// This function is now responsible for processing GameBoy audio
// and passing it to Kotlin via JNI.
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

// - - - | EMULATOR THREAD | - - -


// - - - Emulator Tick Loop - - -

// - - - The main emulation loop running on a separate thread.
void* tickLoop(void* ARG)
{
  u64 lastFrame = 0;
  // APUcontext* apu_ctx = apuGetContext(); // This is not needed here as apuUpdate handles the audio buffering and calling playAudio

  while (true)
  {
    pthread_mutex_lock(&isRunningMutex);
    if (!isRunning)
    {
      pthread_mutex_unlock(&isRunningMutex);
      break;
    }
    pthread_mutex_unlock(&isRunningMutex);

    cpuTick(); // cpuTick() will internally call apuUpdate() which then calls playAudio()

    // Only trigger render if a full frame has been generated (e.g. at VBlank)
    u64 currentFrame = ppuGetContext()->currentFrame;
    if (lastFrame != currentFrame)
    {
      lastFrame = currentFrame;
      callJavaRequestRender(); // Let Java decide when to draw
    }
  }

  return nullptr;
}


// - - - | JNI EXPORTED FUNCTIONS | - - -


// - - - Get Audio Buffer - - -
/*
// This function is no longer needed as audio data is pushed from C++ to Kotlin.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetAudioBuffer(
  JNIEnv* ENV,
  jobject    THIZ,
  jbyteArray AUDIO_BUFFER)
{
  jbyte* BUFFER = ENV->GetByteArrayElements(AUDIO_BUFFER, nullptr);
  //getAudio(reinterpret_cast<u8*>(BUFFER));
  ENV->ReleaseByteArrayElements(AUDIO_BUFFER, BUFFER, 0);
}
*/

// - - - Loads the Game Boy ROM i32o the native core.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
    JNIEnv* ENV,
    jobject    THIZ,
    jbyteArray ROM,
    jint       SIZE)
{
  jbyte* BUFFER = ENV->GetByteArrayElements(ROM, nullptr);
  cartridgeLoad(reinterpret_cast<u8*>(BUFFER), SIZE);
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

// - - - Starts the Game Boy emulator thread.
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
    JNIEnv* ENV,
    jobject THIZ)
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
  startEmulator();
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
    // Not a fatal error, but good to log.
  }

  // - - - Initialize pthread mutexes and condition variables
  pthread_mutex_init(&frameMutex, NULL);
  pthread_cond_init(&frameReadyCv, NULL);
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
  pthread_mutex_destroy(&frameMutex);
  pthread_cond_destroy(&frameReadyCv);
  pthread_mutex_destroy(&isRunningMutex);

  // - - - Clear cached JNI references
  cachedJvm             = nullptr;
  requestRenderMethodId = nullptr;
  playAudioMethodId     = nullptr; // Clear new method ID
  stopAudioMethodId     = nullptr; // Clear new method ID
}

} // extern "C"

#ifdef __cplusplus
#endif
#endif // __ANDROID__
