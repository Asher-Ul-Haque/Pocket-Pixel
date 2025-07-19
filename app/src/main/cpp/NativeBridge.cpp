#ifdef __ANDROID__
#include <jni.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoyCore.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/emu.h"
#include "GameBoy/include/cartridge.h"

#ifdef __cplusplus
extern "C" {
#endif


static ANativeWindow* window = nullptr;
static std::thread emulator_thread;
static std::atomic<bool> is_running = false;
static std::mutex window_mutex;

void renderLoop() {
    std::lock_guard<std::mutex> lock(window_mutex);

    if (!window) return;

    ANativeWindow_Buffer buffer;
    const int maxRetries = 5;

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        if (ANativeWindow_lock(window, &buffer, nullptr) == 0) {
            u32* pixels = reinterpret_cast<u32*>(buffer.bits);
            getFrame(pixels);
            ANativeWindow_unlockAndPost(window);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Retry after ~1 frame
    }
    // If we get here, lock failed
    // Optional: add logging here
}

void tickLoop() {
    const int targetFps = 60;
    const int frameDurationMs = 1000 / targetFps;

    while (emuGetContext()->running && is_running.load()) {
        auto start = std::chrono::steady_clock::now();

        cpuTick();           // Advance emulation
        renderLoop();        // Render current frame

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        int sleepMs = frameDurationMs - static_cast<int>(elapsed.count());

        if (sleepMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }
}

extern "C" {

// Audio bridge
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetAudioBuffer(
        JNIEnv* env, jobject /*thiz*/, jbyteArray audioBuffer) {
    jbyte* buffer = env->GetByteArrayElements(audioBuffer, nullptr);
    getAudio(reinterpret_cast<u8*>(buffer));
    env->ReleaseByteArrayElements(audioBuffer, buffer, 0);
}

// ROM loading
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
        JNIEnv* env, jobject /*thiz*/, jbyteArray rom, jint size) {
    jbyte* buffer = env->GetByteArrayElements(rom, nullptr);
    cartridgeLoad(reinterpret_cast<u8*>(buffer), size);
    env->ReleaseByteArrayElements(rom, buffer, JNI_ABORT);
}

// Button input
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(
        JNIEnv* /*env*/, jobject /*thiz*/, jint button, jboolean pressed) {
    setButton(static_cast<u8>(button), static_cast<bool>(pressed));
}

// Set or clear rendering surface
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetSurface(
        JNIEnv* env, jobject /*thiz*/, jobject surface) {
    std::lock_guard<std::mutex> lock(window_mutex);

    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }

    if (surface != nullptr) {
        window = ANativeWindow_fromSurface(env, surface);
        ANativeWindow_setBuffersGeometry(window, 160, 144, WINDOW_FORMAT_RGBA_8888);
    }
}

// Start emulator thread
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
        JNIEnv* /*env*/, jobject /*thiz*/) {
    if (is_running.load()) return;

    startEmulator(); // Resets internal emulator state
    is_running.store(true);
    emulator_thread = std::thread(tickLoop);
}

// Stop emulator thread and release resources
JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(
        JNIEnv* /*env*/, jobject /*thiz*/) {
    is_running.store(false);
    stopEmulator();

    if (emulator_thread.joinable()) {
        emulator_thread.join();
    }

    std::lock_guard<std::mutex> lock(window_mutex);
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
}

} // extern "C"


#ifdef __cplusplus
}
#endif
#endif
