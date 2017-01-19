#include <utils/Log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>
#include "MediaHal.h"
#include "camera_custom_lens.h"
#include "kd_imgsensor.h"

extern PFUNC_GETLENSDEFAULT pDummy_getDefaultData;

#if defined(SENSORDRIVE)
extern PFUNC_GETLENSDEFAULT pSensorDrive_getDefaultData;
#endif
#if defined(FM50AF)
extern PFUNC_GETLENSDEFAULT pFM50AF_getDefaultData;
#endif
#if defined(AD5820AF)
extern PFUNC_GETLENSDEFAULT pAD5820AF_getDefaultData;
#endif

#if defined(DW9714AF)
extern PFUNC_GETLENSDEFAULT pDW9714AF_getDefaultData;
#endif
#if defined(GAF001AF)
extern PFUNC_GETLENSDEFAULT pGAF001AF_getDefaultData;
#endif
#if defined(BU64745GWZAF)
extern PFUNC_GETLENSDEFAULT pBU64745GWZAF_getDefaultData;
#endif
#if defined(LC898122AF)
extern PFUNC_GETLENSDEFAULT pLC898122AF_getDefaultData;
#endif


MSDK_LENS_INIT_FUNCTION_STRUCT LensList_main[MAX_NUM_OF_SUPPORT_LENS] =
{
    {DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
    #if defined(SENSORDRIVE)    
        {OV3640_SENSOR_ID, SENSOR_DRIVE_LENS_ID, "kd_camera_hw", pSensorDrive_getDefaultData},
    #endif
    #if defined(FM50AF)
        {DUMMY_SENSOR_ID, FM50AF_LENS_ID, "FM50AF", pFM50AF_getDefaultData},
    #endif
    #if defined(DW9714AF)
        {IMX135_SENSOR_ID, DW9714AF_LENS_ID, "DW9714AF", pDW9714AF_getDefaultData},
    #endif
    #if defined(AD5820AF)
        {OV5648MIPI_SENSOR_ID, AD5820AF_LENS_ID, "AD5820AF", pAD5820AF_getDefaultData},
    #endif
    #if defined(GAF001AF)
        {IMX135_SENSOR_ID, GAF001AF_LENS_ID, "0x9714", pGAF001AF_getDefaultData},
    #endif
    #if defined(LC898122AF)
        {IMX214_SENSOR_ID, LC898122AF_LENS_ID, "LC898122AF", pLC898122AF_getDefaultData},
    #endif
    #if defined(GAF001AF)
        {S5K2P8_SENSOR_ID, GAF001AF_LENS_ID, "0x6474", pGAF001AF_getDefaultData},
    #endif
    #if defined(BU64745GWZAF)
        {S5K2P8_SENSOR_ID, BU64745GWZAF_LENS_ID, "BU64745GWZAF", pBU64745GWZAF_getDefaultData},
    #endif

};
MSDK_LENS_INIT_FUNCTION_STRUCT LensList_sub[MAX_NUM_OF_SUPPORT_LENS] =
{
    {DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
    #if defined(SENSORDRIVE)    
        {OV3640_SENSOR_ID, SENSOR_DRIVE_LENS_ID, "kd_camera_hw", pSensorDrive_getDefaultData},
    #endif
    #if defined(FM50AF)
        {DUMMY_SENSOR_ID, FM50AF_LENS_ID, "FM50AF", pFM50AF_getDefaultData},
    #endif
    #if defined(DW9714AF)
        {IMX135_SENSOR_ID, DW9714AF_LENS_ID, "DW9714AF", pDW9714AF_getDefaultData},
    #endif
    #if defined(AD5820AF)
        {OV5648MIPI_SENSOR_ID, AD5820AF_LENS_ID, "AD5820AF", pAD5820AF_getDefaultData},
    #endif

};
MSDK_LENS_INIT_FUNCTION_STRUCT LensList_main2[MAX_NUM_OF_SUPPORT_LENS] =
{
    {DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
    #if defined(SENSORDRIVE)    
        {OV3640_SENSOR_ID, SENSOR_DRIVE_LENS_ID, "kd_camera_hw", pSensorDrive_getDefaultData},
    #endif
    #if defined(FM50AF)
        {DUMMY_SENSOR_ID, FM50AF_LENS_ID, "FM50AF", pFM50AF_getDefaultData},
    #endif
    #if defined(DW9714AF)
        {IMX135_SENSOR_ID, DW9714AF_LENS_ID, "DW9714AF", pDW9714AF_getDefaultData},
    #endif
    #if defined(AD5820AF)
        {OV5648MIPI_SENSOR_ID, AD5820AF_LENS_ID, "AD5820AF", pAD5820AF_getDefaultData},
    #endif
};



UINT32 GetLensInitFuncList(PMSDK_LENS_INIT_FUNCTION_STRUCT pLensList, unsigned int a_u4CurrSensorDev)
{
    if(a_u4CurrSensorDev==2) //sub
        memcpy(pLensList, &LensList_sub[0], sizeof(MSDK_LENS_INIT_FUNCTION_STRUCT)* MAX_NUM_OF_SUPPORT_LENS);
    else if(a_u4CurrSensorDev==4) //main 2
        memcpy(pLensList, &LensList_main2[0], sizeof(MSDK_LENS_INIT_FUNCTION_STRUCT)* MAX_NUM_OF_SUPPORT_LENS);
    else  // main or others
        memcpy(pLensList, &LensList_main[0], sizeof(MSDK_LENS_INIT_FUNCTION_STRUCT)* MAX_NUM_OF_SUPPORT_LENS);

    return MHAL_NO_ERROR;
}
