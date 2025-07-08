package just.somebody.templates.appModule.nativeBridge

class NativeBridge
{
  companion object
  { init { System.loadLibrary("native-lib") } }

  external fun stringFromJNI() : String
}