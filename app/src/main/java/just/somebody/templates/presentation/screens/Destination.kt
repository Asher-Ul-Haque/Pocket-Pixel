package just.somebody.templates.presentation.screens

import just.somebody.templates.domain.models.Game
import kotlinx.serialization.Serializable

/**
 * A type-safe navigation map representing all accessible screen locations in the application.
 *
 * Utilizes a sealed hierarchy integrated with Kotlin Serialization to enforce compile-time path verification.
 * Standard navigation targets are modeled as singleton objects, while engine routes requiring runtime variables
 * (such as target ROM locations) leverage serialized parameter data structures.
 */
sealed interface Destination
{
  /** Represents the default catalog screen displaying the local game compilation library. */
  @Serializable data object Home                        : Destination

  /** Represents the filtered bookmarked layout collection view. */
  @Serializable data object Favorites                   : Destination

  /** Represents the user-defined custom game list management screen. */
  @Serializable data object Collections                 : Destination

  /** Represents the dedicated searchable query input and matching result grid panel. */
  @Serializable data object Search                      : Destination

  /** Represents the primary device global modifier panel. */
  @Serializable data object Settings                    : Destination

  /** * Represents the core runtime emulation console layout container surface.
   *
   * @property URI The explicit string data address location pointing straight to the target game ROM file.
   */
  @Serializable data class  Emulator(val URI : String)  : Destination
}