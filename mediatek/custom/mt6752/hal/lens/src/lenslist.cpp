#include <utils/Log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>
#include "MediaHal.h"
#include "camera_custom_lens.h"
#include "kd_imgsensor.h"

extern PFUNC_GETLENSDEFAULT pDummy_getDefaultData;

#if defined(GAF001AF)
extern PFUNC_GETLENSDEFAULT pGAF001AF_getDefaultData;
#endif
#if defined(GAF002AF)
extern PFUNC_GETLENSDEFAULT pGAF002AF_getDefaultData;
#endif
#if defined(GAF008AF)
extern PFUNC_GETLENSDEFAULT pGAF008AF_getDefaultData;
#endif

//name must be > 0x1000, and corresponding to GAF00xAF.c
#define DW9714AF_LENS_NAME "0x9714"
#define AD5820AF_LENS_NAME "0x5820"
#define DW9718AF_LENS_NAME "0x9718"
#define AD5823AF_LENS_NAME "0x5823"
#define BU6424AF_LENS_NAME "0x6424"
#define BU6429AF_LENS_NAME "0x6429"
#define FM50AF_LENS_NAME   "0xF050"
#define RUMBAAF_LENS_NAME  "0x6334"
#define OV8825AF_LENS_NAME "0x8825"


MSDK_LENS_INIT_FUNCTION_STRUCT LensList_main[MAX_NUM_OF_SUPPORT_LENS] =
{
	{DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
#if defined(GAF001AF)
	{IMX135_SENSOR_ID, GAF001AF_LENS_ID, DW9714AF_LENS_NAME, pGAF001AF_getDefaultData},
#endif

};
MSDK_LENS_INIT_FUNCTION_STRUCT LensList_sub[MAX_NUM_OF_SUPPORT_LENS] =
{
	{DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
#if defined(GAF002AF)
	{OV5648MIPI_SENSOR_ID, GAF002AF_LENS_ID, AD5820AF_LENS_NAME, pGAF002AF_getDefaultData},
#endif

};
MSDK_LENS_INIT_FUNCTION_STRUCT LensList_main2[MAX_NUM_OF_SUPPORT_LENS] =
{
	{DUMMY_SENSOR_ID, DUMMY_LENS_ID, "Dummy", pDummy_getDefaultData},
#if defined(GAF008AF)
	{OV5648MIPI_SENSOR_ID, GAF008AF_LENS_ID, AD5820AF_LENS_NAME, pGAF008AF_getDefaultData},
	{IMX135_SENSOR_ID, GAF008AF_LENS_ID, DW9714AF_LENS_NAME, pGAF008AF_getDefaultData},
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

