package just.somebody.templates.presentation.widgets

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.media.AudioTrack.PLAYSTATE_PLAYING
import just.somebody.templates.appModule.ForgeLogger

class GameBoySpeaker
{
  private val sampleRate  = 44100
  private val channelMask = AudioFormat.CHANNEL_OUT_STEREO
  private val encoding    = AudioFormat.ENCODING_PCM_8BIT

  private val audioTrack : AudioTrack by lazy ()
  {
    AudioTrack.Builder()
      .setAudioAttributes(
        AudioAttributes.Builder()
          .setUsage(AudioAttributes.USAGE_GAME)
          .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
          .build()
      )
      .setAudioFormat(
        AudioFormat.Builder()
          .setEncoding(encoding)
          .setSampleRate(sampleRate)
          .setChannelMask(channelMask)
          .build()
      )
      .setBufferSizeInBytes(AudioTrack.getMinBufferSize(sampleRate, channelMask, encoding))
      .setTransferMode(AudioTrack.MODE_STREAM)
      .build().apply ()
      {
        if (playState != PLAYSTATE_PLAYING) play()
      }
  }

  fun play(SAMPLE_BUFFER : ByteArray)
  {
    val bytesWritten = audioTrack.write(SAMPLE_BUFFER, 0, SAMPLE_BUFFER.size, AudioTrack.WRITE_NON_BLOCKING)
    if (bytesWritten < 0)
    { ForgeLogger.error("Game boy speaker : error writing audio data. $bytesWritten") }
    else if (bytesWritten != SAMPLE_BUFFER.size){}
    //{ ForgeLogger.warn("Game boy speaker : partial write only") }

    if (audioTrack.playState != PLAYSTATE_PLAYING)
    {
      audioTrack.play()
      ForgeLogger.info("Game boy speaker : AudioTrack started playing.")
    }
  }

  fun release()
  {
    if (audioTrack.playState == PLAYSTATE_PLAYING) audioTrack.stop()
  }
}