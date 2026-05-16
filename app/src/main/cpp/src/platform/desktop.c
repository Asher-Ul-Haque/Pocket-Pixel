#include <SDL3/SDL_rect.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL.h>
#include <platform.h>
#include <utils/asserts.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <SDL3/SDL_video.h>

typedef struct
{
  SDL_Window*   window;
  SDL_Renderer* renderer;
  SDL_Texture*  gameTexture;
  SDL_Texture*  tileTexture;
  SDL_Texture*  mapTexture;

  // - - - physical RGB color lookup tables 
  u32 dmgColors[4];
} SdlInternalContext;

static SdlInternalContext rendererCTX;

static u32 colorConvert555To8888(u16 COLOR_555)
{
  u8 red    = (COLOR_555 & 0x1F);
  u8 green  = ((COLOR_555 >> 5) & 0x1F);
  u8 blue   = ((COLOR_555 >> 10) & 0x1F);

  // - - - scale 5 bit to 8 bit 
  red   = (red << 3) | (red >> 2);
  green = (green << 3) | (green >> 2);
  blue  = (blue << 3) | (blue >> 2);

  return (u32) ((red << 24) | (green << 16) | (blue << 8) | 0xFF);
}

void sdlInit(void)
{
  FORGE_LOG_DEBUG("%s", "Initializing video subsystem");
  FORGE_ASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD));

  // EXPANSION: Expanded width from 1024 to 1300 to comfortably clear the 1234px layout right boundary
  rendererCTX.window = SDL_CreateWindow("Pocket Pixel Debugger", 1300, 720, 0);
  FORGE_ASSERT(rendererCTX.window);

  rendererCTX.renderer = SDL_CreateRenderer(rendererCTX.window, NULL);
  FORGE_ASSERT(rendererCTX.renderer);

  rendererCTX.gameTexture = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
  rendererCTX.tileTexture = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 192);
  rendererCTX.mapTexture  = SDL_CreateTexture(rendererCTX.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
  FORGE_ASSERT(rendererCTX.gameTexture && rendererCTX.tileTexture && rendererCTX.mapTexture);

  rendererCTX.dmgColors[0] = 0x9BBC0FFF;
  rendererCTX.dmgColors[1] = 0x8BAC0FFF;
  rendererCTX.dmgColors[2] = 0x306230FF;
  rendererCTX.dmgColors[3] = 0x0F380FFF;
  
  FORGE_LOG_DEBUG("%s", "Successfully initialized expanded video subsystem");
}

void renderFrame(const PpuFrame* FRAME)
{
  void* pixels;
  i32   pitch;
  if (SDL_LockTexture(rendererCTX.gameTexture, NULL, &pixels, &pitch) != 0) return;

  u32*  dest = (u32*) pixels;
  u8    mode = cartridgeGetContext()->mode;

  for (i32 i = 0; i < WIDTH * HEIGHT; ++i)
  {
    PpuPixel pixel = FRAME->pixels[i];

    if (mode == MODE_DMG_GAMEBOY)
    {
      u8 paletteReg = (pixel.bits.layer == 0) ? 
        FRAME->palettes.dmg.bgp : 
        (
          pixel.bits.paletteId == 0 ?
            FRAME->palettes.dmg.obp0 : 
            FRAME->palettes.dmg.obp1
        );

      u8 phsyicalIndex  = (paletteReg >> (pixel.bits.colorIndex * 2)) & 0x03;
      dest[i]           = rendererCTX.dmgColors[phsyicalIndex];
    }
    else 
    {
      u16 color555 = (pixel.bits.layer == 0) ?
        FRAME->palettes.cgb.bg[(pixel.bits.paletteId * 4)  + pixel.bits.colorIndex] : 
        FRAME->palettes.cgb.obj[(pixel.bits.paletteId * 4) + pixel.bits.colorIndex];

      dest[i] = colorConvert555To8888(color555);
    }
  }

  SDL_UnlockTexture(rendererCTX.gameTexture);
}

void drawTileView(const u8* VRAM_BANK_0, const u8* VRAM_BANK_1)
{
  (void) VRAM_BANK_1;
  void* pixels;
  i32   pitch;
  SDL_LockTexture(rendererCTX.tileTexture, NULL, &pixels, &pitch);
  u32* dest = (u32*) pixels;

  // - - - 384 tiles total (16 x 24)
  for (i32 tileIndex = 0; tileIndex < TILE_COUNT_X * TILE_COUNT_Y; ++tileIndex)
  {
    for (i32 y = 0; y < TILE_SIDE; ++y)
    {
      u8 byte1 = VRAM_BANK_0[tileIndex * (TILE_SIDE * 2) + y * 2];
      u8 byte2 = VRAM_BANK_0[tileIndex * (TILE_SIDE * 2) + y * 2 + 1];

      for (i32 x = 0; x < TILE_SIDE; ++x)
      {
        u8 bit        = 7 - x;
        u8 colorIndex = ((byte2 >> bit) & 0x01) << 1 | ((byte1 >> bit) & 0x01);

        i32 px = (tileIndex % (TILE_SIDE * 2)) * TILE_SIDE + x;
        i32 py = (tileIndex / (TILE_SIDE * 2)) * TILE_SIDE + y;
        dest[py * (TILE_COUNT_X * TILE_SIDE) + px] = rendererCTX.dmgColors[colorIndex];
      }
    }
  }
  SDL_UnlockTexture(rendererCTX.tileTexture);
}

void drawMapView(const u8* VRAM_BANK_0, const u8* VRAM_BANK_1, u8 MAP_SELECT)
{
  void* pixels;
  i32 pitch;
  SDL_LockTexture(rendererCTX.mapTexture, NULL, &pixels, &pitch);
  u32* dest = (u32*) pixels;
  u16 mapOffset = (MAP_SELECT == 0) ? 0x1800 : 0x1C00;

  for (i32 ty = 0; ty < 32; ++ty)
  {
    for (i32 tx = 0; tx < 32; ++tx)
    {
      u8 tileId = VRAM_BANK_0[mapOffset + (ty * 32) + tx];

      for (i32 y = 0; y < 8; ++y)
      {
        u8 b1 = VRAM_BANK_0[tileId * 16 + y * 2];
        u8 b2 = VRAM_BANK_1[tileId * 16 + y * 2 + 1];
        for (i32 x = 0; x < 8; ++x)
        {
          u8 colorIndex = ((b2 >> (7 - x)) & 0x01) << 1 | ((b1 >> (7 - x)) & 0x01);
          dest[((ty * 8 + y) * 256) + (tx * 8 + x)] = rendererCTX.dmgColors[colorIndex];
        }
      }
    }
  }

  SDL_UnlockTexture(rendererCTX.mapTexture);
}

void present(void)
{
  if (!rendererCTX.renderer) return;

  // 1. Clear the main window to a dark graphite background
  SDL_SetRenderDrawColor(rendererCTX.renderer, 25, 25, 25, 255);
  SDL_RenderClear(rendererCTX.renderer);

  // --- Layout Configuration ---
  // Coordinates are structured as: { X, Y, Width, Height }
  SDL_FRect rGame      = { 30,  50, 160 * 3, 144 * 3 }; // 480x432
  SDL_FRect rTile      = { 550, 50, 128 * 2, 192 * 2 }; // 256x384
  SDL_FRect rMap       = { 850,  50, 256 * 1.5f, 256 * 1.5f }; // 384x384

  // Outer framing layout boundaries for label highlights
  SDL_FRect borderGame = { rGame.x - 4, rGame.y - 4, rGame.w + 8, rGame.h + 8 };
  SDL_FRect borderTile = { rTile.x - 4, rTile.y - 4, rTile.w + 8, rTile.h + 8 };
  SDL_FRect borderMap  = { rMap.x  - 4, rMap.y  - 4, rMap.w  + 8, rMap.h  + 8 };

  // 2. Draw distinctive background boxes & borders so viewports stand out
  // Game Screen Area (Blue Highlight Border)
  SDL_SetRenderDrawColor(rendererCTX.renderer, 0, 120, 215, 255);
  SDL_RenderRect(rendererCTX.renderer, &borderGame);
  
  // VRAM Tile Viewer Area (Green Highlight Border)
  SDL_SetRenderDrawColor(rendererCTX.renderer, 0, 200, 115, 255);
  SDL_RenderRect(rendererCTX.renderer, &borderTile);
  
  // VRAM Map Viewer Area (Red Highlight Border)
  SDL_SetRenderDrawColor(rendererCTX.renderer, 230, 70, 70, 255);
  SDL_RenderRect(rendererCTX.renderer, &borderMap);

  // 3. Render the actual generated textures inside their respective borders
  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.gameTexture, NULL, &rGame);
  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.tileTexture, NULL, &rTile);
  SDL_RenderTexture(rendererCTX.renderer, rendererCTX.mapTexture,  NULL, &rMap);

  // 4. Update the screen display
  SDL_RenderPresent(rendererCTX.renderer);
}

void cleanup(void)
{
  SDL_DestroyTexture(rendererCTX.gameTexture);
  SDL_DestroyTexture(rendererCTX.tileTexture);
  SDL_DestroyTexture(rendererCTX.mapTexture);
  SDL_DestroyRenderer(rendererCTX.renderer);
  SDL_DestroyWindow(rendererCTX.window);
  SDL_Quit();
}

static PlatformContext ctx = 
{
  .name       = "Desktop SDL 3 Harness",
  .rendering  = 
  {
    .init         = sdlInit,
    .renderFrame  = renderFrame,
    .drawTileView = drawTileView,
    .drawMapView  = drawMapView, 
    .present      = present,
    .cleanup      = cleanup 
  }
};

PlatformContext* platformGetContext(void) { return &ctx; }

void platformInit(void) { ctx.rendering.init(); }
#endif
