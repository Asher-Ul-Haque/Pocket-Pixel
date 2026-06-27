package just.somebody.templates.data.entities

import androidx.room.Entity
import androidx.room.ForeignKey
import androidx.room.Index
import androidx.room.PrimaryKey

@Entity(
  tableName   = "achievements",
  foreignKeys = [
    ForeignKey(
      entity        = GameEntity::class,
      parentColumns = ["id"],
      childColumns  = ["gameId"],
      onDelete      = ForeignKey.CASCADE)
  ],
  indices = [Index("gameId")]
)
data class AchievementEntity(
  @PrimaryKey
  val raId        : Int,  // - - - RetroAchievements internal ID
  val gameId      : Long, // - - - Local Game ID
  val title       : String,
  val description : String,
  val points      : Int,
  val badgeUrl    : String,
  val unlockDate  : Long,
  val isHardcore  : Boolean
)
