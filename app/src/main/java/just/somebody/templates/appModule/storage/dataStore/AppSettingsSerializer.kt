package just.somebody.templates.appModule.storage.dataStore

import androidx.datastore.core.Serializer
import kotlinx.serialization.SerializationException
import kotlinx.serialization.json.Json
import java.io.InputStream
import java.io.OutputStream

/**
 * Serialization bridge object facilitating IO storage mutations for the application settings layer.
 *
 * Implements the DataStore [Serializer] contract, marshaling [AppSettings] memory states
 * directly into raw UTF-8 JSON structures over standard platform file system streams.
 */
object AppSettingsSerializer : Serializer<AppSettings>
{
  override val defaultValue: AppSettings
    get() = AppSettings()

  /**
   * Decodes incoming stream arrays into an explicitly structured [AppSettings] memory state instance.
   *
   * @param INPUT The active storage read pipeline stream.
   * @return A valid [AppSettings] entity structure, or [defaultValue] if data parsing corruptions occur.
   */
  override suspend fun readFrom(INPUT : InputStream) : AppSettings
  {
    return try
    {
      Json.decodeFromString(
        deserializer  = AppSettings.serializer(),
        string        = INPUT.readBytes().decodeToString())
    }
    catch (e : SerializationException)
    {
      e.printStackTrace()
      defaultValue
    }
  }

  /**
   * Encodes and writes structural [AppSettings] states into the file system storage output channel.
   *
   * @param APP_SETTINGS The memory data state model to serialize.
   * @param OUTPUT The active target destination file writing stream.
   */
  override suspend fun writeTo(APP_SETTINGS : AppSettings, OUTPUT : OutputStream)
  {
    OUTPUT.write(
      Json.encodeToString(
        serializer  = AppSettings.serializer(),
        value       = APP_SETTINGS).encodeToByteArray())
  }
}