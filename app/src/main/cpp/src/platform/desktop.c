#include "cartridge/cartridge.h"
#include "ppu/internal.h"
#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL3/SDL.h>
#include <platform.h>
#include <utils/asserts.h>

static const u32 DMG_THEME[4] = 
  {
    0xFF9BBC0F,   // - - - Lightest Green
    0xFF8BAC0F,   // - - - Light Green
    0xFF306230,   // - - - Dark Green
    0xFF0F380F    // - - - Darkest Green
  };

// - - - Internal SDL3 state
static struct 
{
  SDL_Window  *   window;
  SDL_Renderer*   renderer;
  SDL_Texture *   texture;
} sdlInternal;

static PlatformContext ctx;

u32 colorToRGB32(u16 CGB_COLOR) 
{
  u8 r = (CGB_COLOR & RGB555_R_MASK);
  u8 g = (CGB_COLOR & RGB555_G_MASK) >> RGB555_G_SHIFT;
  u8 b = (CGB_COLOR & RGB555_B_MASK) >> RGB555_B_SHIFT;

  // - - - Convert 5-bit to 8-bit, hardware-accurate scaling is (channel * 8) + (channel >> 2)
  u8 R = (r << 3) | (r >> 2);
  u8 G = (g << 3) | (g >> 2);
  u8 B = (b << 3) | (b >> 2);

  return ARGB_ALPHA_OPAQUE | (R << 16) | (G << 8) | B;
}

// - - - Renderer Implementations - - -

static void desktopInit(void) 
{
  // - - - SDL3 uses booleans for success/failure
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) 
  {
    FORGE_LOG_ERROR("SDL_Init Error: %s", SDL_GetError());
    return;
  }

  // - - - Create a 4x scaled window by default
  sdlInternal.window = SDL_CreateWindow(
    "Pocket Pixel Desktop",
    160 * 2, 144 * 2,
    SDL_WINDOW_RESIZABLE
  );

  if (!sdlInternal.window) 
  {
    FORGE_LOG_ERROR("SDL_CreateWindow Error: %s", SDL_GetError());
    return;
  }

  sdlInternal.renderer = SDL_CreateRenderer(sdlInternal.window, NULL);
  if (!sdlInternal.renderer) 
  {
    FORGE_LOG_ERROR("SDL_CreateRenderer Error: %s", SDL_GetError());
    return;
  }

  // - - - Create a texture to act as the Game Boy's LCD screen
  sdlInternal.texture = SDL_CreateTexture(
    sdlInternal.renderer,
    SDL_PIXELFORMAT_ARGB8888,   // - - - 32-bit: Alpha, Red, Green, Blue
    SDL_TEXTUREACCESS_STREAMING,
    160, 144
  );
}

static void desktopRenderFrame(const u32* FRAME_BUFFER, u32 WIDTH, u32 HEIGHT) 
{
  if (!sdlInternal.texture) return;
  (void) HEIGHT;
  
  u32 finalPixels[SCREEN_PIXELS_X * SCREEN_PIXELS_Y];
  bool dmg = (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY);

  for (i32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) 
  {
    if (dmg) 
    {
      // - - - Core gives us 0-3, we map it to our theme
      finalPixels[i] = DMG_THEME[FRAME_BUFFER[i] & 0x03];
    } 
    else 
    {
      finalPixels[i] = colorToRGB32(FRAME_BUFFER[i]);
    }
  }

  // - - - Upload pixels to the GPU
  SDL_UpdateTexture(sdlInternal.texture, NULL, finalPixels, WIDTH * sizeof(u32));

  // - - - Clear and draw
  SDL_RenderClear(sdlInternal.renderer);
  SDL_RenderTexture(sdlInternal.renderer, sdlInternal.texture, NULL, NULL);
  SDL_RenderPresent(sdlInternal.renderer);
}

static void desktopCleanup(void) 
{
  SDL_DestroyTexture(sdlInternal.texture);
  SDL_DestroyRenderer(sdlInternal.renderer);
  SDL_DestroyWindow(sdlInternal.window);
  SDL_Quit();
}


// - - - Platform Public API - - -
PlatformContext* platformGetContext(void) 
{
  return &ctx;
}

void platformInit(void) 
{
  ctx.name = "Desktop (SDL3)";
    
  // - - - Hook up function pointers
  ctx.rendering.init          = desktopInit;
  ctx.rendering.renderFrame   = desktopRenderFrame;
  ctx.rendering.cleanup       = desktopCleanup;

  // - - - Execute the initialization
  ctx.rendering.init();
}

#endif
