#ifndef __ANDROID__
#include <SFML/Audio/SoundBuffer.hpp>
#include "GameBoy/include/ppu.h"
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
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
#include "GameBoy/include/bus.h"
#include "GameBoy/include/cartridge.h"
#include "GameBoy/include/apu.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>

// --- Global variable to store the current ROM's path for save files ---
static std::string currentRomPath;

// --- Forward declarations for platform-specific file I/O functions ---
bool desktopSaveRamToFile(const u8* ram_data, u32 ram_size);
bool desktopLoadRamFromFile(u8* ram_data_buffer, u32 buffer_size);
u32  desktopGetExpectedSaveSize();

// --- Instance of CartridgeFileIO for desktop platform ---
CartridgeFileIO desktopFileIO = 
{
    .saveRamToFile       = desktopSaveRamToFile,
    .loadRamFromFile     = desktopLoadRamFromFile,
    .getExpectedSaveSize = desktopGetExpectedSaveSize
};

static int scale = 4;
static const u64 tileColors[4] = 
{
    0xFF0F380F, // dark green
    0xFF206230, // medium green
    0xFF8BAC0F, // light green 
    0xFF9BBC0F  // pale green
};

sf::RenderWindow debugWindow;
sf::Texture      debugTexture;
sf::Sprite       debugSprite;
sf::Image        debugImage;
sf::RenderWindow mainWindow;
sf::Texture      mainTexture;
sf::Sprite       mainSprite;
sf::Image        mainImage;

// --- Helper to get save file path from current ROM path ---
static std::string getSaveFilePath(const std::string& rom_filepath) {
    std::string save_path = rom_filepath;
    size_t dot_pos = save_path.rfind('.');
    if (dot_pos != std::string::npos) {
        save_path = save_path.substr(0, dot_pos);
    }
    save_path += ".sav";
    return save_path;
}

// --- Updated implementations using currentRomPath ---

bool desktopSaveRamToFile(const u8* ram_data, u32 ram_size) {
    std::string save_path = getSaveFilePath(currentRomPath);
    File saveFile;
    if (!openFile(save_path.c_str(), FILE_MODE_WRITE, true, &saveFile)) {
        FORGE_LOG_ERROR("Desktop: Could not open save file for writing: %s", save_path.c_str());
        return false;
    }

    unsigned long long written_bytes = 0;
    bool success = writeFile(&saveFile, ram_size, ram_data, &written_bytes);
    closeFile(&saveFile);

    if (!success || written_bytes != ram_size) {
        FORGE_LOG_ERROR("Desktop: Failed to write all RAM data to file. Wrote %llu of %u bytes.", written_bytes, ram_size);
        return false;
    }

    FORGE_LOG_INFO("Desktop: Saved %u bytes to %s", ram_size, save_path.c_str());
    return true;
}

bool desktopLoadRamFromFile(u8* ram_data_buffer, u32 buffer_size) {
    std::string save_path = getSaveFilePath(currentRomPath);
    File saveFile;

    if (!fileExists(save_path.c_str())) {
        FORGE_LOG_INFO("Desktop: No save file found for %s. Starting with fresh RAM.", save_path.c_str());
        memset(ram_data_buffer, 0, buffer_size);
        return false;
    }

    if (!openFile(save_path.c_str(), FILE_MODE_READ, true, &saveFile)) {
        FORGE_LOG_ERROR("Desktop: Could not open save file for reading: %s", save_path.c_str());
        memset(ram_data_buffer, 0, buffer_size);
        return false;
    }

    unsigned long long file_size_on_disk = getFileSize(&saveFile);
    if (file_size_on_disk != buffer_size) {
        FORGE_LOG_WARNING("Desktop: Save file size mismatch for %s (expected: %u, got: %llu).", save_path.c_str(), buffer_size, file_size_on_disk);
        closeFile(&saveFile);
        memset(ram_data_buffer, 0, buffer_size);
        return false;
    }

    unsigned long long read_bytes = 0;
    bool success = readFile(&saveFile, buffer_size, ram_data_buffer, &read_bytes);
    closeFile(&saveFile);

    if (!success || read_bytes != buffer_size) {
        FORGE_LOG_ERROR("Desktop: Failed to read all RAM data from file. Read %llu of %u bytes.", read_bytes, buffer_size);
        memset(ram_data_buffer, 0, buffer_size);
        return false;
    }

    FORGE_LOG_INFO("Desktop: Loaded %u bytes from %s", buffer_size, save_path.c_str());
    return true;
}

u32 desktopGetExpectedSaveSize() {
    return 0; // Placeholder
}

// --- UI, Display, Audio, and Event Handling ---

void playAudio() {
    static sf::Sound sound;
    static sf::SoundBuffer buffer;
    static std::vector<sf::Int16> convertedSamples;
    static bool initialized = false;

    APUcontext* ctx = apuGetContext();
    u32 num_8bit_samples = ctx->bufferPtr;

    if (num_8bit_samples == 0) return;

    if (convertedSamples.size() < num_8bit_samples) {
        convertedSamples.resize(num_8bit_samples);
    }

    for (u32 i = 0; i < num_8bit_samples; ++i) {
        convertedSamples[i] = static_cast<sf::Int16>((static_cast<int>(ctx->sampleBuffer[i]) - 128) * 256);
    }

    u32 num_frames = num_8bit_samples / 2;

    if (!initialized) {
        if (!buffer.loadFromSamples(convertedSamples.data(), num_frames, 2, 44100)) {
            FORGE_LOG_ERROR("SFML: Failed to load sound buffer from samples!");
            return;
        }
        sound.setBuffer(buffer);
        sound.setLoop(false);
        sound.play();
        initialized = true;
    } else {
        if (sound.getStatus() == sf::Sound::Playing) {
            sound.stop();
        }

        if (!buffer.loadFromSamples(convertedSamples.data(), num_frames, 2, 44100)) {
            FORGE_LOG_ERROR("SFML: Failed to load sound buffer from samples on update!");
            return;
        }
        sound.setBuffer(buffer);
        sound.play();
    }
}

void uiInit() {
    int dbgWidth  = 16 * 8 * scale  + 16 * scale;
    int dbgHeight = 32 * 8 * scale  + 64 * scale;

    debugWindow.create(sf::VideoMode(dbgWidth, dbgHeight), "Debug");
    debugImage.create(dbgWidth, dbgHeight, sf::Color(0x11, 0x11, 0x11));
    debugTexture.create(dbgWidth, dbgHeight);
    debugSprite.setTexture(debugTexture, true);

    debugWindow.setFramerateLimit(0);
    debugWindow.setVerticalSyncEnabled(false);

    int mainWidth  = 160 * scale;
    int mainHeight = 144 * scale;
    mainWindow.create(sf::VideoMode(mainWidth, mainHeight), "Emulator");
    mainImage.create(mainWidth, mainHeight, sf::Color(0x11, 0x11, 0x11));
    mainTexture.create(mainWidth, mainHeight);
    mainSprite.setTexture(mainTexture, true);

    mainWindow.setFramerateLimit(0);
    mainWindow.setVerticalSyncEnabled(false);
}

void delay(u32 MS) {
    sf::sleep(sf::milliseconds(MS));
}

void displayTile(sf::Image& surface, u16 start, u16 tileNum, u32 tileX, u32 tileY) {
    for (int y = 0; y < 16; y += 2) {
        u8 b1 = busRead(start + (tileNum * 16) + y);
        u8 b2 = busRead(start + (tileNum * 16) + y + 1);

        for (int bit = 7; bit >= 0; --bit) {
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

            for (int dx = 0; dx < scale; ++dx) {
                for (int dy = 0; dy < scale; ++dy) {
                    if (px + dx < surface.getSize().x && py + dy < surface.getSize().y) {
                        surface.setPixel(px + dx, py + dy, col);
                    }
                }
            }
        }
    }
}

void updateWindows(bool force = false) {
    static int prevFrame = 0;
    if (prevFrame == ppuGetContext()->currentFrame) return;
    prevFrame = ppuGetContext()->currentFrame;

    for (unsigned y = 0; y < debugImage.getSize().y; ++y) {
        for (unsigned x = 0; x < debugImage.getSize().x; ++x) {
            debugImage.setPixel(x, y, sf::Color(0x11, 0x11, 0x11));
        }
    }

    u16 addr = 0x8000;
    int xDraw = 0, yDraw = 0, tileNum = 0;

    for (int y = 0; y < 24; ++y) {
        for (int x = 0; x < 16; ++x) {
            displayTile(debugImage, addr, tileNum, xDraw + (x * scale), yDraw + (y * scale));
            xDraw += (8 * scale);
            tileNum++;
        }
        yDraw += (8 * scale);
        xDraw = 0;
    }

    u32* fb = ppuGetContext()->frameBuffer;
    for (int y = 0; y < 144; ++y) {
        for (int x = 0; x < 160; ++x) {
            u32 color = fb[x + y * 160];
            sf::Color col(
                (color >> 16) & 0xFF,
                (color >> 8)  & 0xFF,
                (color >> 0)  & 0xFF,
                (color >> 24) & 0xFF
            );

            for (int dx = 0; dx < scale; ++dx) {
                for (int dy = 0; dy < scale; ++dy) {
                    mainImage.setPixel(x * scale + dx, y * scale + dy, col);
                }
            }
        }
    }

    mainTexture.update(mainImage);
    mainWindow.clear();
    mainWindow.draw(mainSprite);
    mainWindow.display();

    debugTexture.update(debugImage);
    debugWindow.clear();
    debugWindow.draw(debugSprite);
    debugWindow.display();

    sf::Event event;
    while (debugWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            debugWindow.close();
        }
    }

    while (mainWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            mainWindow.close();
        } else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::A      : setButton(A,      true); break;
                case sf::Keyboard::B      : setButton(B,      true); break;
                case sf::Keyboard::K      : setButton(START,  true); break;
                case sf::Keyboard::Enter  : setButton(SELECT, true); break;
                case sf::Keyboard::Down   : setButton(DOWN,   true); break;
                case sf::Keyboard::Left   : setButton(LEFT,   true); break;
                case sf::Keyboard::Right  : setButton(RIGHT,  true); break;
                case sf::Keyboard::Up     : setButton(UP,     true); break;
                case sf::Keyboard::F     : setButton(DOUBLE_SPEED,     true); break;
                default: break;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            switch (event.key.code) {
                case sf::Keyboard::A      : setButton(A,      false); break;
                case sf::Keyboard::B      : setButton(B,      false); break;
                case sf::Keyboard::K      : setButton(START,  false); break;
                case sf::Keyboard::Enter  : setButton(SELECT, false); break;
                case sf::Keyboard::Down   : setButton(DOWN,   false); break;
                case sf::Keyboard::Left   : setButton(LEFT,   false); break;
                case sf::Keyboard::Right  : setButton(RIGHT,  false); break;
                case sf::Keyboard::Up     : setButton(UP,     false); break;
                case sf::Keyboard::F     : setButton(DOUBLE_SPEED,     false); break;
                default: break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    FORGE_LOG_INFO("Starting LocalBridge...");

    if (argc < 2) {
        FORGE_LOG_ERROR("Usage: LocalBridge <path_to_rom>");
        return 1;
    }

    const char* romPath = argv[1];
    currentRomPath = romPath;

    File romFile;
    if (!openFile(romPath, FILE_MODE_READ, true, &romFile)) {
        FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
        return 1;
    }

    unsigned char* buffer = nullptr;
    unsigned long long fileSize = 0;

    if (!readAllBytes(&romFile, &buffer, &fileSize)) {
        FORGE_LOG_ERROR("Failed to read ROM file: %s", romPath);
        closeFile(&romFile);
        return 1;
    }
    closeFile(&romFile);

    cartridgeLoad(reinterpret_cast<u8*>(buffer), fileSize, &desktopFileIO);
    startEmulator();
    uiInit();

    while (debugWindow.isOpen() && mainWindow.isOpen()) {
        cpuTick();
        updateWindows();
        cartridgeTickRTC();
    }

    stopEmulator();
    cartridgeFlushRAM();

    if (buffer) {
        free(buffer);
    }

    return 0;
}
#endif
