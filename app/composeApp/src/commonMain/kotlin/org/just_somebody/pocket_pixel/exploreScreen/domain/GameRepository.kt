package org.just_somebody.pocket_pixel.exploreScreen.domain

import org.just_somebody.pocket_pixel.core.DataError
import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.core.Gamer
import org.just_somebody.pocket_pixel.core.Result

interface GameRepository
{
  suspend fun searchGames     (QUERY : String)  : Result<List<Game>, DataError.Remote>;
  suspend fun getFavoriteGames(GAMER : Gamer)   : Result<List<Game>, DataError.Remote>
}