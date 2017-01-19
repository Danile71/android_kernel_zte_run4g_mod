#include "AudioALSAPlaybackHandlerHDMI.h"


#define LOG_TAG "AudioALSAPlaybackHandlerHDMI"
//#define __2CH_TO_8CH
//#define _TDM_DEBUG

namespace android
{

#ifdef _TDM_DEBUG
static FILE *pOutFile = NULL;
static FILE *pOutFileorg = NULL;
#endif


AudioALSAPlaybackHandlerHDMI::AudioALSAPlaybackHandlerHDMI(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source)
{
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_HDMI;
}


AudioALSAPlaybackHandlerHDMI::~AudioALSAPlaybackHandlerHDMI()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerHDMI::open()
{
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

#ifdef _TDM_DEBUG

#ifdef __2CH_TO_8CH
    pOutFile = fopen("/sdcard/mtklog/hdmi8ch.pcm", "wb");
    if (pOutFile == NULL)
    {
        ALOGW("%s(), open file fail ", __FUNCTION__);
    }
#endif

    pOutFileorg = fopen("/sdcard/mtklog/hdmi.pcm", "wb");
    if (pOutFileorg == NULL)
    {
        ALOGW("%s(), open file fail ", __FUNCTION__);
    }
#else
    OpenPCMDump(LOG_TAG);
#endif


#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    //mStreamAttributeTarget.audio_format = (mStreamAttributeSource->audio_format == AUDIO_FORMAT_PCM_32_BIT) ? AUDIO_FORMAT_PCM_8_24_BIT : AUDIO_FORMAT_PCM_16_BIT;
#endif

    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = android_audio_legacy::AudioSystem::popCount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mStreamAttributeSource->sample_rate; // same as source stream


    // HW pcm config
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;


#ifdef __2CH_TO_8CH
    mConfig.period_size = 24576;
    mConfig.period_count = 8;
#else
    mConfig.period_size = 3072;
    mConfig.period_count = 2;
#endif

    mConfig.format = transferAudioFormatToPcmFormat(mStreamAttributeTarget.audio_format);

    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
    ALOGD("%s(), mConfig: channels = %d, rate = %d, period_size = %d, period_count = %d, format = %d",
          __FUNCTION__, mConfig.channels, mConfig.rate, mConfig.period_size, mConfig.period_count, mConfig.format);


    // SRC
    initBliSrc();


    // bit conversion
    initBitConverter();


    // open pcm driver
    openPcmDriver(3);

#if 0
    ASSERT(mPcm == NULL);
    mPcm = pcm_open(0, 3, PCM_OUT, &mConfig);

    if (mPcm == NULL)
    {
        ALOGE("%s(), mPcm == NULL!!", __FUNCTION__);
    }
    else if (pcm_is_ready(mPcm) == false)
    {
        ALOGE("%s(), pcm_is_ready(%p) == false due to %s, close pcm.", __FUNCTION__, mPcm, pcm_get_error(mPcm));
        pcm_close(mPcm);
        mPcm = NULL;
    }
    else
    {
        pcm_start(mPcm);
    }

    ALOGD("-%s(), mPcm = %p", __FUNCTION__, mPcm);
    ASSERT(mPcm != NULL);

#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerHDMI::close()
{
    ALOGD("+%s()", __FUNCTION__);

    // close pcm driver
    closePcmDriver();


    // bit conversion
    deinitBitConverter();


    // SRC
    deinitBliSrc();


#ifdef _TDM_DEBUG

#ifdef __2CH_TO_8CH
    if (pOutFile != NULL)
    {
        fclose(pOutFile);
    }
#endif
    if (pOutFileorg != NULL)
    {
        fclose(pOutFileorg);
    }
#else
    ClosePCMDump();
#endif


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerHDMI::routing(const audio_devices_t output_devices)
{
    return INVALID_OPERATION;
}


ssize_t AudioALSAPlaybackHandlerHDMI::write(const void *buffer, size_t bytes)
{
    ALOGV("%s(), buffer = %p, bytes = %d", __FUNCTION__, buffer, bytes);

    if (mPcm == NULL)
    {
        ALOGE("%s(), mPcm == NULL, return", __FUNCTION__);
        return bytes;
    }

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);


    // SRC
    void *pBufferAfterBliSrc = NULL;
    uint32_t bytesAfterBliSrc = 0;
    doBliSrc(pBuffer, bytes, &pBufferAfterBliSrc, &bytesAfterBliSrc);


    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBufferAfterBliSrc, bytesAfterBliSrc, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


#ifdef __2CH_TO_8CH
    void *newbuffer[12 * 1024 * 4] = {0};
    unsigned char *aaa;
    unsigned char *bbb;
    int i = 0;
    int j = 0;

    aaa = (unsigned char *)newbuffer;
    bbb = (unsigned char *)pBufferAfterBitConvertion;

    for (i = 0 ; j < bytesAfterBitConvertion; i += 16)
    {
        memcpy(aaa + i,      bbb + j, 4);
        memcpy(aaa + i + 4,  bbb + j, 4);
        memcpy(aaa + i + 8,  bbb + j, 4);
        memcpy(aaa + i + 12, bbb + j, 4);
        j += 4;
    }

    // write data to pcm driver
    int retval = pcm_write(mPcm, newbuffer, bytesAfterBitConvertion << 2);
#else
    // write data to pcm driver
    int retval = pcm_write(mPcm, pBufferAfterBitConvertion, bytesAfterBitConvertion);
#endif


    if (retval != 0)
    {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }


#ifdef _TDM_DEBUG

#ifdef __2CH_TO_8CH

    if (pOutFile != NULL)
    {
        ALOGD("%s(), newbuffer = %p", __FUNCTION__, newbuffer);
        fwrite(newbuffer, sizeof(char), bytesAfterBitConvertion * 4, pOutFile);
    }
#endif
    if (pOutFileorg != NULL)
    {
        ALOGD("%s(), pBufferAfterBitConvertion = %p", __FUNCTION__, pBufferAfterBitConvertion);
        fwrite(pBufferAfterBitConvertion, sizeof(char), bytesAfterBitConvertion, pOutFileorg);
    }
#else
    WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);
#endif


    return bytes;
}


} // end of namespace android
