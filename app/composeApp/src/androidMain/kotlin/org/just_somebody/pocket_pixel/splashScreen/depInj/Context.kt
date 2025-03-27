package org.just_somebody.pocket_pixel.splashScreen.depInj

import android.content.Context
import java.lang.ref.WeakReference

private var contextRef: WeakReference<Context>? = null

fun getContext(): Context
{
  return contextRef?.get() ?: throw IllegalStateException("Context has not been initialized. Call setContext() first.")
}

fun setContext(CONTEXT: Context)
{
  // - - - Always store the Application context to avoid leaks
  if (contextRef == null || contextRef?.get() == null)
  {
    contextRef = WeakReference(CONTEXT.applicationContext)
  }
}
