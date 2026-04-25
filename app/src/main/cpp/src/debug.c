#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <ppu/ppuRegisters.h>
#include <cartridge/cartridge.h>
#include <utils/logger.h>

/**
 * @file debug.c
 * @brief Debug visualization overlay for emulator diagnostics.
 * Draws tile grids, sprite bounding boxes, and other useful debugging info.
 */

// Debug mode flags
static bool debugTileGridEnabled = false;  // Toggle with 'G' key
static bool debugSpritesEnabled = false;    // Toggle with 'S' key
static bool debugWindowEnabled = false;     // Toggle with 'W' key

void debugToggleTileGrid(void)
{
  debugTileGridEnabled = !debugTileGridEnabled;
}

void debugToggleSprites(void)
{
  debugSpritesEnabled = !debugSpritesEnabled;
}

void debugToggleWindow(void)
{
  debugWindowEnabled = !debugWindowEnabled;
}

bool debugIsTileGridEnabled(void) { return debugTileGridEnabled; }
bool debugIsSpritesEnabled(void) { return debugSpritesEnabled; }
bool debugIsWindowEnabled(void) { return debugWindowEnabled; }

/**
 * @brief Draw a single-pixel wide line on the frame buffer.
 * Simple 2D line drawing for overlay visualization.
 */
static void debugDrawHorizontalLine(u32* frameBuffer, u32 y, u32 x1, u32 x2, u32 color)
{
  if (y >= SCREEN_HEIGHT) return;
  for (u32 x = x1; x < x2 && x < SCREEN_WIDTH; x++)
  {
    frameBuffer[y * SCREEN_WIDTH + x] = color;
  }
}

static void debugDrawVerticalLine(u32* frameBuffer, u32 x, u32 y1, u32 y2, u32 color)
{
  if (x >= SCREEN_WIDTH) return;
  for (u32 y = y1; y < y2 && y < SCREEN_HEIGHT; y++)
  {
    frameBuffer[y * SCREEN_WIDTH + x] = color;
  }
}

/**
 * @brief Overlay tile grid on frame buffer.
 * Shows 8x8 tile boundaries to verify tile fetching and alignment.
 */
void debugOverlayTileGrid(u32* frameBuffer)
{
  if (!debugTileGridEnabled) return;

  // Grid color: semi-transparent green
  u32 gridColor = 0xFF00FF00;  // ARGB: fully opaque green

  // Draw vertical lines every 8 pixels
  for (u32 x = 0; x < SCREEN_WIDTH; x += 8)
  {
    debugDrawVerticalLine(frameBuffer, x, 0, SCREEN_HEIGHT, gridColor);
  }

  // Draw horizontal lines every 8 pixels
  for (u32 y = 0; y < SCREEN_HEIGHT; y += 8)
  {
    debugDrawHorizontalLine(frameBuffer, y, 0, SCREEN_WIDTH, gridColor);
  }
}

/**
 * @brief Overlay sprite bounding boxes.
 * Shows where sprites are rendered relative to their OAM position.
 */
void debugOverlaySprites(u32* frameBuffer)
{
  if (!debugSpritesEnabled) return;

  PpuContext* ppu = ppuGetContext();

  // Sprite color: semi-transparent red
  u32 spriteColor = 0xFFFF0000;  // ARGB: fully opaque red

  for (u32 i = 0; i < 40; i++)
  {
    u8* oamEntry = &ppu->oam[i * 4];
    u8 spriteY = oamEntry[0];
    u8 spriteX = oamEntry[1];

    // Convert OAM coordinates to screen coordinates
    i32 screenY = (i32)spriteY - SPRITE_Y_REG_BIAS;
    i32 screenX = (i32)spriteX - SPRITE_X_REG_BIAS;

    // Determine sprite height (8x8 or 8x16)
    bool isLcdc8x16 = (ppu->lcdc & 0x04) != 0;
    u32 spriteHeight = isLcdc8x16 ? 16 : 8;

    // Draw bounding box if sprite is visible on screen
    if (screenX >= -8 && screenX < SCREEN_WIDTH && screenY >= -16 && screenY < SCREEN_HEIGHT)
    {
      // Top edge
      if (screenY >= 0)
        debugDrawHorizontalLine(frameBuffer, screenY, 
                                screenX >= 0 ? screenX : 0, 
                                screenX + 8 < SCREEN_WIDTH ? screenX + 8 : SCREEN_WIDTH, 
                                spriteColor);

      // Bottom edge
      i32 bottomY = screenY + spriteHeight;
      if (bottomY < SCREEN_HEIGHT && bottomY >= 0)
        debugDrawHorizontalLine(frameBuffer, bottomY, 
                                screenX >= 0 ? screenX : 0, 
                                screenX + 8 < SCREEN_WIDTH ? screenX + 8 : SCREEN_WIDTH, 
                                spriteColor);

      // Left edge
      if (screenX >= 0)
        debugDrawVerticalLine(frameBuffer, screenX, 
                              screenY >= 0 ? screenY : 0, 
                              bottomY < SCREEN_HEIGHT ? bottomY : SCREEN_HEIGHT, 
                              spriteColor);

      // Right edge
      i32 rightX = screenX + 8;
      if (rightX < SCREEN_WIDTH && rightX >= 0)
        debugDrawVerticalLine(frameBuffer, rightX, 
                              screenY >= 0 ? screenY : 0, 
                              bottomY < SCREEN_HEIGHT ? bottomY : SCREEN_HEIGHT, 
                              spriteColor);
    }
  }
}

/**
 * @brief Overlay window region.
 * Highlights the area where the window layer is active.
 */
void debugOverlayWindow(u32* frameBuffer)
{
  if (!debugWindowEnabled) return;

  PpuContext* ppu = ppuGetContext();

  if (!LCDC_WIN_ENABLED(ppu)) return;

  // Window region color: semi-transparent blue
  u32 windowColor = 0xFF0000FF;  // ARGB: fully opaque blue

  u8 wx = ppu->wx;
  u8 wy = ppu->wy;

  // Window X has 7-pixel bias
  i32 screenWx = (i32)wx - WINDOW_X_REG_BIAS;

  // Draw window boundaries
  if (screenWx >= 0 && screenWx < SCREEN_WIDTH)
  {
    // Vertical line at window left edge
    debugDrawVerticalLine(frameBuffer, screenWx, 
                         wy < SCREEN_HEIGHT ? wy : 0, 
                         SCREEN_HEIGHT, 
                         windowColor);
  }

  if (wy < SCREEN_HEIGHT)
  {
    // Horizontal line at window top edge
    debugDrawHorizontalLine(frameBuffer, wy, 
                           screenWx >= 0 ? screenWx : 0, 
                           SCREEN_WIDTH, 
                           windowColor);
  }
}

/**
 * @brief Apply all active debug overlays to frame buffer.
 * Call this before rendering the frame.
 */
void debugApplyOverlays(u32* frameBuffer)
{
  debugOverlayTileGrid(frameBuffer);
  debugOverlaySprites(frameBuffer);
  debugOverlayWindow(frameBuffer);
}
