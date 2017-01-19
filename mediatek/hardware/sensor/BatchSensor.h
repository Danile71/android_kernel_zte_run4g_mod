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

#ifndef ANDROID_BATCH_SENSOR_H
#define ANDROID_BATCH_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


#include "nusensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"
#include <linux/hwmsensor.h> 

/*****************************************************************************/
struct input_event;

class BatchSensor : public SensorBase {
	
	enum {
        Accelerometer    	= 0,
        MagneticField    	= 1,
        Orientation      		= 2, 
        Gyro             		= 3,
        light            			= 4,
        proximity        		= 5,
        pressure		 	= 6,
        temperature		= 7,
       // stepcounter		 	= 8,
        //stepdetector	 	= 9,
       // significantmotion	= 10,
        numSensors       	= 8,//BatchSensor driver process 8 device data
    };
	
    	int mEnabled;
    	InputEventCircularReader mInputReader;
	int64_t mEnabledTime;
	char input_sysfs_path[PATH_MAX];
	int input_sysfs_path_len;
	int mDataDiv[numSensors];
	int flushSensorBitMap;
	bool mHasPendingEvent;
	//uint32_t mPendingMask;
	int64_t mTimestampHi;

public:
	int mdata_fd;	
    sensors_meta_data_event_t mPendingEvent;

            BatchSensor();
    virtual ~BatchSensor();

    virtual int readEvents(sensors_event_t* data, int count);
    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled);
	virtual int batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs);
	virtual int flush(int handle);
    void processEvent(int code, int value);
	int TypeToSensor(int type);
	void GetSensorDiv(int div[]);
	virtual bool hasPendingEvents() const;
	int FindDataFd();
};

/*****************************************************************************/

#endif  // ANDROID_BATCH_SENSOR_H
