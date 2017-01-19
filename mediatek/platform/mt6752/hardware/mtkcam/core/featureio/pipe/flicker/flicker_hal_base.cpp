#define LOG_TAG "flicker_hal_base"

#include <stdlib.h>
#include <stdio.h>
//#include "pipe_types.h"
#include <mtkcam/common.h>
#include <mtkcam/common.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <camera_custom_nvram.h>
//#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
//#include <isp_tuning.h>
#include <ispdrv_mgr.h>
#include <mtkcam/featureio/tuning_mgr.h>
#include <camera_custom_cam_cal.h>
#include <cam_cal_drv.h>
#include <mtkcam/hal/IHalSensor.h>
#include <ctype.h>
#include <sys/stat.h>
#include <aaa_sensor_mgr.h>
#include <time.h>
#include <kd_camera_feature.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include <vector>
//#include <mtkcam/acdk/cct_feature.h>



#include "Flicker_type.h"

#include <utils/threads.h>
#include <cutils/log.h>
//#include "CamTypes.h"
//#include "isp_reg.h"
//#include "isp_drv.h"
//#include <aaa_hal_base.h>
//#include "eis_hal_base.h"

#include "camera_custom_nvram.h"

//#include "CamDefs.h"
#include "awb_param.h"
#include "ae_param.h"
#include "af_param.h"
#include "camera_custom_AEPlinetable.h"
//#include "ae_mgr.h"


#include "sequential_testing.h"
#include "FlickerDetection.h"
//#include "isp_sysram_drv.h"
//#include "sensor_drv.h"

//#include "isp_mgr.h"
//#include "mcu_drv.h"
//#include "af_mgr.h"
#include "camera_custom_flicker.h"
//#include <content/IContentManager.h>
#include "mtkcam/drv/imem_drv.h"

#include "flicker_hal_base.h"

#include "flicker_hal.h"
#include "camera_custom_nvram.h"
#include <nvram_drv.h>
#include "flicker_util.h"
#include "aaa_sensor_mgr.h"
#include "camera_custom_flicker_para.h"
#include "ae_feature.h"




using namespace NS3A;


inline int errOp(int e1, int e2, int e3)
{
    if(e1!=0)
        return 0;
    if(e2!=0)
        return 0;
    if(e3!=0)
        return 0;
    return 1;
}

#define def_Imp(function) \
int FlickerHalBase::function(int senorDev)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function();    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function();  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function();  \
    return errOp(errMain, errSub, errMain2);    \
}

#define def_Imp1(function) \
int FlickerHalBase::function(int senorDev, int v1)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function(v1);    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function(v1);  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function(v1);  \
    return errOp(errMain, errSub, errMain2);    \
}

#define def_Imp2(function) \
int FlickerHalBase::function(int senorDev, int v1, int v2)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function(v1, v2);    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function(v1, v2);  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function(v1, v2);  \
    return errOp(errMain, errSub, errMain2);    \
}

//------------------------------
// imp
#define def_ImpBody(function,senorDev) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function();    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function();  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function();  \
    return errOp(errMain, errSub, errMain2);

#define def_ImpBody1(function,senorDev,p1) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function(p1);    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function(p1);  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function(p1);  \
    return errOp(errMain, errSub, errMain2);


#define def_ImpBody2(function,senorDev,p1, p2) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function(p1, p2);    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function(p1, p2);  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function(p1, p2);  \
    return errOp(errMain, errSub, errMain2);


#define def_ImpBody2_noRet(function,senorDev,p1, p2, ret) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    if (senorDev & e_SensorDevMain)    \
        errMain = FlickerHal::getInstance(e_SensorDevMain)->function(p1, p2);    \
    if (senorDev & e_SensorDevSub)  \
        errSub = FlickerHal::getInstance(e_SensorDevSub)->function(p1, p2);  \
    if (senorDev & e_SensorDevMain2)    \
        errMain2 = FlickerHal::getInstance(e_SensorDevMain2)->function(p1, p2);  \
    ret = errOp(errMain, errSub, errMain2);


enum
{
    e_SensorDevMain = 1,
    e_SensorDevSub = 2,
    e_SensorDevMain2 = 8,
};


//=======================================================
//  implementation
//=======================================================
static int g_flickerEn=1;
FlickerHalBase& FlickerHalBase::getInstance()
{
    static FlickerHalBase obj;
    return obj;

}

int FlickerHalBase::createBuf(int sensorDev)
{
    g_flickerEn = FlickerUtil::getPropInt("z.flicker_en", 1);
    if(g_flickerEn==0)
        return 1;
    def_ImpBody(createBuf, sensorDev);
}

int FlickerHalBase::releaseBuf(int sensorDev)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody(releaseBuf, sensorDev);
}

int FlickerHalBase::close(int sensorDev)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody(close, sensorDev);
}

int FlickerHalBase::setSensorMode(int sensorDev, int mode)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody1(setSensorMode, sensorDev, mode);
}

int FlickerHalBase::setFlickerMode(int sensorDev, int mode)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody1(setFlickerMode, sensorDev, mode);
}

int FlickerHalBase::open(int sensorDev, int a, int b)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody2(open, sensorDev, a, b);
}

/*
def_Imp1(setSensorMode)
def_Imp1(setFlickerMode)

def_Imp(createBuf);
def_Imp(releaseBuf);

def_Imp2(open)
def_Imp(close)
*/
static int gFlickerResult = HAL_FLICKER_AUTO_50HZ;
int FlickerHalBase::update(int sensorDev, FlickerInput* in, FlickerOutput* out)
{
    if(g_flickerEn==0)
        return 1;
    //def_ImpBody2(update, sensorDev, in, out);
    int ret;
    def_ImpBody2_noRet(update, sensorDev, in, out, ret);
    gFlickerResult = out->flickerResult;
    return ret;
}


int FlickerHalBase::getFlickerResult(int& flickerResult)
{
    flickerResult = gFlickerResult;
    return 1;
}
