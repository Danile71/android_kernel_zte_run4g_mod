#ifndef __FLASH_TUNING_CUSTOM_H__
#define __FLASH_TUNING_CUSTOM_H__



int getDefaultStrobeNVRam_main2(void* data, int* ret_size);
int getDefaultStrobeNVRam_sub2(void* data, int* ret_size);
int getDefaultStrobeNVRam(int sensorType, void* data, int* ret_size);

int cust_fillDefaultStrobeNVRam_main (void* data);
int cust_fillDefaultStrobeNVRam_main2 (void* data);
int cust_fillDefaultStrobeNVRam_sub (void* data);
int cust_fillDefaultStrobeNVRam_sub2 (void* data);
int cust_fillDefaultStrobeNVRam(int sensorType, void* data);


FLASH_PROJECT_PARA& cust_getFlashProjectPara_main2(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub2(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);

int cust_isNeedAFLamp(int flashMode, int afLampMode, int isBvHigherTriger);


int cust_getFlashModeStyle(int sensorType, int flashMode);
int cust_getVideoFlashModeStyle(int sensorType, int flashMode);
void cust_getEvCompPara(int& maxEvTar10Bit, int& indNum, float*& evIndTab, float*& evTab, float*& evLevel);

int cust_isSubFlashSupport();

int cust_isDualFlashSupport(int sensorDev);



enum
{
    e_PrecapAf_None,
    e_PrecapAf_BeforePreflash,
    e_PrecapAf_AfterPreflash,
};
int cust_getPrecapAfMode();

int cust_isNeedDoPrecapAF_v2(int isLastFocusModeTAF, int isFocused, int flashMode, int afLampMode, int isBvLowerTriger);

void cust_setFlashPartId_main(int id);
void cust_setFlashPartId_sub(int id);

void cust_setFlashPartId(int dev, int id);

int cust_getDefaultStrobeNVRam_V2(int sensorType, void* data, int* ret_size);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_V2(int sensorDev, int AEScene, NVRAM_CAMERA_STROBE_STRUCT* nvrame);

FLASH_PROJECT_PARA& cust_getFlashProjectPara_V3(int sensorDev, int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame); //isForceFlash: 0: auto, 1: forceOn

typedef int (*FlashIMapFP)(int, int );
FlashIMapFP cust_getFlashIMapFunc(int sensorDev);


void cust_getFlashQuick2CalibrationExp_main(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_main2(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_sub(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_sub2(int* exp, int* afe, int* isp);

void cust_getFlashITab2_main(short* ITab2);
void cust_getFlashITab2_main2(short* ITab2);
void cust_getFlashITab2_sub(short* ITab2);
void cust_getFlashITab2_sub2(short* ITab2);

void cust_getFlashITab1_main(short* ITab1);
void cust_getFlashITab1_main2(short* ITab1);
void cust_getFlashITab1_sub(short* ITab1);
void cust_getFlashITab1_sub2(short* ITab1);


void cust_getFlashQuick2CalibrationExp(int sensorDev, int* exp, int* afe, int* isp);
void cust_getFlashITab2(int sensorDev, short* ITab2);
void cust_getFlashITab1(int sensorDev, short* ITab1);




#endif //#ifndef __FLASH_TUNING_CUSTOM_H__

