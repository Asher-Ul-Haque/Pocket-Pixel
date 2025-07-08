#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_just_somebody_templates_appModule_nativeBridge_NativeBridge_stringFromJNI(
  JNIEnv* ENV,
  jobject THIS)
{
  std::string msg = "Hello! from C++";
  return ENV->NewStringUTF(msg.c_str());
}