package just.somebody.templates.appModule.storage.dataStore

import just.somebody.templates.domain.models.Palette
import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.persistentListOf
import kotlinx.serialization.Serializable

/**
 * Data architecture layer capturing localized user preferences and state machine values.
 *
 * This structure is entirely managed by Proto DataStore to enable safe, transactional,
 * and asynchronous read/write mutations on critical system variables.
 *
 * @property otherThings Persistent container tracking unstructured mathematical states.
 * @property externalUris Mapping descriptor matching unique directory identifiers to structural storage path paths.
 * @property channelVolume Quantized audio channels volume distribution index tracks.
 * @property paletteIndex Tracked selection pointer for the standard default retro visualization overlay.
 * @property shaderIndex Tracked selection pointer mapping active graphical matrix filter structures.
 * @property customPalettes User-configured color arrays added to modify system lookups dynamically.
 * @property gamepadMapping Explicit structural physical device mapping matrix tracking physical interface controls.
 * @property launchCount Running indicator calculation tracking application lifecycle initialization passes.
 * @property hasRated Evaluation status determining if feedback metrics have been fulfilled by the client.
 * @property isImmersiveModeEnabled State toggle controlling system overlay behavior to maximize active view margins.
 */
@Serializable
data class AppSettings
(
	val otherThings     				: PersistentList<Int>     = persistentListOf(),
	val externalUris    				: Map<String, String>     = emptyMap(),
	val channelVolume   				: List<Float>             = persistentListOf(0.6f, 0.6f, 0.6f, 0.6f),
	val paletteIndex    				: Int                     = 0,
	val shaderIndex     				: Int                     = 0,
	val customPalettes  				: List<Palette>           = emptyList(),
	val gamepadMapping  				: GamepadMapping          = GamepadMapping(),
	val launchCount             : Int                     = 0,
	val hasRated                : Boolean                 = false,
	val isImmersiveModeEnabled  : Boolean                 = true)