package just.somebody.templates.appModule.storage.dataStore

import just.somebody.templates.domain.models.Palette
import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.persistentListOf
import kotlinx.serialization.Serializable

/**
 * Data architecture layer capturing localized user preferences and state machine values.
 */
@Serializable
data class AppSettings
(
	val otherThings     				: PersistentList<Int>     = persistentListOf(),
	val externalUris    				: Map<String, String>     = emptyMap(),
	val channelVolume   				: List<Float>             = persistentListOf(0.6f, 0.6f, 0.6f, 0.3f),
	val paletteIndex    				: Int                     = 0,
	val shaderIndex     				: Int                     = 4,
	val customPalettes  				: List<Palette>           = emptyList(),
	val gamepadMapping  				: GamepadMapping          = GamepadMapping(),
	val launchCount             : Int                     = 0,
	val hasRated                : Boolean                 = false,
	val isImmersiveModeEnabled  : Boolean                 = true,
	val isDeferredSavingEnabled : Boolean                 = false,
	val raUsername              : String                  = "",
	val raToken                 : String                  = "",
	val isRaHardcoreEnabled     : Boolean                 = false,
	val assetUrlMapping         : Map<String, String>     = emptyMap())
