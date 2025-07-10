package just.somebody.templates.domain.models

data class Game(
  val id              : Long = 0,
  val title           : String,
  val publisher       : String,
  val romUri          : String,
  val batterySavePath : String?,
  val lastPlayed      : Long?,
  val isFavorite      : Boolean
)