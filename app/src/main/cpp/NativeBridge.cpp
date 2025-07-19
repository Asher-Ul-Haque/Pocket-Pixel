#ifdef __ANDROID__
#include <jni.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <GLES2/gl2.h> // Include OpenGL ES 2.0 headers
#include <GLES2/gl2ext.h> // Include OpenGL ES extensions
#include <android/log.h> // For logging in C++

#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoyCore.h" // Contains getFrame()
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/emu.h"
#include "GameBoy/include/cartridge.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define logging macros for C++
#define LOG_TAG "GameBoyNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// --- Emulator Thread Management ---
static std::thread emulator_thread;
static std::atomic<bool> is_running = false;

// --- Rendering Globals ---
const int GB_SCREEN_WIDTH = 160;
const int GB_SCREEN_HEIGHT = 144;

// OpenGL ES texture ID for the Game Boy screen
static GLuint gb_texture_id = 0;

// Shader program object
static GLuint gb_program_object = 0;

// Pixel buffer to hold the Game Boy frame data from the emulator core
// This buffer is written by the emulator thread and read by the rendering thread.
static uint32_t* gb_pixel_buffer = nullptr;

// --- Thread Synchronization for Frame Buffer ---
static std::mutex frame_mutex;
static std::condition_variable frame_ready_cv;
static bool new_frame_available = false;

// --- JNI Callback to Kotlin UI ---
static JavaVM* cached_jvm = nullptr; // Cache JVM for attaching threads
static jclass gameboy_class_global_ref = nullptr; // Global ref to GameBoy.kt class
static jmethodID request_render_method_id = nullptr; // Method ID for GameBoy.requestRenderFromNative()

/**
 * Attaches the current thread to the JVM and returns a JNIEnv.
 * This is necessary for non-UI threads (like the emulator thread) to make JNI calls.
 */
JNIEnv* getJNIEnv() {
    JNIEnv* env;
    int status = cached_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        LOGI("Attaching current thread to JVM");
        status = cached_jvm->AttachCurrentThread(&env, NULL);
        if (status != JNI_OK) {
            LOGE("Failed to attach current thread to JVM: %d", status);
            return nullptr;
        }
    }
    return env;
}

/**
 * Calls the static requestRenderFromNative() method in GameBoy.kt.
 * This signals the GLSurfaceView to redraw.
 */
void callJavaRequestRender() {
    JNIEnv* env = getJNIEnv();
    if (env && gameboy_class_global_ref && request_render_method_id) {
        env->CallStaticVoidMethod(gameboy_class_global_ref, request_render_method_id);
    } else {
        LOGE("Cannot call Java requestRender: JNIEnv or class/method ID not cached.");
    }
}

// --- OpenGL ES Shader Compilation ---
GLuint loadShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("Failed to create shader of type %d", type);
        return 0;
    }
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOGE("Error compiling shader (type %d):\n%s", type, infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// --- OpenGL ES Rendering Logic ---
void renderFrameGL() {
    // Wait for a new frame from the emulator thread
    std::unique_lock<std::mutex> lock(frame_mutex);
    frame_ready_cv.wait(lock, []{ return new_frame_available; });
    new_frame_available = false; // Consume the flag

    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the compiled shader program
    glUseProgram(gb_program_object);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gb_texture_id);

    // Update the texture with the latest Game Boy pixel data
    if (gb_pixel_buffer) {
        // Use GL_RGBA and GL_UNSIGNED_BYTE as getFrame generates 32-bit ARGB (AARRGGBB)
        // OpenGL expects RGBA, so ensure your getFrame output matches (or swap bytes if needed).
        // For simplicity, assuming getFrame output is already in ARGB format where A is MSB.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, gb_pixel_buffer);
    } else {
        LOGE("gb_pixel_buffer is NULL in renderFrameGL!");
    }

    // Define vertices for a screen-filling quad
    GLfloat vertices[] = {
            -1.0f,  1.0f, 0.0f, // Top-left
            -1.0f, -1.0f, 0.0f, // Bottom-left
            1.0f, -1.0f, 0.0f, // Bottom-right
            1.0f,  1.0f, 0.0f  // Top-right
    };

    // Define texture coordinates (standard for full texture)
    GLfloat texCoords[] = {
            0.0f, 0.0f, // Top-left of texture
            0.0f, 1.0f, // Bottom-left of texture
            1.0f, 1.0f, // Bottom-right of texture
            1.0f, 0.0f  // Top-right of texture
    };

    // Define indices for two triangles forming a quad
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    // Get attribute locations
    GLint positionLoc = glGetAttribLocation(gb_program_object, "a_position");
    GLint texCoordLoc = glGetAttribLocation(gb_program_object, "a_texCoord");
    GLint samplerLoc = glGetUniformLocation(gb_program_object, "s_texture");

    // Pass vertex data
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    // Pass texture coordinate data
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    // Set the sampler uniform to use texture unit 0
    glUniform1i(samplerLoc, 0);

    // Draw the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    // Disable vertex attribute arrays
    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(texCoordLoc);
}

// Vertex Shader Source
const char* gVertexShader =
        "attribute vec4 a_position;\n"
        "attribute vec2 a_texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_Position = a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}\n";

// Fragment Shader Source
const char* gFragmentShader =
        "precision mediump float;\n"
        "varying vec2 v_texCoord;\n"
        "uniform sampler2D s_texture;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(s_texture, v_texCoord);\n"
        "}\n";


// --- Emulator Tick Loop (runs on a separate thread) ---
void tickLoop() {
    const int targetFps = 60;
    const int frameDurationMs = 1000 / targetFps;

    // Allocate pixel buffer once for the emulator to write to
    gb_pixel_buffer = new uint32_t[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];
    if (!gb_pixel_buffer) {
        LOGE("Failed to allocate gb_pixel_buffer!");
        is_running.store(false); // Stop if allocation fails
        return;
    }

    while (emuGetContext()->running && is_running.load()) {
        auto start = std::chrono::steady_clock::now();

        cpuTick();           // Advance emulation
        // Call getFrame to populate the gb_pixel_buffer
        getFrame(gb_pixel_buffer);

        // Signal the rendering thread that a new frame is available
        {
            std::unique_lock<std::mutex> lock(frame_mutex);
            new_frame_available = true;
        }
        frame_ready_cv.notify_one();

        // Request render on the GLSurfaceView from the UI thread
        callJavaRequestRender();

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        int sleepMs = frameDurationMs - static_cast<int>(elapsed.count());

        if (sleepMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }

    // Clean up pixel buffer when emulator stops
    if (gb_pixel_buffer) {
        delete[] gb_pixel_buffer;
        gb_pixel_buffer = nullptr;
    }
}

extern "C" {

// --- JNI Functions for GameBoy.kt ---

// Audio bridge (existing)
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetAudioBuffer(
        JNIEnv* env, jobject /*thiz*/, jbyteArray audioBuffer) {
    jbyte* buffer = env->GetByteArrayElements(audioBuffer, nullptr);
    getAudio(reinterpret_cast<u8*>(buffer));
    env->ReleaseByteArrayElements(audioBuffer, buffer, 0);
}

// ROM loading (existing)
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
        JNIEnv* env, jobject /*thiz*/, jbyteArray rom, jint size) {
    jbyte* buffer = env->GetByteArrayElements(rom, nullptr);
    cartridgeLoad(reinterpret_cast<u8*>(buffer), size);
    env->ReleaseByteArrayElements(rom, buffer, JNI_ABORT);
}

// Button input (existing)
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(
        JNIEnv* /*env*/, jobject /*thiz*/, jint button, jboolean pressed) {
    setButton(static_cast<u8>(button), static_cast<bool>(pressed));
}

// Start emulator thread (existing, modified to start tickLoop)
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
        JNIEnv* /*env*/, jobject /*thiz*/) {
    if (is_running.load()) return;

    startEmulator(); // Resets internal emulator state
    is_running.store(true);
    emulator_thread = std::thread(tickLoop); // Start the tickLoop (emulator + rendering signal)
}

// Stop emulator thread and release resources (existing)
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(
        JNIEnv* /*env*/, jobject /*thiz*/) {
    is_running.store(false);
    stopEmulator(); // Sets emuGetContext()->running = false

    if (emulator_thread.joinable()) {
        emulator_thread.join(); // Wait for the emulator thread to finish
    }
}

// --- NEW JNI Functions for OpenGL ES Rendering ---

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceCreated(JNIEnv* env, jobject /*thiz*/) {
    LOGI("nativeOnSurfaceCreated: Initializing OpenGL ES");

    // Compile shaders
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, gVertexShader);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, gFragmentShader);

    // Link program
    gb_program_object = glCreateProgram();
    glAttachShader(gb_program_object, vertexShader);
    glAttachShader(gb_program_object, fragmentShader);
    glLinkProgram(gb_program_object);

    GLint linked;
    glGetProgramiv(gb_program_object, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(gb_program_object, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(gb_program_object, infoLen, NULL, infoLog);
            LOGE("Error linking program:\n%s", infoLog);
            free(infoLog);
        }
        glDeleteProgram(gb_program_object);
        gb_program_object = 0;
        return;
    }

    // Generate and bind texture for Game Boy screen
    glGenTextures(1, &gb_texture_id);
    glBindTexture(GL_TEXTURE_2D, gb_texture_id);

    // Set texture parameters (nearest neighbor for pixel art, clamp to edge)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate initial texture memory (can be NULL data for now)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glUseProgram(gb_program_object);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceChanged(JNIEnv* env, jobject /*thiz*/, jint width, jint height) {
    LOGI("nativeOnSurfaceChanged: %d x %d", width, height);
    glViewport(0, 0, width, height); // Set the viewport to cover the entire surface
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnDrawFrame(JNIEnv* env, jobject /*thiz*/) {
    // Perform the actual OpenGL ES rendering
    renderFrameGL();
}

// --- JNI_OnLoad and JNI_OnUnload for JVM lifecycle management ---

// Called when the native library is loaded by the JVM
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad: Caching JVM and method IDs.");
    cached_jvm = vm; // Cache the JVM instance

    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("Failed to get JNIEnv on JNI_OnLoad");
        return JNI_ERR;
    }

    // Get a global reference to the GameBoy class
    jclass local_gameboy_class = env->FindClass("just/somebody/templates/domain/GameBoy");
    if (!local_gameboy_class) {
        LOGE("Failed to find GameBoy class");
        return JNI_ERR;
    }
    gameboy_class_global_ref = (jclass)env->NewGlobalRef(local_gameboy_class);
    env->DeleteLocalRef(local_gameboy_class); // Delete local ref after creating global one

    // Get the method ID for requestRenderFromNative
    request_render_method_id = env->GetStaticMethodID(
            gameboy_class_global_ref,
            "requestRenderFromNative",
            "()V" // Signature for a static void method with no arguments
    );
    if (!request_render_method_id) {
        LOGE("Failed to find method ID for requestRenderFromNative");
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

// Called when the native library is unloaded by the JVM
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload: Cleaning up resources.");

    // Clean up global JNI references
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        if (gameboy_class_global_ref) {
            env->DeleteGlobalRef(gameboy_class_global_ref);
            gameboy_class_global_ref = nullptr;
        }
    }

    // Clean up OpenGL ES resources
    if (gb_program_object != 0) {
        glDeleteProgram(gb_program_object);
        gb_program_object = 0;
    }
    if (gb_texture_id != 0) {
        glDeleteTextures(1, &gb_texture_id);
        gb_texture_id = 0;
    }

    // Ensure emulator thread is stopped and joined
    if (is_running.load()) {
        is_running.store(false);
        stopEmulator(); // Signal emulator core to stop
        if (emulator_thread.joinable()) {
            emulator_thread.join();
        }
    }

    // Clean up pixel buffer if it was allocated
    if (gb_pixel_buffer) {
        delete[] gb_pixel_buffer;
        gb_pixel_buffer = nullptr;
    }

    cached_jvm = nullptr;
    request_render_method_id = nullptr;
}

} // extern "C"

#ifdef __cplusplus
}
#endif
#endif
