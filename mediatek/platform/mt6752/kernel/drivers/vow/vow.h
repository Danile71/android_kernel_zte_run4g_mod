#ifndef __VOW_H__
#define __VOW_H__

/***********************************************************************************
** VOW Control Message
************************************************************************************/
#define VOW_DEVNAME "vow"
#define VOW_IOC_MAGIC    'a' 

//below is control message
#define TEST_VOW_PRINT            _IO(VOW_IOC_MAGIC,  0x00)
#define VOWEINT_GET_BUFSIZE       _IOW(VOW_IOC_MAGIC, 0x01, unsigned int)
#define VOW_GET_STATUS            _IOW(VOW_IOC_MAGIC, 0x02, unsigned int)
#define VOW_SET_CONTROL           _IOW(VOW_IOC_MAGIC, 0x03, unsigned int)
#define VOW_SET_SPEAKER_MODEL     _IOW(VOW_IOC_MAGIC, 0x04, unsigned int)
#define VOW_CLR_SPEAKER_MODEL     _IOW(VOW_IOC_MAGIC, 0x05, unsigned int)
#define VOW_SET_INIT_MODEL        _IOW(VOW_IOC_MAGIC, 0x06, unsigned int)
#define VOW_SET_FIR_MODEL         _IOW(VOW_IOC_MAGIC, 0x07, unsigned int)
#define VOW_SET_NOISE_MODEL       _IOW(VOW_IOC_MAGIC, 0x08, unsigned int)
#define VOW_SET_APREG_INFO        _IOW(VOW_IOC_MAGIC, 0x09, unsigned int)
#define VOW_SET_REG_MODE          _IOW(VOW_IOC_MAGIC, 0x0A, unsigned int)
#define VOW_FAKE_WAKEUP           _IOW(VOW_IOC_MAGIC, 0x0B, unsigned int) 

/***********************************************************************************
** VOW IPI Service
************************************************************************************/
#define MD32_IPI_AUDMSG_BASE      0x5F00 
#define AP_IPI_AUDMSG_BASE        0x7F00

#define CLR_SPEAKER_MODEL_FLAG    0x3535

#define MD32_DM_BASE          0xF0030000
#define MD32_PM_BASE          0xF0020000

#define MAX_VOW_SPEAKER_MODEL         10

#define VOW_SWAPTCM_TIMEOUT           50
#define VOW_IPIMSG_TIMEOUT            50
#define VOW_WAITCHECK_INTERVAL_MS      1
#define MAX_VOW_INFO_LEN               4
#define VOW_VOICE_DATA_LENGTH_BYTES  320

/***********************************************************************************
** Type Define
************************************************************************************/
enum VOW_Control_Cmd
{
    VOWControlCmd_Init = 0,
    VOWControlCmd_ReadVoiceData,
    VOWControlCmd_EnableDebug,
    VOWControlCmd_DisableDebug,
};

typedef enum vow_ipi_msgid_t {
   //AP to MD32 MSG
   AP_IPIMSG_VOW_ENABLE = AP_IPI_AUDMSG_BASE,
   AP_IPIMSG_VOW_DISABLE,
   AP_IPIMSG_VOW_SETMODE,
   AP_IPIMSG_VOW_APREGDATA_ADDR,
   AP_IPIMSG_VOW_DATAREADY_ACK,
   AP_IPIMSG_SET_VOW_MODEL,
   AP_IPIMSG_VOW_SETGAIN,
   AP_IPIMSG_VOW_SET_FLAG,
   AP_IPIMSG_VOW_RECOGNIZE_OK_ACK,
   AP_IPIMSG_VOW_CHECKREG,

   //MD32 to AP MSG
   MD32_IPIMSG_VOW_ENABLE_ACK = MD32_IPI_AUDMSG_BASE,
   MD32_IPIMSG_VOW_DISABLE_ACK,
   MD32_IPIMSG_VOW_SETMODE_ACK,
   MD32_IPIMSG_VOW_APREGDATA_ADDR_ACK,
   MD32_IPIMSG_VOW_DATAREADY,
   MD32_IPIMSG_SET_VOW_MODEL_ACK,
   MD32_IPIMSG_VOW_SETGAIN_ACK,
   MD32_IPIMSG_SET_FLAG_ACK, 
   MD32_IPIMSG_VOW_RECOGNIZE_OK,
}vow_ipi_msgid_t;

typedef enum VOW_REG_MODE_T {
   VOW_MODE_MD32_VOW = 0,
   VOW_MODE_VOICECOMMAND,
   VOW_MODE_MULTIPLE_KEY,
   VOW_MODE_MULTIPLE_KEY_VOICECOMMAND
}VOW_REG_MODE_T;

typedef enum VOW_EINT_STATUS{
    VOW_EINT_DISABLE = -2,
    VOW_EINT_FAIL = -1,
    VOW_EINT_PASS = 0,
    VOW_EINT_RETRY = 1,
    NUM_OF_VOW_EINT_STATUS
}VOW_EINT_STATUS;

typedef enum VOW_FLAG_TYPE{
    VOW_FLAG_DEBUG,
    VOW_FLAG_PRE_LEARN,
    VOW_FLAG_DMIC_LOWPOWER,
    NUM_OF_VOW_FLAG_TYPE
}VOW_FLAG_TYPE;

typedef enum VOW_PWR_STATUS{
    VOW_PWR_OFF = 0,
    VOW_PWR_ON = 1,
    NUM_OF_VOW_PWR_STATUS
}VOW_PWR_STATUS;

typedef enum VOW_IPI_RESULT
{
   VOW_IPI_SUCCESS = 0,
   VOW_IPI_CLR_SMODEL_ID_NOTMATCH,
   VOW_IPI_SET_SMODEL_NO_FREE_SLOT,
}VOW_IPI_RESULT;

typedef enum VOW_MODEL_TYPE{
    VOW_MODEL_INIT = 0,
    VOW_MODEL_SPEAKER = 1, 
    VOW_MODEL_NOISE = 2, 
    VOW_MODEL_FIR = 3
}VOW_MODEL_TYPE;

struct VOW_EINT_DATA_STRUCT{
   int size;        // size of data section
   int eint_status; // eint status
   int id;
   char *data;      // reserved for future extension
}VOW_EINT_DATA_STRUCT;

typedef struct
{
    short id;
    short size;
    short *buf;
} vow_ipi_msg_t;

typedef struct{
   void *model_ptr;
   int  id;
   int  enabled;
}VOW_SPEAKER_MODEL_T;

typedef struct{
   long  id;
   long  addr;
   long  size;
   void *data;
}VOW_MODEL_INFO_T;

void VowDrv_SetFlag(VOW_FLAG_TYPE type, bool set);


#define ReadREG(_addr, _value) ((_value) = *(volatile unsigned int *)(_addr) )
#define WriteREG(_addr, _value) (*(volatile unsigned int *)(_addr) = (_value))
#define ReadREG16(_addr, _value) ((_value) = *(volatile unsigned short *)(_addr) )
#define WriteREG16(_addr, _value) (*(volatile unsigned short *)(_addr) = (_value))

#endif //__VOW_H__
