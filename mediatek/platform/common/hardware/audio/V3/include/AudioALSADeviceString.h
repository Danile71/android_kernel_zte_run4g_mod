#ifndef ANDROID_AUDIO_MTK_DEVICE_STRING_H
#define ANDROID_AUDIO_MTK_DEVICE_STRING_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Mutex.h>
#include <utils/String8.h>
#include <media/AudioSystem.h>
#include "AudioType.h"
#include <utils/KeyedVector.h>

namespace android
{

static String8 keypcmDl1Meida = String8("MultiMedia1_PLayback");
static String8 keypcmUl1Capture = String8("MultiMedia1_Capture");
static String8 keypcmPcm2voice = String8("PCM2_PLayback");
static String8 keypcmHDMI = String8("HMDI_PLayback");
static String8 keypcmUlDlLoopback = String8("ULDL_Loopback");
static String8 keypcmI2Splayback = String8("I2S0_PLayback");
static String8 keypcmMRGrxPlayback = String8("MRGRX_PLayback");
static String8 keypcmMRGrxCapture = String8("MRGRX_CAPTURE");
static String8 keypcmFMI2SPlayback = String8("FM_I2S_Playback");
static String8 keypcmFMI2SCapture = String8("FM_I2S_Capture");
static String8 keypcmI2S0Dl1Playback = String8("I2S0DL1_PLayback");
static String8 keypcmDl1AwbCapture = String8("DL1_AWB_Record");
static String8 keypcmVoiceCallBT = String8("Voice_Call_BT_Playback");
static String8 keypcmVOIPCallBTPlayback = String8("VOIP_Call_BT_Playback");
static String8 keypcmVOIPCallBTCapture = String8("VOIP_Call_BT_Capture");
static String8 keypcmTDMLoopback = String8("TDM_Debug_Record");
static String8 keypcmMRGTxPlayback = String8("FM_MRGTX_Playback");
static String8 keypcmUl2Capture = String8("MultiMediaData2_Capture");
static String8 keypcmI2SAwbCapture = String8("I2S0AWB_Capture");
static String8 keypcmMODADCI2S = String8("ANC_Debug_Record_MOD");
static String8 keypcmADC2AWB = String8("ANC_Debug_Record_ADC2");
static String8 keypcmIO2DAI = String8("ANC_Debug_Record_IO2");
static String8 keypcmHpimpedancePlayback = String8("HP_IMPEDANCE_Playback");
static String8 keypcmModomDaiCapture = String8("Moddai_Capture");

}

#endif
