package org.just_somebody.pocket_pixel.emulatorScreen.domain

class AndroidGameBoy : GameBoy()
{
    // - - - memory
    private val frameBuffer = ByteArray(160 * 144 / 4)  // - - - 2 bits per pixel
    private val audioBuffer = ByteArray(1024)           // - - - TODO : find out how audio on gameboy works
    private val romData     = ByteArray(1024 * 1024)    // - - - 1MB ROM size
    private val ramData     = ByteArray(1024 * 8)       // - - - 8KB RAM

    // - - - emulation
    override fun loadROM        (ROM : ByteArray)   {   nativeLoadROM(ROM);  }
    override fun startEmulator  ()                  { nativeStartEmulator(); }
    override fun stopEmulator   ()                  {  nativeStopEmulator(); }
    override fun stepFrame      ()                  {   nativeStepFrame();   }
    override fun resetEmulator  ()
    {
        nativeStopEmulator();
        nativeStartEmulator();
    }

    // - - - video and audio
    override fun getFrameBuffer () : ByteArray
    {
        nativeGetFrameBuffer(frameBuffer)
        return frameBuffer
    }
    override fun getAudioBuffer () : ByteArray
    {
        nativeGetAudioBuffer(audioBuffer)
        return audioBuffer
    }

    // - - - input
    override fun sendButton(
        BUTTON      : Buttons,
        IS_PRESSED  : Boolean)
    { nativeSetButtonState(BUTTON.ordinal, IS_PRESSED); }


    // - - - Native - - -

    private external fun nativeStepFrame()
    private external fun nativeGetFrameBuffer(FRAME_BUFFER: ByteArray)
    private external fun nativeGetAudioBuffer(AUDIO_BUFFER: ByteArray)
    private external fun nativeLoadROM(ROM: ByteArray)
    private external fun nativeSetButtonState(button: Int, pressed: Boolean)
    private external fun nativeStartEmulator();
    private external fun nativeStopEmulator();

    // - - - actual c++
    companion object { init { System.loadLibrary("pocketPixel") } }
}

actual fun getGameBoy(): GameBoy = AndroidGameBoy()
