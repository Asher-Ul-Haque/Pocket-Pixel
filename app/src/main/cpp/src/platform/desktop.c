#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL3/SDL.h>
#include <platform.h>
#include <utils/asserts.h>

// - - - Internal SDL3 state
static struct 
{
  SDL_Window  *   window;
  SDL_Renderer* renderer;
  SDL_Texture *  texture;
} sdlInternal;

static PlatformContext ctx;


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
    160 * 4, 144 * 4,
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

  // - - - Upload pixels to the GPU
  SDL_UpdateTexture(sdlInternal.texture, NULL, FRAME_BUFFER, WIDTH * sizeof(u32));

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
