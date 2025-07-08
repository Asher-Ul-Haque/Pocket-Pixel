package just.somebody.templates.appModule.network

enum class HttpMethod
{
  GET,
  POST,
  PUT,
  DELETE
}

data class NetworkRequest(
  val url         : String,
  val method      : HttpMethod          = HttpMethod.GET,
  val headers     : Map<String, String> = emptyMap(),
  val queryParams : Map<String, String> = emptyMap(),
  val body        : String?             = null,
  val contentType : String              = "application/json"
)