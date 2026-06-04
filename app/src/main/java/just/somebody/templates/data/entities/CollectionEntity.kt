package just.somebody.templates.data.entities

import androidx.room.Entity
import androidx.room.PrimaryKey

/**
 * Represents a user-defined or system-defined grouping of games.
 */
@Entity(tableName = "collections")
data class CollectionEntity(
  @PrimaryKey(autoGenerate = true)
  val id       : Long = 0,
  val name     : String,
  val isSystem : Boolean = false
)
