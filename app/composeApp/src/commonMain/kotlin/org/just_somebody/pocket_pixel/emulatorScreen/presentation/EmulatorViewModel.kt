package org.just_somebody.pocket_pixel.emulatorScreen.presentation

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel

class EmulatorViewModel : ViewModel()
{
    var state by mutableStateOf(EmulatorState())
        private set;
}