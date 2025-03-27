package org.just_somebody.pocket_pixel.emulatorScreen.presentation

import org.just_somebody.pocket_pixel.emulatorScreen.domain.GameBoy
import org.just_somebody.pocket_pixel.emulatorScreen.domain.getGameBoy

data class EmulatorState(val gameBoy: GameBoy = getGameBoy())