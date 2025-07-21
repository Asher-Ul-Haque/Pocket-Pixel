#ifdef __ANDROID__
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <atomic>
#include <chrono>

// Include necessary OpenGL ES headers
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoyCore.h" // Assumed to contain startEmulator(), stopEmulator()
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/emu.h"
#include "GameBoy/include/cartridge.h"
#include "GameBoy/include/ppu.h" // Assuming ppuGetContext() and PPUcontext are defined here

#ifdef __cplusplus
extern "C"
{
#endif

// - - - Emulator Thread Management - - -
static pthread_t            emulatorThread;
static pthread_mutex_t      isRunningMutex; // Mutex to protect isRunning flag
static bool                 isRunning       = false;

// - - - Rendering Globals - - -
const int GB_SCREEN_WIDTH  = 160;
const int GB_SCREEN_HEIGHT = 144;

static GLuint gbTextureId      = 0;
static GLuint gbProgramObject  = 0;
// gbPixelBuffer is no longer needed as a separate allocation,
// as the frameBuffer is now managed by the emulator core.
// We will directly use ppuGetContext()->frameBuffer.

// - - - Thread Synchronization for Frame Buffer - - -
static pthread_mutex_t frameMutex;
static pthread_cond_t  frameReadyCv;
static bool            newFrameAvailable = false;

// - - - JNI Callback to Kotlin UI - - -
static JavaVM* cachedJvm            = nullptr;
static jclass           gameboyClassGlobalRef = nullptr;
static jmethodID        requestRenderMethodId = nullptr;

/**
 * @brief Attaches the current thread to the JVM and returns a JNIEnv.
 * @return JNIEnv* The JNI environment pointer, or nullptr on failure.
 */
JNIEnv* getJniEnv()
{
  JNIEnv* env;
  // GetEnv can return JNI_EDETACHED if the thread is not attached.
  // In that case, attach it.
  int status = cachedJvm->GetEnv((void**)&env, JNI_VERSION_1_6);
  if (status == JNI_EDETACHED)
  {
    FORGE_LOG_INFO("Attaching current thread to JVM");
    status = cachedJvm->AttachCurrentThread(&env, NULL);
    if (status != JNI_OK)
    {
      FORGE_LOG_ERROR("Failed to attach current thread to JVM: %d", status);
      return nullptr;
    }
  }
  return env;
}

/**
 * @brief Calls the static requestRenderFromNative() method in GameBoy.kt,
 * signaling GLSurfaceView to redraw.
 */
void callJavaRequestRender()
{
  JNIEnv* env = getJniEnv();
  if (env && gameboyClassGlobalRef && requestRenderMethodId)
  {
    env->CallStaticVoidMethod(gameboyClassGlobalRef, requestRenderMethodId);
  }
  else
  {
    FORGE_LOG_ERROR("Cannot call Java requestRender: JNIEnv or class/method ID not cached.");
  }
}

/**
 * @brief Compiles an OpenGL ES shader from source.
 * @param TYPE The shader type (e.g., GL_VERTEX_SHADER, GL_FRAGMENT_SHADER).
 * @param SOURCE The shader source code string.
 * @return GLuint The compiled shader ID, or 0 on failure.
 */
GLuint loadShader(GLenum TYPE, const char* SOURCE)
{
  GLuint shader = glCreateShader(TYPE);
  if (shader == 0)
  {
    FORGE_LOG_ERROR("Failed to create shader of type %d", TYPE);
    return 0;
  }
  glShaderSource(shader, 1, &SOURCE, NULL);
  glCompileShader(shader);

  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
  {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1)
    {
      char* infoLog = (char*)malloc(sizeof(char) * infoLen);
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      FORGE_LOG_ERROR("Error compiling shader (type %d):\n%s", TYPE, infoLog);
      free(infoLog);
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

/**
 * @brief Performs the actual OpenGL ES rendering of the Game Boy frame.
 * It waits for a new frame to be available from the emulator thread
 * and then updates the texture with the emulator's frameBuffer.
 */
void renderFrameGl()
{
  // Wait for a new frame to be available from the emulator thread
  pthread_mutex_lock(&frameMutex);
  while (!newFrameAvailable)
  {
    pthread_cond_wait(&frameReadyCv, &frameMutex);
  }
  newFrameAvailable = false; // Consume the frame
  pthread_mutex_unlock(&frameMutex);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gbProgramObject);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gbTextureId);

  // Directly use the emulator's frameBuffer
  PPUcontext* ppu_ctx = ppuGetContext();
  if (ppu_ctx && ppu_ctx->frameBuffer)
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, ppu_ctx->frameBuffer);
  }
  else
  {
    FORGE_LOG_ERROR("Emulator frameBuffer is NULL in renderFrameGl!");
  }

  GLfloat vertices[] =
      {
          -1.0f,  1.0f, 0.0f, // Top-left
          -1.0f, -1.0f, 0.0f, // Bottom-left
          1.0f, -1.0f, 0.0f, // Bottom-right
          1.0f,  1.0f, 0.0f  // Top-right
      };

  GLfloat texCoords[] =
      {
          0.0f, 0.0f, // Top-left of texture
          0.0f, 1.0f, // Bottom-left of texture
          1.0f, 1.0f, // Bottom-right of texture
          1.0f, 0.0f  // Top-right of texture
      };

  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  GLint positionLoc = glGetAttribLocation(gbProgramObject, "a_position");
  GLint texCoordLoc = glGetAttribLocation(gbProgramObject, "a_texCoord");
  GLint samplerLoc  = glGetUniformLocation(gbProgramObject, "s_texture");

  glEnableVertexAttribArray(positionLoc);
  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

  glEnableVertexAttribArray(texCoordLoc);
  glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

  glUniform1i(samplerLoc, 0);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glDisableVertexAttribArray(positionLoc);
  glDisableVertexAttribArray(texCoordLoc);
}

// - - - Vertex Shader Source
const char* gVertexShader =
    "attribute vec4 a_position;\n"
    "attribute vec2 a_texCoord;\n"
    "varying vec2 v_texCoord;\n"
    "void main() {\n"
    "    gl_Position = a_position;\n"
    "    v_texCoord = a_texCoord;\n"
    "}\n";

// - - - Fragment Shader Source
const char* gFragmentShader =
    "precision mediump float;\n"
    "varying vec2 v_texCoord;\n"
    "uniform sampler2D s_texture;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(s_texture, v_texCoord);\n"
    "}\n";

/**
 * @brief The main emulation loop running on a separate thread.
 * It continuously ticks the Game Boy CPU, and signals the rendering
 * thread when a new frame is ready.
 * @param ARG Unused thread argument.
 * @return void* Always returns nullptr.
 */
void* tickLoop(void* ARG)
{
  // The target FPS and frame duration are no longer used for explicit sleep,
  // allowing the emulator to run at maximum speed.
  // const long targetFps        = 60;
  // const long frameDurationNs  = 1000000000L / targetFps; // Nanoseconds per frame

  bool currentIsRunning;
  pthread_mutex_lock(&isRunningMutex);
  currentIsRunning = isRunning;
  pthread_mutex_unlock(&isRunningMutex);

  static u16 ticks = 0;

  while (currentIsRunning)
  {
    // Tick the CPU and update the PPU, which generates the frameBuffer.
    // It's crucial that cpuTick() (or an internal PPU update)
    // completes a full frame before signaling for a render.
    // If cpuTick() only advances by a few cycles, you might see
    // partially drawn frames. For optimal results, ensure that
    // the newFrameAvailable signal is triggered only when the
    // PPU has completed a full frame (e.g., at VBlank interrupt).
    cpuTick();

    // Signal that a new frame is available for rendering
    pthread_mutex_lock(&frameMutex);
    newFrameAvailable = true;
    pthread_cond_signal(&frameReadyCv);
    pthread_mutex_unlock(&frameMutex);

    // Request a render from the Java/Kotlin UI thread
    if (ticks++ % 1000 == 0)
    {
      callJavaRequestRender();
      ticks = 0;
    }

    // Removed explicit nanosleep to allow the emulator to run at full speed.
    // The rendering will now be implicitly throttled by GLSurfaceView's
    // own rendering frequency.
    // auto end          = std::chrono::steady_clock::now();
    // auto elapsedNs    = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    // long sleepNs      = frameDurationNs - elapsedNs;
    // if (sleepNs > 0)
    // {
    //     struct timespec req = {0, 0};
    //     req.tv_sec  = sleepNs / 1000000000L;
    //     req.tv_nsec = sleepNs % 1000000000L;
    //     nanosleep(&req, NULL);
    // }

    // Check if the emulator should continue running
    pthread_mutex_lock(&isRunningMutex);
    currentIsRunning = isRunning;
    pthread_mutex_unlock(&isRunningMutex);
  }

  return nullptr;
}

extern "C"
{

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetAudioBuffer(
    JNIEnv* ENV, jobject THIZ, jbyteArray AUDIO_BUFFER)
{
// Bridges audio buffer retrieval to the native Game Boy core.
jbyte* buffer = ENV->GetByteArrayElements(AUDIO_BUFFER, nullptr);
getAudio(reinterpret_cast<u8*>(buffer));
ENV->ReleaseByteArrayElements(AUDIO_BUFFER, buffer, 0);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
    JNIEnv* ENV, jobject THIZ, jbyteArray ROM, jint SIZE)
{
// Loads the Game Boy ROM into the native core.
jbyte* buffer = ENV->GetByteArrayElements(ROM, nullptr);
cartridgeLoad(reinterpret_cast<u8*>(buffer), SIZE);
ENV->ReleaseByteArrayElements(ROM, buffer, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(
    JNIEnv* ENV, jobject THIZ, jint BUTTON, jboolean PRESSED)
{
// Sets the state of a Game Boy button in the native core.
setButton(static_cast<u8>(BUTTON), static_cast<bool>(PRESSED));
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
    JNIEnv* ENV, jobject THIZ)
{
// Starts the Game Boy emulator thread.
pthread_mutex_lock(&isRunningMutex);
if (isRunning)
{
pthread_mutex_unlock(&isRunningMutex);
return; // Emulator is already running
}
isRunning = true;
pthread_mutex_unlock(&isRunningMutex);

// Initialize and allocate frameBuffer within the emulator core
startEmulator();
// Create the emulator tick loop thread
pthread_create(&emulatorThread, NULL, tickLoop, NULL);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(
    JNIEnv* ENV, jobject THIZ)
{
// Stops the Game Boy emulator thread and releases resources.
pthread_mutex_lock(&isRunningMutex);
if (!isRunning)
{
pthread_mutex_unlock(&isRunningMutex);
return; // Emulator is not running
}
isRunning = false; // Signal the tickLoop to stop
pthread_mutex_unlock(&isRunningMutex);

// Wait for the emulator thread to finish its execution
pthread_join(emulatorThread, NULL);

// Deallocate frameBuffer and clean up within the emulator core
stopEmulator();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceCreated(JNIEnv* ENV, jobject THIZ)
{
// Initializes OpenGL ES resources when the rendering surface is created.
FORGE_LOG_INFO("nativeOnSurfaceCreated: Initializing OpenGL ES");

GLuint vertexShader   = loadShader(GL_VERTEX_SHADER, gVertexShader);
GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, gFragmentShader);

gbProgramObject = glCreateProgram();
glAttachShader(gbProgramObject, vertexShader);
glAttachShader(gbProgramObject, fragmentShader);
glLinkProgram(gbProgramObject);

GLint linked;
glGetProgramiv(gbProgramObject, GL_LINK_STATUS, &linked);
if (!linked)
{
GLint infoLen = 0;
glGetProgramiv(gbProgramObject, GL_INFO_LOG_LENGTH, &infoLen);
if (infoLen > 1)
{
char* infoLog = (char*)malloc(sizeof(char) * infoLen);
glGetProgramInfoLog(gbProgramObject, infoLen, NULL, infoLog);
FORGE_LOG_ERROR("Error linking program:\n%s", infoLog);
free(infoLog);
}
glDeleteProgram(gbProgramObject);
gbProgramObject = 0;
return;
}

glGenTextures(1, &gbTextureId);
glBindTexture(GL_TEXTURE_2D, gbTextureId);

// Set texture parameters for nearest filtering (pixelated look) and clamping
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

// Allocate texture memory on the GPU. Data will be updated with glTexSubImage2D.
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

glUseProgram(gbProgramObject);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceChanged(
    JNIEnv* ENV, jobject THIZ, jint WIDTH, jint HEIGHT)
{
// Updates the OpenGL ES viewport when the rendering surface changes size.
FORGE_LOG_INFO("nativeOnSurfaceChanged: %d x %d", WIDTH, HEIGHT);
glViewport(0, 0, WIDTH, HEIGHT);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnDrawFrame(JNIEnv* ENV, jobject THIZ)
{
// Triggers the OpenGL ES rendering for a new frame.
renderFrameGl();
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* VM, void* RESERVED)
{
  // Called when the native library is loaded by the JVM, caching essential JNI references.
  FORGE_LOG_INFO("JNI_OnLoad: Caching JVM and method IDs.");
  cachedJvm = VM;

  JNIEnv* env;
  if (VM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)
  {
    FORGE_LOG_ERROR("Failed to get JNIEnv on JNI_OnLoad");
    return JNI_ERR;
  }

  // Find the GameBoy Java class and create a global reference
  jclass localGameboyClass = env->FindClass("just/somebody/templates/domain/GameBoy");
  if (!localGameboyClass)
  {
    FORGE_LOG_ERROR("Failed to find GameBoy class");
    return JNI_ERR;
  }
  gameboyClassGlobalRef = (jclass)env->NewGlobalRef(localGameboyClass);
  env->DeleteLocalRef(localGameboyClass); // Release local reference

  // Get the method ID for requestRenderFromNative
  requestRenderMethodId = env->GetStaticMethodID(
      gameboyClassGlobalRef,
      "requestRenderFromNative",
      "()V");
  if (!requestRenderMethodId)
  {
    FORGE_LOG_ERROR("Failed to find method ID for requestRenderFromNative");
    return JNI_ERR;
  }

  // Initialize pthread mutexes and condition variables
  pthread_mutex_init(&frameMutex, NULL);
  pthread_cond_init(&frameReadyCv, NULL);
  pthread_mutex_init(&isRunningMutex, NULL);

  return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* VM, void* RESERVED)
{
// Called when the native library is unloaded, cleaning up resources.
FORGE_LOG_INFO("JNI_OnUnload: Cleaning up resources.");

JNIEnv* env;
// Attempt to get JNIEnv, but proceed with cleanup even if it fails
if (VM->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK)
{
if (gameboyClassGlobalRef)
{
env->DeleteGlobalRef(gameboyClassGlobalRef);
gameboyClassGlobalRef = nullptr;
}
}

// Ensure emulator thread is stopped and joined before cleaning up
pthread_mutex_lock(&isRunningMutex);
if (isRunning)
{
isRunning = false; // Signal the tickLoop to stop
pthread_mutex_unlock(&isRunningMutex);
stopEmulator(); // Signal emulator core to stop and free its buffer
pthread_join(emulatorThread, NULL); // Wait for thread to terminate
}
else
{
pthread_mutex_unlock(&isRunningMutex);
}

// OpenGL ES resource cleanup
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

// frameBuffer is managed by stopEmulator(), no need to delete here.

// Destroy pthread mutexes and condition variables
pthread_mutex_destroy(&frameMutex);
pthread_cond_destroy(&frameReadyCv);
pthread_mutex_destroy(&isRunningMutex);

// Clear cached JNI references
cachedJvm             = nullptr;
requestRenderMethodId = nullptr;
}

} // extern "C"

#ifdef __cplusplus
}
#endif
#endif // __ANDROID__
