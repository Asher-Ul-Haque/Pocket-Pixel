package just.somebody.templates.appModule.network

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.plugins.HttpTimeout
import io.ktor.client.plugins.ResponseException
import io.ktor.client.plugins.contentnegotiation.*
import io.ktor.client.plugins.defaultRequest
import io.ktor.client.plugins.logging.*
import io.ktor.client.request.*
import io.ktor.http.*
import io.ktor.serialization.kotlinx.json.*
import just.somebody.templates.appModule.ForgeLogger
import kotlinx.serialization.json.Json

class NetworkService
{
  val client = HttpClient()
  {
    install(ContentNegotiation)
    {
      json(json = Json { ignoreUnknownKeys = true })
    }
    install(HttpTimeout)
    {
      socketTimeoutMillis = 20_000L
      requestTimeoutMillis = 20_000L
    }
    install(Logging)
    {
      logger = object : Logger
      {
        override fun log(MESSAGE : String)
        { ForgeLogger.info(MESSAGE ) }
      }
      level = LogLevel.ALL
    }
    defaultRequest() { contentType(ContentType.Application.Json) }
  }

  suspend inline fun <reified T> get(
    URL     : String,
    HEADERS : Map<String, String> = emptyMap(),
    PARAMS  : Map<String, Any?>   = emptyMap()
  ): NetworkResult<T>
  {
    return safeRequest()
    {
      client.get(URL)
      {
        HEADERS.forEach()
        { (key, value) -> header(key, value) }
        url ()
        {
          PARAMS.forEach ()
          { (key, value) -> parameters.append(key, value?.toString() ?: "") }
        }
      }.body()
    }
  }


  suspend inline fun <reified T> post(
    URL     : String,
    BODY    : Any?                = null,
    HEADERS : Map<String, String> = emptyMap()
  ): NetworkResult<T>
  {
    return safeRequest ()
    {
      client.post(URL)
      {
        HEADERS.forEach { (key, value) -> header(key, value) }
        setBody(BODY ?: "")
      }.body()
    }
  }

  suspend fun head(
    URL     : String,
    HEADERS : Map<String, String> = emptyMap()
  ): NetworkResult<Boolean>
  {
    return safeRequest ()
    {
      val response = client.request(URL)
      {
        method = HttpMethod.Head
        HEADERS.forEach { (key, value) -> header(key, value) }
      }
      response.status.isSuccess()
    }
  }


  suspend inline fun <T> safeRequest(crossinline BLOCK : suspend () -> T): NetworkResult<T>
  {
    return try
    {
      val result = BLOCK()
      NetworkResult.Success(result)
    } catch (e: ResponseException)
    {
      val code    = e.response.status.value
      val message = e.message ?: "Unknown error"

      val type = when (code)
      {
        401 -> NetworkErrorType.Unauthorized
        404 -> NetworkErrorType.NotFound
        in 500..599 -> NetworkErrorType.ServerError
        else -> NetworkErrorType.Unexpected
      }
      NetworkResult.Error(type, message, code)
    }
    catch (e: java.net.SocketTimeoutException) { NetworkResult.Error(NetworkErrorType.Timeout, e.message) }
    catch (e: java.net.UnknownHostException)   { NetworkResult.Error(NetworkErrorType.NoInternet, e.message) }
    catch (e: Exception)                       { NetworkResult.Error(NetworkErrorType.Unexpected, e.message) }
  }
}