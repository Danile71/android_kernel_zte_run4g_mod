#include "AudioALSACaptureDataProviderNormal.h"

#include <pthread.h>

#include <linux/rtpm_prio.h>
#include <sys/prctl.h>

#include "AudioType.h"


#define LOG_TAG "AudioALSACaptureDataProviderNormal"

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec )+ (double)( x.tv_nsec - y.tv_nsec ) / (double)1000000000)

namespace android
{


/*==============================================================================
 *                     Constant
 *============================================================================*/

//static const uint32_t kReadBufferSize = 0x2000; // 8k
#ifdef RECORD_INPUT_24BITS // 24bit record
static const uint32_t kReadBufferSize = ((0x1e00)); // 48K\stereo\40ms data (framebase: get 40ms data/per time)
#else
static const uint32_t kReadBufferSize = ((0x1e00) / 2); // 48K\stereo\40ms data (framebase: get 40ms data/per time)
#endif

static const uint32_t kDCRReadBufferSize = 0x2EE00; //48K\stereo\1s data , calculate 1time/sec

//static FILE *pDCCalFile = NULL;


/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioALSACaptureDataProviderNormal *AudioALSACaptureDataProviderNormal::mAudioALSACaptureDataProviderNormal = NULL;
AudioALSACaptureDataProviderNormal *AudioALSACaptureDataProviderNormal::getInstance()
{
    AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSACaptureDataProviderNormal == NULL)
    {
        mAudioALSACaptureDataProviderNormal = new AudioALSACaptureDataProviderNormal();
    }
    ASSERT(mAudioALSACaptureDataProviderNormal != NULL);
    return mAudioALSACaptureDataProviderNormal;
}

AudioALSACaptureDataProviderNormal::AudioALSACaptureDataProviderNormal()
{
    ALOGD("%s()", __FUNCTION__);
    int pcmindex = AudioALSADeviceParser::getInstance()->GetPcmIndexByString(keypcmUl1Capture);
    int cardindex = AudioALSADeviceParser::getInstance()->GetCardIndexByString(keypcmUl1Capture);
    ALOGD("%s cardindex = %d  pcmindex = %d", __FUNCTION__, cardindex, pcmindex);

    struct pcm_params *params;
    params = pcm_params_get(cardindex, pcmindex,  PCM_IN);
    if (params == NULL)
    {
        ALOGD("Device does not exist.\n");
    }
    unsigned int buffersizemax = pcm_params_get_max(params, PCM_PARAM_BUFFER_BYTES);
    ALOGD("buffersizemax = %d", buffersizemax);
    pcm_params_free(params);

    // TODO(Harvey): query this
    mConfig.channels = 2;
    mConfig.rate = 48000;

    // Buffer size: 2048(period_size) * 2(ch) * 2(byte) * 8(period_count) = 64 kb
#ifdef RECORD_INPUT_24BITS // 24bit record
    mConfig.period_count = 2;
    mConfig.period_size = (buffersizemax / mConfig.channels) / 4 / mConfig.period_count;
    mConfig.format = PCM_FORMAT_S32_LE;
#else
    mConfig.period_count = 2;
    mConfig.period_size = (buffersizemax / mConfig.channels) / 2 / mConfig.period_count;
    mConfig.format = PCM_FORMAT_S16_LE;
#endif

    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;

    mCaptureDataProviderType = CAPTURE_PROVIDER_NORMAL;

    mCaptureDropSize = 0;

#ifdef MTK_VOW_SUPPORT
    //DC cal
    memset((void *)&mDCCalBuffer, 0, sizeof(mDCCalBuffer));
    mDCCalEnable = false;
    mDCCalBufferFull = false;
    mDCCalDumpFile = NULL;
#endif
}

AudioALSACaptureDataProviderNormal::~AudioALSACaptureDataProviderNormal()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSACaptureDataProviderNormal::open()
{
    ALOGD("%s()", __FUNCTION__);
    ASSERT(mClientLock.tryLock() != 0); // lock by base class attach
    AudioAutoTimeoutLock _l(mEnableLock);

    ASSERT(mEnable == false);

    // config attribute (will used in client SRC/Enh/... later) // TODO(Harvey): query this
    mStreamAttributeSource.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeSource.audio_channel_mask = AUDIO_CHANNEL_IN_STEREO;
    mStreamAttributeSource.num_channels = android_audio_legacy::AudioSystem::popCount(mStreamAttributeSource.audio_channel_mask);
    mStreamAttributeSource.sample_rate = 48000;

#ifdef RECORD_INPUT_24BITS // 24bit record
    mCaptureDropSize = ((mStreamAttributeSource.sample_rate * CAPTURE_DROP_MS << 3) / 1000);    //32bit, drop data which get from kernel
#else
    mCaptureDropSize = ((mStreamAttributeSource.sample_rate * CAPTURE_DROP_MS << 2) / 1000);    //16bit
#endif
    ALOGD("%s(), mCaptureDropSize=%d, CAPTURE_DROP_MS=%d", __FUNCTION__, mCaptureDropSize, CAPTURE_DROP_MS);

    OpenPCMDump(LOG_TAG);

    // enable pcm
    ASSERT(mPcm == NULL);
    mPcm = pcm_open(0, 1, PCM_IN | PCM_MONOTONIC, &mConfig);
    ASSERT(mPcm != NULL && pcm_is_ready(mPcm) == true);
    ALOGV("%s(), mPcm = %p", __FUNCTION__, mPcm);

    pcm_start(mPcm);

    // create reading thread
    mEnable = true;
    int ret = pthread_create(&hReadThread, NULL, AudioALSACaptureDataProviderNormal::readThread, (void *)this);
    if (ret != 0)
    {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

#ifdef MTK_VOW_SUPPORT
    //create DC cal thread
    ret = pthread_create(&hDCCalThread, NULL, AudioALSACaptureDataProviderNormal::DCCalThread, (void *)this);
    if (ret != 0)
    {
        ALOGE("%s() create DCCal thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }

    ret = pthread_mutex_init(&mDCCal_Mutex, NULL);
    if (ret != 0)
    {
        ALOGE("%s, Failed to initialize mDCCal_Mutex!", __FUNCTION__);
    }

    ret = pthread_cond_init(&mDCCal_Cond, NULL);
    if (ret != 0)
    {
        ALOGE("%s, Failed to initialize mDCCal_Cond!", __FUNCTION__);
    }

    mDCCalEnable = true;

    mDCCalBuffer.pBufBase = new char[kDCRReadBufferSize];
    mDCCalBuffer.bufLen   = kDCRReadBufferSize;
    mDCCalBuffer.pRead    = mDCCalBuffer.pBufBase;
    mDCCalBuffer.pWrite   = mDCCalBuffer.pBufBase;
    ASSERT(mDCCalBuffer.pBufBase != NULL);

    AudioAutoTimeoutLock _lDC(mDCCalEnableLock);
    mDCCalEnable = true;

    OpenDCCalDump();
#if 0
    pDCCalFile = fopen("/sdcard/mtklog/DCCalFile.pcm", "wb");
    if (pDCCalFile == NULL)
    {
        ALOGW("%s(), create pDCCalFile fail ", __FUNCTION__);
    }
#endif
    //create DC cal ---
#endif

    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderNormal::close()
{
    ALOGD("%s()", __FUNCTION__);
    ASSERT(mClientLock.tryLock() != 0); // lock by base class detach

    mEnable = false;
    AudioAutoTimeoutLock _l(mEnableLock);

    ClosePCMDump();

    pcm_stop(mPcm);
    pcm_close(mPcm);
    mPcm = NULL;

#ifdef MTK_VOW_SUPPORT
    mDCCalEnable = false;
    AudioAutoTimeoutLock _lDC(mDCCalEnableLock);
    pthread_cond_signal(&mDCCal_Cond);
#if 0
    if (pDCCalFile != NULL)
    {
        fclose(pDCCalFile);
    }
#endif

    CloseDCCalDump();

    if (mDCCalBuffer.pBufBase != NULL) { delete[] mDCCalBuffer.pBufBase; }
#endif

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSACaptureDataProviderNormal::GetCaptureTimeStamp(time_info_struct_t *Time_Info, size_t read_size)
{
    ALOGV("%s()", __FUNCTION__);
    ASSERT(mPcm != NULL);

    long ret_ns;
    size_t avail;
    Time_Info->timestamp_get.tv_sec  = 0;
    Time_Info->timestamp_get.tv_nsec = 0;
    Time_Info->frameInfo_get = 0;
    Time_Info->buffer_per_time = 0;
    Time_Info->kernelbuffer_ns = 0;

    //ALOGD("%s(), Going to check pcm_get_htimestamp", __FUNCTION__);
    if (pcm_get_htimestamp(mPcm, &Time_Info->frameInfo_get, &Time_Info->timestamp_get) == 0)
    {
        Time_Info->buffer_per_time = pcm_bytes_to_frames(mPcm, read_size);
        Time_Info->kernelbuffer_ns = 1000000000 / mStreamAttributeSource.sample_rate * (Time_Info->buffer_per_time + Time_Info->frameInfo_get);
        ALOGV("%s pcm_get_htimestamp sec= %ld, nsec=%ld, frameInfo_get = %d, buffer_per_time=%d, ret_ns = %ld\n",
              __FUNCTION__, Time_Info->timestamp_get.tv_sec, Time_Info->timestamp_get.tv_nsec, Time_Info->frameInfo_get,
              Time_Info->buffer_per_time, Time_Info->kernelbuffer_ns);
#if 0
        if ((TimeStamp->tv_nsec - ret_ns) >= 0)
        {
            TimeStamp->tv_nsec -= ret_ns;
        }
        else
        {
            TimeStamp->tv_sec -= 1;
            TimeStamp->tv_nsec = 1000000000 + TimeStamp->tv_nsec - ret_ns;
        }

        ALOGD("%s calculate pcm_get_htimestamp sec= %ld, nsec=%ld, avail = %d, ret_ns = %ld\n",
              __FUNCTION__, TimeStamp->tv_sec, TimeStamp->tv_nsec, avail, ret_ns);
#endif
    }
    else
    {
        ALOGE("%s pcm_get_htimestamp fail %s\n", __FUNCTION__, pcm_get_error(mPcm));
    }
    return NO_ERROR;
}

void *AudioALSACaptureDataProviderNormal::readThread(void *arg)
{
    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderNormal *pDataProvider = static_cast<AudioALSACaptureDataProviderNormal *>(arg);

    uint32_t open_index = pDataProvider->mOpenIndex;

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
    ALOGD("+%s(), pid: %d, tid: %d, kReadBufferSize=%x, open_index=%d", __FUNCTION__, getpid(), gettid(), kReadBufferSize, open_index);


    // read raw data from alsa driver
    char linear_buffer[kReadBufferSize];
    uint32_t Read_Size = kReadBufferSize;
    uint32_t kReadBufferSize_new;
    while (pDataProvider->mEnable == true)
    {
        if (open_index != pDataProvider->mOpenIndex)
        {
            ALOGD("%s(), open_index(%d) != mOpenIndex(%d), return", __FUNCTION__, open_index, pDataProvider->mOpenIndex);
            break;
        }

        retval = pDataProvider->mEnableLock.lock_timeout(300);
        ASSERT(retval == NO_ERROR);
        if (pDataProvider->mEnable == false)
        {
            pDataProvider->mEnableLock.unlock();
            break;
        }

        ASSERT(pDataProvider->mPcm != NULL);
        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[0] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;

        if (pDataProvider->mCaptureDropSize > 0)
        {
            Read_Size = (pDataProvider->mCaptureDropSize > kReadBufferSize) ? kReadBufferSize : pDataProvider->mCaptureDropSize;
            int retval = pcm_read(pDataProvider->mPcm, linear_buffer, Read_Size);

            if (retval != 0)
            {
                ALOGE("%s(), pcm_read() drop error, retval = %d", __FUNCTION__, retval);
            }
            ALOGD("%s(), mCaptureDropSize = %d, Read_Size=%d", __FUNCTION__, pDataProvider->mCaptureDropSize, Read_Size);
            pDataProvider->mCaptureDropSize -= Read_Size;
            pDataProvider->mEnableLock.unlock();
            continue;
        }
        else
        {
            int retval = pcm_read(pDataProvider->mPcm, linear_buffer, kReadBufferSize);
            if (retval != 0)
            {
                ALOGE("%s(), pcm_read() error, retval = %d", __FUNCTION__, retval);
            }
        }
        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[1] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;

        //struct timespec tempTimeStamp;
        pDataProvider->GetCaptureTimeStamp(&pDataProvider->mStreamAttributeSource.Time_Info, kReadBufferSize);

#ifdef RECORD_INPUT_24BITS // 24bit record
        uint32_t *ptr32bit_r = (uint32_t *)linear_buffer;
        int16_t *ptr16bit_w = (int16_t *)linear_buffer;
        int i;
        ALOGV("24bit record, kReadBufferSize=%d, init ptr32bit_r=0x%x, ptr16bit_w =0x%x", kReadBufferSize, ptr32bit_r, ptr16bit_w);
        for (i = 0; i < kReadBufferSize / 4; i++)
        {
            *(ptr16bit_w + i) = (int16_t)(*(ptr32bit_r + i) >> 8);
        }
        kReadBufferSize_new = kReadBufferSize >> 1;
#else
        kReadBufferSize_new = kReadBufferSize;
#endif

#ifdef MTK_VOW_SUPPORT
        //copy data to DC Cal
        pDataProvider->copyCaptureDataToDCCalBuffer(linear_buffer, kReadBufferSize_new);
#endif

        // use ringbuf format to save buffer info
        pDataProvider->mPcmReadBuf.pBufBase = linear_buffer;
        pDataProvider->mPcmReadBuf.bufLen   = kReadBufferSize_new + 1; // +1: avoid pRead == pWrite
        pDataProvider->mPcmReadBuf.pRead    = linear_buffer;
        pDataProvider->mPcmReadBuf.pWrite   = linear_buffer + kReadBufferSize_new;
        pDataProvider->mEnableLock.unlock();

        pDataProvider->provideCaptureDataToAllClients(open_index);

        clock_gettime(CLOCK_REALTIME, &pDataProvider->mNewtime);
        pDataProvider->timerec[2] = calc_time_diff(pDataProvider->mNewtime, pDataProvider->mOldtime);
        pDataProvider->mOldtime = pDataProvider->mNewtime;
        ALOGD("%s, latency_in_us,%1.6lf,%1.6lf,%1.6lf", __FUNCTION__, pDataProvider->timerec[0], pDataProvider->timerec[1], pDataProvider->timerec[2]);
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    pthread_exit(NULL);
    return NULL;
}

#ifdef MTK_VOW_SUPPORT
void AudioALSACaptureDataProviderNormal::copyCaptureDataToDCCalBuffer(void *buffer, size_t size)
{
    size_t copysize = size;
    uint32_t freeSpace = RingBuf_getFreeSpace(&mDCCalBuffer);
    ALOGV("%s(), freeSpace(%u), dataSize(%u),mDCCalBufferFull=%d", __FUNCTION__, freeSpace, size, mDCCalBufferFull);

    if (mDCCalBufferFull == false)
    {
        if (freeSpace > 0)
        {
            if (freeSpace < size)
            {
                ALOGD("%s(), freeSpace(%u) < dataSize(%u), buffer full!!", __FUNCTION__, freeSpace, size);
                //ALOGD("%s before,pBase = 0x%x pWrite = 0x%x  bufLen = %d  pRead = 0x%x",__FUNCTION__,
                //mDCCalBuffer.pBufBase,mDCCalBuffer.pWrite, mDCCalBuffer.bufLen,mDCCalBuffer.pRead);

                RingBuf_copyFromLinear(&mDCCalBuffer, (char *)buffer, freeSpace);

                //ALOGD("%s after,pBase = 0x%x pWrite = 0x%x  bufLen = %d  pRead = 0x%x",__FUNCTION__,
                //mDCCalBuffer.pBufBase,mDCCalBuffer.pWrite, mDCCalBuffer.bufLen,mDCCalBuffer.pRead);
            }
            else
            {
                //ALOGD("%s before,pBase = 0x%x pWrite = 0x%x  bufLen = %d  pRead = 0x%x",__FUNCTION__,
                //mDCCalBuffer.pBufBase,mDCCalBuffer.pWrite, mDCCalBuffer.bufLen,mDCCalBuffer.pRead);

                RingBuf_copyFromLinear(&mDCCalBuffer, (char *)buffer, size);

                //ALOGD("%s after,pBase = 0x%x pWrite = 0x%x  bufLen = %d  pRead = 0x%x",__FUNCTION__,
                //mDCCalBuffer.pBufBase,mDCCalBuffer.pWrite, mDCCalBuffer.bufLen,mDCCalBuffer.pRead);
            }
        }
        else
        {
            mDCCalBufferFull = true;
            pthread_cond_signal(&mDCCal_Cond);
        }
    }
}

size_t AudioALSACaptureDataProviderNormal::CalulateDC(short *buffer , size_t size)
{
    //ALOGV("%s()+,Size(%u)", __FUNCTION__, size);
    int checksize = size >> 2;  //stereo, 16bits
    int count = checksize;
    int accumulateL = 0, accumulateR = 0;
    short DCL = 0, DCR = 0;

#if 0
    if (pDCCalFile != NULL)
    {
        fwrite(buffer, sizeof(char), size, pDCCalFile);
    }
#endif
    WriteDCCalDumpData((void *)buffer, size);

    while (count)
    {
        accumulateL += *(buffer);
        accumulateR += *(buffer + 1);
        buffer += 2;
        count--;
    }
    DCL = (short)(accumulateL / checksize);
    DCR = (short)(accumulateR / checksize);

    ALOGD("%s()- ,checksize(%d),accumulateL(%d),accumulateR(%d), DCL(%d), DCR(%d)", __FUNCTION__, checksize, accumulateL, accumulateR, DCL, DCR);
    return size;
}

void *AudioALSACaptureDataProviderNormal::DCCalThread(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    ALOGD("+%s(), pid: %d, tid: %d, kDCRReadBufferSize=%x", __FUNCTION__, getpid(), gettid(), kDCRReadBufferSize);

    status_t retval = NO_ERROR;
    AudioALSACaptureDataProviderNormal *pDataProvider = static_cast<AudioALSACaptureDataProviderNormal *>(arg);


    //char linear_buffer[kDCRReadBufferSize];
    char *plinear_buffer = new char[kDCRReadBufferSize];
    uint32_t Read_Size = kDCRReadBufferSize;
    while (pDataProvider->mDCCalEnable == true)
    {
        pthread_mutex_lock(&pDataProvider->mDCCal_Mutex);
        pthread_cond_wait(&pDataProvider->mDCCal_Cond, &pDataProvider->mDCCal_Mutex);
        //ALOGD("%s(), signal get", __FUNCTION__);

        retval = pDataProvider->mDCCalEnableLock.lock_timeout(300);
        ASSERT(retval == NO_ERROR);
        if (pDataProvider->mDCCalEnable == false)
        {
            pDataProvider->mDCCalEnableLock.unlock();
            pthread_mutex_unlock(&pDataProvider->mDCCal_Mutex);
            break;
        }

        if (pDataProvider->mDCCalBufferFull)
        {
            Read_Size = RingBuf_getDataCount(&pDataProvider->mDCCalBuffer);
            //ALOGD("%s(), Read_Size =%u, kDCRReadBufferSize=%u", __FUNCTION__,Read_Size,kDCRReadBufferSize);
            if (Read_Size > kDCRReadBufferSize)
            {
                Read_Size = kDCRReadBufferSize;
            }

            //ALOGD("%s,pBase = 0x%x pWrite = 0x%x  bufLen = %d  pRead = 0x%x",__FUNCTION__,
            //pDataProvider->mDCCalBuffer.pBufBase,pDataProvider->mDCCalBuffer.pWrite, pDataProvider->mDCCalBuffer.bufLen,pDataProvider->mDCCalBuffer.pRead);

            RingBuf_copyToLinear(plinear_buffer, &pDataProvider->mDCCalBuffer, Read_Size);
            //ALOGD("%s after copy,pBase = 0x%x pWrite = 0x%x  bufLen = %d  pRead = 0x%x",__FUNCTION__,
            //pDataProvider->mDCCalBuffer.pBufBase,pDataProvider->mDCCalBuffer.pWrite, pDataProvider->mDCCalBuffer.bufLen,pDataProvider->mDCCalBuffer.pRead);
            pDataProvider->CalulateDC((short *)plinear_buffer, Read_Size);

            pDataProvider->mDCCalBufferFull = false;
        }

        pDataProvider->mDCCalEnableLock.unlock();
        pthread_mutex_unlock(&pDataProvider->mDCCal_Mutex);
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    delete[] plinear_buffer;
    pthread_exit(NULL);
    return NULL;
}


void AudioALSACaptureDataProviderNormal::OpenDCCalDump()
{
    ALOGV("%s()", __FUNCTION__);
    char DCCalDumpFileName[128];
    sprintf(DCCalDumpFileName, "%s.pcm", "/sdcard/mtklog/audio_dump/DCCalData");

    mDCCalDumpFile = NULL;
    mDCCalDumpFile = AudioOpendumpPCMFile(DCCalDumpFileName, streamin_propty);

    if (mDCCalDumpFile != NULL)
    {
        ALOGD("%s, DCCalDumpFileName = %s", __FUNCTION__, DCCalDumpFileName);
    }
}

void AudioALSACaptureDataProviderNormal::CloseDCCalDump()
{
    if (mDCCalDumpFile)
    {
        AudioCloseDumpPCMFile(mDCCalDumpFile);
        ALOGD("%s()", __FUNCTION__);
    }
}

void  AudioALSACaptureDataProviderNormal::WriteDCCalDumpData(void *buffer , size_t size)
{
    if (mDCCalDumpFile)
    {
        //ALOGD("%s()", __FUNCTION__);
        AudioDumpPCMData((void *)buffer , size, mDCCalDumpFile);
    }
}
#endif

} // end of namespace android
