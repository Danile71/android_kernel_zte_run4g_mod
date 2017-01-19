#include "AudioALSACaptureDataProviderVOW.h"

#include <pthread.h>

#include <linux/rtpm_prio.h>
#include <sys/prctl.h>

#include "AudioType.h"

#include <hardware_legacy/power.h>

#define LOG_TAG "AudioALSACaptureDataProviderVOW"

static const char VOW_DEBUG_WAKELOCK_NAME[] = "VOW_DEBUG_WAKELOCK_NAME";

namespace android
{


/*==============================================================================
 *                     Constant
 *============================================================================*/

static const uint32_t kReadBufferSize = 0xA00;



/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderVOW *AudioALSACaptureDataProviderVOW::mAudioALSACaptureDataProviderVOW = NULL;
AudioALSACaptureDataProviderVOW *AudioALSACaptureDataProviderVOW::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderVOW == NULL)
    {
        mAudioALSACaptureDataProviderVOW = new AudioALSACaptureDataProviderVOW();
    }
    ASSERT(mAudioALSACaptureDataProviderVOW != NULL);
    ALOGD("%s(), mAudioALSACaptureDataProviderVOW=%p", __FUNCTION__, mAudioALSACaptureDataProviderVOW);
    return mAudioALSACaptureDataProviderVOW;
}

AudioALSACaptureDataProviderVOW::AudioALSACaptureDataProviderVOW()
{
    //use for VOW phase 2 debug tempory. Read from VOW kernel driver, not from PCM
    ALOGD("%s()", __FUNCTION__);

    mCaptureDataProviderType = CAPTURE_PROVIDER_VOW;
    mFd = 0;
    mFd = ::open(kVOWDeviceName, O_RDWR);

    if (mFd == 0)
    {
        ALOGE("%s(), open mFd fail", __FUNCTION__);
        ASSERT(mFd != 0);
    }
    ALOGD("%s()-", __FUNCTION__);
}

AudioALSACaptureDataProviderVOW::~AudioALSACaptureDataProviderVOW()
{
    ALOGD("%s()", __FUNCTION__);
    if (mFd > 0)
    {
        ::close(mFd);
        mFd = 0;
    }
    ALOGD("%s()-", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderVOW::open()
{
    ALOGD("%s()+", __FUNCTION__);
    //ASSERT(mClientLock.tryLock() != 0); // lock by base class attach
    AudioAutoTimeoutLock _l(mEnableLock);

    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Harvey): query this
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_MONO;
    mStreamAttributeSource.num_channels = android_audio_legacy::AudioSystem::popCount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = 16000;


    OpenPCMDump(LOG_TAG);

    //open VOW driver
    // enable pcm
    // here open audio hardware for register setting

    if (mFd == 0)
    {
        ALOGE("%s(), open mFd fail", __FUNCTION__);
        ASSERT(mFd != 0);
    }

    memset(&vow_info_buf, 0, sizeof(vow_info_buf));

    // create reading thread
    //mOpenIndex++;
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderVOW::readThread, (void *)this);
    if (ret != 0)
    {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    acquire_wake_lock(PARTIAL_WAKE_LOCK, VOW_DEBUG_WAKELOCK_NAME);
    ALOGD("%s()-", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderVOW::close()
{
    ALOGD("%s()", __FUNCTION__);
    int ret;
    //ASSERT(mClientLock.tryLock() != 0); // lock by base class detach

    mEnable = false;

    ret = ::ioctl(mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_DisableDebug);
    ALOGD("%s(), VOWControlCmd_DisableDebug set, ret = %d", __FUNCTION__, ret);
    if (ret != 0)
    {
        ALOGE("%s(), VOWControlCmd_DisableDebug error, ret = %d", __FUNCTION__, ret);
    }
    AudioAutoTimeoutLock _l(mEnableLock);

    ClosePCMDump();

    //close VOW kernel driver
    release_wake_lock(VOW_DEBUG_WAKELOCK_NAME);
    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void  AudioALSACaptureDataProviderVOW::WriteVOWPcmData()
{
    ALOGV("+%s()", __FUNCTION__);
    if (mPCMDumpFile)
    {
        //ALOGD("%s()", __FUNCTION__);
        AudioDumpPCMData((void *)vow_info_buf.addr , vow_info_buf.size, mPCMDumpFile);
    }
    ALOGV("-%s()", __FUNCTION__);
}

void *AudioALSACaptureDataProviderVOW::readThread(void *arg)
{
    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderVOW *pDataProvider = static_cast<AudioALSACaptureDataProviderVOW *>(arg);


    char nameset[32];
    sprintf(nameset, "%s%d", __FUNCTION__, pDataProvider->mCaptureDataProviderType);
    prctl(PR_SET_NAME, (unsigned long)nameset, 0, 0, 0);

#ifdef MTK_AUDIO_ADJUST_PRIORITY
    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_RECORD + 1;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
    {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    }
    else
    {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_RECORD + 1;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
#endif
    ALOGD("+%s(), pid: %d, tid: %d, kReadBufferSize=%x", __FUNCTION__, getpid(), gettid(), kReadBufferSize);

    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    uint32_t Read_Size = kReadBufferSize;

    //memset(linear_buffer, 0xCCCC, Read_Size);

    pDataProvider->vow_info_buf.addr = (long)linear_buffer;
    pDataProvider->vow_info_buf.size = (long)Read_Size;
    unsigned int vow_info = (unsigned int)&pDataProvider->vow_info_buf;

    ALOGD("%s(), vow_info = %u, addr=%lu, size=%ld", __FUNCTION__, vow_info, pDataProvider->vow_info_buf.addr, pDataProvider->vow_info_buf.size);

    int ret = ::ioctl(pDataProvider->mFd, VOW_SET_APREG_INFO, vow_info);
    ALOGD("%s(), VOW_SET_APREG_INFO set, ret = %d", __FUNCTION__, ret);
    if (ret != 0)
    {
        ALOGE("%s(), VOW_SET_APREG_INFO error, ret = %d", __FUNCTION__, ret);
    }

    ret = ::ioctl(pDataProvider->mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_EnableDebug);
    ALOGD("%s(), VOWControlCmd_EnableDebug set, ret = %d", __FUNCTION__, ret);
    if (ret != 0)
    {
        ALOGE("%s(), VOWControlCmd_EnableDebug error, ret = %d", __FUNCTION__, ret);
    }

    while (pDataProvider->mEnable == true)
    {
#if 0
        if (open_index != pDataProvider->mOpenIndex)
        {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }
#endif

        ALOGD("%s(), try to get enable lock", __FUNCTION__);
        retval = pDataProvider->mEnableLock.lock_timeout(300);
        ASSERT(retval == NO_ERROR);
        if (pDataProvider->mEnable == false)
        {
            pDataProvider->mEnableLock.unlock();
            ALOGD("%s(), readthread disable", __FUNCTION__);
            break;
        }

        //ASSERT(pDataProvider->mPcm != NULL);
        ALOGV("%s(), VOW_SET_CONTROL set", __FUNCTION__);
        int retval = ::ioctl(pDataProvider->mFd, VOW_SET_CONTROL, (unsigned int)VOWControlCmd_ReadVoiceData);
        //usleep(30 * 1000);
        if (retval != 0)
        {
            ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
        }
        else
        {
            ALOGV("%s(), pcm_read() retval = %d", __FUNCTION__, retval);
        }

        pDataProvider->mEnableLock.unlock();

        pDataProvider->WriteVOWPcmData();

    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

} // end of namespace android
