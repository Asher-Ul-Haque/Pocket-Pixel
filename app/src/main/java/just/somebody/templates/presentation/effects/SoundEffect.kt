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

enum class SoundEffect
{ Click }

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

    soundMap[SoundEffect.Click] = soundPool.load(App.appModule.context, R.raw.click, 1);
  }

  suspend fun playSound(EFFECT : SoundEffect)
  { _effects.send(EFFECT) }

  fun play(EFFECT : SoundEffect)
  {
    currentID?.let { soundPool.stop(it) }
    soundMap[EFFECT]?.let()
    { sound -> currentID = soundPool.play(sound, 1f, 1f, 0, 0, 1f) }
  }

  fun release() { soundPool.release() }
}
