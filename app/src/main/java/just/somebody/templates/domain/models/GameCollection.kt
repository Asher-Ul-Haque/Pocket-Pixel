package just.somebody.templates.domain.models

/**
 * Domain model representing a grouping of games.
 */
data class GameCollection(
  val id       : Long = 0,
  val name     : String,
  val isSystem : Boolean = false,
  val games    : List<Game> = emptyList())
