#ifndef __DBG_FLASH_PARAM_H__
#define __DBG_FLASH_PARAM_H__

// flash Debug Tag
enum
{
	// flash
    FL_T_VERSION=0,
    FL_T_IS_FLASH_ON,
    FL_T_DUTY,
    FL_T_DUTY_LT,
    FL_T_EXP_TIME,
    FL_T_ISO,
    FL_T_AFE_GAIN,
    FL_T_ISP_GAIN,
    FL_T_DUTY_NUM,
    FL_T_DUTY_LT_NUM,
    FL_T_FLASH_MODE,
    FL_T_SCENE_MODE,
    FL_T_EV_COMP,
    FL_T_DIG_ZOOM_100,
    FL_T_IS_BURST,
    FL_T_VBAT,
    FL_T_IS_BATLOW_TIMEOUT,
    FL_T_PF_START,
    FL_T_PF_RUN,
    FL_T_PF_END,
    FL_T_MF_START,
    FL_T_MF_END,
    FL_T_TIME_OUT,
    FL_T_TIME_OUT_LT,
    FL_T_PF_EPOCH_FRAMES,
    FL_T_ERR_NUM,
    FL_T_ERR1,
    FL_T_ERR2,
    FL_T_ERR3,
    FL_T_ERR1_TIME,
    FL_T_ERR2_TIME,
    FL_T_ERR3_TIME,
    FL_T_NUM,
};

#define FLASH_DEBUG_TAG_VERSION (0)
#define FLASH_TAG_NUM (FL_T_NUM + 1500)

typedef struct
{
    AAA_DEBUG_TAG_T Tag[FLASH_TAG_NUM];
} FLASH_DEBUG_INFO_T;

#endif // __DBG_FLASH_PARAM_H__

