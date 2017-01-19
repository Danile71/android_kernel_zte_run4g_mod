#define LOG_TAG  "AudioMTKDCRemoval"

#include <cutils/compiler.h>


#include "AudioMTKDcRemoval.h"
#include <cutils/xlog.h>


#define ENABLE_DC_REMOVE
//#define DUMP_DCR_DEBUG
#ifdef DUMP_DCR_DEBUG
#include "AudioUtility.h"
#endif

namespace android
{

DcRemove::DcRemove()
    : mHandle(NULL)
{
}
DcRemove::~DcRemove()
{
    close();
}

status_t  DcRemove::init(uint32 channel, uint32 samplerate, uint32 dcrMode)
{
    Mutex::Autolock _l(&mLock);
    unsigned int internal_buf_size_in_bytes = 0;
    DCR_GetBufferSize(&internal_buf_size_in_bytes);
    if (!mHandle)
    {
        p_internal_buf = (signed char *)malloc(internal_buf_size_in_bytes);
        mHandle = DCR_Open(p_internal_buf, channel, samplerate, dcrMode);
    }
    else
    {
        mHandle = DCR_ReConfig(mHandle, channel, samplerate, dcrMode);
    }
    if (!mHandle)
    {
        SXLOGW("Fail to get DCR Handle");
        if(p_internal_buf != NULL)
        {
            free(p_internal_buf);
            p_internal_buf = NULL;
        }
        return NO_INIT;
    }
    mSamplerate = samplerate;
    return NO_ERROR;
}

status_t  DcRemove::close()
{
    Mutex::Autolock _l(&mLock);
    SXLOGV("DcRemove::deinit");
    if (mHandle)
    {
        DCR_Close(mHandle);
        if(p_internal_buf != NULL)
        {
            free(p_internal_buf);
            p_internal_buf = NULL;
        }
    }
    return NO_ERROR;
}

size_t DcRemove::process(const void *inbuffer, size_t bytes, void *outbuffer)
{
    Mutex::Autolock _l(&mLock);
#ifdef ENABLE_DC_REMOVE
    if (mHandle)
    {
        size_t outputBytes = 0;
        uint32_t inputBufSize  = bytes;
        uint32_t outputBufSize = bytes;

#ifdef DUMP_DCR_DEBUG
        FILE *pDumpDcrIn;
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/before_dcr.pcm");
        pDumpDcrIn = fopen("/sdcard/mtklog/audio_dump/before_dcr.pcm", "ab");
        if (pDumpDcrIn == NULL) ALOGW("Fail to Open pDumpDcrIn");
        fwrite(inbuffer, sizeof(long), outputBufSize/sizeof(long), pDumpDcrIn);
        fclose(pDumpDcrIn);
#endif

#ifdef MTK_HD_AUDIO_ARCHITECTURE
        if(mSamplerate <=48000)
        {
            outputBytes = DCR_Process_24(mHandle, (long *)inbuffer, &inputBufSize, (long *)outbuffer, &outputBufSize);
        }
        else{              
            //ALOGD("DCR_Process_24 High Coef Precision");
            //memcpy(outbuffer, inbuffer, bytes);
            outputBytes = DCR_Process_24_High_Precision(mHandle, (long *)inbuffer, &inputBufSize, (long *)outbuffer, &outputBufSize);
        }
        //ALOGD("DCR_Process_24");
#else
        outputBytes = DCR_Process(mHandle, (short *)inbuffer, &inputBufSize, (short *)outbuffer, &outputBufSize);
        //ALOGD("DCR_Process");
#endif

        //ALOGD("DcRemove::process inputBufSize = %d,outputBufSize=%d,outputBytes=%d ", inputBufSize, outputBufSize, outputBytes);

#ifdef DUMP_DCR_DEBUG
        FILE *pDumpDcrOut;
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/after_dcr.pcm");
        pDumpDcrOut = fopen("/sdcard/mtklog/audio_dump/after_dcr.pcm", "ab");
        if (pDumpDcrOut == NULL) ALOGW("Fail to Open pDumpDcrOut");
        fwrite(outbuffer, sizeof(long), outputBufSize/sizeof(long), pDumpDcrOut);
        fclose(pDumpDcrOut);
#endif
        return outputBytes;
    }
    //SXLOGW("DcRemove::process Dcr not initialized");
#endif
    return 0;
}

}

