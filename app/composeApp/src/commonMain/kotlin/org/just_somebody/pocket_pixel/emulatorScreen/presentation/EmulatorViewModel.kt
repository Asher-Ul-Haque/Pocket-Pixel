package org.just_somebody.pocket_pixel.emulatorScreen.presentation

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getGame
import org.just_somebody.pocket_pixel.depInj.getNetworkCalls
import kotlin.coroutines.resume

class EmulatorViewModel : ViewModel()
{
    var state by mutableStateOf(EmulatorState())
        private set;
}