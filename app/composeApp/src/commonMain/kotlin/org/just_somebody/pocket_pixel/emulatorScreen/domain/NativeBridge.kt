package org.just_somebody.pocket_pixel.emulatorScreen.domain

abstract class GameBoy
{
    // - - - emulation
    abstract fun loadROM        (ROM : ByteArray);
    abstract fun startEmulator  ();
    abstract fun stopEmulator   ();
    abstract fun resetEmulator  ();
    abstract fun stepFrame      ();

    // - - - video and audio
    abstract fun getFrameBuffer () : ByteArray;
    abstract fun getAudioBuffer () : ByteArray;

    // - - - input
    enum class Buttons
    {
        BUTTON_A,
        BUTTON_B,
        BUTTON_START,
        BUTTON_SELECT,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_TOP_LEFT,
        BUTTON_TOP_RIGHT,
        BUTTON_BOTTOM_RIGHT,
        BUTTON_BOTTOM_LEFT
    }
    abstract fun sendButton(BUTTON : Buttons, IS_PRESSED : Boolean);
}

expect fun getGameBoy() : GameBoy