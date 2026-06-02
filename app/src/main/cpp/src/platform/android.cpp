#ifdef __ANDROID__

// 1. Wrap the C-headers so the C++ compiler knows not to mangle their symbols
// (Fixes the cartridgeGetContext error)
extern "C" {
#include <platform.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
}

#include <android/log.h>

#define LOG_TAG "PocketPixel_Platform"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static PlatformContext g_platformCtx;

// - - - Palette Definitions - - -
// Format: 0xAABBGGRR for little-endian memory layout as R, G, B, A
typedef struct {
    const char* name;
    u32 colors[4];
} PaletteEntry;

#define PACK_RGBA(R, G, B, A) ((u32)(A) << 24 | (u32)(B) << 16 | (u32)(G) << 8 | (u32)(R))

static PaletteEntry g_palettes[] = {
        {"Default", {PACK_RGBA(0xD1, 0xCB, 0x95, 0xFF), PACK_RGBA(0x40, 0x98, 0x5E, 0xFF), PACK_RGBA(0x1A, 0x64, 0x4E, 0xFF), PACK_RGBA(0x04, 0x37, 0x3B, 0xFF)}},
        {"Classic", {PACK_RGBA(0x9A, 0xA1, 0x3C, 0xFF), PACK_RGBA(0x6C, 0x71, 0x2A, 0xFF), PACK_RGBA(0x4D, 0x51, 0x1E, 0xFF), PACK_RGBA(0x1F, 0x20, 0x0C, 0xFF)}},
        {"Fizzle", {PACK_RGBA(0xCE, 0xE5, 0xFF, 0xFF), PACK_RGBA(0xC5, 0x89, 0xDC, 0xFF), PACK_RGBA(0x56, 0x49, 0x91, 0xFF), PACK_RGBA(0x1E, 0x18, 0x2A, 0xFF)}},
        {"Ice cream", {PACK_RGBA(0xFF, 0xF6, 0xD3, 0xFF), PACK_RGBA(0xF9, 0xA8, 0x75, 0xFF), PACK_RGBA(0xEB, 0x6B, 0x6F, 0xFF), PACK_RGBA(0x7C, 0x3F, 0x58, 0xFF)}},
        {"Hollow", {PACK_RGBA(0xFA, 0xFB, 0xF6, 0xFF), PACK_RGBA(0xC6, 0xB7, 0xBE, 0xFF), PACK_RGBA(0x56, 0x5A, 0x75, 0xFF), PACK_RGBA(0x0F, 0x0F, 0x1B, 0xFF)}},
        {"Rustic", {PACK_RGBA(0xED, 0xB4, 0xA1, 0xFF), PACK_RGBA(0xA9, 0x68, 0x68, 0xFF), PACK_RGBA(0x76, 0x44, 0x62, 0xFF), PACK_RGBA(0x2C, 0x21, 0x37, 0xFF)}},
        {"Mint", {PACK_RGBA(0xC4, 0xF0, 0xC2, 0xFF), PACK_RGBA(0x5A, 0xB9, 0xA8, 0xFF), PACK_RGBA(0x1E, 0x60, 0x6E, 0xFF), PACK_RGBA(0x2D, 0x1B, 0x00, 0xFF)}},
        {"SpaceHaze", {PACK_RGBA(0xF8, 0xE3, 0xC4, 0xFF), PACK_RGBA(0xCC, 0x34, 0x95, 0xFF), PACK_RGBA(0x6B, 0x1F, 0xB1, 0xFF), PACK_RGBA(0x0B, 0x06, 0x30, 0xFF)}},
        {"Fiery Plague", {PACK_RGBA(0x71, 0x31, 0x41, 0xFF), PACK_RGBA(0x51, 0x28, 0x39, 0xFF), PACK_RGBA(0x31, 0x21, 0x37, 0xFF), PACK_RGBA(0x1A, 0x21, 0x29, 0xFF)}},
        {"Gold", {PACK_RGBA(0xCF, 0xAB, 0x51, 0xFF), PACK_RGBA(0x9D, 0x65, 0x4C, 0xFF), PACK_RGBA(0x4D, 0x22, 0x2C, 0xFF), PACK_RGBA(0x21, 0x0B, 0x1B, 0xFF)}},
        {"Honey", {PACK_RGBA(0xE9, 0xF5, 0xDA, 0xFF), PACK_RGBA(0xF0, 0xB6, 0x95, 0xFF), PACK_RGBA(0x87, 0x72, 0x86, 0xFF), PACK_RGBA(0x3E, 0x3A, 0x42, 0xFF)}},
        {"Coral", {PACK_RGBA(0xFF, 0xD0, 0xA4, 0xFF), PACK_RGBA(0xF4, 0x94, 0x9C, 0xFF), PACK_RGBA(0x7C, 0x9A, 0xAC, 0xFF), PACK_RGBA(0x68, 0x51, 0x8A, 0xFF)}},
        {"Rabbit", {PACK_RGBA(0xF1, 0xE0, 0xCD, 0xFF), PACK_RGBA(0xFF, 0xA4, 0x9A, 0xFF), PACK_RGBA(0xDA, 0x34, 0x67, 0xFF), PACK_RGBA(0x35, 0x33, 0x3F, 0xFF)}},
        {"Caramel autumn", {PACK_RGBA(0xFF, 0xF4, 0xB8, 0xFF), PACK_RGBA(0xFF, 0x8B, 0x40, 0xFF), PACK_RGBA(0xA2, 0x2F, 0xC9, 0xFF), PACK_RGBA(0x29, 0x01, 0x43, 0xFF)}},
        {"Snow flake", {PACK_RGBA(0xE7, 0xED, 0xEB, 0xFF), PACK_RGBA(0x8E, 0xCE, 0xCE, 0xFF), PACK_RGBA(0x62, 0xA1, 0xC7, 0xFF), PACK_RGBA(0x3F, 0x6E, 0xCC, 0xFF)}},
        {"Lemon and Lime", {PACK_RGBA(0xFF, 0xF3, 0x7B, 0xFF), PACK_RGBA(0x5F, 0xCC, 0x86, 0xFF), PACK_RGBA(0x39, 0x80, 0x9C, 0xFF), PACK_RGBA(0x28, 0x37, 0x5B, 0xFF)}},
        {"Kirokaze", {PACK_RGBA(0xE2, 0xF3, 0xE4, 0xFF), PACK_RGBA(0x94, 0xE3, 0x44, 0xFF), PACK_RGBA(0x46, 0x87, 0x8F, 0xFF), PACK_RGBA(0x33, 0x2C, 0x50, 0xFF)}},
        {"Red is dead", {PACK_RGBA(0xFF, 0xFC, 0xFE, 0xFF), PACK_RGBA(0xFF, 0x00, 0x15, 0xFF), PACK_RGBA(0x86, 0x00, 0x20, 0xFF), PACK_RGBA(0x11, 0x07, 0x0A, 0xFF)}}
};

static DmgPalette g_activeDmgPalette;
static u32 g_frameBufferRGBA[WIDTH * HEIGHT];
static bool g_shaderEnabled = false;

// - - - Video Implementation - - -

static bool androidVideoInit(void) {
    g_activeDmgPalette.color0 = g_palettes[0].colors[0];
    g_activeDmgPalette.color1 = g_palettes[0].colors[1];
    g_activeDmgPalette.color2 = g_palettes[0].colors[2];
    g_activeDmgPalette.color3 = g_palettes[0].colors[3];
    return true;
}

static void androidSetDmgPalette(DmgPalette PALETTE) {
    g_activeDmgPalette = PALETTE;
}

static void androidEnableShader(bool ENABLE) {
    g_shaderEnabled = ENABLE;
}

static void androidRenderFrame(const PpuFrame* FRAME) {
    GameBoyMode romMode = cartridgeGetContext()->mode;
    bool isCgbMode = (romMode == MODE_CGB_GAMEBOY || romMode == MODE_CGB_ONLY_GAMEBOY);

    for (i32 y = 0; y < HEIGHT; y++) {
        for (i32 x = 0; x < WIDTH; x++) {
            u16 raw = FRAME->pixels[y][x];
            u32 rgba;

            if (!isCgbMode) {
                switch (raw) {
                    case 0: rgba = g_activeDmgPalette.color0; break;
                    case 1: rgba = g_activeDmgPalette.color1; break;
                    case 2: rgba = g_activeDmgPalette.color2; break;
                    case 3: rgba = g_activeDmgPalette.color3; break;
                    default: rgba = PACK_RGBA(0, 0, 0, 255); break;
                }
            } else {
                u8 r5 = (raw & 0x001F);
                u8 g5 = (raw & 0x03E0) >> 5;
                u8 b5 = (raw & 0x7C00) >> 10;
                u8 r8 = (r5 << 3) | (r5 >> 2);
                u8 g8 = (g5 << 3) | (g5 >> 2);
                u8 b8 = (b5 << 3) | (b5 >> 2);
                rgba = PACK_RGBA(r8, g8, b8, 0xFF);
            }
            g_frameBufferRGBA[y * WIDTH + x] = rgba;
        }
    }
}

static void androidPresent(void) {
    // Frame is ready in g_frameBufferRGBA
    // Callback to requestRender is handled in the emulator thread
}

static void androidCleanup(void) {}

// - - - Audio Implementation - - -

extern "C" {
void android_audio_push(const f32* SAMPLES, u32 COUNT);
bool android_audio_init(void);
void android_audio_cleanup(void);
}

static bool androidAudioInit(void) {
    return android_audio_init();
}

static void androidAudioPushSamples(const f32* SAMPLES, u32 COUNT) {
    android_audio_push(SAMPLES, COUNT);
}

static void androidAudioCleanup(void) {
    android_audio_cleanup();
}

// - - - Input Implementation - - -
static void androidInputPoll(bool* IS_RUNNING) {}
static void androidInputSetConfig(InputConfig NEW_CONFIG) {}

// ============================================================================
// 2. Wrap the exposed implementations in `extern "C"` to prevent mangling
// (Fixes platformInit, platformGetContext, androidGetFrameBuffer, etc.)
// ============================================================================
extern "C" {

PlatformContext* platformGetContext(void) {
    return &g_platformCtx;
}

void platformInit(void) {
    g_platformCtx.name = "Android Native Platform";

    g_platformCtx.video.init = androidVideoInit;
    g_platformCtx.video.setDmgPalette = androidSetDmgPalette;
    g_platformCtx.video.enableShader = androidEnableShader;
    g_platformCtx.video.renderFrame = androidRenderFrame;
    g_platformCtx.video.present = androidPresent;
    g_platformCtx.video.cleanup = androidCleanup;

    g_platformCtx.audio.init = androidAudioInit;
    g_platformCtx.audio.pushSamples = androidAudioPushSamples;
    g_platformCtx.audio.cleanup = androidAudioCleanup;

    g_platformCtx.input.poll = androidInputPoll;
    g_platformCtx.input.setConfig = androidInputSetConfig;

    androidVideoInit();
    androidAudioInit();
}

// - - - Public access for JNI - - -
u32* androidGetFrameBuffer(void) {
    return g_frameBufferRGBA;
}

void androidSetPaletteByIndex(u8 index) {
    if (index >= sizeof(g_palettes) / sizeof(PaletteEntry)) return;
    g_activeDmgPalette.color0 = g_palettes[index].colors[0];
    g_activeDmgPalette.color1 = g_palettes[index].colors[1];
    g_activeDmgPalette.color2 = g_palettes[index].colors[2];
    g_activeDmgPalette.color3 = g_palettes[index].colors[3];
}

} // End of extern "C"

#endif