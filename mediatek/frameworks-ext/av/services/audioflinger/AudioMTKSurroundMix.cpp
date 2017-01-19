#define LOG_TAG "AudioMTKSurroundDownMix"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Log.h>

#include <cutils/bitops.h>
#include <cutils/compiler.h>
#include <utils/Debug.h>

#include <system/audio.h>

#include <media/EffectsFactoryApi.h>


#include <cutils/xlog.h>

#include <media/AudioSystem.h>
#include <AudioPolicyParameters.h>

#include "AudioMTKHardwareCommand.h"

#include "AudioMTKSurroundMix.h"

#define DEBUG_MIXER_PCM
#ifdef DEBUG_MIXER_PCM
#include "AudioUtilmtk.h"
#endif

#define MTK_ALOG_V(fmt, arg...) SXLOGV(fmt, ##arg)
#define MTK_ALOG_D(fmt, arg...) SXLOGD(fmt, ##arg)
#define MTK_ALOG_W(fmt, arg...) SXLOGW(fmt, ##arg)
#define MTK_ALOG_E(fmt, arg...) SXLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#undef  ALOGV
#define ALOGV   MTK_ALOG_V

#ifdef DEBUG_MIXER_PCM
static   const char * gaf_mixer_before_downmix_pcm = "/sdcard/mtklog/audio_dump/mixer_dwnmx_before";
static   const char * gaf_mixer_after_downmix_pcm  = "/sdcard/mtklog/audio_dump/mixer_dwnmx_after";
static   const char * gaf_mixer_downmix_propty     = "af.mixer.dwnmx.pcm";

#define MixerDumpPcm(name, propty, tid, value, buffer, size) \
{\
  char fileName[256]; \
  sprintf(fileName,"%s_%d_%d.pcm", name, tid, value); \
  AudioDump::dump(fileName, buffer, size, propty); \
}
#else
#define MixerDumpPcm(name, propty, tid, value, buffer, size)
#endif

namespace android {
	effect_descriptor_t AudioMTKSurroundDownMix::dwnmDesc;
	bool AudioMTKSurroundDownMix::isAndroidMultichannelCapable = false;

AudioMTKSurroundDownMix :: AudioMTKSurroundDownMix() : 
	mDownmixHandle(NULL),mBesSurround_enable(0),mBesSurround(NULL),mBesSurroundMode(0),mDevice(0),
        IsBesSurroundInitialized(0), mDownmix_enable(0), mTempBuf(NULL)
{
                // New Bessourround class;
                mBesSurround = new BesSurround();
		// find multichannel downmix effect if we have to play multichannel content
		uint32_t numEffects = 0;
		int ret = EffectQueryNumberEffects(&numEffects);
		if (ret != 0) {
			ALOGE("AudioMTKMixer() error %d querying number of effects", ret);
			return;
		}
		ALOGV("EffectQueryNumberEffects() numEffects=%d", numEffects);
		
		for (uint32_t i = 0 ; i < numEffects ; i++) {
			if (EffectQueryEffect(i, &dwnmDesc) == 0) {
				ALOGV("effect %d is called %s", i, dwnmDesc.name);
				if (memcmp(&dwnmDesc.type, EFFECT_UIID_DOWNMIX, sizeof(effect_uuid_t)) == 0) {
					ALOGI("found effect \"%s\" from %s",
							dwnmDesc.name, dwnmDesc.implementor);
					isAndroidMultichannelCapable = true;
					break;
				}
			}
		}
        
// initial ramp status
        mSurroundRamp = RAMP_NORMAL;
}
    
    status_t AudioMTKSurroundDownMix :: DownMix_Init(int trackName, int sessionID, int channelMask, int sampleRate)
    {
        
            // Init android downmix.    
       ALOGD("DownMix_Init");     
       if(!isAndroidMultichannelCapable)
       {
        goto DownMixInitFail;
       }
       if (EffectCreate(&(dwnmDesc.uuid),
               sessionID /*sessionId*/, -2 /*ioId not relevant here, using random value*/,
               &mDownmixHandle/*pHandle*/) != 0) {
           ALOGE("prepareTrackForDownmix(%d) fails: error creating downmixer effect", trackName);
           goto DownMixInitFail;
       }
       //mMaxNumCh = pop_count(pTrack->channelMask);
          // channel input configuration will be overridden per-track
       mDownmixConfig.inputCfg.channels = channelMask;
       mDownmixConfig.outputCfg.channels = AUDIO_CHANNEL_OUT_STEREO;
       mDownmixConfig.inputCfg.format = AUDIO_FORMAT_PCM_32_BIT;
       mDownmixConfig.outputCfg.format = AUDIO_FORMAT_PCM_32_BIT;
       mDownmixConfig.inputCfg.samplingRate = sampleRate;
       mDownmixConfig.outputCfg.samplingRate = sampleRate;
       mDownmixConfig.inputCfg.accessMode = EFFECT_BUFFER_ACCESS_READ;
       mDownmixConfig.outputCfg.accessMode = EFFECT_BUFFER_ACCESS_WRITE;
       // input and output buffer provider, and frame count will not be used as the downmix effect
       // process() function is called directly (see DownmixerBufferProvider::getNextBuffer())
       mDownmixConfig.inputCfg.mask = EFFECT_CONFIG_SMP_RATE | EFFECT_CONFIG_CHANNELS |
               EFFECT_CONFIG_FORMAT | EFFECT_CONFIG_ACC_MODE;
       mDownmixConfig.outputCfg.mask = mDownmixConfig.inputCfg.mask;
    
           {// scope for local variables that are not used in goto label "noDownmixForActiveTrack"
                int cmdStatus;
                uint32_t replySize = sizeof(int);
                status_t status;
                // Configure and enable downmixer
                status = (*mDownmixHandle)->command(mDownmixHandle,
                        EFFECT_CMD_SET_CONFIG /*cmdCode*/, sizeof(effect_config_t) /*cmdSize*/,
                        &mDownmixConfig /*pCmdData*/,
                        &replySize /*replySize*/, &cmdStatus /*pReplyData*/);
                if ((status != 0) || (cmdStatus != 0)) {
                    ALOGE("error %d while configuring downmixer for track %d", status, trackName);
                    goto DownMixInitFail;
                }
                replySize = sizeof(int);
                status = (*mDownmixHandle)->command(mDownmixHandle,
                        EFFECT_CMD_ENABLE /*cmdCode*/, 0 /*cmdSize*/, NULL /*pCmdData*/,
                        &replySize /*replySize*/, &cmdStatus /*pReplyData*/);
                if ((status != 0) || (cmdStatus != 0)) {
                    ALOGE("error %d while enabling downmixer for track %d", status, trackName);
                    goto DownMixInitFail;
                }
    
                // Set downmix type
                // parameter size rounded for padding on 32bit boundary
                const int psizePadded = ((sizeof(downmix_params_t) - 1)/sizeof(int) + 1) * sizeof(int);
                const int downmixParamSize =
                        sizeof(effect_param_t) + psizePadded + sizeof(downmix_type_t);
                effect_param_t * const param = (effect_param_t *) malloc(downmixParamSize);
                param->psize = sizeof(downmix_params_t);
                const downmix_params_t downmixParam = DOWNMIX_PARAM_TYPE;
                memcpy(param->data, &downmixParam, param->psize);
                const downmix_type_t downmixType = DOWNMIX_TYPE_FOLD;
                param->vsize = sizeof(downmix_type_t);
                memcpy(param->data + psizePadded, &downmixType, param->vsize);
    
                status = (*mDownmixHandle)->command(mDownmixHandle,
                        EFFECT_CMD_SET_PARAM /* cmdCode */, downmixParamSize/* cmdSize */,
                        param /*pCmndData*/, &replySize /*replySize*/, &cmdStatus /*pReplyData*/);
    
                free(param);
    
                if ((status != 0) || (cmdStatus != 0)) {
                    ALOGE("error %d while setting downmix type for track %d", status, trackName);
                    goto DownMixInitFail;
                } else {
                    ALOGV("downmix type set to %d for track %d", (int) downmixType, trackName);
                }
            }
    
       return 0;
    
    DownMixInitFail:
       return -1; 
    }
    status_t AudioMTKSurroundDownMix ::Init(int trackName, int sessionID, int channelMask, int sampleRate )
    {
        // check if Besurround is enabled.
           int32_t status;
           mTrackName = trackName;
           mSessionId = sessionID;
           mChannelMask = channelMask;
           mSampleRate = sampleRate;
           
           if(mBesSurround_enable)
           {
                ALOGV("BesSurround enable");
                int temp_channelmask = mChannelMask;
                if(Qurey_Channel_Support(mChannelMask) == FAIL)
                {
                    temp_channelmask = AUDIO_CHANNEL_OUT_STEREO;
                    mDownmix_enable = 1;
                }
                if(mBesSurround->BesSurround_init( temp_channelmask,mSampleRate) == OK)
                {
                    IsBesSurroundInitialized = 1;
                    // set bessurround mode here , in case track  does not has new mode when init.
                    mBesSurround->BesSurround_SetMode( mBesSurroundMode);
                    if(mSurroundRamp != RAMP_DOWN){
                    mDownmixRamp = RAMP_DOWN; //ramp_dowon;
                        }
                   /* if( !mDownmix_enable)
                    {
                        return OK;
                        }*/
                        }
                }
           else{            
            mDownmix_enable  = 1;
           }

           // Besurround is not enabled or fail Init.
           ALOGD(" AudioMTKSurroundDownMix ::Init DownMix_Init");
           if(channelMask == AUDIO_CHANNEL_OUT_MONO)
           {
            // Resampler will turn mono to stereo
                mChannelMask = AUDIO_CHANNEL_OUT_STEREO;                
                ALOGV(" AudioMTKSurroundDownMix ::Init DownMix_Init, set Channel mask mono to stereo");
           }
           if(DownMix_Init( mTrackName,  mSessionId,  mChannelMask,  mSampleRate) == OK)
            {
                return OK;
            }

            mDownmix_enable  = 0;
           return -1; 
    }
    
    AudioMTKSurroundDownMix :: ~AudioMTKSurroundDownMix()
    {
    
        ALOGV("AudioMTKSurroundDownMix deleting Downmixer Effect (%p)", this);
        EffectRelease(mDownmixHandle);
        if(mBesSurround !=NULL)
        {  delete mBesSurround;}
        if(mTempBuf !=NULL)
        {
            delete [] mTempBuf;
        }
    }
    
    
    void AudioMTKSurroundDownMix ::volumeRampStereo(int32_t* out, size_t frameCount, int32_t* temp, int* UpDown)
    {
        int32_t vl ;
        int32_t vlInc ;
        vlInc = 65536/frameCount;
        if(*UpDown == RAMP_UP)
        {// ramp up
            vl = 0;
           * UpDown = RAMP_NORMAL;
        }
        else if(*UpDown == RAMP_DOWN)
        { // ramp down
             vl = 65536;
            vlInc = -vlInc;
            * UpDown = RAMP_UP;
        }
        else{
                return;
            }
        ALOGD("ramp %d , vl %d, vlInc %d", *UpDown, vl, vlInc );    
        // ramp volume
            do {
                        *out++ = (int32_t) (((int64_t)vl * (*temp++)>> 16));                        
                        *out++ = (int32_t) (((int64_t)vl * (*temp++)>> 16));
                vl += vlInc;
            } while (--frameCount);
    }
    status_t AudioMTKSurroundDownMix:: process(int32_t* inBuffer, int32_t* ouBuffer, int32_t numFrames)
    {
            int32_t tempChannelMask =  mChannelMask;
            MixerDumpPcm(gaf_mixer_before_downmix_pcm, gaf_mixer_downmix_propty, gettid(), (int)this, (void*)inBuffer,popcount(mChannelMask)*4* numFrames);
            ALOGV("dump%d, channel count %d, numFrameCount %d", popcount(mChannelMask)*4* numFrames,popcount(mChannelMask), numFrames);
            ALOGV(" mDownmixRamp %d,mSurroundRamp %d",mDownmixRamp,mSurroundRamp);

            ////////////////BesSurround Not initialized, or OFF /////////////////////////////            
            // first condition: need downmix
            // second condition : downmix is ending and start to ramp down. 
            if( mDownmix_enable || (!mDownmix_enable  && mDownmixRamp == RAMP_DOWN))
            {
                ALOGV(" mDownmix_enable %d",mDownmix_enable);
                    if(mDownmixHandle ==NULL)
                    {
                        ALOGE("AudioMTKSurroundDownMix::process() error: NULL mDownmixHandle");
                        goto BesSurroundProcess;
                    } else {
                    ///////////////////////////////////////////////////////////////
                    // set mode for channel count not support => status is ramp down, surround is initialized,  
                    //                                                                            mDownmix_enable ==1, mBesSurround_enable == 1
                    //                   => can't go to surround directly.
                    // set off for channel count supported => status = ramp down, surround is initialized, 
                    //                                                                => mDownmix_enable ==1,mBesSurround_enable == 0
                    //                       => can't go to surround directly.
                    // Bessuruuound is ramp down => go to bessurround directly.
                    ///////////////////////////////////////////////////////////////
                    if(                       
                            mSurroundRamp == RAMP_DOWN
                        && IsBesSurroundInitialized )
                    {
                        if(Qurey_Channel_Support( mChannelMask) == SUCCESS)
                        {
                            ALOGV(" goto BesSurroundProcess");
                            goto BesSurroundProcess;
                        }
                    }
                     /// need replace buffer
                     // 1. downmix is needed and bessurround is on , and is not in ramp down status
                     // 2. downmix is needed and bessurround is off but in ramp down status
                    if(mDownmix_enable && ((mBesSurround_enable && mDownmixRamp != RAMP_DOWN ) || (!mBesSurround_enable &&  mSurroundRamp == RAMP_DOWN)) )
                    {
                        if(mTempBuf ==NULL)
                        {
                            mTempBuf = new int[numFrames*sizeof(int)*2];
                        }
                        mDownmixConfig.outputCfg.buffer.raw = mTempBuf;
                        ALOGV("AudioMTKSurroundDownMix use Temp buf as downmix outbuf");
                    }
                    else
                    {
                        mDownmixConfig.outputCfg.buffer.raw = ouBuffer;
                    }
                     ALOGV("AudioMTKSurroundDownMix:: process, android downmixer");
                     int res;
                     mDownmixConfig.inputCfg.buffer.frameCount = numFrames;
                     mDownmixConfig.inputCfg.buffer.raw = inBuffer;
                     mDownmixConfig.outputCfg.buffer.frameCount = numFrames;
                     
                	if ( popcount(mChannelMask) == 2 )
                	{
                        memcpy( mDownmixConfig.outputCfg.buffer.raw, mDownmixConfig.inputCfg.buffer.raw,
                          popcount(mChannelMask)*4* numFrames );
                    }
                    else
	                {    
                         res = (*mDownmixHandle)->process(mDownmixHandle,
                             &mDownmixConfig.inputCfg.buffer, &mDownmixConfig.outputCfg.buffer);

                         ALOGV("downmix res %d", res);
                    } 
                    
                     // if surround is ramp down go to surround
                     // if surround is on but  downmix is ramp down , don't go surround.
                     if((mBesSurround_enable && mDownmixRamp != RAMP_DOWN) ||mSurroundRamp == RAMP_DOWN )
                     {
                        if(mDownmix_enable){
                            ALOGV("keep processing surround, and change input buffer");
                            tempChannelMask = AUDIO_CHANNEL_OUT_STEREO;
                            inBuffer = mTempBuf;
                            }
                        goto BesSurroundProcess;
                     }
                     
                     //MixerDumpPcm(gaf_mixer_before_downmix_pcm, gaf_mixer_downmix_propty, gettid(), (int)this,mDownmixConfig.outputCfg.buffer.s32,  popcount(mDownmixConfig.outputCfg.channels)*sizeof(int)* numFrames);
                     volumeRampStereo(mDownmixConfig.outputCfg.buffer.s32, numFrames, mDownmixConfig.outputCfg.buffer.s32, &mDownmixRamp);
                     volumeRampStereo(mDownmixConfig.outputCfg.buffer.s32, numFrames, mDownmixConfig.outputCfg.buffer.s32, &mSurroundRamp);
                     MixerDumpPcm(gaf_mixer_after_downmix_pcm, gaf_mixer_downmix_propty, gettid(), (int)this, (void*)mDownmixConfig.outputCfg.buffer.s32, popcount(mDownmixConfig.outputCfg.channels)*sizeof(int)* numFrames);
                    return res;
                }
             }

            BesSurroundProcess:
            ///////////turn ON Bessurround ///////////////////////////
            if(IsBesSurroundInitialized != 0 )
            { 
                // ALOGD("AudioMTKSurroundDownMix:: process IsBesSurroundInitialized");
                //ALOGD("ramp status %d ",mBesSurround->BesSurround_GetRampStatus());
                ALOGD("mBesSurround_enable %d",mBesSurround_enable);
                   // process when enabled, or ramp down phase before disabled.
                   //ALOGD("set mode %d original %d",mBesSurroundMode, mBesSurround->mMode);
                    mBesSurround->mEnable = mBesSurround_enable;
                    if( mBesSurround->mMode != mBesSurroundMode)
                    {
                        // for set mode , wait till ramp status is RAMP_NORMAL to set mode
                        // Set Mode only when BeSurround is enabled.
                        ALOGD("set mode %d original %d",mBesSurroundMode, mBesSurround->mMode);
                        if(mBesSurround_enable && 
                            (mSurroundRamp == RAMP_UP || mDownmixRamp == RAMP_UP)
                            )
                        { 
                            mBesSurround->BesSurround_SetMode( mBesSurroundMode);
                            //mBesSurround->BesSurround_ramp_up();                            
                            mBesSurround->mMode = mBesSurroundMode;                            
                            mBesSurround->BesSurround_dumpInfo();
                        }
                    }
                                        
                    mBesSurround->BesSurround_process( inBuffer, ouBuffer,  popcount(tempChannelMask) * numFrames);
                    
                    //MixerDumpPcm(gaf_mixer_before_downmix_pcm, gaf_mixer_downmix_propty, gettid(), (int)this, ouBuffer,2*4* numFrames);

                    volumeRampStereo(ouBuffer, numFrames,ouBuffer, &mSurroundRamp);
                    volumeRampStereo(ouBuffer, numFrames,ouBuffer ,&mDownmixRamp);

                    MixerDumpPcm(gaf_mixer_after_downmix_pcm, gaf_mixer_downmix_propty, gettid(), (int)this, (void*)ouBuffer, 2*4* numFrames);
                    return OK;
            }
            return -1;
    }

    status_t AudioMTKSurroundDownMix:: SetBesSurroundOnOFF(int value)
    {
        if (value != 1 && value != 0)
        {
            ALOGE("SetBesSurroundOnOFF BAD_VALUE %d", value);   
            return BAD_VALUE;
        }
        
        //value = 1;
        MTK_ALOG_V("value %d mBesSurround_enable %d", value,mBesSurround_enable);
        if(mBesSurround_enable == value)
        {
            return OK;
        }
        
        mBesSurround_enable = value;
        ALOGD("SetBesSurroundOnOFF %d", mBesSurround_enable);
        
        if(mDownmixHandle ==NULL && IsBesSurroundInitialized == 0 )
        {   // both NULL, track just started. let AudioMTKSurroundMIx do INIT.
            return OK;
        }

        if ( mBesSurround_enable == 1)
        {
            // initially use another dowmixer.
            if(IsBesSurroundInitialized == 0 )
            {
                int temp_mChannelMask = mChannelMask;
                ALOGD("SetBesSurroundOnOFF start BesSurround");
                if(Qurey_Channel_Support(mChannelMask) == FAIL)
                {
                    ALOGD("channmask 0x%x not supported",mChannelMask);
                    temp_mChannelMask = AUDIO_CHANNEL_OUT_STEREO;
                    mDownmix_enable = 1;
                }
                else
                {
                    ALOGD("channmask 0x%x  supported",mChannelMask);
                    mDownmix_enable = 0;
                }

                if(mBesSurround->BesSurround_init(temp_mChannelMask,mSampleRate) == OK)
                {
                    IsBesSurroundInitialized = 1;
                    //mBesSurround->BesSurround_Reset();
                }
            }
            else
            {
               if(Qurey_Channel_Support(mChannelMask) == SUCCESS )
                {
                    ALOGD("channmask 0x%x  supported",mChannelMask);
                    mDownmix_enable = 0;
                }
              
            }
            if(IsBesSurroundInitialized){
            //mBesSurround->BesSurround_ramp_up();
            mDownmixRamp = RAMP_DOWN; //ramp_dowon;
                }
        }
        
        if( mBesSurround_enable == 0 || (mBesSurround_enable == 1 && IsBesSurroundInitialized == 0))
        {
            ALOGD("SetBesSurroundOFF and BesSurround init fail ");
            // BesSurround already in use, ramp Down.
             if(IsBesSurroundInitialized != 0) 
             {
                        mSurroundRamp = RAMP_DOWN;
                }
                else{
                        mBesSurround_enable= 0;
                    }
                mDownmix_enable = 1;
                // Check if ordinary downmix is ready or not
                if(mDownmixHandle == NULL)
                {
                    if(DownMix_Init( mTrackName,  mSessionId,  mChannelMask,  mSampleRate) == OK)
                     {
                         return OK;
                     }
                    else{
                            ALOGE("Init ordinary downmix fail");
                        return NO_INIT;
                        }
                }
        }
        mBesSurround->BesSurround_dumpInfo();
        return OK;
    }
    
    status_t AudioMTKSurroundDownMix:: SetBesSurroundMode(int value)
    {
        MTK_ALOG_V("SetBesSurroundMode value %d", value);
        if (value !=0 && value !=1)
        {
            return BAD_VALUE;
        }
        if( IsBesSurroundInitialized == 0)
        {
            return NO_INIT;
        }
        if(mBesSurroundMode != value &&mBesSurround_enable == 1)
        {
            ALOGD("SetBesSurroundMode %d", value);
            
                if(mDownmixRamp != RAMP_DOWN)
                  {  mSurroundRamp = RAMP_DOWN;}
            mBesSurroundMode = value;
        }
        return OK;
    } 
    
    status_t AudioMTKSurroundDownMix:: ResetBesSurround()
    {
        if(mBesSurround == NULL)
        {
            return INVALID_OPERATION;
        }
        return mBesSurround->BesSurround_Reset();
    }
    status_t AudioMTKSurroundDownMix:: SetBesSurroundDevice(audio_devices_t Device)
    {
        if(mDevice == Device)
        {
            return OK;
        }
        mDevice = Device;
        if(mDevice &(AUDIO_DEVICE_OUT_EARPIECE |
                                 AUDIO_DEVICE_OUT_WIRED_HEADSET |
                                 AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                                 AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET |
                                 AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET 
                                 ))
            {
                mBesSurround->mDevice = HEADPHONE;
            }
        else
        {
            mBesSurround->mDevice = SPEAKER;
        }
        if(IsBesSurroundInitialized)
        {
            int temp_mChannelMask = mChannelMask;
            if(Qurey_Channel_Support(mChannelMask) == FAIL)
            {
                ALOGD("channmask 0x%x not supported",mChannelMask);
                temp_mChannelMask = AUDIO_CHANNEL_OUT_STEREO;
                
                if(DownMix_Init( mTrackName,  mSessionId,  mChannelMask,  mSampleRate) == OK)
                 {
                     mDownmix_enable  =1;
                 }
                else
                {                    
                    mDownmix_enable = 0;
                }
            }
            else
            {
                ALOGD("channmask 0x%x  supported",mChannelMask);
                mDownmix_enable = 0;
            }
            mBesSurround->BesSurround_reinit(temp_mChannelMask, mSampleRate);
        }
        ALOGD("mBesSurround->mDevice %d", mBesSurround->mDevice);
        ALOGD("mDevice %d", mDevice);
        return OK;
    }
    
    status_t AudioMTKSurroundDownMix::Qurey_Channel_Support( unsigned int channel_mask)
    {
        ALOGD("Qurey_Channel_Support");
        if(((mDevice & (AUDIO_DEVICE_OUT_EARPIECE |
                                 AUDIO_DEVICE_OUT_WIRED_HEADSET |
                                 AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                 AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                                 AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET |
                                 AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET 
                                 )) == 0) && (channel_mask != AUDIO_CHANNEL_OUT_STEREO) )
        {
            ALOGD("mDevice %d not support MutiChannel", mDevice );
            return    FAIL;
        }
        return BesSurround_Qurey_Channel_Support(mChannelMask);
    }
    
    AudioMTKSurroundDownMix ::BesSurround::BesSurround():
        mBesSurround_hdl(NULL),
        mBesSurround_info(NULL),
        mBesTempbuf(NULL),
        mEnable(0),
        mMode(0),
        mDevice(0),
        mBitsPerSample(32)
    {
        status_t status;
        // status = BesSurround_init( channelMask,  sampleRate);
         ALOGD("BesSurround Init status %d", status);
    }
        
    AudioMTKSurroundDownMix ::BesSurround::~BesSurround()
    {
        BesSurround_Close();
    }
    
    status_t AudioMTKSurroundDownMix :: BesSurround::BesSurround_info_initialize(MTK_VS_INFO *vs_info, int channel_mask, int sample_rate, int bits_per_sample, uint32_t device)
    {
        if (vs_info == NULL)
        {
            ALOGD("vs_info allocate buffer fail!\n");
            return NO_INIT;
        }
        ALOGD("BesSurround_info_initialize channel_mask 0x%x, sample_rate %d,  bits_per_sample %d device %d",channel_mask,sample_rate,bits_per_sample,device);
     
        vs_info->sampling_rate = sample_rate;
        vs_info->valid_bit =32;// bits_per_sample;
        vs_info->device =(MTK_VS_DEVICE) device;
    
        vs_info->channel_mask = channel_mask;
        switch(mDevice)
        {
         case HEADPHONE:
                    // for music mode
                    vs_info->param.rev_band[VS_MUSIC] = BESSURROUND_REV_BAND_MUSIC_HEADSET;
                    vs_info->param.rev_dis[VS_MUSIC] = BESSURROUND_REV_DIS_MUSIC_HEADSET;
                    vs_info->param.rev_lpf[VS_MUSIC] = BESSURROUND_REV_LPF_MUSIC_HEADSET;
                    vs_info->param.rev_weight_f[VS_MUSIC] = BESSURROUND_REVF_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.rev_weight_s[VS_MUSIC] = BESSURROUND_REVS_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.sur_weight[VS_MUSIC] = BESSURROUND_SUR_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.ori_weight[VS_MUSIC] = BESSURROUND_ORI_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.output_gain[VS_MUSIC] = BESSURROUND_OUT_GAIN_MUSIC_HEADSET;
                    vs_info->param.sur_enh[VS_MUSIC] = BESSURROUND_SUR_ENH_MUSIC_HEADSET;
                    // for movie mode
                    vs_info->param.rev_band[VS_MOVIE] = BESSURROUND_REV_BAND_MOVIE_HEADSET;
                    vs_info->param.rev_dis[VS_MOVIE] = BESSURROUND_REV_DIS_MOVIE_HEADSET;
                    vs_info->param.rev_lpf[VS_MOVIE] = BESSURROUND_REV_LPF_MOVIE_HEADSET;
                    vs_info->param.rev_weight_f[VS_MOVIE] = BESSURROUND_REVF_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.rev_weight_s[VS_MOVIE] = BESSURROUND_REVS_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.sur_weight[VS_MOVIE] = BESSURROUND_SUR_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.ori_weight[VS_MOVIE] = BESSURROUND_ORI_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.output_gain[VS_MOVIE] = BESSURROUND_OUT_GAIN_MOVIE_HEADSET;
                    vs_info->param.sur_enh[VS_MOVIE] = BESSURROUND_SUR_ENH_MOVIE_HEADSET;
                    break;
             case MONO_SPEAKER:
             case SPEAKER:
                
                // for music mode
                vs_info->param.rev_band[VS_MUSIC] = BESSURROUND_REV_BAND_MUSIC_SPEAKER;
                vs_info->param.rev_dis[VS_MUSIC] = BESSURROUND_REV_DIS_MUSIC_SPEAKER;
                vs_info->param.rev_lpf[VS_MUSIC] = BESSURROUND_REV_LPF_MUSIC_SPEAKER;
                vs_info->param.rev_weight_f[VS_MUSIC] = BESSURROUND_REVF_WEIGHT_MUSIC_SPEAKER;
                vs_info->param.rev_weight_s[VS_MUSIC] = BESSURROUND_REVS_WEIGHT_MUSIC_SPEAKER;
                vs_info->param.sur_weight[VS_MUSIC] = BESSURROUND_SUR_WEIGHT_MUSIC_SPEAKER;
                vs_info->param.ori_weight[VS_MUSIC] = BESSURROUND_ORI_WEIGHT_MUSIC_SPEAKER;
                vs_info->param.output_gain[VS_MUSIC] = BESSURROUND_OUT_GAIN_MUSIC_SPEAKER;
                vs_info->param.sur_enh[VS_MUSIC] = BESSURROUND_SUR_ENH_MUSIC_SPEAKER;
                // for movie mode
                vs_info->param.rev_band[VS_MOVIE] = BESSURROUND_REV_BAND_MOVIE_SPEAKER;
                vs_info->param.rev_dis[VS_MOVIE] = BESSURROUND_REV_DIS_MOVIE_SPEAKER;
                vs_info->param.rev_lpf[VS_MOVIE] = BESSURROUND_REV_LPF_MOVIE_SPEAKER;
                vs_info->param.rev_weight_f[VS_MOVIE] = BESSURROUND_REVF_WEIGHT_MOVIE_SPEAKER;
                vs_info->param.rev_weight_s[VS_MOVIE] = BESSURROUND_REVS_WEIGHT_MOVIE_SPEAKER;
                vs_info->param.sur_weight[VS_MOVIE] = BESSURROUND_SUR_WEIGHT_MOVIE_SPEAKER;
                vs_info->param.ori_weight[VS_MOVIE] = BESSURROUND_ORI_WEIGHT_MOVIE_SPEAKER;
                vs_info->param.output_gain[VS_MOVIE] = BESSURROUND_OUT_GAIN_MOVIE_SPEAKER;
                vs_info->param.sur_enh[VS_MOVIE] = BESSURROUND_SUR_ENH_MOVIE_SPEAKER;
                break;
                default:
                    
                    // for music mode
                    vs_info->param.rev_band[VS_MUSIC] = BESSURROUND_REV_BAND_MUSIC_HEADSET;
                    vs_info->param.rev_dis[VS_MUSIC] = BESSURROUND_REV_DIS_MUSIC_HEADSET;
                    vs_info->param.rev_lpf[VS_MUSIC] = BESSURROUND_REV_LPF_MUSIC_HEADSET;
                    vs_info->param.rev_weight_f[VS_MUSIC] = BESSURROUND_REVF_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.rev_weight_s[VS_MUSIC] = BESSURROUND_REVS_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.sur_weight[VS_MUSIC] = BESSURROUND_SUR_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.ori_weight[VS_MUSIC] = BESSURROUND_ORI_WEIGHT_MUSIC_HEADSET;
                    vs_info->param.output_gain[VS_MUSIC] = BESSURROUND_OUT_GAIN_MUSIC_HEADSET;
                    vs_info->param.sur_enh[VS_MUSIC] = BESSURROUND_SUR_ENH_MUSIC_HEADSET;
                    // for movie mode
                    vs_info->param.rev_band[VS_MOVIE] = BESSURROUND_REV_BAND_MOVIE_HEADSET;
                    vs_info->param.rev_dis[VS_MOVIE] = BESSURROUND_REV_DIS_MOVIE_HEADSET;
                    vs_info->param.rev_lpf[VS_MOVIE] = BESSURROUND_REV_LPF_MOVIE_HEADSET;
                    vs_info->param.rev_weight_f[VS_MOVIE] = BESSURROUND_REVF_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.rev_weight_s[VS_MOVIE] = BESSURROUND_REVS_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.sur_weight[VS_MOVIE] = BESSURROUND_SUR_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.ori_weight[VS_MOVIE] = BESSURROUND_ORI_WEIGHT_MOVIE_HEADSET;
                    vs_info->param.output_gain[VS_MOVIE] = BESSURROUND_OUT_GAIN_MOVIE_HEADSET;
                    vs_info->param.sur_enh[VS_MOVIE] = BESSURROUND_SUR_ENH_MOVIE_HEADSET;
                    break;
            }

        vs_info->param.debug = NORMAL;
        ALOGD("  vs_info->param.debug%d", vs_info->param.debug);
        ALOGD("BesSurround_info_initialize done");
        return 0;
    
    }
    
status_t AudioMTKSurroundDownMix:: BesSurround::BesSurround_reinit( int channelMask, int sampleRate)
{
    status_t res = OK;
    if (BesSurround_info_initialize(mBesSurround_info, channelMask, sampleRate, mBitsPerSample, mDevice) != SUCCESS)
    {
        ALOGE("BesSurround_info_initialize fail!\n");        
        res = NO_INIT;
        goto exit;
    }
    ALOGD("mBesSurround_info (0x%x) param(0x%x) mBesSurround_info->param.debug %d\n", mBesSurround_info, &(mBesSurround_info->param), mBesSurround_info->param.debug);
    if (mtk_vs_init((void*)mBesSurround_hdl, mBesSurround_info, mBesTempbuf) != SUCCESS)
    {
        ALOGE("[init] mtk_vs_init fail!\n");
        res = NO_INIT;
        goto exit;
    }
    
    ALOGD("  mBesSurround_info->param.debug%d", mBesSurround_info->param.debug);
    ALOGD("BesSurround_reinit done");    
    exit:
        return res;
}
status_t AudioMTKSurroundDownMix:: BesSurround::BesSurround_init( int channelMask, int sampleRate)
{
    status_t res = OK;
    
    int hdl_size = 0;
    int temp_buffer_size = 0;
    
    // vs_info initialize
    mBesSurround_info = (MTK_VS_INFO*) new uint8_t[sizeof(MTK_VS_INFO)];
    
    mtk_vs_get_memsize(&hdl_size, &temp_buffer_size);
    mBesSurround_hdl = new int8_t[hdl_size];
    mBesTempbuf = new char[temp_buffer_size];
    if (mBesSurround_hdl == NULL || mBesTempbuf==NULL)
    {
        ALOGE("[init]- mBesSurround_hdl or temp_buffer allocate fail\n!");
        res = NO_MEMORY;
        goto exit;
        
    }

    if (BesSurround_reinit( channelMask, sampleRate)!= SUCCESS)
    {
        ALOGE("[init] mtk_vs_init fail!\n");
        res = NO_INIT;
        goto exit;
    }
    ALOGD("BesSurround_reinit sucess");
    return res;
    
    exit:
        
        if (mBesSurround_info) { delete [] mBesSurround_info;
        mBesSurround_info= NULL;
        }
        if (mBesSurround_hdl) { delete [] mBesSurround_hdl; 
        mBesSurround_hdl= NULL;}
        if (mBesTempbuf) { delete [] mBesTempbuf ; 
        mBesTempbuf = NULL;
        }
        return res;
}
status_t AudioMTKSurroundDownMix::BesSurround::BesSurround_process( int32_t* inBuffer, int32_t* ouBuffer, int32_t numFrames)
{
    status_t res = OK;
    if(mBesSurround_hdl == NULL)
    {
        ALOGE("NULL handle");
        return NO_INIT;
    }
    ALOGV("BesSurround_process numFrames %d", numFrames);
    int32_t* inPtr = inBuffer;
    int32_t* outPtr = ouBuffer;
    int32_t ChannelCount  = popcount( mBesSurround_info->channel_mask );
    while(numFrames >0){
        int processed;
        processed = mtk_vs_process((void*)mBesSurround_hdl, inPtr, outPtr, numFrames);
        ALOGD("processed %d, ChannelCount %d", processed,ChannelCount);
        numFrames -= processed*ChannelCount;
        inPtr += processed * ChannelCount;
        outPtr += processed <<1; // output channel count is fixed to 2
    }
    return res;
}
status_t AudioMTKSurroundDownMix::BesSurround::BesSurround_ramp_up()
{
    status_t res = OK;
    if(mBesSurround_hdl == NULL)
    {
        ALOGE("NULL handle");
        return  NO_INIT;
    }
    
    ALOGV("BesSurround_ramp_up");
    //res = mtk_vs_ramp_up((void*)mBesSurround_hdl);
    ALOGV(" Get Ramp status %d", mtk_vs_get_ramp_status((void*)mBesSurround_hdl));
    return res;
}
status_t AudioMTKSurroundDownMix::BesSurround::BesSurround_ramp_down( )
{
    status_t res = OK;
    if(mBesSurround_hdl == NULL)
    {
        ALOGE("NULL handle");
        return NO_INIT;
    }
    
    ALOGV("BesSurround_ramp_down");
    if(mEnable)
    {
        ALOGV("BesSurround_ramp_down actual ramp down");
        //mtk_vs_ramp_down((void*)mBesSurround_hdl);
        }
    else{
            return INVALID_OPERATION;
        }
    ALOGV(" BesSurround_ramp_down Get Ramp status %d", mtk_vs_get_ramp_status(mBesSurround_hdl));
        return res;
}

int AudioMTKSurroundDownMix::BesSurround::BesSurround_GetRampStatus( void)
{
    status_t res = OK;
    if(mBesSurround_hdl == NULL)
    {
        ALOGE("NULL handle");
       return  NO_INIT;
    }
    ALOGV("BesSurround_GetRampStatus");
        return mtk_vs_get_ramp_status((void*)mBesSurround_hdl);
}

status_t AudioMTKSurroundDownMix::  BesSurround::BesSurround_SetMode( int value)
{
    status_t res = OK;
    if(value != 0 && value != 1)
    {
        return BAD_VALUE;
    }
    if (mBesSurround_hdl !=NULL)
    {
        ALOGV("BesSurround_SetMode %d", value);
       mtk_vs_set_mode((void*) mBesSurround_hdl,( MTK_VS_MODE )value);
    }
    else{
        return NO_INIT;
    }
    return res;
}

status_t AudioMTKSurroundDownMix::BesSurround::BesSurround_Reset( )
{
    status_t res = OK;
    if (mBesSurround_hdl !=NULL)
    {
//        ALOGD("mtk_vs_reset");
        mtk_vs_reset((void*) mBesSurround_hdl);
    }
    else{
        return NO_INIT;
    }
    return res;
}
status_t AudioMTKSurroundDownMix::BesSurround::BesSurround_dumpInfo( )
{
      if(  mBesSurround_info != NULL)
      {
        ALOGV("mBesSurroundMode %d",mMode);
        ALOGV("mBesSurround_info->channel_mask 0x%x",mBesSurround_info->channel_mask); 
        ALOGV("mBesSurround_info->device %d",mBesSurround_info->device); 
        ALOGV("mBesSurround_info->sampling_rate %d",mBesSurround_info->sampling_rate); 
        ALOGV("mBesSurround_info->valid_bit %d",mBesSurround_info->valid_bit); 
        ALOGV("mBesSurround_info->param.debug %d",mBesSurround_info->param.debug); 
        ALOGV("mBesSurround_info->param.ori_weight %d",mBesSurround_info->param.ori_weight[mMode]); 
        ALOGV("mBesSurround_info->param.output_gain %d",mBesSurround_info->param.output_gain[mMode]); 
        ALOGV("mBesSurround_info->param.rev_band %d",mBesSurround_info->param.rev_band[mMode]); 
        ALOGV("mBesSurround_info->param.rev_dis %d",mBesSurround_info->param.rev_dis[mMode]); 
        ALOGV("mBesSurround_info->param.rev_lpf %d",mBesSurround_info->param.rev_lpf[mMode]);        
        ALOGV("mBesSurround_info->param.rev_weight_f %d",mBesSurround_info->param.rev_weight_f[mMode]);
        ALOGV("mBesSurround_info->param.rev_weight_s %d",mBesSurround_info->param.rev_weight_s[mMode]); 
        ALOGV("mBesSurround_info->param.sur_enh %d",mBesSurround_info->param.sur_enh[mMode]); 
        ALOGV("mBesSurround_info->param.sur_weight %d",mBesSurround_info->param.sur_weight[mMode]); 
        return OK;
      }
       ALOGD("mBesSurround_info NULL");
      return NO_INIT;
    
}
status_t AudioMTKSurroundDownMix::BesSurround_Qurey_Channel_Support( unsigned int channel_mask)
{
    status_t res = OK;
    ALOGV("BesSurround_Qurey_Channel_Support");
    return mtk_vs_query_channel_support(channel_mask);
}
void AudioMTKSurroundDownMix::BesSurround::BesSurround_Close( void)
{
    
    ALOGV("BesSurround_Close ");
    if(mBesSurround_hdl !=NULL)
    {
        delete [] mBesSurround_hdl;
        mBesSurround_hdl = NULL;
    }

    if(mBesTempbuf != NULL)
    {
        delete [] mBesTempbuf;
        mBesTempbuf = NULL;
    }
    if (mBesSurround_info != NULL)
    {
        delete [] mBesSurround_info;
        mBesSurround_info = NULL;
    }
    return;
}

// ----------------------------------------------------------------------------
}; // namespace android

