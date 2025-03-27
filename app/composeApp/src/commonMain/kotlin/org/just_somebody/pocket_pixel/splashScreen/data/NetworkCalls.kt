package org.just_somebody.pocket_pixel.splashScreen.data

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.request.*
import io.ktor.client.statement.*
import io.ktor.http.*
import org.just_somebody.pocket_pixel.core.Result
import org.just_somebody.pocket_pixel.core.networking.NetworkError
import org.just_somebody.pocket_pixel.core.networking.Request
import org.just_somebody.pocket_pixel.core.networking.Response
import org.just_somebody.pocket_pixel.splashScreen.domain.Gamer
import org.just_somebody.pocket_pixel.core.networking.createHttpClient
import org.just_somebody.pocket_pixel.core.networking.getHttpEngine

class SplashScreenNetworkCalls
{
  private val client: HttpClient = createHttpClient(getHttpEngine())
  private val baseUrl = "http://localhost:8000/api/auth"

  suspend fun connectToServer(gamer: Gamer): Result<Boolean, NetworkError>
  {
    return try
    {
      val response: Response = client.post("$baseUrl/login")
      {
        contentType(ContentType.Application.Json)
        setBody(Request(gamer.name, gamer.password))
      }.body()

      println("Login successful: ${response.gamerTag}, Token: ${response.token}")

      // - - - TODO: Store JWT token securely
      Result.Success(true)
    }
    catch (e: Exception)
    {
      println("Login error: ${e.message}")
      Result.Error(NetworkError.UNKNOWN)
    }
  }

  suspend fun registerGamer(gamer: Gamer): Result<Boolean, NetworkError>
  {
    return try
    {
      val response: HttpResponse = client.post("$baseUrl/signup")
      {
        contentType(ContentType.Application.Json)
        setBody(Request(gamer.name, gamer.password))
      }

      if (response.status == HttpStatusCode.OK)
      {
        println("Signup successful")
        Result.Success(true)
      }
      else
      {
        println("Signup failed: ${response.status}")
        Result.Error(NetworkError.CONFLICT)
      }
    }
    catch (e: Exception)
    {
      println("Signup error: ${e.message}")
      Result.Error(NetworkError.UNKNOWN)
    }
  }
}
