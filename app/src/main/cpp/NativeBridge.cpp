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
  jobject THIS,       jbyteArray audio__buffer) {
  // TODO: implement nativeGetAudioBuffer()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(JNIEnv *env, jobject thiz,
                                                          jbyteArray rom) {
  // TODO: implement nativeLoadROM()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(JNIEnv *env, jobject thiz,
                                                                 jint button, jboolean pressed) {
  // TODO: implement nativeSetButtonState()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(JNIEnv *env, jobject thiz) {
  // TODO: implement nativeStartEmulator()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(JNIEnv *env, jobject thiz) {
  // TODO: implement nativeStopEmulator()
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartCPU(JNIEnv *env, jobject thiz) {
  // TODO: implement nativeStartCPU()
}

#ifdef __cplusplus
}
#endif
