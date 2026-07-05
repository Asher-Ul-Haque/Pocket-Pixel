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
      onDelete      = ForeignKey.SET_NULL)
  ],
  indices = [Index("gameId"), Index("raGameId")]
)
data class AchievementEntity(
  @PrimaryKey
  val raId            : Int,     // - - - RetroAchievements internal ID
  val raGameId        : Int,     // - - - RA Game ID
  val gameId          : Long?,   // - - - Local Game ID (null if not on device)
  val title           : String,
  val description     : String,
  val points          : Int,
  val badgeUrl        : String,
  val unlockDate      : Long,    // - - - 0 if locked
  val isUnlocked      : Boolean,
  val isHardcore      : Boolean,
  val rarity          : Float    = 0f,
  val measuredProgress: String   = "",
  val measuredPercent : Float    = 0f,
  val type            : Int      = 0 // - - - 0: Standard, 1: Missable, 2: Progression, 3: Win
)
