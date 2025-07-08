package just.somebody.templates.appModule.network

sealed class NetworkResult<out T>
{
  data class Success<T>(
    val data : T,
    val code : Int
  ) : NetworkResult<T>()

  data class Error(
    val TYPE    : NetworkErrorType,
    val MESSAGE : String? = null) : NetworkResult<Nothing>()
}

enum class NetworkErrorType
{
  Timeout,
  NoInternet,
  ServerError,
  ClientError,
  Unexpected,
  SerializationError
}