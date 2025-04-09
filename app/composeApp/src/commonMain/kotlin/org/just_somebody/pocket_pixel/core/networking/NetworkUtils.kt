package org.just_somebody.pocket_pixel.core.networking

import io.ktor.client.call.NoTransformationFoundException
import io.ktor.client.call.body
import io.ktor.client.network.sockets.SocketTimeoutException
import io.ktor.client.statement.HttpResponse
import io.ktor.util.network.UnresolvedAddressException
import kotlinx.coroutines.ensureActive
import org.just_somebody.pocket_pixel.core.DataError
import org.just_somebody.pocket_pixel.core.Result
import kotlin.coroutines.coroutineContext


// - - - Most of this file comes from Philip Lackner's course, puting this as a citation : https://youtu.be/WT9-4DXUqsM?si=2mryPphnPAJOCumF

suspend inline fun <reified T> responseToResult(RESPONSE : HttpResponse) : Result<T, DataError.Remote>
{
  // - - - reified allows for generic type 'T', inline is required to infer that type at the place that it is called
  return when (RESPONSE.status.value)
  {
    // - - - coding error
    in 200 .. 299 ->
      {
        try                                         { Result.Success(RESPONSE.body<T>()) }
        catch (e : NoTransformationFoundException)  { Result.Error(DataError.Remote.SERIALIZATION) }
      }

    // - - - bad internet
    408 -> Result.Error(DataError.Remote.REQUEST_TIMEOUT)
    429 -> Result.Error(DataError.Remote.TOO_MANY_REQUESTS)

    // - - - server side error
    in 500..599 -> Result.Error(DataError.Remote.SERVER_FAIL)

    // - - - no idea
    else -> Result.Error(DataError.Remote.UNKNOWN)
  }
}

suspend inline fun <reified T> safeCall(EXECUTE : () -> HttpResponse) : Result<T, DataError.Remote>
{
  val response =
    try { EXECUTE() }
    catch (e : SocketTimeoutException)      { return Result.Error(DataError.Remote.REQUEST_TIMEOUT) }
    catch (e : UnresolvedAddressException)  { return Result.Error(DataError.Remote.NO_INTERNET) }
    catch (e : Exception)
    {
      // - - - catch suspending exception
      coroutineContext.ensureActive()
      return Result.Error(DataError.Remote.UNKNOWN)
    }

  return responseToResult(response)
}