#include <jni.h>
#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "GameBoyCore.h"
#include "Machine/Parts/cpu.h"
#include "Machine/Parts/cartridge.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStepFrame(
  JNIEnv* JVM,
  jobject THIS)
{
  cpuTick();
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetFrameBuffer(
  JNIEnv*    JVM,
  jobject    THIS,
  jbyteArray FRAME_BUFFER)
{
  jbyte* buffer = JVM->GetByteArrayElements(FRAME_BUFFER, nullptr);
  getFrame(reinterpret_cast<u8*>(buffer));
  JVM->ReleaseByteArrayElements(FRAME_BUFFER, buffer, 0);
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeGetAudioBuffer
(
  JNIEnv*    JVM,
  jobject    THIS,
  jbyteArray AUDIO_BUFFER)
{
  jbyte* buffer = JVM->GetByteArrayElements(AUDIO_BUFFER, nullptr);
  getAudio(reinterpret_cast<uint8_t *>(buffer));
  JVM->ReleaseByteArrayElements(AUDIO_BUFFER, buffer, 0);
}


JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeLoadROM(
  JNIEnv*    JVM,
  jobject    THIS,
  jbyteArray ROM,
  jint SIZE)
{
  FORGE_LOG_DEBUG("Attempting to read the rom");
  jbyte* buffer = JVM->GetByteArrayElements(ROM, nullptr);
  loadCartridge(reinterpret_cast<uint8_t *>(buffer), SIZE);
  JVM->ReleaseByteArrayElements(ROM, buffer, JNI_ABORT);
  FORGE_LOG_DEBUG("Successfully read the rom");
}

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeSetButtonState(
  JNIEnv*  ENV,
  jobject  THIS,
  jint     BUTTON,
  jboolean PRESSED)
{ setButton(BUTTON, PRESSED); }

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStartEmulator(
  JNIEnv* ENV,
  jobject THIS)
{ cpuInit(); }

JNIEXPORT void JNICALL
Java_just_somebody_templates_domain_GameBoy_nativeStopEmulator(
  JNIEnv* ENV,
  jobject THIS)
{ stopEmulator(); }

#ifdef __cplusplus
}
#endif
