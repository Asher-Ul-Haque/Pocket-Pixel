#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL3/SDL.h>
#include <platform.h>
#include <debug.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <joypad.h>

static SDL_Window* gWindow           = NULL;
static SDL_Renderer* gRenderer         = NULL;
static SDL_Texture* gDisplayTexture   = NULL;
static DmgPalette     gActiveDmgPalette;
static bool           gShaderEnabled    = false;
static bool           gFirstFrameTraced = false;

static PlatformContext gPlatformCtx;

static bool sdlVideoInit(void)
{
  FORGE_LOG_DEBUG("%s", "[PLATFORM] Initializing SDL3 Video...");
  if (!SDL_Init(SDL_INIT_VIDEO))
  { 
    FORGE_LOG_DEBUG("[PLATFORM] SDL_Init Failed: %s", SDL_GetError());
    return false; 
  }

  gWindow = SDL_CreateWindow("Game Boy Emulator", WIDTH * 4, HEIGHT * 4, SDL_WINDOW_RESIZABLE);
  if (!gWindow) 
  { 
    FORGE_LOG_DEBUG("[PLATFORM] SDL_CreateWindow Failed: %s", SDL_GetError());
    return false; 
  }

  gRenderer = SDL_CreateRenderer(gWindow, NULL);
  if (!gRenderer) 
  { 
    FORGE_LOG_DEBUG("[PLATFORM] SDL_CreateRenderer Failed: %s", SDL_GetError());
    return false; 
  }

  gDisplayTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (!gDisplayTexture)
  {
    FORGE_LOG_DEBUG("[PLATFORM] SDL_CreateTexture Failed: %s", SDL_GetError());
    return false;
  }
  
  gActiveDmgPalette.color0 = 0x9BBC0FFF; 
  gActiveDmgPalette.color1 = 0x8BAC0FFF;
  gActiveDmgPalette.color2 = 0x306230FF;
  gActiveDmgPalette.color3 = 0x0F380FFF;

  SDL_SetTextureScaleMode(gDisplayTexture, SDL_SCALEMODE_NEAREST);
  FORGE_LOG_DEBUG("%s", "[PLATFORM] SDL3 Video successfully initialized.");
  return true;
}

static void sdlSetDmgPalette(DmgPalette PALETTE)
{ gActiveDmgPalette = PALETTE; }

static void sdlSetShader(bool ENABLE)
{
  gShaderEnabled = ENABLE;
  if (gShaderEnabled)
  {
    SDL_SetTextureScaleMode(gDisplayTexture, SDL_SCALEMODE_LINEAR);
  }
  else
  {
    SDL_SetTextureScaleMode(gDisplayTexture, SDL_SCALEMODE_NEAREST);
  }
}

static void sdlRenderFrame(const PpuFrame* FRAME)
{
  if (!gFirstFrameTraced)
  {
    FORGE_LOG_DEBUG("%s", "[RENDERER] First frame arrived at the platform! Locking texture...");
    gFirstFrameTraced = true;
  }

  void* lockedPixels = NULL;
  int   pitch        = 0;

  if (!SDL_LockTexture(gDisplayTexture, NULL, &lockedPixels, &pitch))
  {
    FORGE_LOG_DEBUG("[RENDERER] Texture lock failed: %s", SDL_GetError());
    return; 
  }

  u32* destination = (u32*)lockedPixels;
  bool isCgbMode   = (cartridgeGetContext()->mode == MODE_CGB_GAMEBOY);
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
  if (gDisplayTexture)  SDL_DestroyTexture(gDisplayTexture);
  if (gRenderer)        SDL_DestroyRenderer(gRenderer);
  if (gWindow)          SDL_DestroyWindow(gWindow);
  SDL_Quit();
}

static void sdlPollEvents(bool* IS_RUNNING)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_EVENT_QUIT) *IS_RUNNING = false;
  }

  // --- Hardware Joypad Routing ---
  // The platform reads the OS and pushes it strictly into the hardware abstraction.
  const bool* keyboard = SDL_GetKeyboardState(NULL);
  
  joypadSetButton(JOYPAD_BUTTON_UP,     keyboard[SDL_SCANCODE_UP]);
  joypadSetButton(JOYPAD_BUTTON_DOWN,   keyboard[SDL_SCANCODE_DOWN]);
  joypadSetButton(JOYPAD_BUTTON_LEFT,   keyboard[SDL_SCANCODE_LEFT]);
  joypadSetButton(JOYPAD_BUTTON_RIGHT,  keyboard[SDL_SCANCODE_RIGHT]);
  joypadSetButton(JOYPAD_BUTTON_A,      keyboard[SDL_SCANCODE_Z]);
  joypadSetButton(JOYPAD_BUTTON_B,      keyboard[SDL_SCANCODE_X]);
  joypadSetButton(JOYPAD_BUTTON_START,  keyboard[SDL_SCANCODE_RETURN]);
  joypadSetButton(JOYPAD_BUTTON_SELECT, keyboard[SDL_SCANCODE_RSHIFT]);
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
  gPlatformCtx.video.present        = sdlPresent;
  gPlatformCtx.video.cleanup        = sdlCleanup;
  gPlatformCtx.input.poll           = sdlPollEvents;

  if (!gPlatformCtx.video.init())
  {
    FORGE_LOG_FATAL("%s", "Hardware Init Failed. Terminating.");
    exit(1);
  }
}

#endif
