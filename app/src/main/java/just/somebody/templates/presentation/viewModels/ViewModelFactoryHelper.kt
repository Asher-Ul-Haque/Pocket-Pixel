package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider

/**
 * A boilerplate-reduction helper function to instantiate [ViewModel]s with custom constructors.
 *
 * This eliminates the need to manually implement a full [ViewModelProvider.Factory] class
 * for every ViewModel that requires dependency injection.
 *
 * @param VM The specific type of [ViewModel] being managed.
 * @param INIT A lambda expression that instantiates and returns the desired [ViewModel].
 * @return A configured [ViewModelProvider.Factory] capable of creating instances of [VM].
 */
fun <VM : ViewModel> viewModelFactory(INIT : () -> VM) : ViewModelProvider.Factory
{
  return object : ViewModelProvider.Factory
  {
    override fun <T : ViewModel> create(modelClass: Class<T>) : T
    { return INIT() as T }
  }
}