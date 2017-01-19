#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/rtpm_prio.h>
#include <sys/prctl.h>

#include <cutils/properties.h>

#include "AudioLock.h"

#include "SpeechMessengerECCCI.h"
#include "SpeechDriverLAD.h"
#include "SpeechBGSPlayer.h"
#include "SpeechPcm2way.h"
#include "SpeechVMRecorder.h"

#include "AudioALSACaptureDataProviderVoice.h"


//#include "AudioMTKStreamInManager.h"
//#include "AudioResourceManager.h"
#include "hardware/ccci_intf.h"

#define LOG_TAG "SpeechMessengerECCCI"

#ifndef sph_msleep
#define sph_msleep(ms) usleep((ms)*1000)
#endif

namespace android
{

/** CCCI driver & ioctl */
/* ECCCI channel buffer structure */
typedef struct
{
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
    unsigned int payload[CCCI_MAX_PAYLOAD_SIZE];
} ECCCI_BUFF_T;

/** CCCI ioctl */

/** CCCI modem status */
static const unsigned int MODEM_STATUS_INVALID = 0; // Boot stage 0 -> Means MD Does NOT run
static const unsigned int MODEM_STATUS_INIT    = 1; // Boot stage 1 -> Means MD begin to run, but not ready
static const unsigned int MODEM_STATUS_READY   = 2; // Boot stage 2 -> Means MD is ready
static const unsigned int MODEM_STATUS_EXPT    = 3; // MD exception -> Means EE occur

/** Property keys*/
static const char PROPERTY_KEY_MODEM_STATUS[NUM_MODEM][PROPERTY_KEY_MAX] = {"af.modem_1.status", "af.modem_2.status", "af.modem_ext.status"};
static const char PROPERTY_KEY_RF_INFO[2][PROPERTY_KEY_MAX] = {"af.rf_info", "af.rf_mode"};


/** CCCI channel No */
static const uint8_t    CCCI_M2A_CHANNEL = 4;
static const uint8_t    CCCI_A2M_CHANNEL = 5;

/** CCCI magic number */
static const uint32_t   CCCI_MAILBOX_MAGIC_NUMBER = 0xFFFFFFFF;
bool mA2M_ECCCI_DATA_READ_ACK;


SpeechMessengerECCCI::SpeechMessengerECCCI(modem_index_t modem_index, SpeechDriverLAD *pLad) :
    mModemIndex(modem_index),
    mLad(pLad)
{
    ALOGD("%s()", __FUNCTION__);
    CCCIEnable = false;
    mA2M_ECCCI_DATA_READ_ACK = false;

    fHdl = -1;

    mA2MShareBufLen = 0;
    mM2AShareBufLen = 0;

    mA2MShareBufBase = NULL;
    mM2AShareBufBase = NULL;

    mA2MShareBufEnd = NULL;
    mM2AShareBufEnd = NULL;

    memset(&mM2AShareBuf, 0, sizeof(mM2AShareBuf));

    //initial the message queue
    memset((void *)pQueue, 0, sizeof(pQueue));
    iQRead = 0;
    iQWrite = 0;

    mWaitAckMessageID = 0;

    //initial modem side modem status
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_MODEM_STATUS[mModemIndex], property_value, "0");  //"0": default all off
    mModemSideModemStatus = atoi(property_value);
    ALOGD("%s(), mModemIndex(%d), property read(0x%x)", __FUNCTION__, mModemIndex, mModemSideModemStatus);

    ResetSpeechParamAckCount();
}

status_t SpeechMessengerECCCI::Initial()
{
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_ECCCI_LPBK_TEST)    //use eccci loopback for test
    fHdl  = open("/dev/ccci_lp", O_RDWR);
    if (fHdl < 0)
    {
        ALOGE("%s() fail to open/dev/ccci_lp ", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
#else
    // Open CCCI and get handler
    char dev_node[32];
    // Open CCCI and get handler
    switch (mModemIndex)
    {
        case MODEM_1:
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS1));
            break;
        case MODEM_2:
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS2));
            break;
    }
    ALOGD("%s(), dev_node=%s", __FUNCTION__, dev_node);
    fHdl = open(dev_node, O_RDWR);
    if (fHdl < 0)
    {
        ALOGE("%s() fail to open %s, errno: %d", __FUNCTION__, dev_node, errno);
        return UNKNOWN_ERROR;
    }
#endif


    // Get total share buffer length & base address
    const uint32_t share_buf_length = 32000;
    mECCCIShareBuf = new char[share_buf_length];

    char *share_buf_address =  mECCCIShareBuf;
    ALOGD("%s(), share_buf_address: %p, share_buf_length: %u", __FUNCTION__, share_buf_address, share_buf_length);

    mA2MShareBufLen = share_buf_length >> 1; // a2m buffer lengh should be half of share_buf_length
    mM2AShareBufLen = share_buf_length >> 1; // m2a buffer lengh should be half of share_buf_length

    mA2MShareBufBase = share_buf_address;
    mM2AShareBufBase = share_buf_address + mA2MShareBufLen;

    mA2MShareBufEnd = mA2MShareBufBase + mA2MShareBufLen;
    mM2AShareBufEnd = mM2AShareBufBase + mM2AShareBufLen;

    mM2AShareBufRead = mM2AShareBufBase;

    mM2AShareBuf.bufLen   = mM2AShareBufLen;
    mM2AShareBuf.pBufBase = mM2AShareBufBase;
    mM2AShareBuf.pRead    = mM2AShareBuf.pBufBase;
    mM2AShareBuf.pWrite   = mM2AShareBuf.pBufBase;

    /* create the CCCI event reading thread */
    CCCIEnable = true;
    return CreateReadingThread();
}

status_t SpeechMessengerECCCI::Deinitial()
{
    ALOGD("%s()", __FUNCTION__);
    CCCIEnable = false;

    if (fHdl >= 0)
    {
        close(fHdl);
        fHdl = -1;
    }

    if (mECCCIShareBuf != NULL)
    {
        delete mECCCIShareBuf;
        mECCCIShareBuf = NULL;

        mA2MShareBufLen = 0;
        mM2AShareBufLen = 0;

        mA2MShareBufBase = NULL;
        mM2AShareBufBase = NULL;

        mA2MShareBufEnd = NULL;
        mM2AShareBufEnd = NULL;
    }

    return NO_ERROR;
}


SpeechMessengerECCCI::~SpeechMessengerECCCI()
{
    ALOGD("%s()", __FUNCTION__);
}

/** Create CCCI message */
ccci_buff_t SpeechMessengerECCCI::InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit)
{
    ccci_buff_t ccci_buff;
    memset(&ccci_buff, 0, sizeof(ccci_buff));


    //package message
    switch (id)
    {
        case MSG_A2M_PNW_DL_DATA_NOTIFY:
        case MSG_A2M_BGSND_DATA_NOTIFY:
        case MSG_A2M_CTM_DATA_NOTIFY:
        case MSG_A2M_EM_NB:
        case MSG_A2M_EM_WB:
        case MSG_A2M_EM_DMNR:
        case MSG_A2M_EM_MAGICON:
        case MSG_A2M_EM_HAC:
        case MSG_A2M_VIBSPK_PARAMETER:
        case MSG_A2M_NXP_SMARTPA_PARAMETER:
            ccci_buff.magic    = param_32bit;//offset
            ccci_buff.message  = param_16bit;//payload length
            ccci_buff.channel  = CCCI_A2M_CHANNEL;
            ccci_buff.reserved = (id << 16) | param_16bit;
            break;
        default:
            ccci_buff.magic    = CCCI_MAILBOX_MAGIC_NUMBER;
            ccci_buff.message  = (id << 16) | param_16bit;
            ccci_buff.channel  = CCCI_A2M_CHANNEL;
            ccci_buff.reserved = param_32bit;
            break;
    }


    return ccci_buff;
}

/** Get CCCI message's ID */
uint16_t SpeechMessengerECCCI::GetMessageID(const ccci_buff_t &ccci_buff)
{
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER)
    {
        return (ccci_buff.reserved) >> 16;
    }
    else
    {
        return (ccci_buff.message) >> 16;
    }
}

/** Get CCCI message's parameters */
uint16_t SpeechMessengerECCCI::GetMessageParam(const ccci_buff_t &ccci_buff)
{
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER)
    {
        return ccci_buff.reserved;
    }
    else
    {
        return ccci_buff.message;
    }
}

uint16_t SpeechMessengerECCCI::GetMessageLength(const ccci_buff_t &ccci_buff)
{
    //payload length(6+data)
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER)
    {
        return ccci_buff.message;
    }
    else
    {
        return 0;
    }
}

uint16_t SpeechMessengerECCCI::GetMessageOffset(const ccci_buff_t &ccci_buff)
{
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER)
    {
        return ccci_buff.magic;
    }
    else
    {
        return 0;//no offset
    }
}


char SpeechMessengerECCCI::GetModemCurrentStatus()
{
    unsigned int  status = (unsigned int)MODEM_STATUS_INVALID;

    int retval = ::ioctl(fHdl, CCCI_IOC_GET_MD_STATE, &status);
    if (retval < 0)
    {
        ALOGE("%s() ioctl CCCI_IOC_GET_MD_STATE fail!! retval = %d, errno: %d", __FUNCTION__, retval, errno);
        status = MODEM_STATUS_EXPT;
    }
#if defined(MTK_ECCCI_LPBK_TEST)    //use eccci loopback for test, md status always set ready
    status = MODEM_STATUS_READY;
#endif
    return (char)status;
}

bool SpeechMessengerECCCI::CheckModemIsReady()
{
    return (GetModemCurrentStatus() == MODEM_STATUS_READY);
}

status_t SpeechMessengerECCCI::WaitUntilModemReady()
{
    char status = 0;
    uint32_t trycnt = 0;
    const uint32_t kMaxTryCnt = 25; // total 2.5 sec
    do
    {
        status = GetModemCurrentStatus();
        if (status == MODEM_STATUS_READY)
        {
            ALOGD("%s(): Modem ready", __FUNCTION__);
            break;
        }
        else
        {
            ALOGW("Wait CCCI open #%d times, modem current status = %d, errno: %d", ++trycnt, status, errno);
            sph_msleep(100);
            if (trycnt == kMaxTryCnt) { break; }
        }
    }
    while (1);

    return (trycnt < kMaxTryCnt) ? NO_ERROR : TIMED_OUT;
}


int32_t SpeechMessengerECCCI::SendMessage(const ccci_buff_t &ccci_buff)
{
    ECCCI_BUFF_T msgBuf;
    uint16_t length_write, size_message;
    int8_t *pShareBuf;
    ALOGV("%s()", __FUNCTION__);

    // check if already initialized
    if (fHdl < 0)
    {
        if (Initial() != NO_ERROR)
        {
            return UNKNOWN_ERROR;
        }
    }

    // check if need ack
    uint16_t message_id = GetMessageID(ccci_buff);
    const bool b_need_ack = (JudgeAckOfMsg(message_id) == MESSAGE_NEED_ACK) ? true : false;
    uint16_t size_payload = GetMessageLength(ccci_buff);//(6+len_data)
    uint16_t offset_buffer = GetMessageOffset(ccci_buff);

    // check modem status during phone call
    char modem_status = GetModemCurrentStatus();
    if (modem_status != MODEM_STATUS_READY)
    {
        ALOGE("%s(), modem_status(%d) != MODEM_STATUS_READY, errno: %d", __FUNCTION__, modem_status, errno);
        mIsModemResetDuringPhoneCall = true;
        ResetSpeechParamAckCount();
    }

    // Do not send any on/off message when mIsModemResetDuringPhoneCall is true
    if (mIsModemResetDuringPhoneCall == true && IsModemFunctionOnOffMessage(message_id) == true)
    {
        ALOGE("%s(), mIsModemResetDuringPhoneCall == true, drop on/off message: 0x%x", __FUNCTION__, ccci_buff.message);
        SendMsgFailErrorHandling(ccci_buff);

        // clean mIsModemResetDuringPhoneCall when phone call/loopback stop
        if (message_id == MSG_A2M_SPH_OFF)
        {
            ALOGD("%s(), Phone call stop. Set mIsModemResetDuringPhoneCall = false", __FUNCTION__);
            mIsModemResetDuringPhoneCall = false;
        }
        else if (message_id == MSG_A2M_SET_ACOUSTIC_LOOPBACK)
        {
            const bool loopback_on = GetMessageParam(ccci_buff) & 0x1;
            if (loopback_on == false)
            {
                ALOGD("%s(), loopback stop. Set mIsModemResetDuringPhoneCall = false", __FUNCTION__);
                mIsModemResetDuringPhoneCall = false;
            }
        }

        return UNKNOWN_ERROR;
    }

    // save ack info before write to avoid race condition with CCCIReadThread
    if (b_need_ack == true)
    {
        mWaitAckMessageID = message_id;
    }

    // send message
    int i = 0;
    int write_length = 0;
    modem_status = 0;
    status_t ret = UNKNOWN_ERROR;
    message_id = GetMessageID(ccci_buff);
    memcpy(&msgBuf, &ccci_buff, sizeof(ccci_buff_t));
    size_message = size_payload + CCCI_BUF_HEADER_SIZE;
    //package message
    switch (message_id)
    {
        case MSG_A2M_PNW_DL_DATA_NOTIFY:
        case MSG_A2M_BGSND_DATA_NOTIFY:
        case MSG_A2M_CTM_DATA_NOTIFY:
        case MSG_A2M_EM_NB:
        case MSG_A2M_EM_WB: // case AUDIO_HD_RECORD_PARAMETER:
        case MSG_A2M_EM_DMNR:
        case MSG_A2M_EM_MAGICON:
        case MSG_A2M_EM_HAC:
        case MSG_A2M_VIBSPK_PARAMETER:
        case MSG_A2M_NXP_SMARTPA_PARAMETER:
            msgBuf.data[0] = 0;
            //copy buffer
            pShareBuf = (int8_t *)GetA2MShareBufBase() + offset_buffer;
            memcpy(msgBuf.payload, pShareBuf, size_payload);
            ALOGV("SendMessage with Buffer: msgBuf.message = 0x%x, size_message = 0x%x, size_payload = 0x%x", msgBuf.data[1], size_message, size_payload);
            break;
        default:
            ALOGV("SendMessage only command: msgBuf.message = 0x%x, size_message = 0x%x", msgBuf.data[1], size_message);
            break;
    }
    //    ALOGD("SendMessage ccci_buff.magic = 0x%x, message=0x%x, channel = 0x%x, reserved=0x%x", ccci_buff.magic, ccci_buff.message, ccci_buff.channel, msgBuf.reserved);
    ALOGD("SendMessage msgBuf.data[0] = 0x%x, data[1]=0x%x, ch = 0x%x, reserved=0x%x", msgBuf.data[0], msgBuf.data[1], msgBuf.channel , msgBuf.reserved);


    for (i = 0; i < 150; i++)   // try 150 times for every 2 ms if sent message fail
    {
        length_write = write(fHdl, (void *)&msgBuf, size_message);
        ALOGV("%s() write %d times, return length_write=%d, size_message=%d", __FUNCTION__, i, length_write, size_message);

        if (length_write == size_message)
        {
            ret = NO_ERROR;
            break;
        }
        else
        {
            modem_status = GetModemCurrentStatus();
            ALOGW("%s(), message: 0x%x, try: #%d, write_length: %d, errno: %d, modem status: %d",
                  __FUNCTION__, ccci_buff.message, i, write_length, errno, modem_status);

            if (errno == 3 || modem_status != MODEM_STATUS_READY)   // md reset cause ccci message fail
            {
                ALOGE("%s(), MD RESET SKIP MESSAGE: 0x%x", __FUNCTION__, ccci_buff.message);
                // if modem reset during phone call, raise mIsModemResetDuringPhoneCall
                if (message_id != MSG_A2M_SPH_OFF && message_id != MSG_A2M_SET_ACOUSTIC_LOOPBACK)
                {
                    mIsModemResetDuringPhoneCall = true;
                }
                ResetSpeechParamAckCount();
                break;
            }
            sph_msleep(2);
        }
    }

    // error handling for ack message
    if (ret != NO_ERROR && b_need_ack == true)
    {
        mWaitAckMessageID = 0;
        SendMsgFailErrorHandling(ccci_buff);
    }

    return ret;

}

status_t SpeechMessengerECCCI::ReadMessage(ccci_buff_t &ccci_buff)
{
    uint32_t length_payload, modem_status, temp_msg1, temp_msg2;
    ALOGV("%s()", __FUNCTION__);

    /* check if already initialized */
    if (fHdl < 0)
    {
        if (Initial() != NO_ERROR)
        {
            return UNKNOWN_ERROR;
        }
    }

    /* read message */
    int length_read = read(fHdl, (void *)&ccci_buff, sizeof(ccci_buff_t));
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER)
    {
        ccci_buff.message -= CCCI_BUF_HEADER_SIZE; // reduce header size
    }
    if (length_read != -1)
    {
        ALOGD("ReadMessage ECCCI, length_read=%d, data[0](magic)=0x%x, data[1](message)=0x%x, ch=0x%x, reserved=0x%x", length_read, ccci_buff.magic, ccci_buff.message, ccci_buff.channel, ccci_buff.reserved);
    }
    modem_status = GetModemCurrentStatus();

    if (length_read != -1  && modem_status != MODEM_STATUS_READY)
    {
        ALOGE("%s() fail, read_length: %d, modem current status: %d, errno: %d", __FUNCTION__, length_read, modem_status, errno);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}


uint32_t SpeechMessengerECCCI::GetQueueCount() const
{
    int32_t count = (iQWrite - iQRead);
    if (count < 0) { count += CCCI_MAX_QUEUE_NUM; }
    return count;
}

bool SpeechMessengerECCCI::CheckOffsetAndLength(const ccci_buff_t &ccci_buff)
{
    uint16_t message_id = GetMessageID(ccci_buff);
    uint16_t length     = GetMessageLength(ccci_buff);
    uint16_t offset     = GetMessageOffset(ccci_buff);

    if (offset > mM2AShareBufLen || length > mM2AShareBufLen)
    {
        ALOGE("%s(), message_id = 0x%x, length(0x%x), offset(0x%x), mM2AShareBufLen(0x%x)", __FUNCTION__, message_id, length, offset, mM2AShareBufLen);
        ASSERT(offset > mM2AShareBufLen || length > mM2AShareBufLen);
        return false;
    }
    else
    {
        return true;
    }
}


ccci_message_ack_t SpeechMessengerECCCI::JudgeAckOfMsg(const uint16_t message_id)
{
    ccci_message_ack_t ack;
    switch (message_id)
    {
        case MSG_A2M_SET_SPH_MODE:
        case MSG_A2M_SPH_ON:
        case MSG_A2M_SPH_OFF:
        case MSG_A2M_SPH_ROUTER_ON:
        case MSG_A2M_PCM_REC_ON:
        case MSG_A2M_VM_REC_ON:
        case MSG_A2M_PCM_REC_OFF:
        case MSG_A2M_VM_REC_OFF:
        case MSG_A2M_BGSND_ON:
        case MSG_A2M_BGSND_OFF:
        case MSG_A2M_PNW_ON:
        case MSG_A2M_PNW_OFF:
        case MSG_A2M_DMNR_RECPLAY_ON:
        case MSG_A2M_DMNR_RECPLAY_OFF:
        case MSG_A2M_DMNR_REC_ONLY_ON:
        case MSG_A2M_DMNR_REC_ONLY_OFF:
        case MSG_A2M_CTM_ON:
        case MSG_A2M_CTM_OFF:
        case MSG_A2M_SET_ACOUSTIC_LOOPBACK:
        case MSG_A2M_EM_NB:
        case MSG_A2M_EM_DMNR:
        case MSG_A2M_EM_MAGICON:
        case MSG_A2M_EM_HAC:
        case MSG_A2M_EM_WB:
        case MSG_A2M_VIBSPK_PARAMETER:
        case MSG_A2M_NXP_SMARTPA_PARAMETER:
        case MSG_A2M_QUERY_RF_INFO:
            ack = MESSAGE_NEED_ACK;
            break;
        default:
            ack = MESSAGE_BYPASS_ACK;
    }
    return ack;
}

bool SpeechMessengerECCCI::IsModemFunctionOnOffMessage(const uint16_t message_id)
{
    bool bIsModemFunctionOnOffMessage = false;

    switch (message_id)
    {
        case MSG_A2M_SPH_ON:
        case MSG_A2M_SPH_OFF:
        case MSG_A2M_SPH_ROUTER_ON:
        case MSG_A2M_PCM_REC_ON:
        case MSG_A2M_VM_REC_ON:
        case MSG_A2M_PCM_REC_OFF:
        case MSG_A2M_VM_REC_OFF:
        case MSG_A2M_BGSND_ON:
        case MSG_A2M_BGSND_OFF:
        case MSG_A2M_PNW_ON:
        case MSG_A2M_PNW_OFF:
        case MSG_A2M_DMNR_RECPLAY_ON:
        case MSG_A2M_DMNR_RECPLAY_OFF:
        case MSG_A2M_DMNR_REC_ONLY_ON:
        case MSG_A2M_DMNR_REC_ONLY_OFF:
        case MSG_A2M_CTM_ON:
        case MSG_A2M_CTM_OFF:
        case MSG_A2M_SET_ACOUSTIC_LOOPBACK:
            bIsModemFunctionOnOffMessage = true;
            break;
        default:
            bIsModemFunctionOnOffMessage = false;
    }

    return bIsModemFunctionOnOffMessage;
}

status_t SpeechMessengerECCCI::SendMessageInQueue(ccci_buff_t ccci_buff)
{
    mCCCIMessageQueueMutex.lock();

    uint32_t count = GetQueueCount();
    ASSERT(count < (CCCI_MAX_QUEUE_NUM - 1));  // check queue full

    ccci_message_ack_t ack_type = JudgeAckOfMsg(GetMessageID(ccci_buff));

    if (ack_type == MESSAGE_NEED_ACK)
    {
        ALOGD("%s(), mModemIndex = %d, need ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    }


    status_t ret = NO_ERROR;
    if (count == 0) // queue is empty
    {
        if (ack_type == MESSAGE_BYPASS_ACK) // no need ack, send directly, don't care ret value
        {
            ret = SendMessage(ccci_buff);
        }
        else // need ack, en-queue and send message
        {
            pQueue[iQWrite].ccci_buff = ccci_buff;
            pQueue[iQWrite].ack_type  = ack_type;
            iQWrite++;
            if (iQWrite == CCCI_MAX_QUEUE_NUM) { iQWrite -= CCCI_MAX_QUEUE_NUM; }

            ret = SendMessage(ccci_buff);
            if (ret != NO_ERROR) // skip this fail CCCI message
            {
                iQRead++;
                if (iQRead == CCCI_MAX_QUEUE_NUM) { iQRead -= CCCI_MAX_QUEUE_NUM; }
            }
        }
    }
    else // queue is not empty, must queue the element
    {
        pQueue[iQWrite].ccci_buff = ccci_buff;
        pQueue[iQWrite].ack_type  = ack_type;
        iQWrite++;
        if (iQWrite == CCCI_MAX_QUEUE_NUM) { iQWrite -= CCCI_MAX_QUEUE_NUM; }

        ALOGD("%s(), Send message(0x%x) to queue, count(%u)", __FUNCTION__, ccci_buff.message, GetQueueCount());
    }

    mCCCIMessageQueueMutex.unlock();
    return ret;
}

status_t SpeechMessengerECCCI::ConsumeMessageInQueue()
{
    mCCCIMessageQueueMutex.lock();

    uint32_t count = GetQueueCount();
    if (count > 10)
    {
        ALOGW("%s(), queue count: %u", __FUNCTION__, count);
    }

    if (count == 0)
    {
        ALOGW("%s(), no message in queue", __FUNCTION__);
        mCCCIMessageQueueMutex.unlock();
        return UNKNOWN_ERROR;
    }

    status_t ret = NO_ERROR;
    while (1)
    {
        // when entering this function, the first message in queue must be a message waiting for ack
        // so we increment index, consuming the first message in queue
        iQRead++;
        if (iQRead == CCCI_MAX_QUEUE_NUM) { iQRead -= CCCI_MAX_QUEUE_NUM; }

        // check if empty
        if (iQRead == iQWrite)
        {
            ret = NO_ERROR;
            break;
        }

        // update count
        count = GetQueueCount();

        // send message
        if (pQueue[iQRead].ack_type == MESSAGE_BYPASS_ACK) // no need ack, send directly, don't care ret value
        {
            ALOGD("%s(), no need ack message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = SendMessage(pQueue[iQRead].ccci_buff);
        }
        else if (pQueue[iQRead].ack_type == MESSAGE_NEED_ACK)
        {
            ALOGD("%s(), need ack message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = SendMessage(pQueue[iQRead].ccci_buff);
            if (ret == NO_ERROR) // Send CCCI message success and wait for ack
            {
                break;
            }
        }
        else if (pQueue[iQRead].ack_type == MESSAGE_CANCELED) // the cancelled message, ignore it
        {
            ALOGD("%s(), cancel on-off-on message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = NO_ERROR;
        }
    }

    mCCCIMessageQueueMutex.unlock();
    return ret;
}

bool SpeechMessengerECCCI::MDReset_CheckMessageInQueue()
{
    mCCCIMessageQueueMutex.lock();
    uint32_t count = GetQueueCount();
    ALOGD("%s(), queue count: %u", __FUNCTION__, count);

    bool ret = true;
    while (1)
    {
        // Modem already reset.
        // Check every CCCI message in queue.
        // These messages that are sent before modem reset, don't send to modem.
        // But AP side need to do related action to make AP side in the correct state.

        // check if empty
        if (iQRead == iQWrite)
        {
            ALOGD("%s(), check message done", __FUNCTION__);
            ret = true;
            break;
        }

        // Need ack message. But modem reset, so simulate that the modem send back ack msg.
        if (JudgeAckOfMsg(GetMessageID(pQueue[iQRead].ccci_buff)) == MESSAGE_NEED_ACK)
        {
            SendMsgFailErrorHandling(pQueue[iQRead].ccci_buff);
        }

        iQRead++;
        if (iQRead == CCCI_MAX_QUEUE_NUM) { iQRead -= CCCI_MAX_QUEUE_NUM; }
    }

    mCCCIMessageQueueMutex.unlock();
    return ret;
}

void SpeechMessengerECCCI::MDReset_FlushMessageInQueue()
{
    mCCCIMessageQueueMutex.lock();

    int32_t count = GetQueueCount();
    ALOGD("%s(), queue count: %u", __FUNCTION__, count);

    if (count != 0)
    {
        ALOGE("%s(), queue is not empty!!", __FUNCTION__);
        iQWrite = 0;
        iQRead = 0;
    }

    mCCCIMessageQueueMutex.unlock();
}

status_t SpeechMessengerECCCI::CreateReadingThread()
{
    int ret = pthread_create(&hReadThread, NULL, SpeechMessengerECCCI::CCCIReadThread, (void *)this);
    if (ret != 0)
    {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerECCCI::CCCIReadThread(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    SpeechMessengerECCCI *pCCCI = (SpeechMessengerECCCI *)arg;

    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
    {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    }
    else
    {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());


    // Handle CCCI Message From Modems
    while (pCCCI->CCCIEnable)
    {
        /// read message
        ccci_buff_t ccci_buff;
        memset((void *)&ccci_buff, 0, sizeof(ccci_buff));
        status_t ret = pCCCI->ReadMessage(ccci_buff);
        if (ret != NO_ERROR)
        {
            ALOGD("%s(), ret(%d) != NO_ERROR", __FUNCTION__, ret);
            sph_msleep(10);
            continue;
        }

        /// handle message
        uint16_t m2a_message_id = pCCCI->GetMessageID(ccci_buff);

        switch (m2a_message_id)
        {
            case MSG_M2A_SET_SPH_MODE_ACK:   // ack of MSG_A2M_SET_SPH_MODE
            {
                // Do nothing... just leave a log
                ALOGD("--SetSpeechMode Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Speech */
            case MSG_M2A_SPH_ON_ACK:   // ack of MSG_A2M_SPH_ON
            {
                phone_call_mode_t phone_call_mode = (phone_call_mode_t)pCCCI->GetMessageParam(ccci_buff);
                if (phone_call_mode == RAT_3G324M_MODE)
                {
                    pCCCI->SetModemSideModemStatus(VT_STATUS_MASK);
                }
                else
                {
                    pCCCI->SetModemSideModemStatus(SPEECH_STATUS_MASK);
                }

                // dump speech enhancement parameter in modem log
                pCCCI->mLad->ModemDumpSpeechParam();

                //pCCCI->mLad->Signal();

                ALOGD("--SpeechOn Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_SPH_OFF_ACK:   // ack of MSG_A2M_SPH_OFF
            {
                if (pCCCI->GetModemSideModemStatus(VT_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(VT_STATUS_MASK);
                }
                else if (pCCCI->GetModemSideModemStatus(SPEECH_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(SPEECH_STATUS_MASK);
                }
                else
                {
                    ALOGE("--SpeechOff Ack is not paired!!");
                }

                pCCCI->mLad->Signal();

                ALOGD("--SpeechOff Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Speech Router */
            case MSG_M2A_SPH_ROUTER_ON_ACK:   // ack of MSG_A2M_SPH_ROUTER_ON
            {
                const bool pcm_route_on = pCCCI->GetMessageParam(ccci_buff) & 0x1;
                if (pcm_route_on == true) // pcm route on
                {
                    pCCCI->SetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
                }
                else // pcm route off
                {
                    if (pCCCI->GetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK) == true)
                    {
                        pCCCI->ResetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
                    }
                    else
                    {
                        ALOGE("--SpeechRouterOn Ack is not paired!!");
                    }
                }

                ALOGD("--SpeechRouterOn Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Record */
            case MSG_M2A_PCM_REC_ON_ACK:   // ack of MSG_A2M_PCM_REC_ON
            {
                pCCCI->SetModemSideModemStatus(RECORD_STATUS_MASK);
                ALOGD("--RecordOn Ack done(0x%x)", ccci_buff.message);
                break;
            }

            case MSG_M2A_VM_REC_ON_ACK:   // ack of MSG_A2M_VM_REC_ON
            {
                pCCCI->SetModemSideModemStatus(VM_RECORD_STATUS_MASK);
                ALOGD("--VMRecordOn Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_PCM_REC_DATA_NOTIFY:   // meaning that we are recording, modem have some data
            {
                ALOGD("%s() MSG_M2A_PCM_REC_DATA_NOTIFY", __FUNCTION__);
                ASSERT(pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true);

                if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == false)
                {
                    ALOGW("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
                }
                else
                {
                    uint16_t Bytes_PCM = pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN;
                    SLOGV("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x), data_length: %d", ccci_buff.message, Bytes_PCM);

                    // Phone record
                    AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                    if ((pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true))
                    {
                        pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PCM_REC_DATA_READ_ACK, 0, 0));
                        mA2M_ECCCI_DATA_READ_ACK = false;
                    }
                    else
                    {
                        ALOGD("%s() RECORD_STATUS_MASK(%d)", __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK));
                    }
                }

                break;
            }

            case MSG_M2A_VM_REC_DATA_NOTIFY:   // meaning that we are recording, modem have some data
            {
                ASSERT(pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true);

                if (pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == false)
                {
                    ALOGW("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
                }
                else
                {
                    SLOGV("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x), data_length: %d", ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN);

                    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
                    // VM
                    pSpeechVMRecorder->CopyBufferToVM(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                    if ((pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true))
                    {
                        pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_VM_REC_DATA_READ_ACK, 0, 0));
                        mA2M_ECCCI_DATA_READ_ACK = false;
                    }
                }

                break;
            }
            case MSG_M2A_PCM_REC_OFF_ACK:   // ack of MSG_A2M_PCM_REC_OFF
            {
                if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(RECORD_STATUS_MASK);
                }
                else
                {
                    ALOGE("--RecordOff Ack is not paired!!");
                }
                ALOGD("--RecordOff Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_VM_REC_OFF_ACK:   // ack of MSG_A2M_VM_REC_OFF
            {
                if (pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(VM_RECORD_STATUS_MASK);
                }
                else
                {
                    ALOGE("--VMRecordOff Ack is not paired!!");
                }
                ALOGD("--VMRecordOff Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Background sound */
            case MSG_M2A_BGSND_ON_ACK:   // ack of MSG_A2M_BGSND_ON
            {
                pCCCI->SetModemSideModemStatus(BGS_STATUS_MASK);
                ALOGD("--BGSoundOn Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_BGSND_DATA_REQUEST:   // modem request bgs data to play
            {
                ASSERT(pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true);

                BGSPlayer *pBGSPlayer = BGSPlayer::GetInstance();
                pBGSPlayer->mBGSMutex.lock(); // avoid AudioMTKStreamOut::standby() destroy BGSPlayBuffer during MSG_M2A_BGSND_DATA_REQUEST
                if (pCCCI->mLad->GetApSideModemStatus(BGS_STATUS_MASK) == false)
                {
                    ALOGW("MSG_M2A_BGSND_DATA_REQUEST(0x%x) after AP side trun off BGS!! Drop it.", ccci_buff.message);
                }
                else
                {
                    SLOGV("MSG_M2A_BGSND_DATA_REQUEST(0x%x), num_data_request: %d", ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

                    // parse size of data request
                    uint16_t num_data_request = pCCCI->GetMessageParam(ccci_buff);
                    uint32_t max_buf_length = A2M_SHARED_BUFFER_BGS_DATA_SIZE;
                    if (num_data_request > max_buf_length) { num_data_request = max_buf_length; }

                    // get bgs share buffer address
                    uint16_t offset = A2M_SHARED_BUFFER_BGS_DATA_BASE;
                    char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
                    char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;

                    // fill playback data
                    uint16_t data_length = pBGSPlayer->PutDataToSpeaker(p_data_address, num_data_request);

                    // fill header info
                    pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                             CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                             SHARE_BUFF_DATA_TYPE_CCCI_BGS_TYPE,
                                             data_length);

                    // send data notify to modem side
                    const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
                    if (pCCCI->mLad->GetApSideModemStatus(BGS_STATUS_MASK) == true)
                    {
                        pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_DATA_NOTIFY, message_length, offset));
                    }
                }

                pBGSPlayer->mBGSMutex.unlock();
                break;
            }
            case MSG_M2A_BGSND_OFF_ACK:   // ack of MSG_A2M_BGSND_OFF
            {
                if (pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(BGS_STATUS_MASK);
                }
                else
                {
                    ALOGE("--BGSoundOff Ack is not paired!!");
                }
                ALOGD("--BGSoundOff Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* PCM2Way */
            case MSG_M2A_PNW_ON_ACK: // ack of MSG_A2M_PNW_ON
            case MSG_M2A_DMNR_RECPLAY_ON_ACK: // ack of MSG_A2M_DMNR_RECPLAY_ON
            case MSG_M2A_DMNR_REC_ONLY_ON_ACK:   // ack of MSG_A2M_DMNR_REC_ONLY_ON
            {
                pCCCI->SetModemSideModemStatus(P2W_STATUS_MASK);
                ALOGD("--PCM2WayOn Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_PNW_UL_DATA_NOTIFY:   // Get Microphone data from Modem
            {
                ASSERT(pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true);

                Record2Way *pRecord2Way = Record2Way::GetInstance();
                if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == false)
                {
                    ALOGW("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x) after AP side trun off PCM2Way!! Drop it.", ccci_buff.message);
                }
                else
                {
                    SLOGV("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x), data_length:%d", ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN);
                    pRecord2Way->GetDataFromMicrophone(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));
                    if ((pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true))
                    {
                        pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PNW_UL_DATA_READ_ACK, 0, 0));
                        mA2M_ECCCI_DATA_READ_ACK = false;
                    }
                }

#if 0 // PCM2WAY: UL -> DL Loopback
                // Used for debug and Speech DVT
                uint16_t size_bytes = 320;
                char buffer[320];
                pRecord2Way->Read(buffer, size_bytes);
                Play2Way::GetInstance()->Write(buffer, size_bytes);
#endif
                break;
            }
            case MSG_M2A_PNW_DL_DATA_REQUEST:   // Put Data to modem and play
            {
                ASSERT(pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true);

                if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == false)
                {
                    ALOGW("MSG_M2A_PNW_DL_DATA_REQUEST(0x%x) after AP side trun off PCM2Way!! Drop it.", ccci_buff.message);
                }
                else
                {
                    SLOGV("MSG_M2A_PNW_DL_DATA_REQUEST(0x%x), num_data_request: %d", ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

                    // parse size of data request
                    uint16_t num_data_request = pCCCI->GetMessageParam(ccci_buff);
                    uint32_t max_buf_length = PCM2WAY_PLAY_BUFFER_WB_LEN;
                    if (num_data_request > max_buf_length) { num_data_request = max_buf_length; }

                    // get pcm2way share buffer address
                    uint16_t offset = A2M_SHARED_BUFFER_P2W_DL_DATA_BASE;
                    char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
                    char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;

                    // fill playback data
                    Play2Way *pPlay2Way = Play2Way::GetInstance();
                    const uint16_t data_length = pPlay2Way->PutDataToSpeaker(p_data_address, num_data_request);

                    // fill header info
                    pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                             CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                             SHARE_BUFF_DATA_TYPE_PCM_FillSpk,
                                             data_length);

                    // send data notify to modem side
                    const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
                    if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == true)
                    {
                        pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PNW_DL_DATA_NOTIFY, message_length, offset));
                    }
                }
                break;
            }
            case MSG_M2A_PNW_OFF_ACK: // ack of MSG_A2M_PNW_OFF
            case MSG_M2A_DMNR_RECPLAY_OFF_ACK: // ack of MSG_A2M_DMNR_RECPLAY_OFF
            case MSG_M2A_DMNR_REC_ONLY_OFF_ACK:   // ack of MSG_A2M_DMNR_REC_ONLY_OFF
            {
                if (pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(P2W_STATUS_MASK);
                }
                else
                {
                    ALOGE("--PCM2WayOff Ack is not paired!!");
                }
                ALOGD("--PCM2WayOff Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* TTY */
            case MSG_M2A_CTM_ON_ACK:   // ack of MSG_A2M_CTM_ON
            {
                pCCCI->SetModemSideModemStatus(TTY_STATUS_MASK);
                ALOGD("--TtyCtmOn Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_CTM_DEBUG_DATA_NOTIFY:
            {
                if (pCCCI->mLad->GetApSideModemStatus(TTY_STATUS_MASK) == false)
                {
                    ALOGW("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x) after AP side trun off TTY!! Drop it.", ccci_buff.message);
                }
                else
                {
                    SLOGV("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x), data_length: %d", ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN);
                    SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::GetInstance();
                    const int data_type = pCCCI->GetM2AShareBufDataType(ccci_buff);
                    if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_IN)
                    {
                        pSpeechVMRecorder->GetCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileUlIn);
                    }
                    else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_IN)
                    {
                        pSpeechVMRecorder->GetCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileDlIn);
                    }
                    else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_OUT)
                    {
                        pSpeechVMRecorder->GetCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileUlOut);
                    }
                    else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_OUT)
                    {
                        pSpeechVMRecorder->GetCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileDlOut);
                    }
                    else
                    {
                        ALOGW("%s(), data_type(0x%x) error", __FUNCTION__, data_type);
                        ASSERT(0);
                    }

                    if (mA2M_ECCCI_DATA_READ_ACK == true)
                    {
                        pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_CTM_DEBUG_DATA_READ_ACK, 0, 0));
                        mA2M_ECCCI_DATA_READ_ACK = false;
                    }
                }
                break;
            }
            case MSG_M2A_CTM_OFF_ACK:   // ack of MSG_A2M_CTM_OFF
            {
                if (pCCCI->GetModemSideModemStatus(TTY_STATUS_MASK) == true)
                {
                    pCCCI->ResetModemSideModemStatus(TTY_STATUS_MASK);
                }
                else
                {
                    ALOGE("--TtyCtmOff Ack is not paired!!");
                }
                ALOGD("--TtyCtmOff Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Loopback */
            case MSG_M2A_SET_ACOUSTIC_LOOPBACK_ACK:   // ack of MSG_A2M_SET_ACOUSTIC_LOOPBACK
            {
                const bool loopback_on = pCCCI->GetMessageParam(ccci_buff) & 0x1;
                if (loopback_on == true) // loopback on
                {
                    pCCCI->SetModemSideModemStatus(LOOPBACK_STATUS_MASK);
                    // dump speech enhancement parameter in modem log
                    pCCCI->mLad->ModemDumpSpeechParam();
                }
                else // loopback off
                {
                    if (pCCCI->GetModemSideModemStatus(LOOPBACK_STATUS_MASK) == true)
                    {
                        pCCCI->ResetModemSideModemStatus(LOOPBACK_STATUS_MASK);
                    }
                    else
                    {
                        ALOGE("--SetAcousticLoopback Ack is not paired!!");
                    }
                }

                pCCCI->mLad->Signal();

                ALOGD("--SetAcousticLoopback Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Speech Enhancement parameters */
            case MSG_M2A_EM_NB_ACK:   // ack of MSG_A2M_EM_NB
            {
                pCCCI->AddSpeechParamAckCount(NB_SPEECH_PARAM);
                pCCCI->A2MBufUnLock();
                ALOGD("--SetNBSpeechParameters Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_EM_DMNR_ACK:   // ack of MSG_A2M_EM_DMNR
            {
                pCCCI->AddSpeechParamAckCount(DMNR_SPEECH_PARAM);
                pCCCI->A2MBufUnLock();
                ALOGD("--SetDualMicSpeechParameters Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_EM_MAGICON_ACK:   // ack of MSG_A2M_EM_MAGICON
            {
                pCCCI->AddSpeechParamAckCount(MAGICON_SPEECH_PARAM);
                pCCCI->A2MBufUnLock();
                ALOGD("--SetMagiConSpeechParameters Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_EM_HAC_ACK:   // ack of MSG_A2M_EM_HAC
            {
                pCCCI->AddSpeechParamAckCount(HAC_SPEECH_PARAM);
                pCCCI->A2MBufUnLock();
                ALOGD("--SetHACSpeechParameters Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_EM_WB_ACK:   // ack of MSG_A2M_EM_WB
            {
                pCCCI->AddSpeechParamAckCount(WB_SPEECH_PARAM);
                pCCCI->A2MBufUnLock();
                ALOGD("--SetWBSpeechParameters Ack done(0x%x)", ccci_buff.message);
                break;
            }

            /* Modem Reset */
            case MSG_M2A_EM_DATA_REQUEST:   // Modem reset. Requset for all EM data(NB/DMNR/WB)
            {
                ALOGW("..[MD Reset Notify(MSG_M2A_EM_DATA_REQUEST: 0x%x)]..", ccci_buff.message);

                // Clean mWaitAckMessageID to avoid the queue receiving the wrong ack
                pCCCI->mWaitAckMessageID = 0;

                // close analog downlink path when modem reset during phone call
                if (pCCCI->mLad->GetApSideModemStatus(SPEECH_STATUS_MASK) == true ||
                    pCCCI->mLad->GetApSideModemStatus(VT_STATUS_MASK)     == true)
                {
                    //AudioResourceManager::getInstance()->StopOutputDevice();
                }
                pCCCI->mIsModemResetDuringPhoneCall = false;

                // Create Send Sph Para Thread
                pCCCI->CreateSendSphParaThread();

                break;
            }
            case MSG_M2A_VIBSPK_PARAMETER_ACK:   // ack of MSG_M2A_VIBSPK_PARAMETER
            {
                pCCCI->A2MBufUnLock();
                ALOGD("--SetVibSpkParam Ack done(0x%x)", ccci_buff.message);
                break;
            }

            case MSG_M2A_NXP_SMARTPA_PARAMETER_ACK:
            {
                pCCCI->A2MBufUnLock();
                ALOGD("--SetNxpSmartpaParam Ack done(0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_QUERY_RF_INFO_ACK:
            {
                uint16_t mRfInfo = ccci_buff.message & 0xFFFF;

                // save mRfInfo in property to avoid medieserver die
                pCCCI->SetRFInfo(0, mRfInfo);
                pCCCI->GetRFInfo();
                ALOGD("--MSG_M2A_QUERY_RF_INFO_ACK (0x%x)", ccci_buff.message);
                break;
            }
            case MSG_M2A_NETWORK_STATUS_NOTIFY:
            {
                uint16_t mRfMode = ccci_buff.message & 0xFFFF;

                // save mRfMode in property to avoid medieserver die
                pCCCI->SetRFInfo(1, mRfMode);
                pCCCI->GetRFInfo();
                ALOGD("--MSG_M2A_NETWORK_STATUS_NOTIFY (0x%x)", ccci_buff.message);
                pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_NETWORK_STATUS_ACK, 0, 0));
                break;
            }

            default:
            {
                //TINA REMOVE TEMP
                //ALOGD("Read modem message(0x%x), don't care. (or no such message)", ccci_buff.message);
                sph_msleep(10);
                break;
            }
        }

        /// If AP side is waiting for this ack, then consume message queue
        uint16_t a2m_message_id_of_m2a_ack = m2a_message_id & 0x7FFF; // 0xAF** -> 0x2F**
        if (pCCCI->JudgeAckOfMsg(a2m_message_id_of_m2a_ack) == MESSAGE_NEED_ACK)
        {
            if (a2m_message_id_of_m2a_ack == pCCCI->mWaitAckMessageID)
            {
                pCCCI->mWaitAckMessageID = 0; // reset
                pCCCI->ConsumeMessageInQueue();
            }
            else
            {
                ALOGW("Message(0x%x) might double ack!! The current mWaitAckMessageID is 0x%x", ccci_buff.message, pCCCI->mWaitAckMessageID);
            }
        }
    }
    pthread_exit(NULL);
    return 0;
}

status_t SpeechMessengerECCCI::CreateSendSphParaThread()
{
    int ret = pthread_create(&hSendSphThread, NULL, SpeechMessengerECCCI::SendSphParaThread, (void *)this);
    if (ret != 0)
    {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerECCCI::SendSphParaThread(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
    {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    }
    else
    {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    SpeechMessengerECCCI *pCCCI = (SpeechMessengerECCCI *)arg;

    // Get SpeechParamLock
    if (pCCCI->SpeechParamLock() == false)
    {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return 0;
    }

    // Check the first message in queue. If need ack, take action.
    pCCCI->MDReset_CheckMessageInQueue();

    // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
    pCCCI->MDReset_FlushMessageInQueue();

    // Send speech parameters to modem side
    pCCCI->ResetSpeechParamAckCount();
    pCCCI->mLad->SetAllSpeechEnhancementInfoToModem();

    // Release SpeechParamLock
    pCCCI->SpeechParamUnLock();

    pthread_exit(NULL);
    return 0;
}

bool SpeechMessengerECCCI::A2MBufLock()
{
    const uint32_t kA2MBufLockTimeout = 3000; // 3 sec

    int rc = mA2MShareBufMutex.lock_timeout(kA2MBufLockTimeout);
    ALOGD("%s()", __FUNCTION__);
    if (rc != 0)
    {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kA2MBufLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerECCCI::A2MBufUnLock()
{
    mA2MShareBufMutex.unlock();
    ALOGD("%s()", __FUNCTION__);
}

bool SpeechMessengerECCCI::SpeechParamLock()
{
    const uint32_t kSphParamLockTimeout = 10000; // 10 sec

    ALOGD("%s()", __FUNCTION__);
    int rc = mSetSpeechParamMutex.lock_timeout(kSphParamLockTimeout);
    if (rc != 0)
    {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kSphParamLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerECCCI::SpeechParamUnLock()
{
    ALOGD("%s()", __FUNCTION__);
    mSetSpeechParamMutex.unlock();
}

void SpeechMessengerECCCI::ResetSpeechParamAckCount()
{
    memset(&mSpeechParamAckCount, 0, sizeof(mSpeechParamAckCount));
    ALOGD("%s(), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)", __FUNCTION__,
          mSpeechParamAckCount[NB_SPEECH_PARAM],
          mSpeechParamAckCount[DMNR_SPEECH_PARAM],
          mSpeechParamAckCount[WB_SPEECH_PARAM],
          mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
          mSpeechParamAckCount[HAC_SPEECH_PARAM]);
}

void SpeechMessengerECCCI::AddSpeechParamAckCount(speech_param_ack_t type)
{
    if (type >= NUM_SPEECH_PARAM_ACK_TYPE || type < 0)
    {
        ALOGE("%s(), no such type: %d", __FUNCTION__, type);
    }
    else
    {
        if (mSpeechParamAckCount[type] < 0xFFFFFFFF) //prevent overflow
        {
            mSpeechParamAckCount[type]++;
        }
        ALOGD("%s(%d), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)", __FUNCTION__, type,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM]);
    }
}

bool SpeechMessengerECCCI::CheckSpeechParamAckAllArrival()
{
    bool ret = true;

    // Get SpeechParamLock
    if (SpeechParamLock() == false)
    {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return false;
    }

    if (mSpeechParamAckCount[NB_SPEECH_PARAM] == 0) { ret = false; }
#if defined(MTK_DUAL_MIC_SUPPORT)
    if (mSpeechParamAckCount[DMNR_SPEECH_PARAM] == 0) { ret = false; }
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
    if (mSpeechParamAckCount[WB_SPEECH_PARAM] == 0) { ret = false; }
#endif
#if defined(MTK_MAGICONFERENCE_SUPPORT) && defined(MTK_DUAL_MIC_SUPPORT)
        if (mSpeechParamAckCount[MAGICON_SPEECH_PARAM] == 0) { ret = false; }
#endif
#if defined(MTK_HAC_SUPPORT)
    if (mSpeechParamAckCount[HAC_SPEECH_PARAM] == 0) { ret = false; }
#endif

    if (ret == true)
    {
        ALOGD("%s() Pass", __FUNCTION__);
    }
    else
    {
        ALOGE("%s() Fail, NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)", __FUNCTION__,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM]);

        // Send speech parameters to modem side again
        mLad->SetAllSpeechEnhancementInfoToModem();
    }

    // Release SpeechParamLock
    SpeechParamUnLock();

    return ret;
}

/** Do error handling here */
void SpeechMessengerECCCI::SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff)
{
    ALOGE("%s(), message: 0x%x", __FUNCTION__, ccci_buff.message);
    switch (GetMessageID(ccci_buff))
    {
        case MSG_A2M_SET_SPH_MODE:
        {
            // Do nothing...
            break;
        }
        case MSG_A2M_SPH_ON:
        {
            phone_call_mode_t phone_call_mode = (phone_call_mode_t)GetMessageParam(ccci_buff);
            if (phone_call_mode == RAT_3G324M_MODE)
            {
                SetModemSideModemStatus(VT_STATUS_MASK);
            }
            else
            {
                SetModemSideModemStatus(SPEECH_STATUS_MASK);
            }

            //mLad->Signal();

            break;
        }
        case MSG_A2M_SPH_OFF:
        {
            if (GetModemSideModemStatus(VT_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(VT_STATUS_MASK);
            }
            else if (GetModemSideModemStatus(SPEECH_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(SPEECH_STATUS_MASK);
            }
            else
            {
                ALOGE("--SpeechOff Ack is not paired!!");
            }

            mLad->Signal();

            break;
        }
        case MSG_A2M_SPH_ROUTER_ON:
        {
            const bool pcm_route_on = GetMessageParam(ccci_buff) & 0x1;
            if (pcm_route_on == true) // pcm route on
            {
                SetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
            }
            else // pcm route off
            {
                if (GetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK) == true)
                {
                    ResetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
                }
                else
                {
                    ALOGE("--SpeechOn(PCM Route On) Ack is not paired!!");
                }
            }

            break;
        }
        case MSG_A2M_PCM_REC_ON:
        {
            SetModemSideModemStatus(RECORD_STATUS_MASK);
            break;
        }
        case MSG_A2M_PCM_REC_OFF:
        {
            if (GetModemSideModemStatus(RECORD_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(RECORD_STATUS_MASK);
            }
            else
            {
                ALOGE("--RecordOff Ack is not paired!!");
            }
            break;
        }
        case MSG_A2M_VM_REC_ON:
        {
            SetModemSideModemStatus(VM_RECORD_STATUS_MASK);
            break;
        }
        case MSG_A2M_VM_REC_OFF:
        {
            if (GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(VM_RECORD_STATUS_MASK);
            }
            else
            {
                ALOGE("--VMRecordOff Ack is not paired!!");
            }
            break;
        }
        case MSG_A2M_BGSND_ON:
        {
            SetModemSideModemStatus(BGS_STATUS_MASK);
            break;
        }
        case MSG_A2M_BGSND_OFF:
        {
            if (GetModemSideModemStatus(BGS_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(BGS_STATUS_MASK);
            }
            else
            {
                ALOGE("--BGSoundOff Ack is not paired!!");
            }
            break;
        }
        case MSG_A2M_PNW_ON:
        case MSG_A2M_DMNR_RECPLAY_ON:
        case MSG_A2M_DMNR_REC_ONLY_ON:
        {
            SetModemSideModemStatus(P2W_STATUS_MASK);
            break;
        }
        case MSG_A2M_PNW_OFF:
        case MSG_A2M_DMNR_RECPLAY_OFF:
        case MSG_A2M_DMNR_REC_ONLY_OFF:
        {
            if (GetModemSideModemStatus(P2W_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(P2W_STATUS_MASK);
            }
            else
            {
                ALOGE("--PCM2WayOff Ack is not paired!!");
            }
            break;
        }
        case MSG_A2M_CTM_ON:
        {
            SetModemSideModemStatus(TTY_STATUS_MASK);
            break;
        }
        case MSG_A2M_CTM_OFF:
        {
            if (GetModemSideModemStatus(TTY_STATUS_MASK) == true)
            {
                ResetModemSideModemStatus(TTY_STATUS_MASK);
            }
            else
            {
                ALOGE("--TtyCtmOff Ack is not paired!!");
            }
            break;
        }
        case MSG_A2M_SET_ACOUSTIC_LOOPBACK:
        {
            const bool loopback_on = GetMessageParam(ccci_buff) & 0x1;
            if (loopback_on == true) // loopback on
            {
                SetModemSideModemStatus(LOOPBACK_STATUS_MASK);
            }
            else // loopback off
            {
                if (GetModemSideModemStatus(LOOPBACK_STATUS_MASK) == true)
                {
                    ResetModemSideModemStatus(LOOPBACK_STATUS_MASK);
                }
                else
                {
                    ALOGE("--SetAcousticLoopback Ack is not paired!!");
                }
            }

            mLad->Signal();
            break;
        }
        case MSG_A2M_EM_NB:
        case MSG_A2M_EM_WB:
        case MSG_A2M_EM_DMNR:
        case MSG_A2M_EM_MAGICON:
        case MSG_A2M_EM_HAC:
        case MSG_A2M_VIBSPK_PARAMETER:
        case MSG_A2M_NXP_SMARTPA_PARAMETER:
        {
            A2MBufUnLock();
            break;
        }
        default:
        {
            ALOGW("%s(), message: 0x%x, ack don't care", __FUNCTION__, ccci_buff.message);
        }
    }
}

RingBuf SpeechMessengerECCCI::GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff)
{
    RingBuf ul_ring_buf;
    uint16_t size_copy1, size_copy2;
    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;

    ul_ring_buf.pRead = mM2AShareBufRead;

    ul_ring_buf.pWrite   = ul_ring_buf.pRead + GetMessageLength(ccci_buff);
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd)
    {
        ul_ring_buf.pWrite -= ul_ring_buf.bufLen;

        size_copy1 = mM2AShareBufEnd - ul_ring_buf.pRead;
        size_copy2 = GetMessageLength(ccci_buff) - size_copy1;
        ALOGD("%s(), mM2AShareBufEnd(0x%x), payloadLen(0x%x), size_copy1(0x%x), size_copy2(0x%x), pRead(0x%x), pWrite(0x%x)",
              __FUNCTION__, mM2AShareBufEnd, GetMessageLength(ccci_buff), size_copy1, size_copy2, ul_ring_buf.pRead, ul_ring_buf.pWrite);
        memcpy(ul_ring_buf.pRead, (char *)ccci_buff.payload, size_copy1);
        memcpy(ul_ring_buf.pBufBase, (char *)ccci_buff.payload + size_copy1, size_copy2);
    }
    else
    {
        memcpy(ul_ring_buf.pRead, (char *)ccci_buff.payload, GetMessageLength(ccci_buff));
    }
    ALOGD("%s(), pBufBase(0x%x), bufLen(0x%x), payloadLen(0x%x), mM2AShareBufRead(0x%x), pRead(0x%x), pWrite(0x%x)",
          __FUNCTION__, ul_ring_buf.pBufBase, ul_ring_buf.bufLen, GetMessageLength(ccci_buff), mM2AShareBufRead, ul_ring_buf.pRead, ul_ring_buf.pWrite);

    // share buffer header
    char *p_sync_word = (char *)ccci_buff.payload + 0; // size(unsigned short)
    char *p_data_type = (char *)ccci_buff.payload + 2; // size(unsigned short)
    char *p_data_len  = (char *)ccci_buff.payload + 4; // size(unsigned short)
    char *p_data_curIdx = (char *)ccci_buff.payload + 6; // size(unsigned short)
    char *p_data_totalIdx  = (char *)ccci_buff.payload + 8; // size(unsigned short)

    if (*(uint16_t *)p_data_curIdx  == *(uint16_t *)p_data_totalIdx) {  mA2M_ECCCI_DATA_READ_ACK = true; }

    ALOGD("%s(), *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x), *p_data_curIdx(0x%x), *p_data_totalIdx(0x%x), mA2M_ECCCI_DATA_READ_ACK(%d)",
          __FUNCTION__, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len, *(uint16_t *)p_data_curIdx, *(uint16_t *)p_data_totalIdx, mA2M_ECCCI_DATA_READ_ACK);

    SLOGV("CCCI get uplink data from modem. message = 0x%x, sync = 0x%x, type = 0x%x, data_len = %d",
          ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);

    ASSERT(*(uint16_t *)p_sync_word == EEMCS_M2A_SHARE_BUFF_HEADER_SYNC);

    ul_ring_buf.pRead += CCCI_PAYLOAD_BUFF_HEADER_LEN;
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    mM2AShareBufRead = ul_ring_buf.pWrite;

    return ul_ring_buf;
}


uint16_t SpeechMessengerECCCI::GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff)
{
    char *p_sync_word = (char *)ccci_buff.payload + 0; // 0 * size(unsigned short)
    SLOGV("%s(), sync = 0x%x", __FUNCTION__, *(uint16_t *)p_sync_word);
    return *(uint16_t *)p_sync_word;
}

uint16_t SpeechMessengerECCCI::GetM2AShareBufDataType(const ccci_buff_t &ccci_buff)
{
    char *p_data_type = (char *)ccci_buff.payload + 2; // 1 * size(unsigned short)
    SLOGV("%s(), type = 0x%x", __FUNCTION__, *(uint16_t *)p_data_type);
    return *(uint16_t *)p_data_type;
}

uint16_t SpeechMessengerECCCI::GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff)
{
    char *p_data_len  = (char *)ccci_buff.payload + 4; // 2 * size(unsigned short)
    SLOGV("%s(), data_len = %d", __FUNCTION__, *(uint16_t *)p_data_len);
    return *(uint16_t *)p_data_len;
}


bool SpeechMessengerECCCI::GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const
{
    return ((mModemSideModemStatus & modem_status_mask) > 0);
}

void SpeechMessengerECCCI::SetModemSideModemStatus(const modem_status_mask_t modem_status_mask)
{
    mModemSideModemStatus |= modem_status_mask;

    // save mModemSideModemStatus in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mModemSideModemStatus);
    property_set(PROPERTY_KEY_MODEM_STATUS[mModemIndex], property_value);
}

void SpeechMessengerECCCI::ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask)
{
    mModemSideModemStatus &= (~modem_status_mask);

    // save mModemSideModemStatus in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mModemSideModemStatus);
    property_set(PROPERTY_KEY_MODEM_STATUS[mModemIndex], property_value);
}

void SpeechMessengerECCCI::GetRFInfo(void)
{
    uint16_t mRfMode = 0, mRfInfo = 0;
    char property_value[PROPERTY_VALUE_MAX], property_value2[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_RF_INFO[0], property_value, "0");  //"0": default all off
    mRfInfo = atoi(property_value);

    property_get(PROPERTY_KEY_RF_INFO[1], property_value2, "0");  //"0": default all off
    mRfMode = atoi(property_value2);

    ALOGD("%s(), mRfInfo(0x%x), mRfMode(%d)", __FUNCTION__, mRfInfo, mRfMode);
}

void SpeechMessengerECCCI::SetRFInfo(char mRFIdx, uint16_t mRfData)
{
    // save mRfMode, mRfInfo in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mRfData);
    property_set(PROPERTY_KEY_RF_INFO[mRFIdx], property_value);

}

void SpeechMessengerECCCI::ResetRFInfo(void)
{
    uint16_t mRfMode = 0, mRfInfo = 0;

    // save mRfMode, mRfInfo in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX], property_value2[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mRfMode);
    property_set(PROPERTY_KEY_RF_INFO[0], property_value);

    sprintf(property_value, "%u", mRfInfo);
    property_set(PROPERTY_KEY_RF_INFO[1], property_value);
}

} // end of namespace android
