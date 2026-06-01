package just.somebody.templates.presentation.effects

import android.content.Context
import android.media.AudioAttributes
import android.media.SoundPool
import androidx.compose.runtime.Composable
import just.somebody.templates.App
import just.somebody.templates.R
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.receiveAsFlow

/**
 * Structural categorization cataloging the supported low-latency systemic acoustic elements
 * mapped within the application workspace environment.
 */
enum class SoundEffect
{
  /** Quick crisp mechanical validation chirp mapping physical or digital keypad interactions. */
  Click,

  /** Dynamic initialization wave chime triggered when moving past introductory layout screens. */
  Splash
}

/**
 * Central hardware audio routing broker wrapping Android's underlying native [SoundPool] API layer.
 * Coordinates swift, concurrent, low-latency audio resource translations while isolating media allocation streams
 * directly away from primary presentation lifecycle blocks.
 */
object SoundController
{
  private val soundPool : SoundPool
  private val soundMap  : MutableMap<SoundEffect, Int> = mutableMapOf()
  private val _effects  : Channel<SoundEffect>         = Channel<SoundEffect>(Channel.UNLIMITED)
  private var currentID : Int?                         = null
  public  val effects   : Flow<SoundEffect>            = _effects.receiveAsFlow()

  init
  {
    val audioAttributes = AudioAttributes.Builder()
      .setUsage(AudioAttributes.USAGE_GAME)
      .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
      .build()

    soundPool = SoundPool.Builder()
      .setMaxStreams(5)
      .setAudioAttributes(audioAttributes)
      .build()

    soundMap[SoundEffect.Click]  = soundPool.load(App.appModule.context, R.raw.click, 1);
    soundMap[SoundEffect.Splash] = soundPool.load(App.appModule.context, R.raw.splash, 1);
  }

  /**
   * Commits an explicit playback task payload into the asynchronous streaming queue pipeline.
   *
   * @param EFFECT Structured categorization identifier mapping the requested acoustic asset.
   */
  suspend fun playSound(EFFECT : SoundEffect)
  { _effects.send(EFFECT) }

  /**
   * Triggers explicit audio playback waveforms instantly over native system mixing channels.
   * Intercepts ongoing executions of previous samples to prevent chaotic sonic crowding mutations.
   *
   * @param EFFECT Structured categorization identifier mapping the requested acoustic asset.
   */
  fun play(EFFECT : SoundEffect)
  {
    currentID?.let { soundPool.stop(it) }
    soundMap[EFFECT]?.let()
    { sound -> currentID = soundPool.play(sound, 1f, 1f, 0, 0, 1f) }
  }

  /** Unloads and deallocates active audio hardware cache layers, returning structural blocks cleanly back to the OS. */
  fun release() { soundPool.release() }
}