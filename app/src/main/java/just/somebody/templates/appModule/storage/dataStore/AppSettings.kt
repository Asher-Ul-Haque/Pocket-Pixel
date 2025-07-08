package just.somebody.templates.appModule.storage.dataStore

import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.persistentListOf
import kotlinx.serialization.Serializable

@Serializable
data class AppSettings
  (
  val something    : Int                 = 0,
  val otherThings  : PersistentList<Int> = persistentListOf(),
  val externalUris : Map<String, String> = emptyMap()
)