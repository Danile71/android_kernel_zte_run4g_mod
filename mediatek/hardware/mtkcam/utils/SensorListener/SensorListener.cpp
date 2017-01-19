#define LOG_TAG "MtkCam/SensorListener"
//
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/List.h>
//
#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
//
#include <gui/SensorManager.h>
#include <utils/Looper.h>
#include <gui/Sensor.h>
//
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/v1/camutils/CamMisc.h>
//
#include <mtkcam/utils/SensorListener.h>
#include <SensorListenerImp.h>
//-----------------------------------------------------------------------------
bool
setThreadPriority(
    int policy,
    int priority)
{
    struct sched_param sched_p;
    //
    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);   //  Note: "priority" is nice value.
        MY_LOGD("tid(%d) policy(SCHED_OTHER:%d) priority(%d)", ::gettid(), policy, priority);
    } else {
        sched_p.sched_priority = priority;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, policy, &sched_p);
        MY_LOGD("tid(%d) policy(Real-Time:%d) priority(%d)", ::gettid(), policy, priority);
    }

    return true;
}
//-----------------------------------------------------------------------------
bool
getThreadPriority(
    int& policy,
    int& priority)
{
    policy = ::sched_getscheduler(0);
    if (policy == SCHED_OTHER) {
        // a conventional process has only the static priority
        priority = ::getpriority(PRIO_PROCESS, 0);
        MY_LOGD("tid(%d) policy(SCHED_OTHER:%d) priority(%d)", ::gettid(), policy, priority);
    } else {
        // a real-time process has both the static priority and real-time priority.
        struct sched_param sched_p;
        ::sched_getparam(0, &sched_p);
        priority = sched_p.sched_priority;
        MY_LOGD("tid(%d) policy(Real-Time:%d) priority(%d)", ::gettid(), policy, priority);
    }

    return true;
}
//-----------------------------------------------------------------------------
int SensorListenerImp_callback(
    int     fd,
    int     events,
    void*   pData)
{
    //MY_LOGD("+");
    SensorListenerImp* pInstance = (SensorListenerImp*)pData;
    pInstance->getEvents();
    //MY_LOGD("-");
    //should return 1 to continue receiving callbacks, or 0 to unregister
    return 1;
}
//-----------------------------------------------------------------------------
SensorListenerImp::
SensorListenerImp()
{
    FUNCTION_NAME;
    mUser = 0;
}
//----------------------------------------------------------------------------
SensorListenerImp::
~SensorListenerImp()
{
    FUNCTION_NAME;
}
//-----------------------------------------------------------------------------
SensorListener*
SensorListener::
createInstance(void)
{
    FUNCTION_NAME;
    SensorListenerImp* pInstance = new SensorListenerImp();
    pInstance->init();
    return pInstance;
}
//----------------------------------------------------------------------------
MVOID
SensorListenerImp::
destroyInstance(void)
{
    FUNCTION_IN;
    //
    uninit();
    delete this;
    //
    FUNCTION_OUT;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
setListener(Listener func)
{
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user(%d)",mUser);
        return MFALSE;
    }
    //
    if(func == NULL)
    {
        MY_LOGW("func is NULL");
        return MFALSE;
    }
    //
    MY_LOGD("func(0x%08X)",(MUINT32)func);
    mpListener = func;
    //
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
enableSensor(
    SensorTypeEnum  sensorType,
    MUINT32         periodInMs)
{
    MBOOL ret = MTRUE;
    //
    MY_LOGD("+Type(%d),Rate(%d)",
            sensorType,
            periodInMs);
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user");
        ret = MFALSE;
        goto EXIT;
    }
    //
    if(sensorType >= SENSOR_TYPE_AMOUNT)
    {
        MY_LOGE("unknown sensorType(%d)",sensorType);
        ret = MFALSE;
    }
    //
    if(ret)
    {
        if(mpSensor[sensorType] != NULL)
        {
            MY_LOGD("enableSensor");
            mspSensorEventQueue->enableSensor(mpSensor[sensorType]);
            MY_LOGD("setEventRate");
            mspSensorEventQueue->setEventRate(mpSensor[sensorType],ms2ns(periodInMs));
            mSensorEnableCnt++;
        }
        else
        {
            MY_LOGW("sensorType(%d) does not exist",sensorType);
            ret = MFALSE;
        }
        //
        MY_LOGD("-Type(%d),Rate(%d),Cnt(%d)",
                sensorType,
                periodInMs,
                mSensorEnableCnt);
    }
    //
    EXIT:
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
disableSensor(SensorTypeEnum sensorType)
{
    MBOOL ret = MTRUE;
    //
    MY_LOGD("+Type(%d)",sensorType);
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user");
        ret = MFALSE;
        goto EXIT;
    }
    //
    if(sensorType >= SENSOR_TYPE_AMOUNT)
    {
        MY_LOGE("unknown sensorType(%d)",sensorType);
        ret = MFALSE;
    }
    //
    if(ret)
    {
        if(mSensorEnableCnt > 0)
        {
            if(mpSensor[sensorType] != NULL)
            {
                MY_LOGD("disableSensor");
                mspSensorEventQueue->disableSensor(mpSensor[sensorType]);
                mSensorEnableCnt--;
            }
            else
            {
                MY_LOGW("sensorType(%d) does not exist",sensorType);
                ret = MFALSE;
            }            
            //
            MY_LOGD("-Type(%d),Cnt(%d)",
                    sensorType,
                    mSensorEnableCnt);
        }
        else
        {
            MY_LOGW("-sensorType(%d),userCnt is 0",sensorType);
            ret = MFALSE;
        }
    }
    //
    EXIT:
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
init(void)
{
    MBOOL Result = MTRUE;
    MUINT32 i;
    //
    if(!isSupport())
    {
        return MFALSE;
    }
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        MY_LOGD("First user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        android_atomic_inc(&mUser);
        goto EXIT;
    }
    //
    for(i=0; i<SENSOR_TYPE_AMOUNT; i++)
    {
        mpSensor[i] = NULL;
    }
    mSensorEnableCnt = 0;
    mpListener = NULL;
    //
    mpSensorManager = &SensorManager::getInstance();
    //
    mpSensor[SensorType_Acc] = mpSensorManager->getDefaultSensor(ASENSOR_TYPE_ACCELEROMETER);
    mpSensor[SensorType_Mag] = mpSensorManager->getDefaultSensor(ASENSOR_TYPE_MAGNETIC_FIELD);
    mpSensor[SensorType_Gyro] = mpSensorManager->getDefaultSensor(ASENSOR_TYPE_GYROSCOPE);
    mpSensor[SensorType_Light] = mpSensorManager->getDefaultSensor(ASENSOR_TYPE_LIGHT);
    mpSensor[SensorType_Proxi] = mpSensorManager->getDefaultSensor(ASENSOR_TYPE_PROXIMITY);
    //
    mspSensorEventQueue = mpSensorManager->createEventQueue();
    //
    mspLooper = new Looper(false);
    mspLooper->addFd(
                mspSensorEventQueue->getFd(),
                0,
                ALOOPER_EVENT_INPUT,
                SensorListenerImp_callback,
                this);
    //
    mspThread = new SensorListenerImpThread(mspLooper.get());
    mspThread->run();
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    FUNCTION_OUT;
    return Result;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
uninit(void)
{
    MBOOL Result = MTRUE;
    MUINT32 i;
    status_t status;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user(%d)",mUser);
        goto EXIT;
    }
    //
    android_atomic_dec(&mUser);
    //
    if(mUser == 0)
    {
        MY_LOGD("Last user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        goto EXIT;
    }
    //
    MY_LOGD("requestExit");
    mspThread->requestExit();
    MY_LOGD("wake");
    mspThread->wake();
    MY_LOGD("join E");
    mspThread->join();
    MY_LOGD("join X");
    mspThread = NULL;
    //
    mspLooper->removeFd(mspSensorEventQueue->getFd());
    //
    mSensorEnableCnt = 0;
    mspLooper = NULL;
    mspSensorEventQueue = NULL;
    mpSensorManager = NULL;
    mpListener = NULL;
    //
    for(i=0; i<SENSOR_TYPE_AMOUNT; i++)
    {
        mpSensor[i] = NULL;
    }
    //
    EXIT:
    FUNCTION_OUT;
    return Result;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
getEvents(void)
{
    //Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user(%d)",mUser);
        return MFALSE;
    }
    //
    ASensorEvent event;
    //
    if(mspSensorEventQueue != NULL)
    {
        while (mspSensorEventQueue->read(&event, 1) > 0)
        {
            if(mpListener != NULL)
            {
                mpListener(event);
            }
        }
    }
    //
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
isSupport(void)
{
    MUINT32 fd;
    size_t s;
    char boot_mode[4] = {'0'};
    //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
    //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5
    fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDWR);
    if (fd < 0)
    {
        MY_LOGE("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
        return 0;
    }

    s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
    close(fd);

    if(s <= 0)
    {
        MY_LOGE("could not read boot mode sys file\n");
        return 0;
    }

    boot_mode[s] = '\0';
    MY_LOGD("Boot Mode %d\n",atoi(boot_mode));
    if( atoi(boot_mode) == 1 ||
        atoi(boot_mode) == 4)
    {
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}


