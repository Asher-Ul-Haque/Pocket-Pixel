package just.somebody.templates.domain.models

/**
 * Pure domain-layer model representing a structured game entry.
 *
 * This object remains completely decoupled from database mechanics or external data schemas,
 * acting as the primary source of truth across repositories, ViewModels, and UI presentation components.
 *
 * @property id Unique core mathematical reference identifier tracking this asset profile.
 * @property title The clean display name generated for UI list presentations.
 * @property romUri Platform Storage Access Framework system file path link pointing to the software binary.
 * @property lastPlayed Unix epoch timestamp identifying when this emulation model was last instantiated.
 * @property isFavorite Relational binary flag prioritizing this asset across sorting and filtering trees.
 * @property boxArtUrl Resolved remote location pointing to promotional cover art assets on external servers.
 */
data class Game(
	val id              : Long = 0,
	val title           : String,
	val romUri          : String,
	val lastPlayed      : Long?,
	val playTime        : Long = 0,
	val isFavorite      : Boolean,
	val boxArtUrl       : String? = null)