#include "camera_custom_types.h"
#include "aaa_scheduling_custom.h"


const char* Job3AName(E_Job_3A eJob)
{
    static const char* Names[E_Job_NUM] = 
    {
        "AAO",
        "Awb",
        "Af",
        "Flicker",
        "Lsc",
        "AeFlare",
        "IspValidate" //IspValidate on/off is not listed in scheduling table entry
    };
    return Names[eJob];
}


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
WorkPerCycle getWorkPerCycle(int normalizeM, MUINT32 senDevId) // M = fps/30
{
    switch(senDevId)
    {
    case 0x01: // SENSOR_DEV_MAIN = 0x01
        return getWorkPerCycle_Main(normalizeM);
    case 0x02: // SENSOR_DEV_SUB  = 0x02
        return getWorkPerCycle_Sub(normalizeM);
    case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
        return getWorkPerCycle_Main2(normalizeM);
    default:
        return getWorkPerCycle_Main(normalizeM);
    }
    return getWorkPerCycle_Main(normalizeM);
}

MVOID resetCycleCtr(MUINT32 senDevId) //M = fps/30
{
    switch(senDevId)
    {
    case 0x01: // SENSOR_DEV_MAIN = 0x01
        return resetCycleCtr_Main();
    case 0x02: // SENSOR_DEV_SUB  = 0x02
        return resetCycleCtr_Sub();
    case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
        return resetCycleCtr_Main2();
    default:
        return resetCycleCtr_Main();
    }
    return resetCycleCtr_Main();
}

int get3AThreadNiceValue(MUINT32 senDevId)
{
	switch(senDevId)
	{
	case 0x01: // SENSOR_DEV_MAIN = 0x01
		return -8;
	case 0x02: // SENSOR_DEV_SUB  = 0x02
		return -8;
	case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
		return -8;
	default:
		return -8;
	}
	return -8;
}


long long int getVsTimeOutLimit_ns(int normalizeM, int fps/*x1000*/)
{
    // you may define your own timeout limit for each fps and each M
    /*
    criterion: as long as possible, but it should satisfy: 
        1. impossible to catch next Update, 
        2. 3A thread has enough time to wait next Vsync
    tolerance: 
        33-6=27ms for 30fps, -> P1 dump might be short (ex: 10ms), choose 8ms
        16-3=13ms for 60fps, -> P1 dump might be short (ex: 10ms), choose 8ms
        11-3=8ms for 90fps, -> might have short-P1-dump issue
        8-3=5ms for 120fps, -> might have short-P1-dump issue
    */
    switch((int)(fps/30000))
    {
    case 0:
        return (long long int) 8000000;
    case 1:
        return (long long int) 8000000;
    case 2:
        return (long long int) 8000000;
    case 3: 
        return (long long int) 8000000;
    case 4:
        return (long long int) 5000000;
    default:
        return (long long int) 5000000; //shortest
    }
    return     (long long int) 5000000; //shortest
}

int queryFramesPerCycle_custom(int fps/*x1000*/)
{
    //might be customized
    return (((fps/30000) >= 1) ? (fps/30000) : 1); 
}

int getResetFrameCount(MUINT32 senDevId)
{
    switch(senDevId)
    {
    case 0x01: // SENSOR_DEV_MAIN = 0x01
        return -2;
    case 0x02: // SENSOR_DEV_SUB  = 0x02
        return -2;
    case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
        return -2;
    default:
        return -3; //conservative
    }
    return -3; //conservative
}


