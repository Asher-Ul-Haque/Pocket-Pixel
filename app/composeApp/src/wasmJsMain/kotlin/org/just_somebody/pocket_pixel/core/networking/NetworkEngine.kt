package org.just_somebody.pocket_pixel.core.networking

import androidx.compose.runtime.Composable
import io.ktor.client.engine.HttpClientEngine
import io.ktor.client.engine.js.Js
import io.ktor.client.engine.js.JsClient

actual fun getHttpEngine() : HttpClientEngine
{ return Js.create() }