package org.just_somebody.pocket_pixel.core.networking

import io.ktor.client.HttpClient
import io.ktor.client.engine.HttpClientEngine
import io.ktor.client.plugins.contentnegotiation.ContentNegotiation
import io.ktor.client.plugins.logging.LogLevel
import io.ktor.client.plugins.logging.Logging
import io.ktor.serialization.kotlinx.json.json
import kotlinx.serialization.json.Json

fun createHttpClient(ENGINE : HttpClientEngine) : HttpClient
{
  return HttpClient(ENGINE)
    {
      install(Logging) { level = LogLevel.ALL }
      install(ContentNegotiation)
      {
        json(json = Json { ignoreUnknownKeys = true })
      }
    }
}