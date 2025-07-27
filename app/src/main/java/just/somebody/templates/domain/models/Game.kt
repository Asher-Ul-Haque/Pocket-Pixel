package just.somebody.templates.domain.models

data class Game(
  val id              : Long = 0,
  val title           : String,
  val romUri          : String,
  val lastPlayed      : Long?,
  val isFavorite      : Boolean
)