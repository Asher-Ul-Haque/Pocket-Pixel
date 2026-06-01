#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <chrono>

extern "C" {
#include <platform.h>
#include <joypad.h>
#include <apu/apu.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <timer.h>
#include <state.h>
#include <cartridge/cartridge.h>
}

#define LOG_TAG "PocketPixel_Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define TARGET_FPS 60.0
#define TARGET_FRAME_TIME (1.0 / TARGET_FPS)

// - - - Globals for JNI Callbacks - - -
static JavaVM* g_jvm = nullptr;
static jclass g_gameBoyClass = nullptr;

static jmethodID g_requestRenderID = nullptr;
static jmethodID g_saveRamID = nullptr;
static jmethodID g_loadRamID = nullptr;
static jmethodID g_getExpectedSaveSizeID = nullptr;
static jmethodID g_playAudioID = nullptr;

static jfloatArray g_audioArray = nullptr;
static u32 g_audioArraySize = 0;
static std::mutex g_audioMutex;

// - - - Emulator Thread Control - - -
static std::thread g_emulatorThread;
static std::atomic<bool> g_running{false};
static std::atomic<bool> g_paused{false};
static std::mutex g_pauseMutex;
static std::condition_variable g_pauseCv;

static std::atomic<bool> g_fastForward{false};

// - - - Buffers - - -
static u8* g_romData = nullptr;
static u32 g_romSize = 0;

// - - - OpenGL State - - -
static GLuint g_program = 0;
static GLuint g_texture = 0;
static GLuint g_prevTexture = 0;
static GLuint g_vbo = 0;
static GLint g_positionLoc = -1;
static GLint g_texCoordLoc = -1;
static GLint g_samplerLoc = -1;
static GLint g_prevSamplerLoc = -1;
static GLint g_shaderTypeLoc = -1;
static GLint g_resolutionLoc = -1;
static int g_currentShader = 0; // 0: LCD Ghosting, 1: CRT, 2: LCD, 3: Chromatic Aberration

// - - - Forward Declarations for Platform Hooks - - -
extern "C" {
    bool android_saveRam(const u8* RAM_DATA, u32 RAM_SIZE);
    bool android_loadRam(u8* RAM_DATA, u32 RAM_SIZE);
    u32  android_getExpectedSaveSize(void);
    u32* androidGetFrameBuffer(void);
    void androidSetPaletteByIndex(u8 index);
}

// - - - Emulator Main Loop - - -
void emulatorLoop() {
    JNIEnv* env;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = "EmulatorThread";
    args.group = nullptr;

    if (g_jvm->AttachCurrentThread(&env, &args) != JNI_OK) {
        LOGE("Failed to attach emulator thread to JVM");
        return;
    }

    LOGI("Emulator thread started");

    PpuContext* ppu = ppuGetContext();
    CpuContext* cpu = cpuGetContext();
    u32 autoSaveFrameCounter = 0;

    auto lastFrameTime = std::chrono::steady_clock::now();

    while (g_running) {
        if (g_paused) {
            std::unique_lock<std::mutex> lock(g_pauseMutex);
            g_pauseCv.wait(lock, []{ return !g_paused || !g_running; });
            if (!g_running) break;
            lastFrameTime = std::chrono::steady_clock::now();
        }

        // Run until frame is ready
        while (g_running && !g_paused && !ppu->frameReady) {
            cpuTick();
            if (cpu->stopped) {
                if (ppu->registers.key1 & 0x01) {
                    ppuExecuteSpeedSwitch();
                    cpu->stopped = false;
                }
            }
            timerStepMCycle();
            apuTick();

            u8 dotsToTick = (ppu->registers.key1 & 0x80) ? 2 : 4;
            for (u8 i = 0; i < dotsToTick; ++i) {
                ppuTick();
            }
        }

        if (ppu->frameReady) {
            ppu->frameReady = false;

            PlatformContext* platform = platformGetContext();
            if (platform && platform->video.renderFrame) {
                platform->video.renderFrame(&ppu->frameBuffer);
            }

            // Request render on GL thread
            env->CallStaticVoidMethod(g_gameBoyClass, g_requestRenderID);

            // --- AUTO-SAVE HEARTBEAT ---
            autoSaveFrameCounter++;
            if (autoSaveFrameCounter >= 300) { // Every 5 seconds (roughly)
                autoSaveFrameCounter = 0;
                CartContext* ctx = cartridgeGetContext();
                if (ctx && ctx->initialized && ctx->ramDirty) {
                    cartridgeFlushRAM();
                }
            }

            double targetFrameTime = g_fastForward ? (TARGET_FRAME_TIME / 2.0) : TARGET_FRAME_TIME;
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - lastFrameTime;

            if (elapsed.count() < targetFrameTime) {
                auto sleepTime = std::chrono::duration<double>(targetFrameTime - elapsed.count());
                std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(sleepTime));
            }
            lastFrameTime = std::chrono::steady_clock::now();
        }
    }

    g_jvm->DetachCurrentThread();
    LOGI("Emulator thread exiting");
}

// - - - Shader Source Code - - -

const char* vShaderSrc = R"(
    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    varying vec2 v_texCoord;
    void main() {
        gl_Position = a_position;
        v_texCoord = a_texCoord;
    }
)";

const char* fShaderSrc = R"(
    precision mediump float;
    varying vec2 v_texCoord;
    uniform sampler2D s_texture;
    uniform sampler2D s_prevTexture;
    uniform int u_shaderType;
    uniform vec2 u_resolution;

    // Helper: Distort coordinates to mimic a curved CRT monitor screen
    vec2 barrelDistort(vec2 coord) {
        vec2 cc = coord - 0.5;
        float dist = dot(cc, cc);
        return coord + cc * dist * 0.08; // 0.08 controls screen bulge curvature
    }

    void main() {
        if (u_shaderType == 0) { // LCD Ghosting
            vec4 current = texture2D(s_texture, v_texCoord);
            vec4 previous = texture2D(s_prevTexture, v_texCoord);
            // Simulating LCD persistence: mix current frame with previous
            gl_FragColor = mix(current, previous, 0.35);
            return;
        }

        if (u_shaderType == 1) { // CRT Display Mode
            // 1. Apply Screen Curvature Mapping
            vec2 uv = barrelDistort(v_texCoord);

            // Vignette: Soft-clip boundaries to produce rounded bezel corners
            if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
                gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                return;
            }

            vec4 color = texture2D(s_texture, uv);

            // 2. High-Fidelity Cosine Scanlines
            // Maps scanlines flawlessly across varying high-res device screens
            float scanline = sin(uv.y * u_resolution.y * (144.0 / u_resolution.y) * 3.14159);
            scanline = mix(1.0, 0.75 + 0.25 * scanline, 0.8);

            // 3. Phosphor Mask & Vignette Shadowing
            float phosphor = sin(uv.x * u_resolution.x * 0.75) * 0.1 + 0.9;
            float vignette = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
            vignette = clamp(pow(16.0 * vignette, 0.25), 0.75, 1.0);

            gl_FragColor = vec4(color.rgb * scanline * phosphor * vignette, 1.0);
            return;
        }

        if (u_shaderType == 2) { // Dot-Matrix LCD Simulator
            vec2 uv = v_texCoord;
            vec4 color = texture2D(s_texture, uv);

            // 1. Calculate Sub-pixel Grid Coordinate Spaces ($160 \times 144$)
            vec2 lcdGridSize = vec2(160.0, 144.0);
            vec2 pixelCoord = uv * lcdGridSize;
            vec2 grid = fract(pixelCoord);

            // 2. Dual-axis Shadow Mask
            // Produces crisp rectangular pixel borders instead of single lines
            float fillFactor = 0.85; // Percent size of the solid pixel face
            float cellX = smoothstep(fillFactor, fillFactor + 0.05, grid.x) + smoothstep(1.0 - fillFactor, 1.0 - fillFactor - 0.05, grid.x);
            float cellY = smoothstep(fillFactor, fillFactor + 0.05, grid.y) + smoothstep(1.0 - fillFactor, 1.0 - fillFactor - 0.05, grid.y);
            float mask = clamp(1.0 - (cellX + cellY), 0.5, 1.0);

            // 3. Sub-Pixel LCD Bleed Emulation
            // Lightly shifts red and blue tracks inside the cell to mimic un-backlit panels
            float rFactor = texture2D(s_texture, uv + vec2(0.001, 0.0)).r;
            float bFactor = texture2D(s_texture, uv - vec2(0.001, 0.0)).b;
            vec3 dynamicLCDColor = vec3(mix(color.r, rFactor, 0.2), color.g, mix(color.b, bFactor, 0.2));

            gl_FragColor = vec4(dynamicLCDColor * mask, 1.0);
            return;
        }

        if (u_shaderType == 3) { // Radial Chromatic Aberration
            vec2 uv = v_texCoord;
            vec2 distFromCenter = uv - vec2(0.5);

            // Lens Distortion Factor scales exponentially from screen center outward
            float strength = 0.015 * length(distFromCenter);

            float r = texture2D(s_texture, uv + distFromCenter * strength).r;
            float g = texture2D(s_texture, uv).g;
            float b = texture2D(s_texture, uv - distFromCenter * strength).b;

            gl_FragColor = vec4(r, g, b, 1.0);
            return;
        }

        // Fallback Default
        gl_FragColor = texture2D(s_texture, v_texCoord);
    }
)";

GLuint loadShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("Could not create shader of type %d", type);
        return 0;
    }
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            char* buf = (char*)malloc(infoLen);
            glGetShaderInfoLog(shader, infoLen, nullptr, buf);
            LOGE("Error compiling shader:\n%s", buf);
            free(buf);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("just/somebody/templates/domain/GameBoy");
    if (clazz == nullptr) {
        LOGE("Could not find GameBoy class");
        return JNI_ERR;
    }
    g_gameBoyClass = reinterpret_cast<jclass>(env->NewGlobalRef(clazz));

    g_requestRenderID = env->GetStaticMethodID(g_gameBoyClass, "requestRenderFromNative", "()V");
    g_saveRamID = env->GetStaticMethodID(g_gameBoyClass, "saveRamToFile", "([BI)Z");
    g_loadRamID = env->GetStaticMethodID(g_gameBoyClass, "loadRamFromFile", "([BI)Z");
    g_getExpectedSaveSizeID = env->GetStaticMethodID(g_gameBoyClass, "getExpectedSaveSize", "()I");
    g_playAudioID = env->GetStaticMethodID(g_gameBoyClass, "nativePlayAudio", "([F)V");

    if (!g_requestRenderID || !g_saveRamID || !g_loadRamID || !g_getExpectedSaveSizeID || !g_playAudioID) {
        LOGE("Could not find all JNI method IDs");
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(JNIEnv *env, jobject thiz, jbyteArray rom, jint size) {
    if (g_romData) {
        free(g_romData);
    }

    g_romSize = size;
    g_romData = (u8*)malloc(g_romSize);
    env->GetByteArrayRegion(rom, 0, size, (jbyte*)g_romData);

    static CartridgeFileIO fileIO = {
        .saveRamToFile = android_saveRam,
        .loadRamFromFile = android_loadRam,
        .getExpectedSaveSize = android_getExpectedSaveSize
    };

    if (!cartridgeInit(&fileIO, g_romData, g_romSize)) {
        LOGE("Failed to initialize cartridge");
        return;
    }

    platformInit();
    joypadInit();
    cpuInit();
    ppuInit();
    apuInit();
    timerInit();

    LOGI("ROM Loaded: %s", cartridgeGetTitle());
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(JNIEnv *env, jobject thiz, jint button, jboolean pressed) {
    joypadSetButton((JoypadButton)button, pressed);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(JNIEnv *env, jobject thiz) {
    if (g_running) return;
    g_running = true;
    g_paused = false;
    g_emulatorThread = std::thread(emulatorLoop);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(JNIEnv *env, jobject thiz) {
    g_running = false;
    g_paused = false;
    g_pauseCv.notify_all();
    if (g_emulatorThread.joinable()) {
        g_emulatorThread.join();
    }
    cartridgeUnload();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativePauseEmulator(JNIEnv *env, jobject thiz) {
    g_paused = true;
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeResumeEmulator(JNIEnv *env, jobject thiz) {
    {
        std::lock_guard<std::mutex> lock(g_pauseMutex);
        g_paused = false;
    }
    g_pauseCv.notify_all();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetVolumes(JNIEnv *env, jobject thiz, jfloatArray volumes) {
    jfloat* vols = env->GetFloatArrayElements(volumes, nullptr);
    apuSetChannelVolumes(vols[1], vols[2], vols[3], vols[4]);
    env->ReleaseFloatArrayElements(volumes, vols, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeFlushSave(JNIEnv *env, jobject thiz) {
    cartridgeFlushRAM();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeChangePalette(JNIEnv *env, jobject thiz, jintArray colors) {
    jint* cols = env->GetIntArrayElements(colors, nullptr);
    DmgPalette palette;
    palette.color0 = (u32)cols[0];
    palette.color1 = (u32)cols[1];
    palette.color2 = (u32)cols[2];
    palette.color3 = (u32)cols[3];

    PlatformContext* platform = platformGetContext();
    if (platform && platform->video.setDmgPalette) {
        platform->video.setDmgPalette(palette);
    }

    env->ReleaseIntArrayElements(colors, cols, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeChangeShader(JNIEnv *env, jobject thiz, jint index) {
    g_currentShader = index;
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetFastForward(JNIEnv *env, jobject thiz, jboolean enabled) {
    g_fastForward = enabled;
    apuSetSpeed(enabled ? 2.0f : 1.0f);
}

JNIEXPORT jintArray JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeCaptureFrame(JNIEnv *env, jobject thiz) {
    u32* pixels = androidGetFrameBuffer();
    if (!pixels) return nullptr;

    jintArray result = env->NewIntArray(160 * 144);
    env->SetIntArrayRegion(result, 0, 160 * 144, (jint*)pixels);
    return result;
}

JNIEXPORT jbyteArray JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSaveState(JNIEnv *env, jobject thiz) {
    u32 size = 0;
    u8* data = systemSaveStateToMemory(&size);
    if (!data) return nullptr;
    jbyteArray array = env->NewByteArray(size);
    env->SetByteArrayRegion(array, 0, size, (jbyte*)data);
    free(data);
    return array;
}

JNIEXPORT jboolean JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadState(JNIEnv *env, jobject thiz, jbyteArray data, jint size) {
    u8* buffer = (u8*)malloc(size);
    env->GetByteArrayRegion(data, 0, size, (jbyte*)buffer);
    bool success = systemLoadStateFromMemory(buffer, size);
    free(buffer);
    return success ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceCreated(JNIEnv *env, jobject thiz) {
    LOGI("OpenGL Surface Created");
    GLuint vShader = loadShader(GL_VERTEX_SHADER, vShaderSrc);
    GLuint fShader = loadShader(GL_FRAGMENT_SHADER, fShaderSrc);
    g_program = glCreateProgram();
    glAttachShader(g_program, vShader);
    glAttachShader(g_program, fShader);
    glLinkProgram(g_program);

    GLint linked;
    glGetProgramiv(g_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(g_program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            char* buf = (char*)malloc(infoLen);
            glGetShaderInfoLog(g_program, infoLen, nullptr, buf);
            LOGE("Error linking program:\n%s", buf);
            free(buf);
        }
        glDeleteProgram(g_program);
        g_program = 0;
        return;
    }

    g_positionLoc = glGetAttribLocation(g_program, "a_position");
    g_texCoordLoc = glGetAttribLocation(g_program, "a_texCoord");
    g_samplerLoc = glGetUniformLocation(g_program, "s_texture");
    g_prevSamplerLoc = glGetUniformLocation(g_program, "s_prevTexture");
    g_shaderTypeLoc = glGetUniformLocation(g_program, "u_shaderType");
    g_resolutionLoc = glGetUniformLocation(g_program, "u_resolution");

    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 160, 144, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &g_prevTexture);
    glBindTexture(GL_TEXTURE_2D, g_prevTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 160, 144, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLfloat verts[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 0.0f
    };
    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnSurfaceChanged(JNIEnv *env, jobject thiz, jint width, jint height) {
    glViewport(0, 0, width, height);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeOnDrawFrame(JNIEnv *env, jobject thiz) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (g_program == 0) return;

    glUseProgram(g_program);

    u32* pixels = androidGetFrameBuffer();
    if (pixels) {
        // Use texture unit 0 for current, texture unit 1 for previous
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_prevTexture); // Upload to the oldest texture

        // --- DYNAMIC TEXTURE FILTERING SWITCH ---
        GLint filter = (g_currentShader == 1 || g_currentShader == 3) ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_texture); // Old newest is now previous
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

        // Swap handles so g_texture is always the newest
        GLuint temp = g_texture;
        g_texture = g_prevTexture;
        g_prevTexture = temp;
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_prevTexture);
    }

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    glEnableVertexAttribArray(g_positionLoc);
    glVertexAttribPointer(g_positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(g_texCoordLoc);
    glVertexAttribPointer(g_texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glUniform1i(g_samplerLoc, 0);
    glUniform1i(g_prevSamplerLoc, 1);
    glUniform1i(g_shaderTypeLoc, g_currentShader);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glUniform2f(g_resolutionLoc, (float)viewport[2], (float)viewport[3]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(g_positionLoc);
    glDisableVertexAttribArray(g_texCoordLoc);
}

// - - - Platform Callbacks implemented in Bridge - - -

void android_audio_push(const f32* SAMPLES, u32 COUNT) {
    JNIEnv* env;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) return;
    if (g_playAudioID == nullptr) return;

    std::lock_guard<std::mutex> lock(g_audioMutex);

    if (g_audioArray == nullptr || g_audioArraySize < COUNT) {
        if (g_audioArray != nullptr) {
            env->DeleteGlobalRef(g_audioArray);
        }
        g_audioArraySize = COUNT;
        jfloatArray local = env->NewFloatArray(COUNT);
        g_audioArray = (jfloatArray)env->NewGlobalRef(local);
        env->DeleteLocalRef(local);
    }

    env->SetFloatArrayRegion(g_audioArray, 0, COUNT, SAMPLES);
    env->CallStaticVoidMethod(g_gameBoyClass, g_playAudioID, g_audioArray);
}

bool android_saveRam(const u8* RAM_DATA, u32 RAM_SIZE) {
    JNIEnv* env;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) return false;
    jbyteArray array = env->NewByteArray(RAM_SIZE);
    env->SetByteArrayRegion(array, 0, RAM_SIZE, (jbyte*)RAM_DATA);
    return env->CallStaticBooleanMethod(g_gameBoyClass, g_saveRamID, array, (jint)RAM_SIZE);
}

bool android_loadRam(u8* RAM_DATA, u32 RAM_SIZE) {
    JNIEnv* env;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) return false;
    jbyteArray array = env->NewByteArray(RAM_SIZE);
    jboolean result = env->CallStaticBooleanMethod(g_gameBoyClass, g_loadRamID, array, (jint)RAM_SIZE);
    if (result == JNI_TRUE) {
        env->GetByteArrayRegion(array, 0, RAM_SIZE, (jbyte*)RAM_DATA);
        return true;
    }
    return false;
}

u32 android_getExpectedSaveSize(void) {
    JNIEnv* env;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) return 0;
    return (u32)env->CallStaticIntMethod(g_gameBoyClass, g_getExpectedSaveSizeID);
}

}
