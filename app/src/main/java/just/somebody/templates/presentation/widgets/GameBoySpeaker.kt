package just.somebody.templates.presentation.widgets

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.media.AudioTrack.PLAYSTATE_PLAYING
import just.somebody.templates.appModule.ForgeLogger
import android.media.AudioTrack.WRITE_NON_BLOCKING

class GameBoySpeaker {

  private val sampleRate = 44100 // 44100 is standard
  private val channelMask = AudioFormat.CHANNEL_OUT_STEREO
  private val encoding = AudioFormat.ENCODING_PCM_8BIT
  private val frameSize = 2 // stereo: 1 byte per channel

  // This is a safe multiple of the system minimum — adjust if needed
  private val minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelMask, encoding)
  private val bufferSizeInBytes = minBufferSize * 4 // More buffering to prevent underruns

  private val audioTrack: AudioTrack by lazy {
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
      .setBufferSizeInBytes(bufferSizeInBytes)
      .setTransferMode(AudioTrack.MODE_STREAM)
      .build().apply {
        play()
      }
  }

  /**
   * Pushes interleaved 8-bit stereo PCM data to the audio stream.
   */
  fun play(sampleBuffer: ByteArray) {
    if (sampleBuffer.isEmpty()) return

    val written = audioTrack.write(sampleBuffer, 0, sampleBuffer.size, WRITE_NON_BLOCKING)

    if (written < 0) {
      ForgeLogger.error("GameBoySpeaker: Audio write failed with code $written")
    } else if (written < sampleBuffer.size) {
      ForgeLogger.warn("GameBoySpeaker: Partial write — $written / ${sampleBuffer.size} bytes")
    }

    if (audioTrack.playState != PLAYSTATE_PLAYING) {
      audioTrack.play()
      ForgeLogger.info("GameBoySpeaker: Re-started playback.")
    }
  }

  fun release() {
    if (audioTrack.playState == PLAYSTATE_PLAYING) {
      audioTrack.stop()
    }
    audioTrack.release()
  }
}
