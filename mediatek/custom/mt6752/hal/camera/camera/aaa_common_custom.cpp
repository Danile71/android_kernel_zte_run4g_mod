#include "camera_custom_types.h"
#include "aaa_common_custom.h"

/********************************************************************************************
 * DO NOT CHANGE!!!DO NOT CHANGE!!!DO NOT CHANGE!!!DO NOT CHANGE!!!DO NOT CHANGE!!!
 *******************************************************************************************/
#define ON       MTRUE
#define OFF      MFALSE




/********************************************************************************************
 ********************************************************************************************
 ****************************** Customize 3A options below **********************************
 ********************************************************************************************
 *******************************************************************************************/


/********************************************************************************************
 * ENABLE_PRECAPTURE_AF:
 * [ON]: During precatpure, AF is executed when environment is dark.
 * [OFF]: No precapture AF
 *******************************************************************************************/
#define ENABLE_PRECAPTURE_AF                ON



/********************************************************************************************
 * PRECAPTURE_AF_AFTER_PREFLASH:
 * [ON] Precapture AF is after preflash
 * [OFF] Precapture AF is before preflash
 *******************************************************************************************/
#define PRECAPTURE_AF_AFTER_PREFLASH        ON



/********************************************************************************************
 * ENABLE_VIDEO_AUTO_FLASH:
 * [ON]: Enable video auto flash
 * (1. When user selects auto mode,  flash on/off is determined by BV.
 *  2. when user selects off mode, flash off.
 *  3. when user selects on mode, flash on. )
 * [OFF]: Disable video auto flash
 *******************************************************************************************/
#define ENABLE_VIDEO_AUTO_FLASH             ON



/********************************************************************************************
 * CAF_WINDOW_FOLLOW_TAF_WINDOW:
 * [ON]: CAF and Monitor window follow touch AF window.
 * [OFF]: CAF and monitor window is set to center
 *******************************************************************************************/
#define CAF_WINDOW_FOLLOW_TAF_WINDOW        OFF



/********************************************************************************************
 * ONE_SHOT_AE_BEFORE_TAF:
 * [ON] : Do one-shot AE before touch AF
 * [OFF] : Do one-shot AE after touch AF
 *******************************************************************************************/
#define ONE_SHOT_AE_BEFORE_TAF              ON



/********************************************************************************************
 * SKIP_ONE_SHOT_AE_FOR_TAF:
 * [ON] : Skip one-shot AE for touch AF if overall exposure is acceptable
 * [OFF] : Don't skip one-shot AE for  touch AF
 *******************************************************************************************/
#define SKIP_ONE_SHOT_AE_FOR_TAF            ON



/********************************************************************************************
 * ENABLE_TOUCH_AE:
 * [ON] : Enable the touch AE
 * [OFF] : Disable the touch AE.
 *******************************************************************************************/
#define ENABLE_TOUCH_AE                     ON



/********************************************************************************************
 * ENABLE_FACE_AE:
 * [ON] : Enable the touch AE
 * [OFF] : Disable the touch AE.
 *******************************************************************************************/
#define ENABLE_FACE_AE                      ON



/********************************************************************************************
 * LOCK_AE_DURING_CAF:
 * [ON]: Lock AE when doing CAF(conti' AF)
 * [OFF]: AE can work when doing CAF
 *******************************************************************************************/
#define LOCK_AE_DURING_CAF                  ON

/********************************************************************************************
 * ENABLE_VIDEO_DYNAMIC_FRAME_RATE:
 * [ON] : Enable the video dynamic frame rate
 * [OFF] : Disable the video dynamic frame rate.
 *******************************************************************************************/
#define ENABLE_VIDEO_DYNAMIC_FRAME_RATE ON



MBOOL CUST_ENABLE_PRECAPTURE_AF(void)
{
    return ENABLE_PRECAPTURE_AF;
}
MBOOL CUST_PRECAPTURE_AF_AFTER_PREFLASH(void)
{
    return PRECAPTURE_AF_AFTER_PREFLASH;
}
MBOOL CUST_ENABLE_VIDEO_AUTO_FLASH(void)
{
    return ENABLE_VIDEO_AUTO_FLASH;
}
MBOOL CUST_CAF_WINDOW_FOLLOW_TAF_WINDOW(void)
{
    return CAF_WINDOW_FOLLOW_TAF_WINDOW;
}
MBOOL CUST_ONE_SHOT_AE_BEFORE_TAF(void)
{
    return ONE_SHOT_AE_BEFORE_TAF;
}
MBOOL CUST_SKIP_ONE_SHOT_AE_FOR_TAF(void)
{
    return SKIP_ONE_SHOT_AE_FOR_TAF;
}
MBOOL CUST_ENABLE_TOUCH_AE(void)
{
    return ENABLE_TOUCH_AE;
}
MBOOL CUST_ENABLE_FACE_AE(void)
{
    return ENABLE_FACE_AE;
}
MBOOL CUST_LOCK_AE_DURING_CAF(void)
{
    return LOCK_AE_DURING_CAF;
}

MBOOL CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE(void)
{
    return ENABLE_VIDEO_DYNAMIC_FRAME_RATE;
}


