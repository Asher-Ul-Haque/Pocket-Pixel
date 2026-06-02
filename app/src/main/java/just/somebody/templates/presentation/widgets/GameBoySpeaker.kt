package just.somebody.templates.presentation.widgets

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.media.AudioTrack.PLAYSTATE_PLAYING
import just.somebody.templates.appModule.ForgeLogger
import android.media.AudioTrack.WRITE_BLOCKING

/**
 * High-performance audio renderer handling the real-time audio pipeline for the emulator.
 *
 * Configures an uncompressed 32-bit Float stereo [AudioTrack] optimized for low-latency gaming.
 * Includes a localized hardware DC offset mitigation filter (High-Pass Filter) and digital
 * noise gates to eliminate pops, crackles, and static during silent periods.
 */
class GameBoySpeaker
{
  private val sampleRate  : Int = 44100 // 44100 is standard
  private val channelMask : Int = AudioFormat.CHANNEL_OUT_STEREO
  private val encoding    : Int = AudioFormat.ENCODING_PCM_FLOAT
  private val frameSize   : Int = 8 // stereo: 4 bytes per channel (float)

  private val minBufferSize     : Int = AudioTrack.getMinBufferSize(sampleRate, channelMask, encoding)
  private val safeMinBufferSize : Int = if (minBufferSize > 0) minBufferSize else (sampleRate * frameSize / 50)
  private val bufferSizeInBytes : Int = safeMinBufferSize * 2

  // - - - Filter state for DC offset removal (High Pass Filter)
  private var lastSampleL : Float = 0f
  private var lastSampleR : Float = 0f
  private var filteredL   : Float = 0f
  private var filteredR   : Float = 0f
  private val hpfCoeff    : Float = 0.995f

  private var audioTrack: AudioTrack? = null

  @Synchronized
  private fun ensureAudioTrack(): AudioTrack
  {
    val existing = audioTrack
    if (existing != null) return existing

    val created = AudioTrack.Builder()
      .setAudioAttributes(
        AudioAttributes.Builder()
          .setUsage(AudioAttributes.USAGE_GAME)
          .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION) // Optimized for emulator/system sounds
          .build())
      .setAudioFormat(
        AudioFormat.Builder()
          .setEncoding(encoding)
          .setSampleRate(sampleRate)
          .setChannelMask(channelMask)
          .build())
      .setBufferSizeInBytes(bufferSizeInBytes)
      .setTransferMode(AudioTrack.MODE_STREAM)
      .apply()
      {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O)
        { setPerformanceMode(AudioTrack.PERFORMANCE_MODE_LOW_LATENCY) }
      }
      .build().apply { play() }

    audioTrack = created
    return created
  }

  fun start()
  {
    val track = ensureAudioTrack()
    if (track.playState != PLAYSTATE_PLAYING)
    { track.play() }
  }

  /**
   * Pushes interleaved 32-bit float stereo PCM data to the audio stream.
   *
   * Uses [WRITE_BLOCKING] to synchronize the emulation rendering thread with the audio hardware clock.
   * Applies a one-pole High-Pass Filter to eliminate constant voltage offsets that cause static/pops,
   * along with a safety threshold gate for zeroing out idle system noise.
   *
   * @param SAMPLE_BUFFER Linear array containing alternating left and right stereo audio float samples.
   */
  fun play(SAMPLE_BUFFER: FloatArray)
  {
    if (SAMPLE_BUFFER.isEmpty()) return

    val track = ensureAudioTrack()

    // - - - 1. DC OFFSET REMOVAL & SILENCE GATE
    // - - - apply a one-pole High-Pass Filter to eliminate constant offsets that cause static/pops.
    var peakAbs : Float = 0f
    for (i in SAMPLE_BUFFER.indices step 2)
    {
      // - - - Left Channel
      val rawL : Float = SAMPLE_BUFFER[i]
      filteredL   = rawL - lastSampleL + (hpfCoeff * filteredL)
      lastSampleL = rawL
      SAMPLE_BUFFER[i] = filteredL.coerceIn(-1.0f, 1.0f)

      // - - - Right Channel
      val rawR : Float = SAMPLE_BUFFER[i + 1]
      filteredR   = rawR - lastSampleR + (hpfCoeff * filteredR)
      lastSampleR = rawR
      SAMPLE_BUFFER[i + 1] = filteredR.coerceIn(-1.0f, 1.0f)

      peakAbs = Math.max(peakAbs, Math.max(Math.abs(SAMPLE_BUFFER[i]), Math.abs(SAMPLE_BUFFER[i + 1])))
    }

    // - - - 2. SILENCE GATE: If the entire buffer is effectively silent, force pure zeros.
    if (peakAbs < 1e-4f) { SAMPLE_BUFFER.fill(0f) }

    // - - - 3. SYNCHRONIZED WRITE
    val written = track.write(SAMPLE_BUFFER, 0, SAMPLE_BUFFER.size, WRITE_BLOCKING)

    if (written < 0)
    { ForgeLogger.error("GameBoySpeaker: Audio write failed with code $written") }

    if (track.playState != PLAYSTATE_PLAYING)
    { track.play() }
  }

  /**
   * Tears down the media pipeline.
   * * Stops active hardware sound rendering and releases the allocated [AudioTrack] instance
   * safely to free up system audio mixers.
   */
  fun release()
  {
    val track = audioTrack ?: return
    if (track.playState == PLAYSTATE_PLAYING)
    { track.stop() }
    track.release()
    audioTrack = null
    lastSampleL = 0f
    lastSampleR = 0f
    filteredL = 0f
    filteredR = 0f
  }
}