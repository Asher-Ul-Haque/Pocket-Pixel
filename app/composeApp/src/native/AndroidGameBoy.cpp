#include <jni.h>
#include "defines.h"
#include "GameBoyCore.h"
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeStepFrame(
   JNIEnv* JVM,
   jobject THIS)
{ stepFrame(); }

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeGetFrameBuffer(
    JNIEnv* JVM,
    jobject THIS,
    jbyteArray FRAME_BUFFER)
{
    jbyte* buffer = JVM->GetByteArrayElements(FRAME_BUFFER, nullptr);
    getFrame(reinterpret_cast<u8*>(buffer));
    JVM->ReleaseByteArrayElements(FRAME_BUFFER, buffer, 0);
}

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeGetAudioBuffer(
    JNIEnv* JVM,
    jobject THIS,
    jbyteArray AUDIO_BUFFER)
{
    jbyte* buffer = JVM->GetByteArrayElements(AUDIO_BUFFER, nullptr);
    getAudio(reinterpret_cast<uint8_t *>(buffer));
    JVM->ReleaseByteArrayElements(AUDIO_BUFFER, buffer, 0);
}

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeLoadROM(
        JNIEnv* JVM,
        jobject THIS,
        jbyteArray ROM)
{
    jbyte* buffer = JVM->GetByteArrayElements(ROM, nullptr);
    loadROM(reinterpret_cast<uint8_t *>(buffer));
    JVM->ReleaseByteArrayElements(ROM, buffer, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeSetButtonState(
    JNIEnv* JVM,
    jobject THIS,
    jint BUTTON,
    jboolean PRESSED)
{ setButton(BUTTON, PRESSED); }

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeStartEmulator(
    JNIEnv* JVM,
    jobject THIS)
{ startEmulator(); }

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeStopEmulator(
    JNIEnv* JVM,
    jobject THIS)
{ stopEmulator(); }

#ifdef __cplusplus
}
#endif