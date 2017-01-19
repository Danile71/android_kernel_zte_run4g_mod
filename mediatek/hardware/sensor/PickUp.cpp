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

#include "PickUp.h"
#define LOG_TAG "PICK_UP"


#define IGNORE_EVENT_TIME 350000000
#define SYSFS_PATH           "/sys/class/input"


/*****************************************************************************/
PickUpSensor::PickUpSensor()
    : SensorBase(NULL, "m_pkup_input"),//_INPUTDEV_NAME
      mEnabled(0),
      mInputReader(32)
{
    input_sysfs_path[PATH_MAX] = {0};
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_PICK_UP;
    mPendingEvent.type = SENSOR_TYPE_PICK_UP;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
	mEnabledTime =0;
	mDataDiv = 1;
	mPendingEvent.timestamp =0;
	input_sysfs_path_len = 0;

	mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strcpy(input_sysfs_path, "/sys/class/misc/m_pkup_misc/");
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
	ALOGD("PickUp misc path =%s", input_sysfs_path);

	char datapath[64]={"/sys/class/misc/m_pkup_misc/pkupactive"};
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

PickUpSensor::~PickUpSensor() {
	if (mdata_fd >= 0)
		close(mdata_fd);
}

int PickUpSensor::FindDataFd() {
	int fd = -1;
	int num = -1;
	char buf[64]={0};
	char *devnum_dir = NULL;
	char buf_s[64] = {0};


	devnum_dir = "/sys/class/misc/m_pkup_misc/pkupdevnum";
	
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

int PickUpSensor::enable(int32_t handle, int en)
{
    int fd;
    int flags = en ? 1 : 0;
	
	ALOGD("pkup enable: handle:%d, en:%d \r\n",handle,en);
    	strcpy(&input_sysfs_path[input_sysfs_path_len], "pkupactive");
	ALOGD("path:%s \r\n",input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd<0)
	{
	  	ALOGD("no pkup enable control attr\r\n" );
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
    ALOGD("pkup enable(%d) done", mEnabled );    
    return 0;
}
int PickUpSensor::setDelay(int32_t handle, int64_t ns)
{
   	//int fd;

	ALOGD("setDelay: regardless of the setDelay() value (handle=%d, ns=%d)",handle, ns);
  
    return 0;

}
int PickUpSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    	int flag;
	int fd;
	
	ALOGE("pkup batch: handle:%d, en:%d, maxBatchReportLatencyNs:%lld \r\n",handle, flags, maxBatchReportLatencyNs);
	if(maxBatchReportLatencyNs == 0){
		flag = 0;
	}else{
		flag = 1;
	}
	
    strcpy(&input_sysfs_path[input_sysfs_path_len], "pkupbatch");
	ALOGD("path:%s \r\n",input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd < 0)
	{
	  	ALOGD("no pkup batch control attr\r\n" );
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
	
    ALOGD("pkup batch(%d) done", flag );    
    return 0;

}

int PickUpSensor::flush(int handle)
{
    return -errno;
}

int PickUpSensor::readEvents(sensors_event_t* data, int count)
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
        if (type == EV_ABS || type == EV_REL) 
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
		else
        { 
            ALOGE("pkup: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }
	//ALOGE("fwq read Event 2\r\n");
    return numEventReceived;
}

void PickUpSensor::processEvent(int code, int value)
{
    ALOGD("PickUpSensor::processEvent code=%d,value=%d\r\n",code, value);
    switch (code) {
	case EVENT_TYPE_PKUP_VALUE:
		mPendingEvent.data[0] = (float) value;
		break;
    }
}
