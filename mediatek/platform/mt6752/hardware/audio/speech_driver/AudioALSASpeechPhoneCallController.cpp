#include "AudioALSASpeechPhoneCallController.h"

#include <media/AudioSystem.h>
#include <cutils/properties.h>

#include <hardware_legacy/power.h>

#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSAVolumeController.h"

#include "SpeechDriverInterface.h"
#include "SpeechDriverFactory.h"

#include "SpeechEnhancementController.h"
#include "SpeechPcm2way.h"
#include "SpeechBGSPlayer.h"
#include "SpeechVMRecorder.h"
#include "SpeechANCController.h"
#include "AudioALSAVolumeController.h"
#include "WCNChipController.h"
#include "AudioALSADriverUtility.h"

#define LOG_TAG "AudioALSASpeechPhoneCallController"

namespace android
{

/*==============================================================================
 *                     Property keys
 *============================================================================*/
static const char PROPERTY_KEY_MIC_MUTE_ON[PROPERTY_KEY_MAX] = "af.recovery.mic_mute_on";
static const char PROPERTY_KEY_DL_MUTE_ON[PROPERTY_KEY_MAX] = "af.recovery.dl_mute_on";
static const char PROPERTY_KEY_UL_MUTE_ON[PROPERTY_KEY_MAX] = "af.recovery.ul_mute_on";


static const char WAKELOCK_NAME[] = "EXT_MD_AUDIO_WAKELOCK";


static struct mixer *mMixer;


AudioALSASpeechPhoneCallController *AudioALSASpeechPhoneCallController::mSpeechPhoneCallController = NULL;
AudioALSASpeechPhoneCallController *AudioALSASpeechPhoneCallController::getInstance()
{
    static AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mSpeechPhoneCallController == NULL)
    {
        mSpeechPhoneCallController = new AudioALSASpeechPhoneCallController();
    }
    ASSERT(mSpeechPhoneCallController != NULL);
    return mSpeechPhoneCallController;
}

AudioALSASpeechPhoneCallController::AudioALSASpeechPhoneCallController() :
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mSpeechDriverFactory(SpeechDriverFactory::GetInstance()),
    mAudioALSAVolumeController(AudioALSAVolumeController::getInstance()),
    mAudioBTCVSDControl(NULL),
    mAudioMode(AUDIO_MODE_NORMAL),
    mMicMute(false),
    mVtNeedOn(false),
    mTtyMode(AUD_TTY_OFF),
    mRoutingForTty(AUDIO_DEVICE_OUT_EARPIECE),
    mBTMode(0),
    mPcmIn(NULL),
    mPcmOut(NULL),
    mRfInfo(0),
    mRfMode(0),
    mASRCNeedOn(0)
{
    // check need mute mic or not after kill mediaserver
    char mic_mute_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_MIC_MUTE_ON, mic_mute_on, "0"); //"0": default off
    mMicMute = (mic_mute_on[0] == '0') ? false : true;

    // Need Mute DL Voice
    char dl_mute_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_DL_MUTE_ON, dl_mute_on, "0"); //"0": default off
    mDlMute = (dl_mute_on[0] == '0') ? false : true;

    // Need Mute UL Voice
    char ul_mute_on[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_UL_MUTE_ON, ul_mute_on, "0"); //"0": default off
    mUlMute = (ul_mute_on[0] == '0') ? false : true;

    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    ALOGD("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);

}

AudioALSASpeechPhoneCallController::~AudioALSASpeechPhoneCallController()
{

}

bool AudioALSASpeechPhoneCallController::checkTtyNeedOn() const
{
    return (mTtyMode != AUD_TTY_OFF && mVtNeedOn == false && mTtyMode != AUD_TTY_ERR);
}

bool AudioALSASpeechPhoneCallController::checkSideToneFilterNeedOn(const audio_devices_t output_device) const
{
    // TTY do not use STMF. Open only for earphone & receiver when side tone != 0.
    return ((checkTtyNeedOn() == false) &&
            (mAudioALSAVolumeController->GetSideToneGain(output_device) != 0) &&
            (output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
             output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
             output_device == AUDIO_DEVICE_OUT_EARPIECE));
}

status_t AudioALSASpeechPhoneCallController::init()
{
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}


audio_devices_t AudioALSASpeechPhoneCallController::getInputDeviceForPhoneCall(const audio_devices_t output_devices)
{
    audio_devices_t input_device;

    switch (output_devices)
    {
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
        {
            input_device = AUDIO_DEVICE_IN_WIRED_HEADSET;
            break;
        }
        case AUDIO_DEVICE_OUT_EARPIECE:
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
        {
            input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
            break;
        }
        case AUDIO_DEVICE_OUT_SPEAKER:
        {
            if (USE_REFMIC_IN_LOUDSPK == 1)
            {
                input_device = AUDIO_DEVICE_IN_BACK_MIC;
            }
            else
            {
                input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
            }
            break;
        }
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        {
            input_device = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
            break;
        }
        default:
        {
            ALOGW("%s(), no support such output_devices(0x%x), default use AUDIO_DEVICE_IN_BUILTIN_MIC(0x%x) as input_device",
                  __FUNCTION__, output_devices, AUDIO_DEVICE_IN_BUILTIN_MIC);
            input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
            break;
        }
    }

    return input_device;
}

status_t AudioALSASpeechPhoneCallController::open(const audio_mode_t audio_mode, const audio_devices_t output_devices, const audio_devices_t input_device)
{
    ALOGD("+%s(), audio_mode = %d, output_devices = 0x%x, input_device = 0x%x",
          __FUNCTION__, audio_mode, output_devices, input_device);
    AudioAutoTimeoutLock _l(mLock);


    // wake lock for external modem
    if (audio_mode == AUDIO_MODE_IN_CALL_EXTERNAL)
    {
        int ret = acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKELOCK_NAME);
        ALOGD("%s(), acquire_wake_lock(%s) = %d", __FUNCTION__, WAKELOCK_NAME, ret);
    }


    // get speech driver instance
    mSpeechDriverFactory->SetActiveModemIndexByAudioMode(audio_mode);
    const modem_index_t    modem_index   = mSpeechDriverFactory->GetActiveModemIndex();
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    // check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)output_devices);
    const uint32_t sample_rate = calculateSampleRate(bt_device_on);
    ALOGD("%s(), bt_device_on = %d, sample_rate = %d", __FUNCTION__, bt_device_on, sample_rate);

#if 0 // TODO(Harvey): do device related control in stream manager
    // set device
    if (checkTtyNeedOn() == true)
    {
        setTtyInOutDevice(getRoutingForTty());
    }
    else
    {
        // Note: set output device in phone call will also assign input device
        //mAudioResourceManager->setDlOutputDevice(mAudioResourceManager->getDlOutputDevice());
    }
#endif

    //--- here to test pcm interface platform driver_attach
    if (bt_device_on)
    {
        if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true)
        {
            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 1;
            mConfig.rate = sample_rate;
            mConfig.period_size = 4096;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            ASSERT(mPcmOut == NULL);

            if (mAudioMode == AUDIO_MODE_IN_CALL_EXTERNAL || mAudioMode == AUDIO_MODE_IN_CALL_2)
            {
                mPcmOut = pcm_open(0, 19, PCM_OUT, &mConfig);
            }
            else
            {
                mPcmOut = pcm_open(0, 10, PCM_OUT, &mConfig);
            }

            ASSERT(mPcmOut != NULL);
            ALOGD("%s(), mPcmOut = %p", __FUNCTION__, mPcmOut);
            pcm_start(mPcmOut);
        }
    }
    else
    {
        if (audio_mode == AUDIO_MODE_IN_CALL_EXTERNAL || audio_mode == AUDIO_MODE_IN_CALL_2)
        {
            mConfig.channels = 2;
            mConfig.rate = sample_rate;
            mConfig.period_size = 1024;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            ASSERT(mPcmIn == NULL && mPcmOut == NULL);
            ALOGD("%s(), audio_mode(%d), open ext md", __FUNCTION__, audio_mode);
            mPcmIn = pcm_open(0, 17, PCM_IN, &mConfig);
            mPcmOut = pcm_open(0, 17, PCM_OUT, &mConfig);
        }
        else
        {
            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 2;
            mConfig.rate = sample_rate;
            mConfig.period_size = 1024;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            ASSERT(mPcmIn == NULL && mPcmOut == NULL);
            ALOGD("%s(), audio_mode(%d), open int md", __FUNCTION__, audio_mode);
            mPcmIn = pcm_open(0, 2, PCM_IN, &mConfig);
            mPcmOut = pcm_open(0, 2, PCM_OUT, &mConfig);
        }
        ASSERT(mPcmIn != NULL && mPcmOut != NULL);
        ALOGD("%s(), mPcmIn = %p, mPcmOut = %p", __FUNCTION__, mPcmIn, mPcmOut);

        pcm_start(mPcmIn);
        pcm_start(mPcmOut);
    }
    ALOGD("%s(), Speech_MD_USAGE set on", __FUNCTION__);
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speech_MD_USAGE"), "On"))
    {
        ALOGE("Error: Speech_MD invalid value");
    }


    if (checkTtyNeedOn() == true)
    {
        setTtyInOutDevice(getRoutingForTty());
    }
    else
    {
        // Set PMIC digital/analog part - uplink has pop, open first
        mHardwareResourceManager->startInputDevice(input_device);
    }


    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    // start Side Tone Filter
    if (checkSideToneFilterNeedOn(output_devices) == true)
    {
        mHardwareResourceManager->EnableSideToneFilter(true);
    }

    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    if (checkTtyNeedOn() == true)
    {
        pSpeechDriver->SetSpeechMode(mHardwareResourceManager->getInputDevice(), mHardwareResourceManager->getOutputDevice());
    }
    else
    {
        pSpeechDriver->SetSpeechMode(input_device, output_devices);
    }

    // Speech/VT on
    if (mVtNeedOn == true)
    {
        pSpeechDriver->VideoTelephonyOn();

        // trun on P2W for Video Telephony
        bool wideband_on = false; // VT default use Narrow Band (8k), modem side will SRC to 16K
        pSpeechDriver->PCM2WayOn(wideband_on);
    }
    else
    {
        pSpeechDriver->SpeechOn();

        // turn on TTY
        if (checkTtyNeedOn() == true)
        {
            pSpeechDriver->TtyCtmOn(BAUDOT_MODE);
        }
    }

    if (checkTtyNeedOn() == false)
    {
        mHardwareResourceManager->startOutputDevice(output_devices, 16000);
    }


    // check VM need open
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
    if (pSpeechVMRecorder->GetVMRecordCapability() == true)
    {
        ALOGD("%s(), Open VM/EPL record", __FUNCTION__);
        pSpeechVMRecorder->Open();
    }

    mAudioMode = audio_mode;

    ALOGD("-%s(), mAudioMode = %d", __FUNCTION__, mAudioMode);
    return NO_ERROR;
}


status_t AudioALSASpeechPhoneCallController::close()
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("+%s(), mAudioMode = %d", __FUNCTION__, mAudioMode);

    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();
    ASSERT((modem_index == MODEM_1 && mAudioMode == AUDIO_MODE_IN_CALL) ||
           (modem_index == MODEM_2 && mAudioMode == AUDIO_MODE_IN_CALL_2) ||
           (modem_index == MODEM_EXTERNAL && mAudioMode == AUDIO_MODE_IN_CALL_EXTERNAL));


    // check VM need close
    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
    if (pSpeechVMRecorder->GetVMRecordStatus() == true)
    {
        ALOGD("%s(), Close VM/EPL record", __FUNCTION__);
        pSpeechVMRecorder->Close();
    }


    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int num_values;

    mHardwareResourceManager->stopOutputDevice();

    // Stop Side Tone Filter
    mHardwareResourceManager->EnableSideToneFilter(false);

    // Stop MODEM_PCM
    if (mPcmIn != NULL)
    {
        pcm_stop(mPcmIn);
        pcm_close(mPcmIn);
        mPcmIn = NULL;
    }

    if (mPcmOut != NULL)
    {
        pcm_stop(mPcmOut);
        pcm_close(mPcmOut);
        mPcmOut = NULL;
    }

    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());


    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

#ifdef MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT
    //ANC off forcely
    if (SpeechANCController::getInstance()->GetApplyANC() == true)
    {

        if (SpeechANCController::getInstance()->GetEanbleANCLog() == true)
        {
            SpeechANCController::getInstance()->StopANCLog();
        }

        SpeechANCController::getInstance()->EanbleANC(false);
    }
    SpeechANCController::getInstance()->SwapANC(false);
#endif

    // Speech/VT off
    if (pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK) == true)
    {
        pSpeechDriver->PCM2WayOff();
        pSpeechDriver->VideoTelephonyOff();
    }
    else if (pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true)
    {
        if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true)
        {
            pSpeechDriver->TtyCtmOff();
        }
        pSpeechDriver->SpeechOff();
    }
    else
    {
        ALOGE("%s(), mAudioMode = %d, Speech & VT are already closed!!", __FUNCTION__, mAudioMode);
        ASSERT(pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK)     == true ||
               pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true);
    }

    // clean VT status
    if (mVtNeedOn == true)
    {
        ALOGD("%s(), Set mVtNeedOn = false", __FUNCTION__);
        mVtNeedOn = false;
    }

    // wake lock for external modem
    if (mAudioMode == AUDIO_MODE_IN_CALL_EXTERNAL)
    {
        int ret = release_wake_lock(WAKELOCK_NAME);
        ALOGD("%s(), release_wake_lock(%s) = %d", __FUNCTION__, WAKELOCK_NAME, ret);
    }
    ALOGD("%s(), Speech_MD_USAGE set off", __FUNCTION__);
    if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Speech_MD_USAGE"), "Off"))
    {
        ALOGE("Error: Speech_MD_USAGE invalid value");
    }

    mAudioMode = AUDIO_MODE_NORMAL; // TODO(Harvey): default value? VoIP?
    ALOGD("-%s(), mAudioMode = %d", __FUNCTION__, mAudioMode);

    return NO_ERROR;
}


status_t AudioALSASpeechPhoneCallController::routing(const audio_devices_t new_output_devices, const audio_devices_t new_input_device)
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGD("+%s(), new_output_devices = 0x%x, new_input_device = 0x%x", __FUNCTION__, new_output_devices, new_input_device);

    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();

    // Get current active speech driver
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();


    // Mute during device change.
    pSpeechDriver->SetDownlinkMute(true);
    pSpeechDriver->SetUplinkMute(true);


    // Stop PMIC digital/analog part - downlink
    mHardwareResourceManager->stopOutputDevice();

    // Stop Side Tone Filter
    mHardwareResourceManager->EnableSideToneFilter(false);

    // Stop MODEM_PCM
    //mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);

    // Stop PMIC digital/analog part - uplink
    mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());

    // Stop AP side digital part
    //CloseModemSpeechDigitalPart(modem_index, (audio_devices_t)mAudioResourceManager->getDlOutputDevice());

    // Get new device
    const audio_devices_t output_device = new_output_devices; //(audio_devices_t)mAudioResourceManager->getDlOutputDevice();
    const audio_devices_t input_device  = new_input_device; //(audio_devices_t)mAudioResourceManager->getUlInputDevice();
    ALOGD("%s(), output_device = 0x%x, input_device = 0x%x", __FUNCTION__, output_device, input_device);



    // Check BT device
    const bool bt_device_on = android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)new_output_devices);
    const uint32_t sample_rate = calculateSampleRate(bt_device_on);
    ALOGD("%s(), bt_device_on = %d, sample_rate = %d", __FUNCTION__, bt_device_on, sample_rate);

    // Set sampling rate
    //mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sample_rate);
    //mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC,  sample_rate);

    // Open ADC/DAC I2S, or DAIBT
    //OpenModemSpeechDigitalPart(modem_index, output_device);

    // Clean Side Tone Filter gain
    pSpeechDriver->SetSidetoneGain(0);

    if (bt_device_on)
    {
        //close previous device
        if (mPcmIn != NULL)
        {
            pcm_stop(mPcmIn);
            pcm_close(mPcmIn);
            mPcmIn = NULL;
        }

        if (mPcmOut != NULL)
        {
            pcm_stop(mPcmOut);
            pcm_close(mPcmOut);
            mPcmOut = NULL;
        }
        if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true)
        {
            ALOGD("%s(), pcm config", __FUNCTION__);

            //open bt sco device
            memset(&mConfig, 0, sizeof(mConfig));

            mConfig.channels = 1;
            mConfig.rate = sample_rate;
            mConfig.period_size = 4096;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;

            ASSERT(mPcmIn == NULL && mPcmOut == NULL);
            ALOGD("%s(), pcm open", __FUNCTION__);

            if (mAudioMode == AUDIO_MODE_IN_CALL_EXTERNAL || mAudioMode == AUDIO_MODE_IN_CALL_2)
            {
                mPcmOut = pcm_open(0, 19, PCM_OUT, &mConfig);
            }
            else
            {
                mPcmOut = pcm_open(0, 10, PCM_OUT, &mConfig);
            }
            ASSERT(mPcmOut != NULL);
            ALOGD("%s(), mPcmIn = %p, mPcmOut = %p", __FUNCTION__, mPcmIn, mPcmOut);

            //pcm_start(mPcmIn);
            pcm_start(mPcmOut);
        }
    }
    else
    {
        //close previous device
        if (mPcmIn != NULL)
        {
            pcm_stop(mPcmIn);
            pcm_close(mPcmIn);
            mPcmIn = NULL;
        }

        if (mPcmOut != NULL)
        {
            pcm_stop(mPcmOut);
            pcm_close(mPcmOut);
            mPcmOut = NULL;
        }
        ALOGD("%s(), pcm config", __FUNCTION__);

        if (mAudioMode == AUDIO_MODE_IN_CALL_EXTERNAL || mAudioMode == AUDIO_MODE_IN_CALL_2)
        {
            mConfig.channels = 2;
            mConfig.rate = sample_rate;
            mConfig.period_size = 1024;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            ASSERT(mPcmIn == NULL && mPcmOut == NULL);
            ALOGD("%s(), audio_mode(%d), open ext md", __FUNCTION__, mAudioMode);
            mPcmIn = pcm_open(0, 17, PCM_IN, &mConfig);
            mPcmOut = pcm_open(0, 17, PCM_OUT, &mConfig);
        }
        else
        {

            memset(&mConfig, 0, sizeof(mConfig));
            mConfig.channels = 2;
            mConfig.rate = sample_rate;
            mConfig.period_size = 1024;
            mConfig.period_count = 2;
            mConfig.format = PCM_FORMAT_S16_LE;
            mConfig.start_threshold = 0;
            mConfig.stop_threshold = 0;
            mConfig.silence_threshold = 0;
            ASSERT(mPcmIn == NULL && mPcmOut == NULL);
            ALOGD("%s(), audio_mode(%d), open int md", __FUNCTION__, mAudioMode);
            mPcmIn = pcm_open(0, 2, PCM_IN, &mConfig);
            mPcmOut = pcm_open(0, 2, PCM_OUT, &mConfig);
            ASSERT(mPcmIn != NULL && mPcmOut != NULL);
            ALOGD("%s(), mPcmIn = %p, mPcmOut = %p", __FUNCTION__, mPcmIn, mPcmOut);
        }
        pcm_start(mPcmIn);
        pcm_start(mPcmOut);
    }

    // Set new device
    if (checkTtyNeedOn() == true)
    {
        setTtyInOutDevice(getRoutingForTty());
    }
    else
    {
        // Set PMIC digital/analog part - uplink has pop, open first
        mHardwareResourceManager->startInputDevice(input_device);

        // Set PMIC digital/analog part - DL need trim code.
        mHardwareResourceManager->startOutputDevice(output_device, sample_rate);
    }

    // start Side Tone Filter
    if (checkSideToneFilterNeedOn(output_device) == true)
    {
        mHardwareResourceManager->EnableSideToneFilter(true);
    }


    // Set MODEM_PCM - open modem pcm here s.t. modem/DSP can learn the uplink background noise, but not zero
    //SetModemPcmAttribute(modem_index, sample_rate);
    //mAudioDigitalInstance->SetModemPcmEnable(modem_index, true);



    // Set MD side sampling rate
    pSpeechDriver->SetModemSideSamplingRate(sample_rate);

    // Set speech mode
    if (checkTtyNeedOn() == true)
    {
        pSpeechDriver->SetSpeechMode(mHardwareResourceManager->getInputDevice(), mHardwareResourceManager->getOutputDevice());
    }
    else
    {
        pSpeechDriver->SetSpeechMode(input_device, output_device);
    }

    // Need recover mute state
    pSpeechDriver->SetUplinkMute(mMicMute);
    pSpeechDriver->SetDownlinkMute(false);

    ALOGD("-%s(), output_device = 0x%x, input_device = 0x%x", __FUNCTION__, mHardwareResourceManager->getOutputDevice(), mHardwareResourceManager->getInputDevice());
    return NO_ERROR;
}

status_t AudioALSASpeechPhoneCallController::setTtyMode(const tty_mode_t tty_mode)
{
    ALOGD("+%s(), mTtyMode = %d, new tty mode = %d", __FUNCTION__, mTtyMode, tty_mode);

#ifdef MTK_TTY_SUPPORT
    if (mTtyMode != tty_mode)
    {
        mTtyMode = tty_mode;

        SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();
        if (pSpeechDriver->GetApSideModemStatus(VT_STATUS_MASK) == false &&
            pSpeechDriver->GetApSideModemStatus(SPEECH_STATUS_MASK) == true)
        {
            pSpeechDriver->SetUplinkMute(true);
            if (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == true)
            {
                pSpeechDriver->TtyCtmOff();
            }
            audio_devices_t output_devices = getRoutingForTty();//"NG:mHardwareResourceManager->getOutputDevice()->HCO->off use main mic
            routing(output_devices, getInputDeviceForPhoneCall(output_devices));
            //            mHardwareResourceManager->stopOutputDevice();
            //            mHardwareResourceManager->stopInputDevice(mHardwareResourceManager->getInputDevice());
            //            setTtyInOutDevice(getRoutingForTty());
            //
            //            const audio_devices_t output_device = (audio_devices_t)mHardwareResourceManager->getOutputDevice();
            //            const audio_devices_t input_device  = (audio_devices_t)mHardwareResourceManager->getInputDevice();
            //            pSpeechDriver->SetSpeechMode(input_device, output_device);

            if ((mTtyMode != AUD_TTY_OFF) && (mTtyMode != AUD_TTY_ERR) &&
                (pSpeechDriver->GetApSideModemStatus(TTY_STATUS_MASK) == false))
            {
                pSpeechDriver->TtyCtmOn(BAUDOT_MODE);
            }
            pSpeechDriver->SetUplinkMute(mMicMute);

        }
    }
#endif

    ALOGD("-%s(), mTtyMode = %d", __FUNCTION__, mTtyMode);
    return NO_ERROR;
}

void AudioALSASpeechPhoneCallController::setTtyInOutDevice(audio_devices_t routing_device)
{
    ALOGD("+%s(), routing_device = 0x%x, mTtyMode = %d", __FUNCTION__, routing_device, mTtyMode);

#ifdef MTK_TTY_SUPPORT
    if (mTtyMode == AUD_TTY_OFF)
    {
        mHardwareResourceManager->startOutputDevice(routing_device, 16000);
        mHardwareResourceManager->startInputDevice(getInputDeviceForPhoneCall(routing_device));
    }
    else
    {
        if (routing_device == 0)
        {
            mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_DEFAULT, 16000);
            mHardwareResourceManager->startInputDevice(getInputDeviceForPhoneCall(AUDIO_DEVICE_OUT_DEFAULT));
        }
        else if (routing_device & AUDIO_DEVICE_OUT_SPEAKER)
        {
            if (mTtyMode == AUD_TTY_VCO)
            {
                ALOGD("%s(), speaker, TTY_VCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 16000);
#else
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET, 16000);
#endif
                mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
                mAudioALSAVolumeController->ApplyMicGain(Handfree_Mic, mAudioMode);
            }
            else if (mTtyMode == AUD_TTY_HCO)
            {
                ALOGD("%s(), speaker, TTY_HCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 16000);
#else
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_SPEAKER, 16000);
#endif
                mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, mAudioMode);
            }
            else if (mTtyMode == AUD_TTY_FULL)
            {
                ALOGD("%s(), speaker, TTY_FULL", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 16000);
#else
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET, 16000);
#endif
                mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, mAudioMode);
            }
        }
        else if ((routing_device == AUDIO_DEVICE_OUT_WIRED_HEADSET) ||
                 (routing_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
        {
            if (mTtyMode == AUD_TTY_VCO)
            {
                ALOGD("%s(), headset, TTY_VCO", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 16000);
#else
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET, 16000);
#endif
                mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
                mAudioALSAVolumeController->ApplyMicGain(Normal_Mic, mAudioMode);
            }
            else if (mTtyMode == AUD_TTY_HCO)
            {
                ALOGD("%s(), headset, TTY_HCO", __FUNCTION__);
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 16000);
                mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, mAudioMode);
            }
            else if (mTtyMode == AUD_TTY_FULL)
            {
                ALOGD("%s(), headset, TTY_FULL", __FUNCTION__);
#if defined(ENABLE_EXT_DAC) || defined(ALL_USING_VOICEBUFFER_INCALL)
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 16000);
#else
                mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET, 16000);
#endif
                mHardwareResourceManager->startInputDevice(AUDIO_DEVICE_IN_WIRED_HEADSET);
                mAudioALSAVolumeController->ApplyMicGain(TTY_CTM_Mic, mAudioMode);
            }
        }
        else if (routing_device == AUDIO_DEVICE_OUT_EARPIECE)
        {
            // tty device is removed. TtyCtm already off in CloseMD.
            mHardwareResourceManager->startOutputDevice(routing_device, 16000);
            mHardwareResourceManager->startInputDevice(getInputDeviceForPhoneCall(routing_device));
            ALOGD("%s(), receiver", __FUNCTION__);
        }
        else
        {
            mHardwareResourceManager->startOutputDevice(routing_device, 16000);
            mHardwareResourceManager->startInputDevice(getInputDeviceForPhoneCall(routing_device));
            ALOGD("%s(), routing = 0x%x", __FUNCTION__, routing_device);
        }
    }


#endif

    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSASpeechPhoneCallController::setVtNeedOn(const bool vt_on)
{
    ALOGD("%s(), new vt_on = %d, old mVtNeedOn = %d", __FUNCTION__, vt_on, mVtNeedOn);
    AudioAutoTimeoutLock _l(mLock);

    mVtNeedOn = vt_on;
}

void AudioALSASpeechPhoneCallController::setMicMute(const bool mute_on)
{
    ALOGD("%s(), new mute_on = %d, old mMicMute = %d", __FUNCTION__, mute_on, mMicMute);
    AudioAutoTimeoutLock _l(mLock);

    mSpeechDriverFactory->GetSpeechDriver()->SetUplinkMute(mute_on);

    property_set(PROPERTY_KEY_MIC_MUTE_ON, (mute_on == false) ? "0" : "1");
    mMicMute = mute_on;
}

void AudioALSASpeechPhoneCallController::setDlMute(const bool mute_on)
{

    ALOGD("%s(), new mute_on = %d, old mDlMute = %d", __FUNCTION__, mute_on, mDlMute);
    AudioAutoTimeoutLock _l(mLock);

    mSpeechDriverFactory->GetSpeechDriver()->SetDownlinkMute(mute_on);

    property_set(PROPERTY_KEY_DL_MUTE_ON, (mute_on == false) ? "0" : "1");
    mDlMute = mute_on;
}

void AudioALSASpeechPhoneCallController::setUlMute(const bool mute_on)
{
    ALOGD("+%s(), new mute_on = %d, old mULMute = %d", __FUNCTION__, mute_on, mUlMute);
    AudioAutoTimeoutLock _l(mLock);

    mSpeechDriverFactory->GetSpeechDriver()->SetUplinkSourceMute(mute_on);

    property_set(PROPERTY_KEY_UL_MUTE_ON, (mute_on == false) ? "0" : "1");
    mMicMute = mute_on;
}

void AudioALSASpeechPhoneCallController::setBTMode(const int mode)
{
    ALOGD("%s(), mBTMode %d => %d", __FUNCTION__, mBTMode, mode);
    AudioAutoTimeoutLock _l(mLock);
    mBTMode = mode;
}

void AudioALSASpeechPhoneCallController::getRFInfo()
{
    WARNING("Not implement yet!!");
}

} // end of namespace android
