package just.somebody.templates.appModule.network

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.plugins.HttpTimeout
import io.ktor.client.plugins.contentnegotiation.*
import io.ktor.client.plugins.logging.*
import io.ktor.client.request.*
import io.ktor.client.statement.*
import io.ktor.http.*
import io.ktor.serialization.kotlinx.json.*
import kotlinx.serialization.json.Json

class NetworkService
{
  val client = HttpClient()
  {
    install(ContentNegotiation)
    {
      json(Json {
        ignoreUnknownKeys = true
        isLenient         = true
        encodeDefaults    = true
      })
    }

    install(HttpTimeout)
    {
      requestTimeoutMillis = 15_000
      connectTimeoutMillis = 10_000
      socketTimeoutMillis  = 15_000
    }

    install(Logging)
    {
      logger = Logger.DEFAULT
      level  = LogLevel.BODY
    }
  }

  suspend inline fun <reified T> execute(REQUEST : NetworkRequest): NetworkResult<T>
  {
    return try
    {
      val response: HttpResponse = client.request()
      {
        url()
        {
          takeFrom(REQUEST.url)
          REQUEST.queryParams.forEach { (k, v) -> parameters.append(k, v) }
        }
        method = when (REQUEST.method)
        {
          HttpMethod.GET    -> io.ktor.http.HttpMethod.Get
          HttpMethod.POST   -> io.ktor.http.HttpMethod.Post
          HttpMethod.PUT    -> io.ktor.http.HttpMethod.Put
          HttpMethod.DELETE -> io.ktor.http.HttpMethod.Delete
        }
        REQUEST.headers.forEach { (k, v) -> headers.append(k, v) }
        if (REQUEST.body != null && REQUEST.method != HttpMethod.GET)
        {
          setBody(REQUEST.body)
          contentType(ContentType.parse(REQUEST.contentType))
        }
      }

      val parsed: T = response.body()
      NetworkResult.Success(parsed, response.status.value)

    }
    catch (e: Exception) { mapExceptionToError(e) }
  }

  fun mapExceptionToError(EXCEPTION : Exception) : NetworkResult.Error
  {
    return when (EXCEPTION)
    {
      is java.net.SocketTimeoutException -> NetworkResult.Error(NetworkErrorType.Timeout,    EXCEPTION.message)
      is java.net.UnknownHostException   -> NetworkResult.Error(NetworkErrorType.NoInternet, EXCEPTION.message)
      is java.io.IOException             -> NetworkResult.Error(NetworkErrorType.Unexpected, EXCEPTION.message)
      else                               -> NetworkResult.Error(NetworkErrorType.Unexpected, EXCEPTION.message)
    }
  }
}
