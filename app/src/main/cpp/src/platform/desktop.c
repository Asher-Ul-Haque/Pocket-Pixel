#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL3/SDL.h>
#include <platform.h>
#include <utils/asserts.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <input/joypad.h>

#define WINDOW_SCALE_FACTOR 3
#define WINDOW_WIDTH        (WIDTH * WINDOW_SCALE_FACTOR)
#define WINDOW_HEIGHT       (HEIGHT * WINDOW_SCALE_FACTOR)

typedef struct
{
  SDL_Window*  window;
  SDL_Renderer* renderer;
  SDL_Texture* gameTexture;
  SDL_Gamepad* gamepad;
  u32 dmgColors[4];
} SdlInternalContext;

static SdlInternalContext rendererCTX;

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

  rendererCTX.window = SDL_CreateWindow("Pocket Pixel", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  FORGE_ASSERT(rendererCTX.window);

  rendererCTX.renderer = SDL_CreateRenderer(rendererCTX.window, NULL);
  FORGE_ASSERT(rendererCTX.renderer);

  rendererCTX.gameTexture = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  FORGE_ASSERT(rendererCTX.gameTexture);

  rendererCTX.dmgColors[0] = 0x9BBC0FFF;
  rendererCTX.dmgColors[1] = 0x8BAC0FFF;
  rendererCTX.dmgColors[2] = 0x306230FF;
  rendererCTX.dmgColors[3] = 0x0F380FFF;
  rendererCTX.gamepad = NULL;
}

void renderFrame(const PpuFrame* FRAME)
{
  void* pixels;
  i32 pitch;
  if (SDL_LockTexture(rendererCTX.gameTexture, NULL, &pixels, &pitch) != 0) return;

  u32* dest = (u32*)pixels;
  const u8 mode = cartridgeGetContext()->mode;

  for (i32 i = 0; i < WIDTH * HEIGHT; ++i)
  {
    PpuPixel pixel = FRAME->pixels[i];

    if (mode == MODE_DMG_GAMEBOY)
    {
      const u8 paletteReg = (pixel.bits.layer == 0)
        ? FRAME->palettes.dmg.bgp
        : ((pixel.bits.paletteId == 0) ? FRAME->palettes.dmg.obp0 : FRAME->palettes.dmg.obp1);

      const u8 physicalIndex = (u8)((paletteReg >> (pixel.bits.colorIndex * 2)) & 0x03);
      dest[i] = rendererCTX.dmgColors[physicalIndex];
    }
    else
    {
      const u16 color555 = (pixel.bits.layer == 0)
        ? FRAME->palettes.cgb.bg[(pixel.bits.paletteId * 4) + pixel.bits.colorIndex]
        : FRAME->palettes.cgb.obj[(pixel.bits.paletteId * 4) + pixel.bits.colorIndex];

      dest[i] = colorConvert555To8888(color555);
    }
  }

  SDL_UnlockTexture(rendererCTX.gameTexture);
}

void drawTileView(const u8* VRAM_BANK_0, const u8* VRAM_BANK_1)
{
  (void)VRAM_BANK_0;
  (void)VRAM_BANK_1;
}

void drawMapView(const u8* VRAM_BANK_0, const u8* VRAM_BANK_1, u8 MAP_SELECT)
{
  (void)VRAM_BANK_0;
  (void)VRAM_BANK_1;
  (void)MAP_SELECT;
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
  if (!rendererCTX.renderer || !rendererCTX.gameTexture) return;

  SDL_SetRenderDrawColor(rendererCTX.renderer, 0, 0, 0, 255);
  SDL_RenderClear(rendererCTX.renderer);

  SDL_FRect frameRect = { 0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT };
  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.gameTexture, NULL, &frameRect);
  SDL_RenderPresent(rendererCTX.renderer);
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
