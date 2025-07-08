package just.somebody.templates.appModule.storage.database.daos

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.Query
import androidx.room.Update
import just.somebody.templates.appModule.storage.database.entities.ExampleEntity

@Dao
interface ExampleDAO
{
  @Insert suspend fun insertExample(EXAMPLE : ExampleEntity) : Long
  @Delete suspend fun deleteExample(EXAMPLE : ExampleEntity) : Unit
  @Update suspend fun updateExample(EXAMPLE : ExampleEntity) : Unit

  @Query("SELECT * FROM example WHERE example_id = :ID")
  suspend fun getExampleByID(ID : Long) : ExampleEntity?

  @Query("SELECT * FROM example")
  suspend fun getAllExamples() : List<ExampleEntity>
}