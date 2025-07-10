package just.somebody.templates.data.entities

import androidx.room.ColumnInfo
import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity(tableName = "example")
data class ExampleEntity(
  @PrimaryKey(autoGenerate = true)
  @ColumnInfo(name ="example_id")
  val exampleID : Long = 0,
  val data      : String
)