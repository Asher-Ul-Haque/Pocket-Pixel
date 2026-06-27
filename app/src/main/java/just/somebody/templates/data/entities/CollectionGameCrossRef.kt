package just.somebody.templates.data.entities

import androidx.room.Entity
import androidx.room.ForeignKey
import androidx.room.Index

/**
 * Cross-reference table linking games to collections, enabling many-to-many relationships.
 */
@Entity(
  tableName  = "collection_game_cross_ref",
  primaryKeys = ["collectionId", "gameId"],
  indices    = [Index("gameId")],
  foreignKeys = [
    ForeignKey(
      entity        = CollectionEntity::class,
      parentColumns = ["id"],
      childColumns  = ["collectionId"],
      onDelete      = ForeignKey.CASCADE
    ),
    ForeignKey(
      entity        = GameEntity::class,
      parentColumns = ["id"],
      childColumns  = ["gameId"],
      onDelete      = ForeignKey.CASCADE
    )
  ]
)
data class CollectionGameCrossRef(
  val collectionId : Long,
  val gameId       : Long)
