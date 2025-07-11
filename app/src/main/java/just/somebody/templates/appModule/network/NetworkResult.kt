package just.somebody.templates.appModule.network

import io.ktor.client.statement.HttpResponse

sealed class NetworkResult<out T>
{
  data class Success<T>(
    val data      : T,
    val response  : HttpResponse? = null,
  ) : NetworkResult<T>()

  data class Error(
    val type    : NetworkErrorType,
    val message : String? = null,
    val code    : Int?    = null) : NetworkResult<Nothing>()
}

enum class NetworkErrorType
{
  Timeout,
  Unauthorized,
  NotFound,
  NoInternet,
  ServerError,
  ClientError,
  Unexpected,
  SerializationError
}