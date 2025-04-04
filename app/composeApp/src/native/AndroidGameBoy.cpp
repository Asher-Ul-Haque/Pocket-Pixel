#ifdef __ANDROID__
#include <jni.h>
#include "defines.h"
#include "GameBoyCore.h"
#include "Machine/testRoms/Zelda.h"
#ifdef __cplusplus
extern "C" {
#endif

// array size is 32768


JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeStepFrame(
   JNIEnv* JVM,
   jobject THIS)
{ cpuTick(); }

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
    //jbyte* buffer = JVM->GetByteArrayElements(ROM, nullptr);
    //loadCartridge(reinterpret_cast<uint8_t *>(buffer), 1);
    //JVM->ReleaseByteArrayElements(ROM, buffer, JNI_ABORT);
    cartridgeLoad((u8*)testRom, size);
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
{ cpuInit(); }

JNIEXPORT void JNICALL
Java_org_just_1somebody_pocket_1pixel_emulatorScreen_domain_AndroidGameBoy_nativeStopEmulator(
    JNIEnv* JVM,
    jobject THIS)
{ stopEmulator(); }

#ifdef __cplusplus
}
#endif
#endif