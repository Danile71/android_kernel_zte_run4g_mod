#ifndef AAA_SCHEDULING_CUSTOM_H_
#define AAA_SCHEDULING_CUSTOM_H_


#define MAX_FRAME_PER_CYCLE (6) //max number of M


enum // E_AE_JOB
{
    E_AE_IDLE      = 0,
    E_AE_AE_CALC   = (1<<0),
    E_AE_AE_APPLY  = (1<<1),
    E_AE_FLARE     = (1<<2)
};

enum E_Job_3A
{
    E_Job_AAO      = 0,
    E_Job_Awb,
    E_Job_Af,
    E_Job_Flicker,
    E_Job_Lsc,
    E_Job_AeFlare,
    E_Job_IspValidate, //IspValidate on/off  is not listed in scheduling table entry
    E_Job_NUM
};


struct WorkPerFrame
{
    MUINT32 AAOJobs;
    MUINT32 AwbJobs;
    MUINT32 AfJobs;
    MUINT32 FlickerJobs;
    MUINT32 LscJobs;
    
    MUINT32 AeFlareJobs;
};

struct WorkPerCycle
{
    WorkPerFrame mWorkPerFrame[MAX_FRAME_PER_CYCLE];
    MUINT32 mValidFrameIdx;
};

const char* Job3AName(E_Job_3A eJob);
WorkPerCycle getWorkPerCycle(int normalizeM, MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/); //M = fps/30
WorkPerCycle getWorkPerCycle_Main(int normalizeM);
WorkPerCycle getWorkPerCycle_Main2(int normalizeM);
WorkPerCycle getWorkPerCycle_Sub(int normalizeM);

MVOID resetCycleCtr(MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/);
MVOID resetCycleCtr_Main();
MVOID resetCycleCtr_Main2();
MVOID resetCycleCtr_Sub();

/*
definition of senDevId:
in IHalSensor.h (\alps_sw\trunk\kk\alps\mediatek\hardware\include\mtkcam\hal)
enum 
{
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_PIP = 0x03,
    SENSOR_DEV_MAIN_2 = 0x04,
    SENSOR_DEV_MAIN_3D = 0x05,
};
*/

int get3AThreadNiceValue(MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/);

long long int getVsTimeOutLimit_ns(int normalizeM, int fps/*x1000*/);
int queryFramesPerCycle_custom(int fps/*x1000*/);
int getResetFrameCount(MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/);

#endif /* AAA_SCHEDULING_CUSTOM_H_ */

