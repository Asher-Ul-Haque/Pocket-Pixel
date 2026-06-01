package just.somebody.templates.appModule

import android.util.Log

/**
 * Unified static runtime tracking facility mapping string layouts directly down to Logcat vectors.
 *
 * Provides organized, formatted string buffer construction handling unstructured vararg input
 * arrays across standard platform level profiles.
 */
object ForgeLogger
{
  private const val DEFAULT_TAG = "ForgeLogger"

  /** Pipes highly critical structural execution fault markers down into the What a Terrible Failure stream layout. */
  fun fatal (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.wtf(TAG, formatMessage(*ARGS)) }

  /** Intercepts and logs structural runtime errors to the system error level channel. */
  fun error (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.e  (TAG, formatMessage(*ARGS)) }

  /** Distributes operational warnings indicating non-fatal unexpected infrastructure variations. */
  fun warn  (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.w  (TAG, formatMessage(*ARGS)) }

  /** Logs basic diagnostic system progression metrics to inform standard workflow verification. */
  fun info  (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.i  (TAG, formatMessage(*ARGS)) }

  /** Outputs fine-grained execution metrics specifically meant for internal code logic debugging. */
  fun debug (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.d  (TAG, formatMessage(*ARGS)) }

  /** Emits verbose trace dumps exposing underlying structural state transitions step by step. */
  fun trace (vararg ARGS : Any?, TAG : String = DEFAULT_TAG) { Log.v  (TAG, formatMessage(*ARGS)) }


  /**
   * Collates arbitrary data blocks sequentially into a single consolidated string segment.
   *
   * @param ARGS Vararg parameters containing references to stringify and combine.
   * @return A flattened string representation of the parsed argument variables.
   */
  private fun formatMessage(vararg ARGS: Any?) : String
  {
    return buildString()
    {
      ARGS.forEach { append(it?.toString() ?: "null") }
    }
  }
}