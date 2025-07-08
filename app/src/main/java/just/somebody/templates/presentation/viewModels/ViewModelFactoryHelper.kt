package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider

fun <VM : ViewModel> viewModelFactory(INIT : () -> VM) : ViewModelProvider.Factory
{
  return object : ViewModelProvider.Factory
  {
    override fun <T : ViewModel> create(modelClass: Class<T>) : T
    {
      return INIT() as T
    }
  }
}