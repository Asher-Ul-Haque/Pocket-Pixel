#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#ifndef __ANDROID__
#include <SFML/Config.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include "ForgeLibrary/include/filesystem.h"
#include "ForgeLibrary/include/logger.h"
#include "defines.h"
#include "GameBoyCore.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/ppu.h"
#include "GameBoy/include/bus.h"
#include "GameBoy/include/cartridge.h"
#include <SFML/Graphics.hpp>

static int scale = 4;
static const u64 tileColors[4] = 
{
  0xFFFFFFFF,
  0xFFAAAAAA,
  0xFF555555,
  0xFF000000
};

sf::RenderWindow debugWindow;
sf::Texture      debugTexture;
sf::Sprite       debugSprite;
sf::Image        debugImage;

void uiInit()
{
  int dbgWidth  = 16 * 8 * scale  + 16 * scale;
  int dbgHeight = 32 * 8 * scale  + 64 * scale;

  debugWindow.create(sf::VideoMode(dbgWidth, dbgHeight), "Debug");
  debugImage.create(dbgWidth, dbgHeight, sf::Color(0x11, 0x11, 0x11));
  debugTexture.create(dbgWidth, dbgHeight);
  debugSprite.setTexture(debugTexture, true);

  debugWindow.setFramerateLimit(0);
  debugWindow.setVerticalSyncEnabled(false);
}

void delay(u32 MS)
{
  sf::sleep(sf::milliseconds(MS));
}

void displayTile(sf::Image& surface, u16 start, u16 tileNum, u32 tileX, u32 tileY)
{
  for (int y = 0; y < 16; y += 2)
  {
    u8 b1 = busRead(start + (tileNum * 16) + y);
    u8 b2 = busRead(start + (tileNum * 16) + y + 1);

    for (int bit = 7; bit >= 0; --bit)
    {
      u8 hi    = !!(b1 & (1 << bit)) << 1;
      u8 lo    = !!(b2 & (1 << bit));
      u8 color = hi | lo;

      u64 argb = tileColors[color];
      sf::Color col(
        (argb >> 16) & 0xFF,
        (argb >> 8)  & 0xFF,
        (argb >> 0)  & 0xFF,
        (argb >> 24) & 0xFF
      );

      int px = tileX + ((7 - bit) * scale);
      int py = tileY + (y / 2 * scale);

      for (int dx = 0; dx < scale; ++dx)
      {
        for (int dy = 0; dy < scale; ++dy)
        {
          if (px + dx < surface.getSize().x && py + dy < surface.getSize().y)
          {
            surface.setPixel(px + dx, py + dy, col);
          }
        }
      }
    }
  }
}

void updateDBGwindow(bool force = false)
{
  static int tickCounter = 0;
  tickCounter++;

  if (tickCounter < 30 && !force)
    return;

  tickCounter = 0;

  u16 addr = 0x8000;
  int xDraw = 0, yDraw = 0, tileNum = 0;

  for (unsigned y = 0; y < debugImage.getSize().y; ++y)
  {
    for (unsigned x = 0; x < debugImage.getSize().x; ++x)
    {
      debugImage.setPixel(x, y, sf::Color(0x11, 0x11, 0x11));
    }
  }

  for (int y = 0; y < 24; ++y)
  {
    for (int x = 0; x < 16; ++x)
    {
      displayTile(debugImage, addr, tileNum, xDraw + (x * scale), yDraw + (y * scale));
      xDraw += (8 * scale);
      tileNum++;
    }
    yDraw += (8 * scale);
    xDraw = 0;
  }

  debugTexture.update(debugImage);
  debugWindow.clear();
  debugWindow.draw(debugSprite);
  debugWindow.display();

  sf::Event event;
  while (debugWindow.pollEvent(event))
  {
    if (event.type == sf::Event::Closed)
    {
      debugWindow.close();
    }
  }
}

int main(int argc, char* argv[])
{
  FORGE_LOG_INFO("Starting LocalBridge...");

  if (argc < 2)
  {
    FORGE_LOG_ERROR("Usage: LocalBridge <path_to_rom>");
    return 1;
  }

  const char* romPath = argv[1];
  FORGE_LOG_DEBUG("Opening File %s", romPath);

  File romFile;
  if (!openFile(romPath, FILE_MODE_READ, true, &romFile))
  {
    FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
    return 1;
  }

  unsigned char* buffer = nullptr;
  unsigned long long fileSize = 0;

  if (!readAllBytes(&romFile, &buffer, &fileSize))
  {
    FORGE_LOG_ERROR("Failed to read ROM file: %s", romPath);
    closeFile(&romFile);
    return 1;
  }

  closeFile(&romFile);

  FORGE_LOG_DEBUG("ROM loaded successfully (%llu bytes)", fileSize);

  cartridgeLoad(reinterpret_cast<u8*>(buffer), fileSize);
  startEmulator();
  uiInit();

  while (debugWindow.isOpen())
  {
    cpuTick();             // Run one tick of emulation
    updateDBGwindow();     // Update the debug window every 30 ticks
  }

  stopEmulator();
  return 0;
}
#endif
