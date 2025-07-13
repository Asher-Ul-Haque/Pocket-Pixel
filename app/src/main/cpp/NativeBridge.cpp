#include <jni.h>
#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "GameBoyCore.h"
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStepFrame(
  JNIEnv* JVM,
  jobject THIS)
{}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetFrameBuffer(
  JNIEnv* JVM,
  jobject THIS,
  jbyteArray FRAME_BUFFER)
{
  jbyte* buffer = JVM->GetByteArrayElements(FRAME_BUFFER, nullptr);
  getFrame(reinterpret_cast<u8*>(buffer));
  JVM->ReleaseByteArrayElements(FRAME_BUFFER, buffer, 0);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetAudioBuffer
(
  JNIEnv* JVM,
  jobject THIS,
  jbyteArray AUDIO_BUFFER)
{
  // TODO: implement nativeGetAudioBuffer()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
  JNIEnv* ENV,
  jobject THIS,
  jbyteArray ROM)
{
  // TODO: implement nativeLoadROM()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(
  JNIEnv* ENV,
  jobject THIS,
  jint BUTTON,
  jboolean PRESSED)
{
  // TODO: implement nativeSetButtonState()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
  JNIEnv* ENV,
  jobject THIS)
{
  startEmulator();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(
  JNIEnv* ENV,
  jobject THIS)
{
  stopEmulator();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartCPU(JNIEnv *env, jobject thiz) {
  // TODO: implement nativeStartCPU()
}

#ifdef __cplusplus
}
#endif
