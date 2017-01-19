#ifndef ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
#define ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H

#include <tinyalsa/asoundlib.h>

#include "AudioType.h"
#include "AudioLock.h"
#include "AudioUtility.h"

namespace android
{

typedef enum
{
    AUDIO_SPEAKER_MODE_D = 0,
    AUDIO_SPEAKER_MODE_AB = 1,

} AUDIO_SPEAKER_MODE;


enum builtin_mic_specific_type
{
    BUILTIN_MIC_DEFAULT,
    BUILTIN_MIC_MIC1_ONLY,
    BUILTIN_MIC_MIC2_ONLY,
    BUILTIN_MIC_MIC3_ONLY,
};



class AudioALSAHardwareResourceManager
{
    public:
        virtual ~AudioALSAHardwareResourceManager();
        static AudioALSAHardwareResourceManager *getInstance();


        /**
         * output devices
         */
        virtual status_t setOutputDevice(const audio_devices_t new_devices, const uint32_t sample_rate);
        virtual status_t startOutputDevice(const audio_devices_t new_devices, const uint32_t SampleRate);
        virtual status_t stopOutputDevice();
        virtual status_t changeOutputDevice(const audio_devices_t new_devices);
        virtual audio_devices_t getOutputDevice();


        /**
         * input devices
         */
        virtual status_t setInputDevice(const audio_devices_t new_device);
        virtual status_t startInputDevice(const audio_devices_t new_device);
        virtual status_t stopInputDevice(const audio_devices_t stop_device);
        virtual status_t changeInputDevice(const audio_devices_t new_device);
        virtual audio_devices_t getInputDevice();


        /**
         * HW Gain2
         */
        virtual status_t setHWGain2DigitalGain(const uint32_t gain);


        /**
         * Interrupt Rate
         */
        virtual status_t setInterruptRate(const uint32_t rate);


        /**
         * sgen
         */
        virtual status_t setSgenMode(const sgen_mode_t sgen_mode);
        virtual status_t setSgenSampleRate(const sgen_mode_samplerate_t sample_rate);
        virtual status_t openAddaOutput(const uint32_t sample_rate);
        virtual status_t closeAddaOutput();

        /**
        * sidetone
        */
        virtual status_t EnableSideToneFilter(const bool enable);
        /**
        *   Current Sensing
        */
        virtual status_t setSPKCurrentSensor(bool bSwitch);

        virtual status_t setSPKCurrentSensorPeakDetectorReset(bool bSwitch);

        /**
         * MIC inverse
         */
        virtual status_t setMicInverse(bool bMicInverse);
        virtual bool getMicInverse(void);

        virtual void setMIC1Mode(bool isphonemic);
        virtual void setMIC2Mode(bool isphonemic);

        /**
         * build in mic specific type
         */
        virtual void     setBuiltInMicSpecificType(const builtin_mic_specific_type type) { mBuiltInMicSpecificType = type; }

        /**
        * build in mic specific type
        */
        virtual void     EnableAudBufClk(bool bEanble);


        /**
         * Headphone Change information
         */
        virtual void     setHeadPhoneChange(bool bchange) { mHeadchange = bchange; }
        virtual bool     getHeadPhoneChange(void) { return mHeadchange ; }


        /**
         * debug dump register & DAC I2S Sgen
         */
        virtual void     setAudioDebug(const bool enable);



    protected:
        AudioALSAHardwareResourceManager();



    private:
        /**
         * singleton pattern
         */
        static AudioALSAHardwareResourceManager *mAudioALSAHardwareResourceManager;

        status_t setMicType(void);
        void setDMICSwap(bool swap);
        status_t SetExtDacGpioEnable(bool bEnable);
        bool GetExtDacPropertyEnable(void);

        AudioLock mLock;

        struct mixer *mMixer;
        struct pcm *mPcmDL;

        audio_devices_t mOutputDevices;
        audio_devices_t mInputDevice;

        uint32_t        mOutputDeviceSampleRate;
        uint32_t        mInputDeviceSampleRate;

        bool mIsChangingOutputDevice;
        bool mIsChangingInputDevice;

        uint32_t mStartOutputDevicesCount;
        uint32_t mStartInputDeviceCount;

        bool mMicInverse;
        builtin_mic_specific_type mBuiltInMicSpecificType;

        bool mHeadchange;
        uint32_t mPhoneMicMode;
        uint32_t mHeadsetMicMode;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_HARDWARE_RESOURCE_MANAGER_H
