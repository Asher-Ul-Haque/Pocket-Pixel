package org.just_somebody.pocket_pixel.browseScreen.domain

import org.just_somebody.pocket_pixel.core.DataError
import org.just_somebody.pocket_pixel.core.Result

interface GameRepository
{
  suspend fun searchGames(QUERY : String) : Result<List<Game>, DataError.Remote>;
  //suspend fun getFavoriteGames() :
}