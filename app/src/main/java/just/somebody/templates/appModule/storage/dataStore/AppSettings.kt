package just.somebody.templates.appModule.storage.dataStore

import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.persistentListOf
import kotlinx.serialization.Serializable

@Serializable
data class AppSettings
(
  val otherThings   : PersistentList<Int>    = persistentListOf(),
  val externalUris  : Map<String, String>    = emptyMap(),
  val channelVolume : List<Float>  = persistentListOf(1f, 1f, 1f, 1f, 1f),
    /*
      index 0 - master volume
      1 - 4 : channel volume
     */
)