/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_AUDIO_MTK_MIXER_H
#define ANDROID_AUDIO_MTK_MIXER_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/threads.h>

#include <media/AudioBufferProvider.h>
#include "AudioResampler.h"

#include <audio_effects/effect_downmix.h>
#include <system/audio.h>

#include <media/nbaio/NBLog.h>

#define MTK_AUDIOMIXER_ENABLE_DRC
#define MTK_AUDIOMIXER_ENABLE_LIMITER

//#ifdef MTK_AUDIOMIXER_ENABLE_DRC
#include "AudioCompFltCustParam.h"
extern "C" {
#include "MtkAudioLoud.h"
}
//#endif

#ifdef MTK_AUDIOMIXER_ENABLE_LIMITER
extern "C" {
#include "Limiter_exp.h"
}
#endif
#ifdef TIME_STRETCH_ENABLE
#include "AudioMTKTimeStretch.h"
#endif

#include "AudioMTKSurroundMix.h"
namespace android {

// ----------------------------------------------------------------------------

class AudioMTKMixer
{
public:
                            AudioMTKMixer(size_t frameCount, uint32_t sampleRate,
                                       uint32_t maxNumTracks = MAX_NUM_TRACKS);

    /*virtual*/             ~AudioMTKMixer();  // non-virtual saves a v-table, restore if sub-classed

    static const uint32_t MAX_NUM_TRACKS = 32;
    // maximum number of channels supported by the mixer
    static const uint32_t MAX_NUM_CHANNELS = 2;
    // maximum number of channels supported for the content
    static const uint32_t MAX_NUM_CHANNELS_TO_DOWNMIX = 8;

    static const uint16_t UNITY_GAIN = 0x1000;

    enum { // names

        // track names (MAX_NUM_TRACKS units)
        TRACK0          = 0x1000,

        // 0x2000 is unused

        // setParameter targets
        TRACK           = 0x3000,
        RESAMPLE        = 0x3001,
        RAMP_VOLUME     = 0x3002, // ramp to new volume
        VOLUME          = 0x3003, // don't ramp
        DRC             = 0x3004, // dynamic range control
#ifdef MTK_BESSURROUND_SUPPORT
        SURROUND        = 0x3005, // dynamic range control
#endif
        BUFFER          = 0x3006,
        // set Parameter names
        // for target TRACK
        CHANNEL_MASK    = 0x4000,
        FORMAT          = 0x4001,
        MAIN_BUFFER     = 0x4002,
        AUX_BUFFER      = 0x4003,
        DOWNMIX_TYPE    = 0X4004,
        STREAM_TYPE     = 0x4005,
        DO_POST_PROC    = 0x4006,
        
#ifdef TIME_STRETCH_ENABLE
	DO_TIMESTRETCH	= 0x4007,
#endif
        STEREO2MONO     = 0x4008,

        // for target RESAMPLE
        SAMPLE_RATE     = 0x4100, // Configure sample rate conversion on this track name;
                                  // parameter 'value' is the new sample rate in Hz.
                                  // Only creates a sample rate converter the first time that
                                  // the track sample rate is different from the mix sample rate.
                                  // If the new sample rate is the same as the mix sample rate,
                                  // and a sample rate converter already exists,
                                  // then the sample rate converter remains present but is a no-op.
        RESET           = 0x4101, // Reset sample rate converter without changing sample rate.
                                  // This clears out the resampler's input buffer.
        REMOVE          = 0x4102, // Remove the sample rate converter on this track name;
                                  // the track is restored to the mix sample rate.
        // for target RAMP_VOLUME and VOLUME (8 channels max)
        VOLUME0         = 0x4200,
        VOLUME1         = 0x4201,
        AUXLEVEL        = 0x4210,
        
        DEVICE          = 0x4300, // output device for DRC
        
#ifdef MTK_AUDIOMIXER_ENABLE_DRC//PUSH_ZERO
        DO_PUSHZERO = 0x4304       , 
#endif
#ifdef MTK_BESSURROUND_SUPPORT
        BESSURND_ENABLE = 0x4302, //BesSurround enable
        BESSURND_MODE = 0x4303
#endif

    };


    // For all APIs with "name": TRACK0 <= name < TRACK0 + MAX_NUM_TRACKS

    // Allocate a track name.  Returns new track name if successful, -1 on failure.
    int         getTrackName(audio_channel_mask_t channelMask, audio_format_t format, int sessionId);

    // Free an allocated track by name
    void        deleteTrackName(int name);

    // Enable or disable an allocated track by name
    void        enable(int name);
    void        disable(int name);

    void        setParameter(int name, int target, int param, void *value);

    void        setBufferProvider(int name, AudioBufferProvider* bufferProvider);
    void        process(int64_t pts);

    uint32_t    trackNames() const { return mTrackNames; }

    size_t      getUnreleasedFrames(int name) const;

    // UI Dynamic Control DRC
    void setDRCEnable(bool enable) { mUIDRCEnable = enable; }

private:

    enum {
        NEEDS_CHANNEL_COUNT__MASK   = 0x00000007,
        NEEDS_FORMAT__MASK          = 0x000000F0,
        NEEDS_MUTE__MASK            = 0x00000100,
        NEEDS_RESAMPLE__MASK        = 0x00001000,
        NEEDS_AUX__MASK             = 0x00010000,
    };

    enum {
        NEEDS_CHANNEL_1             = 0x00000000,
        NEEDS_CHANNEL_2             = 0x00000001,

        NEEDS_FORMAT_16             = 0x00000010,

        NEEDS_MUTE_DISABLED         = 0x00000000,
        NEEDS_MUTE_ENABLED          = 0x00000100,

        NEEDS_RESAMPLE_DISABLED     = 0x00000000,
        NEEDS_RESAMPLE_ENABLED      = 0x00001000,

        NEEDS_AUX_DISABLED     = 0x00000000,
        NEEDS_AUX_ENABLED      = 0x00010000,
    };

    struct state_t;
    struct track_t;
    class DownmixerBufferProvider;
    class TimeStretchBufferProvider;

    typedef void (*hook_t)(track_t* t, size_t numOutFrames, int32_t* temp, int32_t* aux, state_t* state);

    struct track_t {
        uint32_t    needs;

        union {
        int16_t     volume[MAX_NUM_CHANNELS]; // [0]3.12 fixed point
        int32_t     volumeRL;
        };

        int32_t     prevVolume[MAX_NUM_CHANNELS];

        // 16-byte boundary

        int32_t     volumeInc[MAX_NUM_CHANNELS];
        int32_t     auxInc;
        int32_t     prevAuxLevel;

        // 16-byte boundary

        int16_t     auxLevel;       // 0 <= auxLevel <= MAX_GAIN_INT, but signed for mul performance
        uint16_t    frameCount;

        uint8_t     channelCount;   // 1 or 2, redundant with (needs & NEEDS_CHANNEL_COUNT__MASK)
        uint8_t     format;         // always 16
        uint16_t    enabled;        // actually bool
        audio_channel_mask_t channelMask;
        uint8_t IsMixerDoPostProc;          // 1: do DC removal ,0 don't

        // actual buffer provider used by the track hooks, see DownmixerBufferProvider below
        //  for how the Track buffer provider is wrapped by another one when dowmixing is required
        AudioBufferProvider*                bufferProvider;

        // 16-byte boundary

        mutable AudioBufferProvider::Buffer buffer; // 8 bytes

        hook_t      hook;
        const void* in;             // current location in buffer

        // 16-byte boundary

        AudioResampler*     resampler;
        uint32_t            sampleRate;
        int32_t*           mainBuffer;
        int32_t*           auxBuffer;

        // 16-byte boundary

        uint64_t    localTimeFreq;

        DownmixerBufferProvider* downmixerBufferProvider; // 4 bytes

#ifdef TIME_STRETCH_ENABLE
	 TimeStretchBufferProvider *timestretchBufferProvider;
#endif
#ifdef MTK_AUDIOMIXER_ENABLE_DRC//PUSH_ZERO
            int32_t mPushZero;
#endif

        int32_t     sessionId;

        // 16-byte boundary

        bool        setResampler(uint32_t sampleRate, uint32_t devSampleRate);
        bool        doesResample() const { return resampler != NULL; }
        void        resetResampler() { if (resampler != NULL) resampler->reset(); }
        void        adjustVolumeRamp(bool aux);
        size_t      getUnreleasedFrames() const { return resampler != NULL ?
                                                    resampler->getUnreleasedFrames() : 0; };
        
        audio_format_t           mBitFormat;

        //  Stereo 2 Mono
        bool DoStereoMonoConvert(void *buffer, size_t bytes);
        BLOUD_S2M_MODE_ENUM      mSteroToMono;
		
#ifdef MTK_AUDIOMIXER_ENABLE_DRC
        audio_stream_type_t      mStreamType;
        bool                     mDRCEnable;
        bool                     mDRCState;
        MtkAudioLoud             *mpDRCObj;
        
        void        setDRCHandler(audio_devices_t device, uint32_t bufferSize, uint32_t sampleRate);
        void        applyDRC(int32_t *ioBuffer, uint32_t frameCount, int32_t *tempBuffer);
        void        updateDRCParam(int devSampleRate);
        
#endif

#ifdef MTK_BESSURROUND_SUPPORT
        AudioMTKSurroundDownMix *mSurroundMixer;
	bool mSurroundEnable;
         bool mSurroundMode;
         uint32_t mDevSampleRate;
        void setDevice(audio_devices_t device);
        void onBufferUnderflow();
#endif
    };

    // pad to 32-bytes to fill cache line
    struct state_t {
        uint32_t        enabledTracks;
        uint32_t        needsChanged;
        size_t          frameCount;
        void            (*hook)(state_t* state, int64_t pts);   // one of process__*, never NULL
        int64_t         *outputTemp;
        int32_t         *resampleTemp;
        NBLog::Writer*  mLog;
        int32_t         reserved[2];
        // FIXME allocate dynamically to save some memory when maxNumTracks < MAX_NUM_TRACKS
        track_t         tracks[MAX_NUM_TRACKS]; __attribute__((aligned(32)));

        int32_t         *aggregationBuffer;
        uint32_t        aggregationCount;

#ifdef MTK_AUDIOMIXER_ENABLE_DRC
        bool            mDRCSupport;
        bool            mDRCEnable;
        int32_t         *pDRCTempBuffer;
#endif

#ifdef MTK_AUDIOMIXER_ENABLE_LIMITER
        Limiter_Handle  *mpLimiterObj;
        uint8_t         *pLimiterInternalBuffer;
        uint8_t         *pLimiterTempBuffer;
#endif
#ifdef MTK_BESSURROUND_SUPPORT
	int32_t 		*downMixBuffer;
#endif
    };
    // AudioBufferProvider that wraps a track AudioBufferProvider by a call to a downmix effect
    class DownmixerBufferProvider : public AudioBufferProvider {
    public:
        virtual status_t getNextBuffer(Buffer* buffer, int64_t pts);
        virtual void releaseBuffer(Buffer* buffer);
        DownmixerBufferProvider();
        virtual ~DownmixerBufferProvider();

        AudioBufferProvider* mTrackBufferProvider;
        effect_handle_t    mDownmixHandle;
        effect_config_t    mDownmixConfig;
    };
#ifdef TIME_STRETCH_ENABLE
			class TimeStretchBufferProvider : public AudioBufferProvider {
			public:
				virtual status_t getNextBuffer(Buffer* buffer, int64_t pts);
				virtual void releaseBuffer(Buffer* buffer);
				TimeStretchBufferProvider(int framecount);
				virtual ~TimeStretchBufferProvider();
				virtual status_t TimeStretchConfig(int ratio) ;
				
				AudioBufferProvider* mTrackBufferProvider;
				AudioMTKTimeStretch* mTimeStretchHandle;		
					effect_config_t    mTimeStretchConfig;
				AudioBufferProvider::Buffer mBuffer;
				short* mOutBuffer;
				int mOutframecount; // allocated outbuffer size
				int mOutRemain;
				//effect_handle_t	 mDownmixHandle;
				//effect_config_t	 mDownmixConfig;
			};
		
#endif
	

    // bitmask of allocated track names, where bit 0 corresponds to TRACK0 etc.
    uint32_t        mTrackNames;

    // bitmask of configured track names; ~0 if maxNumTracks == MAX_NUM_TRACKS,
    // but will have fewer bits set if maxNumTracks < MAX_NUM_TRACKS
    const uint32_t  mConfiguredNames;

    const uint32_t  mSampleRate;

    NBLog::Writer   mDummyLog;
public:
    void            setLog(NBLog::Writer* log);
private:
    state_t         mState __attribute__((aligned(32)));

    // effect descriptor for the downmixer used by the mixer
    static effect_descriptor_t dwnmFxDesc;
    // indicates whether a downmix effect has been found and is usable by this mixer
    static bool                isMultichannelCapable;
	#ifdef TIME_STRETCH_ENABLE
    static bool 		isTimeStretchCapable;
	#endif
    // Call after changing either the enabled status of a track, or parameters of an enabled track.
    // OK to call more often than that, but unnecessary.
    void invalidateState(uint32_t mask);

    static status_t initTrackDownmix(track_t* pTrack, int trackNum, audio_channel_mask_t mask);
    static status_t prepareTrackForDownmix(track_t* pTrack, int trackNum);
    static void unprepareTrackForDownmix(track_t* pTrack, int trackName);
#ifdef TIME_STRETCH_ENABLE
	status_t initTrackTimeStretch(track_t* pTrack, int trackNum,   int ratio);

	static void unprepareTrackForTimeStretch(track_t* pTrack, int trackName);
	status_t prepareTrackForTimeStretch(track_t* pTrack, int trackName, int framecount, int ratio);

#endif
    static void volumeRampStereo(track_t* t, int64_t* out, size_t frameCount, int32_t* temp, int32_t* aux);
    static void volumeStereo(track_t* t, int64_t* out, size_t frameCount, int32_t* temp, int32_t* aux);

    static void process__validate(state_t* state, int64_t pts);
    static void process__nop(state_t* state, int64_t pts);
    static void process__genericResampling(state_t* state, int64_t pts);

    static int64_t calculateOutputPTS(const track_t& t, int64_t basePTS,
                                      int outputFrameIndex);

    static void track__genericResample(track_t* t, size_t numFrames, int32_t* temp, int32_t* aux, state_t* state);
    static void track__nop(track_t* t, size_t numFrames, int32_t* temp, int32_t* aux, state_t* state);
    static void track__addToAggregationBuffer(state_t* state, int32_t* inBuffer, size_t numFrames);
    static void track__addToAggregationBuffer_16(state_t* state, const int16_t* inBuffer, size_t numFrames, uint32_t channelNum);

    static void process__resetAggregationBuffer(state_t* state);
    
    static void genericVolumeRampStereo(state_t* state, track_t* t, int64_t* out);
#ifdef MTK_BESSURROUND_SUPPORT
	static status_t prepareTrackForSurroundMix(track_t* pTrack, int trackName);
	static void unprepareTrackForSurroundMix(track_t* pTrack, int trackName);

	static void track__Downmix(track_t* t, int32_t* inBuffer, size_t numFrames,state_t* state);

	static void setDevice(audio_devices_t device);

	static void onBufferUnderflow();
#endif
    // UI Dynamic Control DRC
    static bool mUIDRCEnable;
//    char dummy[2048];
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_AUDIO_MTK_MIXER_H
