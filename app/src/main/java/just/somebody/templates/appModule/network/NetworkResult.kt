package just.somebody.templates.appModule.network

import io.ktor.client.statement.HttpResponse

/**
 * A sealed hierarchy representing the outcome of a remote network operation.
 *
 * @param T The type of data payload returned upon successful fulfillment.
 */
sealed class NetworkResult<out T>
{
  /**
   * Represents a successful network delivery event.
   *
   * @property data The deserialized payload response object.
   * @property response The underlying engine HttpResponse details, if accessible.
   */
  data class Success<T>(
    val data      : T,
    val response  : HttpResponse? = null) : NetworkResult<T>()

  /**
   * Represents a failed network transmission, execution or protocol status issue.
   *
   * @property type Categorized structural archetype of the encountered failure.
   * @property message A descriptive, debuggable overview of the underlying exception context.
   * @property code The raw HTTP status code returned by the destination node, if available.
   */
  data class Error(
    val type    : NetworkErrorType,
    val message : String? = null,
    val code    : Int?    = null) : NetworkResult<Nothing>()
}

/**
 * Categorized error typologies used to determine application behavior during transport failure events.
 */
enum class NetworkErrorType
{
  /** Remote node did not fulfill communication within the designated timeout thresholds. */
  Timeout,

  /** Authentication is missing or invalid for requested context (HTTP 401). */
  Unauthorized,

  /** Target endpoint resource could not be found on the destination server (HTTP 404). */
  NotFound,

  /** Direct resolution or routing layer failed due to client interface disconnections. */
  NoInternet,

  /** Internal system faults encountered within remote server architecture bounds (HTTP 5xx). */
  ServerError,

  /** Erroneous payload structures or bad syntax dispatched by local application layer (HTTP 4xx). */
  ClientError,

  /** Indeterminate unhandled runtime, pipeline or platform mutations. */
  Unexpected,

  /** Local object parser failed to bridge structural representation to object maps. */
  SerializationError
}