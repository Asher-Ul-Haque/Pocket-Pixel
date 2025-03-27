package org.just_somebody.pocket_pixel.core.networking

import androidx.compose.runtime.Composable
import io.ktor.client.engine.HttpClientEngine

expect fun getHttpEngine(): HttpClientEngine