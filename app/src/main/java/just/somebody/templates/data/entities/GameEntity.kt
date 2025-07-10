package just.somebody.templates.data.entities

import androidx.room.Entity
import androidx.room.Index
import androidx.room.PrimaryKey

@Entity(
  tableName = "games",
  indices   =
    [
      Index("romUri"),
      Index("lastPlayed"),
      Index("isFavorite")
    ]
)
data class GameEntity(
  @PrimaryKey(autoGenerate = true)
  val id          : Long = 0,
  val title       : String,
  val publisher   : String,
  val romUri      : String,
  val batterySave : String?,
  val lastPlayed  : Long?,
  val isFavorite  : Boolean = false
)