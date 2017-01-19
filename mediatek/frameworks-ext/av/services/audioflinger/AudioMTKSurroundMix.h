
#ifndef ANDROID_AUDIO_MTK_SURROUNDMIX_H
#define ANDROID_AUDIO_MTK_SURROUNDMIX_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/threads.h>

#include <media/AudioBufferProvider.h>
#include "AudioResampler.h"

#include <audio_effects/effect_downmix.h>
#include <system/audio.h>

#include <media/nbaio/NBLog.h>
extern "C"
{
#include "BesSurround_api.h"
#include "BesSurround_audio.h"
#include"BesSurround_setting.h"
}

namespace android {

// ----------------------------------------------------------------------------
typedef void     BesSur_HANDLE;

class AudioMTKSurroundDownMix
{
public:
	
	AudioMTKSurroundDownMix() ;
	~AudioMTKSurroundDownMix();
	status_t Init(int trackName, int sessionID, int channelMask, int sampleRate );
	status_t process(int32_t* inBuffer, int32_t* ouBuffer, int32_t numFrames);
	status_t ResetBesSurround();
	status_t SetBesSurroundOnOFF(int value);
	status_t SetBesSurroundMode(int value);
    	status_t DownMix_Init(int trackName, int sessionID, int channelMask, int sampleRate);
	status_t  SetBesSurroundDevice(audio_devices_t Device);	
    	status_t SetBesSurroundOnOffFlag(int value);
	status_t Qurey_Channel_Support( unsigned int channel_mask);
         status_t BesSurround_Qurey_Channel_Support( unsigned int channel_mask);
         
         void volumeRampStereo(int32_t* out, size_t frameCount, int32_t* temp, int* UpDown);
	bool IsBesSurroundInitialized;
	int mDownmixRamp; // rampup : 1, rampdown 2, normal 0
	int* mTempBuf;
    
        int mSurroundRamp; // 
	class BesSurround{
		public:
			BesSurround();
			~BesSurround();
			
			status_t BesSurround_reinit( int channelMask, int sampleRate);
			status_t BesSurround_info_initialize(MTK_VS_INFO *vs_info, int channel_mask, int sample_rate, int bits_per_sample, uint32_t device);	
			status_t BesSurround_init( int channelMask, int sampleRate);
			status_t BesSurround_process( int32_t* inBuffer, int32_t* ouBuffer, int32_t numFrames);
			status_t BesSurround_ramp_up(void);
			status_t BesSurround_ramp_down(void);
			int BesSurround_GetRampStatus( void);
			status_t BesSurround_SetMode( int value);
			status_t BesSurround_Reset( );
			void BesSurround_Close();
                            status_t BesSurround_dumpInfo( );
			//status_t BesSurround_Qurey_Channel_Support( unsigned int channel_mask);
			uint32_t mMode;
			bool mEnable;
			uint32_t mDevice;
		private:
			
			// BesSurround related
			int8_t *mBesSurround_hdl;
			MTK_VS_INFO *mBesSurround_info;
			char* mBesTempbuf;
			uint32_t mSampleRate;
			uint32_t mMaxNumCh;
			uint32_t mframeCount;
			uint32_t mBitsPerSample;
		};
	
	
	BesSurround* mBesSurround;
	static effect_descriptor_t dwnmDesc;
	static bool                isAndroidMultichannelCapable;
	uint32_t mTrackName;
	uint32_t mSessionId;
	uint32_t mChannelMask;
	uint32_t mDevice;
	uint32_t mSampleRate;
	uint32_t mMaxNumCh;
	uint32_t mframeCount;
	bool mBesSurround_enable;
	bool mDownmix_enable;
	uint32_t mBesSurroundMode;
	//effect_uuid_t mDwmxUUID;
	effect_handle_t mDownmixHandle;
	effect_config_t    mDownmixConfig;

};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_AUDIO_MTK_SURROUNDMIX_H
