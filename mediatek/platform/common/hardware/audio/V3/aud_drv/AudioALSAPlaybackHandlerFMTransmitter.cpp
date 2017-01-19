#include "AudioALSAPlaybackHandlerFMTransmitter.h"

#include "WCNChipController.h"

#include "AudioALSADeviceParser.h"


#define LOG_TAG "AudioALSAPlaybackHandlerFMTransmitter"

namespace android
{

AudioALSAPlaybackHandlerFMTransmitter::AudioALSAPlaybackHandlerFMTransmitter(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mWCNChipController(WCNChipController::GetInstance())
{
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_FM_TX;
}


AudioALSAPlaybackHandlerFMTransmitter::~AudioALSAPlaybackHandlerFMTransmitter()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerFMTransmitter::open()
{
    ALOGD("+%s(), mDevice = 0x%x", __FUNCTION__, mStreamAttributeSource->output_devices);

    // debug pcm dump
    OpenPCMDump(LOG_TAG);


    // HW attribute config // TODO(Harvey): query this
#ifdef PLAYBACK_USE_24BITS_ONLY
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
#else
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
#endif
    mStreamAttributeTarget.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeTarget.num_channels = android_audio_legacy::AudioSystem::popCount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = 44100; // TODO(Harvey, Chipeng): query?


    // HW pcm config
    mConfig.channels = mStreamAttributeTarget.num_channels;
    mConfig.rate = mStreamAttributeTarget.sample_rate;

    // Buffer size
    mConfig.period_size = 3072;
    mConfig.period_count = 2;

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


    // Get pcm open Info
    int card_index = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmMRGTxPlayback);
    int pcm_index = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmMRGTxPlayback);
    ALOGD("%s(), card_index = %d, pcm_index = %d", __FUNCTION__, card_index, pcm_index);

    // open pcm driver
    openPcmDriver(pcm_index);


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerFMTransmitter::close()
{
    ALOGD("+%s()", __FUNCTION__);

    // close pcm driver
    closePcmDriver();


    // bit conversion
    deinitBitConverter();


    // SRC
    deinitBliSrc();


    // debug pcm dump
    ClosePCMDump();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerFMTransmitter::routing(const audio_devices_t output_devices)
{
    return INVALID_OPERATION;
}


ssize_t AudioALSAPlaybackHandlerFMTransmitter::write(const void *buffer, size_t bytes)
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


    // write data to pcm driver
    WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);
    int retval = pcm_write(mPcm, pBufferAfterBitConvertion, bytesAfterBitConvertion);


    if (retval != 0)
    {
        ALOGE("%s(), pcm_write() error, retval = %d", __FUNCTION__, retval);
    }

    return bytes;
}


} // end of namespace android
