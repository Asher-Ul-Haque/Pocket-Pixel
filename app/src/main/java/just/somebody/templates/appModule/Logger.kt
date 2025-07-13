package just.somebody.templates.appModule

import android.util.Log

object ForgeLogger
{
  private const val DEFAULT_TAG = "ForgeLogger"

  fun fatal (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.wtf(TAG, formatMessage(*ARGS)) }
  fun error (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.e  (TAG, formatMessage(*ARGS)) }
  fun warn  (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.w  (TAG, formatMessage(*ARGS)) }
  fun info  (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.i  (TAG, formatMessage(*ARGS)) }
  fun debug (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.d  (TAG, formatMessage(*ARGS)) }
  fun trace (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.v  (TAG, formatMessage(*ARGS)) }
  

  private fun formatMessage(vararg ARGS: Any?) : String
  {
    return buildString()
    {
      ARGS.forEach { append(it?.toString() ?: "null") }
    }
  }
}
