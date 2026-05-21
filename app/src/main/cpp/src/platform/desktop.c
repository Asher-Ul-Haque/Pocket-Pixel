#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL3/SDL.h>
#include <platform.h>
#include <utils/asserts.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <joypad.h>

#define WINDOW_SCALE_FACTOR 3
#define WINDOW_WIDTH        (WIDTH * WINDOW_SCALE_FACTOR)
#define WINDOW_HEIGHT       (HEIGHT * WINDOW_SCALE_FACTOR)
#define DESIGN_WIDTH  1280
#define DESIGN_HEIGHT 720

typedef struct
{
  SDL_Window*   window;
  SDL_Renderer* renderer;
  SDL_Texture*  gameTexture;
  SDL_Texture*  tileTexture;
  SDL_Texture*  mapTexture;
  SDL_Gamepad*  gamepad;
  u32           dmgColors[4];
} SdlInternalContext;

static SdlInternalContext rendererCTX;

static bool uploadTextureRGBA8888(SDL_Texture* TEXTURE, const u32* PIXELS, i32 WIDTH_PX, i32 HEIGHT_PX, const char* LABEL)
{
  (void) HEIGHT_PX;
  const i32 pitch = WIDTH_PX * (i32)sizeof(u32);
  if (SDL_UpdateTexture(TEXTURE, NULL, PIXELS, pitch) != 0)
  {
    FORGE_LOG_ERROR("[SDL] SDL_UpdateTexture failed for %s: %s", LABEL, SDL_GetError());
    return false;
  }
  return true;
}

static u32 colorConvert555To8888(u16 COLOR_555)
{
  u8 red   = (COLOR_555 & 0x1F);
  u8 green = ((COLOR_555 >> 5) & 0x1F);
  u8 blue  = ((COLOR_555 >> 10) & 0x1F);

  red   = (red << 3) | (red >> 2);
  green = (green << 3) | (green >> 2);
  blue  = (blue << 3) | (blue >> 2);

  return (u32)((red << 24) | (green << 16) | (blue << 8) | 0xFF);
}

void sdlInit(void)
{
  FORGE_LOG_DEBUG("%s", "Initializing desktop SDL runtime");
  FORGE_ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD));

  rendererCTX.window = SDL_CreateWindow("Pocket Pixel Debugger", DESIGN_WIDTH, DESIGN_HEIGHT, 0);
  FORGE_ASSERT(rendererCTX.window);

  rendererCTX.renderer = SDL_CreateRenderer(rendererCTX.window, NULL);
  FORGE_ASSERT(rendererCTX.renderer);

  rendererCTX.gameTexture = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  rendererCTX.tileTexture = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 192);
  rendererCTX.mapTexture  = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
  FORGE_ASSERT(rendererCTX.gameTexture && rendererCTX.tileTexture && rendererCTX.mapTexture);

  rendererCTX.dmgColors[0] = 0x9BBC0FFF;
  rendererCTX.dmgColors[1] = 0x8BAC0FFF;
  rendererCTX.dmgColors[2] = 0x306230FF;
  rendererCTX.dmgColors[3] = 0x0F380FFF;
  rendererCTX.gamepad = NULL;
}

void renderFrame(const PpuFrame* FRAME)
{
  static u32 renderCalls = 0;
  static u32 framePixels[WIDTH * HEIGHT];
  const u8 mode = cartridgeGetContext()->mode;
  u32 checksum = 0;
  u32 nonBlack = 0;

  for (i32 y = 0; y < HEIGHT; ++y)
  {
    for (i32 x = 0; x < WIDTH; ++x)
    {
      const i32 index = (y * WIDTH) + x;
      checksum ^= (u32)(FRAME->resolvedColor[index] + (u16)index);
      if (mode == MODE_DMG_GAMEBOY)
      {
        framePixels[index] = rendererCTX.dmgColors[FRAME->resolvedColor[index] & 0x03];
      }
      else
      {
        const u16 color555 = FRAME->resolvedColor[index] & 0x7FFF;
        framePixels[index] = colorConvert555To8888(color555);
      }
      if ((framePixels[index] & 0x00FFFFFFu) != 0) nonBlack++;
    }
  }
  if (!uploadTextureRGBA8888(rendererCTX.gameTexture, framePixels, WIDTH, HEIGHT, "gameTexture")) return;

  renderCalls++;
  if ((renderCalls % 120) == 0)
  {
    FORGE_LOG_INFO(
      "[SDL] renderFrame calls=%u mode=%u checksum=0x%08X firstSrc=0x%04X firstOut=0x%08X nonBlack=%u",
      renderCalls,
      (u32)mode,
      checksum,
      FRAME->resolvedColor[0],
      framePixels[0],
      nonBlack
    );
    if (nonBlack == 0) FORGE_LOG_WARNING("%s", "[SDL] renderFrame output is fully black (RGB)");
  }
}

void drawTileView(const u8* VRAM_BANK_0, const u8* VRAM_BANK_1)
{
  (void) VRAM_BANK_1;
  if (!VRAM_BANK_0 || !rendererCTX.tileTexture) return;
  static u32 tilePixels[128 * 192];
  u32 nonBlack = 0;
  
  for (i32 tile = 0; tile < 384; ++tile)
  {
    for (i32 y = 0; y < 8; ++y)
    {
      u8 b1 = VRAM_BANK_0[tile * 16 + y * 2];
      u8 b2 = VRAM_BANK_0[tile * 16 + y * 2 + 1];
      const i32 rowY = (tile / 16) * 8 + y;
      for (i32 x = 0; x < 8; ++x)
      {
        u8 color = (((b2 >> (7 - x)) & 0x01) << 1) | ((b1 >> (7 - x)) & 0x01);
        const i32 index = (rowY * 128) + ((tile % 16) * 8) + x;
        tilePixels[index] = rendererCTX.dmgColors[color];
        if ((tilePixels[index] & 0x00FFFFFFu) != 0) nonBlack++;
      }
    }
  }
  if (!uploadTextureRGBA8888(rendererCTX.tileTexture, tilePixels, 128, 192, "tileTexture")) return;
  static u32 tileCalls = 0;
  tileCalls++;
  if ((tileCalls % 120) == 0)
  {
    FORGE_LOG_INFO("[SDL] drawTileView calls=%u nonBlack=%u firstPx=0x%08X", tileCalls, nonBlack, tilePixels[0]);
    if (nonBlack == 0) FORGE_LOG_WARNING("%s", "[SDL] tile texture output is fully black (RGB)");
  }
}

void drawMapView(const u8* VRAM_BANK_0, const u8* VRAM_BANK_1, u8 MAP_SELECT)
{
  (void) VRAM_BANK_1;
  if (!VRAM_BANK_0 || !rendererCTX.mapTexture) return;
  static u32 mapPixels[256 * 256];
  u32 nonBlack = 0;

  u16 mapOffset = (MAP_SELECT == 0) ? 0x1800 : 0x1C00;

  for (i32 ty = 0; ty < 32; ++ty)
  {
    for (i32 tx = 0; tx < 32; ++tx)
    {
      u8 tileId = VRAM_BANK_0[mapOffset + (ty * 32) + tx];
      for (i32 y = 0; y < 8; ++y)
      {
        u8 b1 = VRAM_BANK_0[tileId * 16 + y * 2];
        u8 b2 = VRAM_BANK_0[tileId * 16 + y * 2 + 1];
        const i32 rowY = (ty * 8) + y;
        for (i32 x = 0; x < 8; ++x)
        {
          u8 color = (((b2 >> (7 - x)) & 0x01) << 1) | ((b1 >> (7 - x)) & 0x01);
          const i32 index = (rowY * 256) + (tx * 8) + x;
          mapPixels[index] = rendererCTX.dmgColors[color];
          if ((mapPixels[index] & 0x00FFFFFFu) != 0) nonBlack++;
        }
      }
    }
  }
  if (!uploadTextureRGBA8888(rendererCTX.mapTexture, mapPixels, 256, 256, "mapTexture")) return;
  static u32 mapCalls = 0;
  mapCalls++;
  if ((mapCalls % 120) == 0)
  {
    FORGE_LOG_INFO("[SDL] drawMapView calls=%u nonBlack=%u firstPx=0x%08X", mapCalls, nonBlack, mapPixels[0]);
    if (nonBlack == 0) FORGE_LOG_WARNING("%s", "[SDL] map texture output is fully black (RGB)");
  }
}

static bool isKeyboardPressed(SDL_Scancode KEY)
{
  const bool* keyboardState = SDL_GetKeyboardState(NULL);
  return keyboardState && keyboardState[KEY];
}

static bool isGamepadPressed(SDL_GamepadButton BUTTON)
{
  if (!rendererCTX.gamepad) return false;
  return SDL_GetGamepadButton(rendererCTX.gamepad, BUTTON) != 0;
}

void pollInput(bool* RUNNING)
{
  if (!RUNNING) return;

  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_EVENT_QUIT)
    {
      *RUNNING = false;
      continue;
    }

    if (event.type == SDL_EVENT_GAMEPAD_ADDED && !rendererCTX.gamepad)
    {
      rendererCTX.gamepad = SDL_OpenGamepad(event.gdevice.which);
      continue;
    }

    if (event.type == SDL_EVENT_GAMEPAD_REMOVED && rendererCTX.gamepad)
    {
      SDL_CloseGamepad(rendererCTX.gamepad);
      rendererCTX.gamepad = NULL;
      continue;
    }
  }

  const bool right  = isKeyboardPressed(SDL_SCANCODE_RIGHT)     || isGamepadPressed(SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
  const bool left   = isKeyboardPressed(SDL_SCANCODE_LEFT)      || isGamepadPressed(SDL_GAMEPAD_BUTTON_DPAD_LEFT);
  const bool up     = isKeyboardPressed(SDL_SCANCODE_UP)        || isGamepadPressed(SDL_GAMEPAD_BUTTON_DPAD_UP);
  const bool down   = isKeyboardPressed(SDL_SCANCODE_DOWN)      || isGamepadPressed(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  const bool a      = isKeyboardPressed(SDL_SCANCODE_X)         || isGamepadPressed(SDL_GAMEPAD_BUTTON_SOUTH);
  const bool b      = isKeyboardPressed(SDL_SCANCODE_Z)         || isGamepadPressed(SDL_GAMEPAD_BUTTON_EAST);
  const bool select = isKeyboardPressed(SDL_SCANCODE_BACKSPACE) || isGamepadPressed(SDL_GAMEPAD_BUTTON_BACK);
  const bool start  = isKeyboardPressed(SDL_SCANCODE_RETURN)    || isGamepadPressed(SDL_GAMEPAD_BUTTON_START);

  joypadSetButton(JOYPAD_BUTTON_RIGHT, right);
  joypadSetButton(JOYPAD_BUTTON_LEFT, left);
  joypadSetButton(JOYPAD_BUTTON_UP, up);
  joypadSetButton(JOYPAD_BUTTON_DOWN, down);
  joypadSetButton(JOYPAD_BUTTON_A, a);
  joypadSetButton(JOYPAD_BUTTON_B, b);
  joypadSetButton(JOYPAD_BUTTON_SELECT, select);
  joypadSetButton(JOYPAD_BUTTON_START, start);
}

void printKeybinds(void)
{
  FORGE_LOG_INFO("%s", "Controls:");
  FORGE_LOG_INFO("%s", "  Keyboard:");
  FORGE_LOG_INFO("%s", "    D-Pad: Arrow Keys");
  FORGE_LOG_INFO("%s", "    A: X");
  FORGE_LOG_INFO("%s", "    B: Z");
  FORGE_LOG_INFO("%s", "    Start: Enter");
  FORGE_LOG_INFO("%s", "    Select: Backspace");
  FORGE_LOG_INFO("%s", "  Gamepad:");
  FORGE_LOG_INFO("%s", "    D-Pad: D-Pad");
  FORGE_LOG_INFO("%s", "    A: South Button");
  FORGE_LOG_INFO("%s", "    B: East Button");
  FORGE_LOG_INFO("%s", "    Start: Start");
  FORGE_LOG_INFO("%s", "    Select: Back/Minus");
}

void present(void)
{
  static u32 presentCalls = 0;
  static bool layoutLogged = false;
  if (!rendererCTX.renderer) return;

  SDL_SetRenderDrawColor(rendererCTX.renderer, 20, 20, 20, 255);
  SDL_RenderClear(rendererCTX.renderer);

  i32 windowW = DESIGN_WIDTH;
  i32 windowH = DESIGN_HEIGHT;
  SDL_GetWindowSize(rendererCTX.window, &windowW, &windowH);

  const float margin = 20.0f;
  SDL_FRect rGame = { margin, margin, WIDTH * 3.0f, HEIGHT * 3.0f };
  SDL_FRect rTile = { rGame.x + rGame.w + margin, margin, 128.0f * 2.0f, 192.0f * 2.0f };
  SDL_FRect rMap  = { rTile.x + rTile.w + margin, margin, 256.0f * 1.5f, 256.0f * 1.5f };

  if ((rMap.x + rMap.w) > (windowW - margin))
  {
    rMap.x = margin;
    rMap.y = rGame.y + rGame.h + margin;
  }
  if ((rTile.x + rTile.w) > (windowW - margin))
  {
    rTile.x = margin;
    rTile.y = rMap.y + rMap.h + margin;
  }

  if (!layoutLogged)
  {
    FORGE_LOG_INFO(
      "[SDL] window=%dx%d game=(%.0f,%.0f,%.0f,%.0f) tile=(%.0f,%.0f,%.0f,%.0f) map=(%.0f,%.0f,%.0f,%.0f)",
      windowW, windowH,
      rGame.x, rGame.y, rGame.w, rGame.h,
      rTile.x, rTile.y, rTile.w, rTile.h,
      rMap.x, rMap.y, rMap.w, rMap.h
    );
    layoutLogged = true;
  }

  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.gameTexture, NULL, &rGame);
  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.tileTexture, NULL, &rTile);
  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.mapTexture,  NULL, &rMap);

  SDL_RenderPresent(rendererCTX.renderer);
  presentCalls++;
  if ((presentCalls % 120) == 0)
  {
    FORGE_LOG_INFO("[SDL] present calls=%u", presentCalls);
  }
}

void cleanup(void)
{
  if (rendererCTX.gamepad)
  {
    SDL_CloseGamepad(rendererCTX.gamepad);
    rendererCTX.gamepad = NULL;
  }

  SDL_DestroyTexture(rendererCTX.gameTexture);
  SDL_DestroyRenderer(rendererCTX.renderer);
  SDL_DestroyWindow(rendererCTX.window);
  SDL_Quit();
}

static PlatformContext ctx =
{
  .name       = "Desktop SDL 3 Runtime",
  .rendering  =
  {
    .init         = sdlInit,
    .renderFrame  = renderFrame,
    .drawTileView = drawTileView,
    .drawMapView  = drawMapView,
    .present      = present,
    .cleanup      = cleanup
  },
  .input =
  {
    .poll         = pollInput,
    .printKeybinds= printKeybinds
  }
};

PlatformContext* platformGetContext(void) { return &ctx; }
void platformInit(void) { ctx.rendering.init(); }

#endif
