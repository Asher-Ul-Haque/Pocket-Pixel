package org.just_somebody.pocket_pixel.core

import androidx.compose.runtime.Composable
import kotlinx.browser.window

@Composable
actual fun getWindowWidth()   : Int { return window.innerWidth }

@Composable
actual fun getWindowHeight()  : Int { return window.innerHeight }