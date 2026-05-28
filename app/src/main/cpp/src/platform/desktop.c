#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__EMSCRIPTEN__)

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <platform.h>
#include <debug.h>
#include <ppu/ppu.h>
#include <apu/apu.h>
#include <cartridge/cartridge.h>
#include <joypad.h>

static SDL_Window* gWindow           = NULL;
static SDL_Renderer* gRenderer       = NULL;
static SDL_Texture* gDisplayTexture  = NULL;
static DmgPalette gActiveDmgPalette;
static bool gShaderEnabled           = false;

static SDL_Window* gDebugWindow      = NULL;
static SDL_Renderer* gDebugRenderer  = NULL;
static SDL_Texture* gDebugTexture    = NULL;
bool gDebugWindowOpen                = false; 

static SDL_AudioStream* gAudioStream = NULL;

static PlatformContext gPlatformCtx;

static bool sdlVideoInit(void)
{
  if (!SDL_Init(SDL_INIT_VIDEO)) return false; 

  gWindow = SDL_CreateWindow("Game Boy Emulator", WIDTH * 4, HEIGHT * 4, SDL_WINDOW_RESIZABLE);
  if (!gWindow) return false; 

  gRenderer = SDL_CreateRenderer(gWindow, NULL);
  if (!gRenderer) return false; 

  gDisplayTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (!gDisplayTexture) return false;
  
  gActiveDmgPalette.color0 = 0xE0F8D0FF; // Lightest (#e0f8d0)
  gActiveDmgPalette.color1 = 0x88C070FF; // Light    (#88c070)
  gActiveDmgPalette.color2 = 0x346856FF; // Dark     (#346856)
  gActiveDmgPalette.color3 = 0x081820FF; // Darkest  (#081820)

  SDL_SetTextureScaleMode(gDisplayTexture, SDL_SCALEMODE_NEAREST);
  return true;
}

static void sdlSetDmgPalette(DmgPalette PALETTE)
{ gActiveDmgPalette = PALETTE; }

static void sdlSetShader(bool ENABLE)
{
  gShaderEnabled = ENABLE;
  if (gShaderEnabled) SDL_SetTextureScaleMode(gDisplayTexture, SDL_SCALEMODE_LINEAR);
  else                SDL_SetTextureScaleMode(gDisplayTexture, SDL_SCALEMODE_NEAREST);
}

static void sdlDrawTileView(const u8* vbk0, const u8* vbk1)
{
  (void) vbk1;
  if (!gDebugWindowOpen) 
  {
    if (gDebugWindow)
    {
      SDL_DestroyTexture(gDebugTexture);
      SDL_DestroyRenderer(gDebugRenderer);
      SDL_DestroyWindow(gDebugWindow);
      gDebugWindow = NULL;
    }
    return;
  }

  if (!gDebugWindow)
  {
    gDebugWindow = SDL_CreateWindow("VRAM Tile Viewer", 128 * 2, 192 * 2, 0);
    gDebugRenderer = SDL_CreateRenderer(gDebugWindow, NULL);
    gDebugTexture = SDL_CreateTexture(gDebugRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 192);
  }

  void* pixels = NULL;
  int pitch = 0;

  if (SDL_LockTexture(gDebugTexture, NULL, &pixels, &pitch))
  {
    u32* dest = (u32*)pixels;
    i32 pixelsPerRow = pitch / sizeof(u32);
    
    GameBoyMode romMode = cartridgeGetContext()->mode;
    bool isCgbMode = (romMode == MODE_CGB_GAMEBOY || romMode == MODE_CGB_ONLY_GAMEBOY);
    PpuContext* ctx = ppuGetContext();

    for (int t = 0; t < 384; t++)
    {
      int tileX = (t % 16) * 8;
      int tileY = (t / 16) * 8;

      for (int y = 0; y < 8; y++)
      {
        u8 byte1 = vbk0[(t * 16) + (y * 2)];
        u8 byte2 = vbk0[(t * 16) + (y * 2) + 1];

        for (int x = 0; x < 8; x++)
        {
          int bit = 7 - x; 
          u8 colorIdx = ((byte1 >> bit) & 1) | (((byte2 >> bit) & 1) << 1);
          u32 color = 0;
          
          if (isCgbMode)
          {
            // THE FIX: Parse the actual CGB Palette 0 memory to colorize the VRAM Viewer!
            u16 cramAddr = colorIdx * 2; 
            u8 dataLow = ctx->bgPaletteRam[cramAddr];
            u8 dataHigh = ctx->bgPaletteRam[cramAddr + 1];
            u16 cgbColor = (((u16)dataHigh) << 8) | dataLow;
            
            u8 r5 = (cgbColor & 0x001F);
            u8 g5 = (cgbColor & 0x03E0) >> 5;
            u8 b5 = (cgbColor & 0x7C00) >> 10;
            u8 r8 = (r5 << 3) | (r5 >> 2);
            u8 g8 = (g5 << 3) | (g5 >> 2);
            u8 b8 = (b5 << 3) | (b5 >> 2);
            color = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
          }
          else 
          {
            switch(colorIdx) 
            {
              case 0: color = gActiveDmgPalette.color0; break;
              case 1: color = gActiveDmgPalette.color1; break;
              case 2: color = gActiveDmgPalette.color2; break;
              case 3: color = gActiveDmgPalette.color3; break;
            }
          }

          dest[((tileY + y) * pixelsPerRow) + (tileX + x)] = color;
        }
      }
    }
    SDL_UnlockTexture(gDebugTexture);
    SDL_RenderClear(gDebugRenderer);
    SDL_RenderTexture(gDebugRenderer, gDebugTexture, NULL, NULL);
    SDL_RenderPresent(gDebugRenderer);
  }
}

static void sdlRenderFrame(const PpuFrame* FRAME)
{
  void* lockedPixels = NULL;
  int   pitch        = 0;

  if (!SDL_LockTexture(gDisplayTexture, NULL, &lockedPixels, &pitch))
  {
    return; 
  }

  u32* destination = (u32*)lockedPixels;
  
  GameBoyMode romMode = cartridgeGetContext()->mode;
  bool isCgbMode = (romMode == MODE_CGB_GAMEBOY || romMode == MODE_CGB_ONLY_GAMEBOY);
  
  i32 pixelsPerRow = pitch / sizeof(u32);

  for (i32 y = 0; y < HEIGHT; y++)
  {
    for (i32 x = 0; x < WIDTH; x++)
    {
      i32 bufferIndex = (y * pixelsPerRow) + x;
      u16 rawCoreData = FRAME->pixels[y][x];

      if (!isCgbMode)
      {
        switch (rawCoreData)
        {
          case 0: destination[bufferIndex] = gActiveDmgPalette.color0; break;
          case 1: destination[bufferIndex] = gActiveDmgPalette.color1; break;
          case 2: destination[bufferIndex] = gActiveDmgPalette.color2; break;
          case 3: destination[bufferIndex] = gActiveDmgPalette.color3; break;
          default: destination[bufferIndex] = 0xFF00FFFF; break; 
        }
      }
      else
      {
        u8 r5 = (rawCoreData & 0x001F);
        u8 g5 = (rawCoreData & 0x03E0) >> 5;
        u8 b5 = (rawCoreData & 0x7C00) >> 10;

        u8 r8 = (r5 << 3) | (r5 >> 2);
        u8 g8 = (g5 << 3) | (g5 >> 2);
        u8 b8 = (b5 << 3) | (b5 >> 2);

        destination[bufferIndex] = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
      }
    }
  }

  SDL_UnlockTexture(gDisplayTexture);
  SDL_RenderClear(gRenderer);
  SDL_RenderTexture(gRenderer, gDisplayTexture, NULL, NULL);
}

static void sdlPresent(void)
{
  SDL_RenderPresent(gRenderer);
}

static void sdlCleanup(void)
{
  if (gDebugTexture)   SDL_DestroyTexture(gDebugTexture);
  if (gDebugRenderer)  SDL_DestroyRenderer(gDebugRenderer);
  if (gDebugWindow)    SDL_DestroyWindow(gDebugWindow);

  if (gDisplayTexture) SDL_DestroyTexture(gDisplayTexture);
  if (gRenderer)       SDL_DestroyRenderer(gRenderer);
  if (gWindow)         SDL_DestroyWindow(gWindow);
  SDL_Quit();
}

static SDL_Gamepad* gGamepad = NULL;

// Unified hold states for system toggles
static bool pauseHeld = false, speedHeld = false, fullHeld = false;

// Default Configuration
static InputConfig gInputConfig = {
    .keyUp = SDL_SCANCODE_UP, .keyDown = SDL_SCANCODE_DOWN, 
    .keyLeft = SDL_SCANCODE_LEFT, .keyRight = SDL_SCANCODE_RIGHT,
    .keyA = SDL_SCANCODE_Z, .keyB = SDL_SCANCODE_X,
    .keyStart = SDL_SCANCODE_RETURN, .keySelect = SDL_SCANCODE_RSHIFT,
    .keyPause = SDL_SCANCODE_P, .keySpeed = SDL_SCANCODE_F,
    .keyFullscreen = SDL_SCANCODE_F11,
    
    .padA = SDL_GAMEPAD_BUTTON_SOUTH, .padB = SDL_GAMEPAD_BUTTON_EAST,
    .padStart = SDL_GAMEPAD_BUTTON_START, .padSelect = SDL_GAMEPAD_BUTTON_BACK,
    .padUp = SDL_GAMEPAD_BUTTON_DPAD_UP, .padDown = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    .padLeft = SDL_GAMEPAD_BUTTON_DPAD_LEFT, .padRight = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    
    // Map system actions to standard modern controller buttons (e.g., Xbox Guide / Stick Clicks)
    .padPause = SDL_GAMEPAD_BUTTON_GUIDE, 
    .padSpeed = SDL_GAMEPAD_BUTTON_RIGHT_STICK, 
    .padFullscreen = SDL_GAMEPAD_BUTTON_LEFT_STICK 
};

static void sdlInputSetConfig(InputConfig NEW_CONFIG) {
    gInputConfig = NEW_CONFIG;
}

static void sdlInputPoll(bool* IS_RUNNING) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      
        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            FORGE_LOG_INFO("[C++ Debug] SDL caught scancode: %d | Down: %d", event.key.scancode, event.key.down);
        }
        if (event.type == SDL_EVENT_QUIT) *IS_RUNNING = false;
        
        // Gamepad Hotplugging
        if (event.type == SDL_EVENT_GAMEPAD_ADDED && !gGamepad) {
            gGamepad = SDL_OpenGamepad(event.gdevice.which);
            FORGE_LOG_INFO("%s", "Gamepad Connected.");
        }
        if (event.type == SDL_EVENT_GAMEPAD_REMOVED && gGamepad) {
            SDL_CloseGamepad(gGamepad);
            gGamepad = NULL;
            FORGE_LOG_INFO("%s", "Gamepad Disconnected.");
        }
    }

    const bool* keyboard = SDL_GetKeyboardState(NULL);
    PlatformContext* platform = platformGetContext();

    // --- System Toggles (Keyboard OR Gamepad) ---
    bool isPausePressed = keyboard[gInputConfig.keyPause] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padPause));
    if (isPausePressed && !pauseHeld) platform->input.paused = !platform->input.paused;
    pauseHeld = isPausePressed;

    bool isSpeedPressed = keyboard[gInputConfig.keySpeed] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padSpeed));
    if (isSpeedPressed && !speedHeld) {
        platform->input.doubleSpeed = !platform->input.doubleSpeed;
        apuSetSpeed(platform->input.doubleSpeed ? 2.0f : 1.0f);
    }
    speedHeld = isSpeedPressed;

    bool isFullPressed = keyboard[gInputConfig.keyFullscreen] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padFullscreen));
    if (isFullPressed && !fullHeld) {
        platform->input.fullscreen = !platform->input.fullscreen;
        // Trigger SDL fullscreen toggle here if needed for the desktop build
    }
    fullHeld = isFullPressed;

    // --- Emulator Joypad Mapping ---
    bool up = keyboard[gInputConfig.keyUp] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padUp));
    bool down = keyboard[gInputConfig.keyDown] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padDown));
    bool left = keyboard[gInputConfig.keyLeft] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padLeft));
    bool right = keyboard[gInputConfig.keyRight] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padRight));
    bool a = keyboard[gInputConfig.keyA] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padA));
    bool b = keyboard[gInputConfig.keyB] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padB));
    bool start = keyboard[gInputConfig.keyStart] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padStart));
    bool select = keyboard[gInputConfig.keySelect] || (gGamepad && SDL_GetGamepadButton(gGamepad, gInputConfig.padSelect));

    joypadSetButton(JOYPAD_BUTTON_SELECT, select);
    joypadSetButton(JOYPAD_BUTTON_START, start);
    joypadSetButton(JOYPAD_BUTTON_UP, up);
    joypadSetButton(JOYPAD_BUTTON_DOWN, down);
    joypadSetButton(JOYPAD_BUTTON_LEFT, left);
    joypadSetButton(JOYPAD_BUTTON_RIGHT, right);
    joypadSetButton(JOYPAD_BUTTON_A, a);
    joypadSetButton(JOYPAD_BUTTON_B, b);
}

static bool sdlAudioInit(void)
{
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    FORGE_LOG_DEBUG("[AUDIO] SDL_Init Audio Failed: %s", SDL_GetError());
    return false;
  }

  SDL_AudioSpec spec = { SDL_AUDIO_F32, 2, 44100 };
  
  // The foolproof SDL3 method: Opens device, creates stream, and binds them atomically.
  gAudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
  if (!gAudioStream) {
    FORGE_LOG_DEBUG("[AUDIO] Open Stream Failed: %s", SDL_GetError());
    return false;
  }

  // Unpause the specific device tied to this stream
  SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(gAudioStream));

  return true;
}

static void sdlPushSamples(const f32* SAMPLES, u32 COUNT)
{
  if (!gAudioStream) return;

  const int MAX_QUEUED_BYTES = 8192;

#ifndef __EMSCRIPTEN__
  // NATIVE: Block thread if full
  while (SDL_GetAudioStreamQueued(gAudioStream) > MAX_QUEUED_BYTES) 
  {
    SDL_Delay(1); 
  }
#else
  // WEB: NEVER silently drop chunks of samples (this causes the static!).
  // Only intervene if the user changes browser tabs and the queue builds up massive latency.
  if (SDL_GetAudioStreamQueued(gAudioStream) > MAX_QUEUED_BYTES * 4) 
  {
    return;
  }
#endif

  // Push every single sample flawlessly to the sound card
  SDL_PutAudioStreamData(gAudioStream, SAMPLES, COUNT * sizeof(float));
}

static void sdlAudioCleanup(void)
{
  if (gAudioStream) SDL_DestroyAudioStream(gAudioStream);
}

PlatformContext* platformGetContext(void)
{ return &gPlatformCtx; }

void platformInit(void)
{
  gPlatformCtx.name                 = "SDL3 Desktop Frontend";

  gPlatformCtx.video.init           = sdlVideoInit;
  gPlatformCtx.video.setDmgPalette  = sdlSetDmgPalette;
  gPlatformCtx.video.enableShader   = sdlSetShader;
  gPlatformCtx.video.renderFrame    = sdlRenderFrame;
  #ifdef DEBUG 
    gPlatformCtx.video.drawTileView   = sdlDrawTileView;
  #endif
  gPlatformCtx.video.present        = sdlPresent;
  gPlatformCtx.video.cleanup        = sdlCleanup;

  gPlatformCtx.input.poll           = sdlInputPoll;
  gPlatformCtx.input.setConfig      = sdlInputSetConfig;

  gPlatformCtx.audio.init           = sdlAudioInit;
  gPlatformCtx.audio.pushSamples    = sdlPushSamples;
  gPlatformCtx.audio.cleanup        = sdlAudioCleanup;

  FORGE_ASSERT(gPlatformCtx.video.init());
  FORGE_ASSERT(gPlatformCtx.audio.init());
}

#endif
