package just.somebody.templates.appModule.storage.dataStore

import just.somebody.templates.domain.models.Palette
import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.persistentListOf
import kotlinx.serialization.Serializable

@Serializable
data class AppSettings
(
  val otherThings     : PersistentList<Int>     = persistentListOf(),
  val externalUris    : Map<String, String>     = emptyMap(),
  val channelVolume   : List<Float>             = persistentListOf(0.75f, 0.6f, 0.6f, 0.6f, 0.6f),
  val paletteIndex    : Int                     = 0,
  val shaderIndex     : Int                     = 0,
  val customPalettes  : List<Palette>           = emptyList(),
  val gamepadMapping  : GamepadMapping          = GamepadMapping()
)