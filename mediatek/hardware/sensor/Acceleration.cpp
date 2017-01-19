/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "Acceleration.h"
#define LOG_TAG "Accel"


#define IGNORE_EVENT_TIME 350000000
#define SYSFS_PATH           "/sys/class/input"


/*****************************************************************************/
AccelerationSensor::AccelerationSensor()
    : SensorBase(NULL, "m_acc_input"),//ACC_INPUTDEV_NAME
      mEnabled(0),
      mOrientationEnabled(0),
      mInputReader(32)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_ACCELEROMETER;
    mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
	mEnabledTime =0;
	mDataDiv = 1;
	mPendingEvent.timestamp =0;
	input_sysfs_path_len = 0;
	input_sysfs_path[PATH_MAX] = {0};

    mdata_fd = FindDataFd(); 	
    if (mdata_fd >= 0) {
        strcpy(input_sysfs_path, "/sys/class/misc/m_acc_misc/");
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
	ALOGD("acc misc path =%s", input_sysfs_path);

	char datapath[64]={"/sys/class/misc/m_acc_misc/accactive"};
	int fd = open(datapath, O_RDWR);
	char buf[64];
	int len;
	len = read(fd,buf,sizeof(buf));
	sscanf(buf, "%d", &mDataDiv);
	ALOGD("read div buf(%s)", datapath);
	ALOGD("mdiv %d",mDataDiv );
	if(len<=0)
	{
		ALOGD("read div err buf(%s)",buf );
	}
	close(fd);
}

AccelerationSensor::~AccelerationSensor() {
	if (mdata_fd >= 0)
		close(mdata_fd);
}

int AccelerationSensor::FindDataFd() {
	int fd = -1;
	int num = -1;
	char buf[64]={0};
	char *devnum_dir = NULL;
	char buf_s[64] = {0};


	devnum_dir = "/sys/class/misc/m_acc_misc/accdevnum";
	
	fd = open(devnum_dir, O_RDONLY);
	if (fd >= 0)
	{
		read(fd, buf, sizeof(buf));
		sscanf(buf, "%d\n", &num);
		close(fd);
	}else{
		return -1;
	}
	sprintf(buf_s, "/dev/input/event%d", num);
	fd = open(buf_s, O_RDONLY);
    ALOGE_IF(fd<0, "couldn't find input device");
	return fd;
}
int AccelerationSensor::enableNoHALDataAcc(int en)
{
	int fd;
	ALOGD("ACC enable nodata en(%d) \r\n",en);
    strcpy(&input_sysfs_path[input_sysfs_path_len], "accenablenodata");
	ALOGD("path:%s \r\n",input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd<0)
	{
	  	ALOGD("no ACC enable nodata control attr\r\n" );
	  	return -1;
	}

	char buf[2];
	buf[1] = 0;
	if(1==en)
	{
		buf[0] = '1';
	}
	if(0==en)
	{
		buf[0] = '0';
	}
	
	write(fd, buf, sizeof(buf));
  	close(fd);
	
    ALOGD("ACC enable nodata done");    
    return 0;
}

int AccelerationSensor::enable(int32_t handle, int en)
{
    int fd;
    int flags = en ? 1 : 0;
	
	ALOGD("ACC enable: handle:%d, en:%d \r\n",handle,en);
    strcpy(&input_sysfs_path[input_sysfs_path_len], "accactive");
	ALOGD("path:%s \r\n",input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd<0)
	{
	  	ALOGD("no ACC enable control attr\r\n" );
	  	return -1;
	}
	
	mEnabled = flags;
	char buf[2];
	buf[1] = 0;
	if (flags) 
	{
 		buf[0] = '1';
     	mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
	}
	else 
 	{
      	buf[0] = '0';
	}
    write(fd, buf, sizeof(buf));
  	close(fd);
	
    ALOGD("ACC enable(%d) done", mEnabled );    
    return 0;
}
int AccelerationSensor::setDelay(int32_t handle, int64_t ns)
{
    int fd;
	//uint32_t ms=0;
	ALOGD("setDelay: (handle=%d, ns=%d)",handle, ns);
    strcpy(&input_sysfs_path[input_sysfs_path_len], "accdelay");
	ALOGD("path:%s \r\n",input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd<0)
	{
	   	ALOGD("no ACC setDelay control attr \r\n" );
	  	return -1;
	}
	//ms = ns/1000000;
    
	char buf[80];
	sprintf(buf, "%lld", ns);
	write(fd, buf, strlen(buf)+1);
	close(fd);
    return 0;
}

int AccelerationSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    int fd;
    int flag;
	
	ALOGE("ACC batch: handle:%d, en:%d, maxBatchReportLatencyNs:%lld \r\n",handle, flags, maxBatchReportLatencyNs);

	//Don't change batch status if dry run.
	if (flags & SENSORS_BATCH_DRY_RUN)
		return 0;
	
	if(maxBatchReportLatencyNs == 0){
		flag = 0;
	}else{
		flag = 1;
	}
	
    strcpy(&input_sysfs_path[input_sysfs_path_len], "accbatch");
	ALOGD("path:%s \r\n",input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd < 0)
	{
	  	ALOGD("no ACC batch control attr\r\n" );
	  	return -1;
	}
	
	char buf[2];
	buf[1] = 0;
	if (flag) 
	{
 		buf[0] = '1';
	}
	else 
 	{
      	buf[0] = '0';
	}
    write(fd, buf, sizeof(buf));
  	close(fd);
	
    ALOGD("ACC batch(%d) done", flag );    
    return 0;

}

int AccelerationSensor::flush(int handle)
{
    return -errno;
}

int AccelerationSensor::readEvents(sensors_event_t* data, int count)
{

    //ALOGE("fwq read Event 1\r\n");
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(mdata_fd);
    if (n < 0)
        return n;
    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
		//ALOGE("fwq1....\r\n");
        if (type == EV_ABS) 
		{
            processEvent(event->code, event->value);
			//ALOGE("fwq2....\r\n");
        } 
		else if (type == EV_SYN) 
        {
            //ALOGE("fwq3....\r\n");
            int64_t time = timevalToNano(event->time);
            mPendingEvent.timestamp = time;
            if (mEnabled) 
			{
                 //ALOGE("fwq4....\r\n");
			     if (mPendingEvent.timestamp >= mEnabledTime) 
				 {
				    //ALOGE("fwq5....\r\n");
				 	*data++ = mPendingEvent;
					numEventReceived++;
			     }
                 count--;
                
            }
        } 
		else if (type != EV_ABS) 
        { 
            ALOGE("AccelerationSensor: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }
	//ALOGE("fwq read Event 2\r\n");
    return numEventReceived;
}

void AccelerationSensor::processEvent(int code, int value)
{
    //ALOGD("processEvent code=%d,value=%d\r\n",code, value);
    switch (code) {
	case EVENT_TYPE_ACCEL_STATUS:
		mPendingEvent.acceleration.status = value;
		break;
       case EVENT_TYPE_ACCEL_X:
            	mPendingEvent.acceleration.x = (float)value / mDataDiv ;
            	break;
       case EVENT_TYPE_ACCEL_Y:
            	mPendingEvent.acceleration.y = (float)value / mDataDiv;
            	break;
       case EVENT_TYPE_ACCEL_Z:
            	mPendingEvent.acceleration.z = (float)value / mDataDiv;
            	break;
    }
}
