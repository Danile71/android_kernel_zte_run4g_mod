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

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

__BEGIN_DECLS

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device);
//static int g_sensor_user_count[32];

/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
/*
#define ID_A  (0)
#define ID_M  (1)
#define ID_O  (2)
#define ID_Gyro  (3)

#define ID_P  (7)
#define ID_L  (4)
*/

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/* the CM3602 is a binary proximity sensor that triggers around 9 cm on
 * this hardware */

/*****************************************************************************/

#define CM_DEVICE_NAME      "/dev/hwmsensor"
#define LS_DEVICE_NAME      "/dev/hwmsensor"


#define EVENT_TYPE_ACCEL_X          			ABS_X
#define EVENT_TYPE_ACCEL_Y          			ABS_Y
#define EVENT_TYPE_ACCEL_Z          			ABS_Z
#define EVENT_TYPE_ACCEL_STATUS     		ABS_WHEEL

#define EVENT_TYPE_GYRO_X          				ABS_X
#define EVENT_TYPE_GYRO_Y          				ABS_Y
#define EVENT_TYPE_GYRO_Z          				ABS_Z
#define EVENT_TYPE_GYRO_STATUS     			ABS_WHEEL

#define EVENT_TYPE_ORIENT_X      				ABS_RX
#define EVENT_TYPE_ORIENT_Y          			ABS_RY
#define EVENT_TYPE_ORIENT_Z             			ABS_RZ
#define EVENT_TYPE_ORIENT_STATUS    		ABS_THROTTLE

#define EVENT_TYPE_MAG_X           				ABS_X
#define EVENT_TYPE_MAG_Y           				ABS_Y
#define EVENT_TYPE_MAG_Z           				ABS_Z
#define EVENT_TYPE_MAG_STATUS    			ABS_WHEEL

#define EVENT_TYPE_TEMPERATURE_VALUE     ABS_THROTTLE
#define EVENT_TYPE_TEMPERATURE_STATUS  	ABS_X

#define EVENT_TYPE_BARO_VALUE          			ABS_X
#define EVENT_TYPE_BARO_STATUS     				ABS_WHEEL

#define EVENT_TYPE_STEP_COUNTER_VALUE          	ABS_X
#define EVENT_TYPE_STEP_DETECTOR_VALUE          REL_Y
#define EVENT_TYPE_SIGNIFICANT_VALUE            REL_Z

#define EVENT_TYPE_INPK_VALUE            REL_X
#define EVENT_TYPE_SHK_VALUE             REL_X
#define EVENT_TYPE_FDN_VALUE             REL_X
#define EVENT_TYPE_PKUP_VALUE            REL_X


#define EVENT_TYPE_PDR_LENGTH    		ABS_X
#define EVENT_TYPE_PDR_FREQUENCY		   ABS_Y
#define EVENT_TYPE_PDR_COUNT    		   ABS_Z
#define EVENT_TYPE_PDR_DISTANCE 		   ABS_RX


#define EVENT_TYPE_ACT_IN_VEHICLE 		ABS_X
#define EVENT_TYPE_ACT_ON_BICYCLE 		ABS_Y
#define EVENT_TYPE_ACT_ON_FOOT 			ABS_Z
#define EVENT_TYPE_ACT_STILL 			   ABS_RX
#define EVENT_TYPE_ACT_UNKNOWN 			ABS_RY
#define EVENT_TYPE_ACT_TILT 			   ABS_RZ


#define EVENT_TYPE_ALS_VALUE         		ABS_X
#define EVENT_TYPE_PS_VALUE         		REL_Z
#define EVENT_TYPE_ALS_STATUS     		ABS_WHEEL
#define EVENT_TYPE_PS_STATUS        		REL_Y

#define EVENT_TYPE_BATCH_X          			ABS_X
#define EVENT_TYPE_BATCH_Y          			ABS_Y
#define EVENT_TYPE_BATCH_Z          			ABS_Z
#define EVENT_TYPE_BATCH_STATUS     		ABS_WHEEL
#define EVENT_TYPE_SENSORTYPE				REL_RZ
#define EVENT_TYPE_BATCH_VALUE          		ABS_RX
#define EVENT_TYPE_END_FLAG         			REL_RY
#define EVENT_TYPE_TIMESTAMP_HI    			REL_HWHEEL
#define EVENT_TYPE_TIMESTAMP_LO    			REL_DIAL


#define EVENT_TYPE_STEP_COUNT       			ABS_GAS


// 720 LSG = 1G
#define LSG                         (720.0f)


// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A                   (GRAVITY_EARTH / LSG)
#define CONVERT_A_X                 (-CONVERT_A)
#define CONVERT_A_Y                 (CONVERT_A)
#define CONVERT_A_Z                 (-CONVERT_A)

// conversion of magnetic data to uT units
#define CONVERT_M                   (1.0f/16.0f)
#define CONVERT_M_X                 (-CONVERT_M)
#define CONVERT_M_Y                 (-CONVERT_M)
#define CONVERT_M_Z                 (CONVERT_M)

#define CONVERT_O                   (1.0f)
#define CONVERT_O_Y                 (CONVERT_O)
#define CONVERT_O_P                 (CONVERT_O)
#define CONVERT_O_R                 (-CONVERT_O)

#define SENSOR_STATE_MASK           (0x7FFF)

/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_H
