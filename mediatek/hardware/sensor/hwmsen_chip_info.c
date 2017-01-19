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
#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "hwmsen_chip_info.h"
#include "hwmsen_custom.h"


/*--------------------------------------------------------*/
#ifdef CUSTOM_KERNEL_ACCELEROMETER
	#ifndef ACCELEROMETER
		#define ACCELEROMETER 			"ACCELEROMETER"
		#define ACCELEROMETER_VENDER 		"MTK"
	#endif
	#ifndef ACCELEROMETER_RANGE
		#define ACCELEROMETER_RANGE		32.0f
	#endif
	#ifndef ACCELEROMETER_RESOLUTION
		#define ACCELEROMETER_RESOLUTION	4.0f/1024.0f
	#endif
	#ifndef ACCELEROMETER_POWER
		#define ACCELEROMETER_POWER		130.0f/1000.0f
	#endif
#endif

#ifdef CUSTOM_KERNEL_ALSPS
	#ifndef PROXIMITY
		#define PROXIMITY 			"PROXIMITY"
		#define PROXIMITY_VENDER 		"MTK"
	#endif
	#ifndef PROXIMITY_RANGE
		#define PROXIMITY_RANGE 		1.00f
	#endif
	#ifndef PROXIMITY_RESOLUTION
		#define PROXIMITY_RESOLUTION  		1.0f	
	#endif
	#ifndef PROXIMITY_POWER			
		#define PROXIMITY_POWER			0.13f
	#endif

	#ifndef LIGHT
		#define LIGHT 				"LIGHT"
		#define LIGHT_VENDER 			"MTK"
	#endif
	#ifndef LIGHT_RANGE
		#define LIGHT_RANGE			10240.0f
	#endif
	#ifndef LIGHT_RESOLUTION
		#define LIGHT_RESOLUTION 		1.0f	
	#endif
	#ifndef LIGHT_POWER
		#define LIGHT_POWER			0.13f		
	#endif
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
	#ifndef MAGNETOMETER
		#define MAGNETOMETER 			"MAGNETOMETER"
		#define MAGNETOMETER_VENDER 		"MTK"
	#endif
	#ifndef MAGNETOMETER_RANGE
		#define MAGNETOMETER_RANGE 		600.0f
	#endif
	#ifndef MAGNETOMETER_RESOLUTION
		#define MAGNETOMETER_RESOLUTION		0.0016667f
	#endif
	#ifndef MAGNETOMETER_POWER
		#define MAGNETOMETER_POWER		0.25f
	#endif

	#ifndef ORIENTATION
		#define ORIENTATION 			"ORIENTATION"
		#define ORIENTATION_VENDER 		"MTK"
	#endif
	#ifndef ORIENTATION_RANGE
		#define ORIENTATION_RANGE		360.0f
	#endif
	#ifndef ORIENTATION_RESOLUTION
		#define ORIENTATION_RESOLUTION		1.0f
	#endif
	#ifndef ORIENTATION_POWER
		#define ORIENTATION_POWER		0.25f
	#endif
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
	#ifndef GYROSCOPE
		#define GYROSCOPE 			"GYROSCOPE"
		#define GYROSCOPE_VENDER 		"MTK"
	#endif
	#ifndef GYROSCOPE_RANGE
		#define GYROSCOPE_RANGE			34.91f
	#endif
	#ifndef GYROSCOPE_RESOLUTION
		#define GYROSCOPE_RESOLUTION		0.0107f
	#endif
	#ifndef GYROSCOPE_POWER
		#define GYROSCOPE_POWER			6.1f
	#endif
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
	#ifndef PRESSURE
		#define PRESSURE 			"PRESSURE"
		#define PRESSURE_VENDER			"MTK"
	#endif
	#ifndef PRESSURE_RANGE
		#define PRESSURE_RANGE 			1100.0f
	#endif
	#ifndef PRESSURE_RESOLUTION
		#define PRESSURE_RESOLUTION 		100.0f
	#endif
	#ifndef PRESSURE_POWER
		#define PRESSURE_POWER			0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_TEMPURATURE
	#ifndef TEMPURATURE
		#define TEMPURATURE 			"TEMPURATURE"
		#define TEMPURATURE_VENDER		"MTK"
	#endif
	#ifndef TEMPURATURE_RANGE
		#define TEMPURATURE_RANGE 		85.0f
	#endif
	#ifndef TEMPURATURE_RESOLUTION
		#define TEMPURATURE_RESOLUTION 		0.1f
	#endif
	#ifndef TEMPURATURE_POWER
		#define TEMPURATURE_POWER 		0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_STEP_COUNTER
	#ifndef STEP_COUNTER
		#define STEP_COUNTER 			"STEP_COUNTER"
		#define STEP_COUNTER_VENDER		"MTK"

		#define STEP_DETECTOR 			"STEP_DETECTOR"
		#define STEP_DETECTOR_VENDER	"MTK"
	#endif
	#ifndef STEP_COUNTER_RANGE
		#define STEP_COUNTER_RANGE 				85.0f
		#define STEP_DETECTOR_RANGE 			85.0f
	#endif
	#ifndef STEP_COUNTER_RESOLUTION
		#define STEP_COUNTER_RESOLUTION 		0.1f
		#define STEP_DETECTOR_RESOLUTION 		0.1f
	#endif
	#ifndef STEP_COUNTER_POWER
		#define STEP_COUNTER_POWER 				0.5f
		#define STEP_DETECTOR_POWER 			0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR
	#ifndef SIGNIFICANT_MOTION
		#define SIGNIFICANT_MOTION 			"SIGNIFICANT_MOTION"
		#define SIGNIFICANT_MOTION_VENDER	"MTK"
	#endif
	#ifndef SIGNIFICANT_MOTION_RANGE
		#define SIGNIFICANT_MOTION_RANGE 		85.0f
	#endif
	#ifndef SIGNIFICANT_MOTION_RESOLUTION
		#define SIGNIFICANT_MOTION_RESOLUTION 	0.1f
	#endif
	#ifndef SIGNIFICANT_MOTION_POWER
		#define SIGNIFICANT_MOTION_POWER 		0.5f
	#endif
#endif



#ifdef CUSTOM_KERNEL_IN_POCKET_SENSOR
	#ifndef IN_POCKET
		#define IN_POCKET 		"IN_POCKET"
		#define IN_POCKET_VENDER	"MTK"
	#endif
	#ifndef IN_POCKET_RANGE
		#define IN_POCKET_RANGE 	85.0f
	#endif
	#ifndef IN_POCKET_RESOLUTION
		#define IN_POCKET_RESOLUTION 	0.1f
	#endif
	#ifndef IN_POCKET_POWER
		#define IN_POCKET_POWER 	0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_PEDOMETER
	#ifndef PEDOMETER
		#define PEDOMETER 		"PEDOMETER"
		#define PEDOMETER_VENDER	"MTK"
	#endif
	#ifndef PEDOMETER_RANGE
		#define PEDOMETER_RANGE 	85.0f
	#endif
	#ifndef PEDOMETER_RESOLUTION
		#define PEDOMETER_RESOLUTION 	0.1f
	#endif
	#ifndef PEDOMETER_POWER
		#define PEDOMETER_POWER 	0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_ACTIVITY_SENSOR
	#ifndef ACTIVITY
		#define ACTIVITY 		"ACTIVITY"
		#define ACTIVITY_VENDER	"MTK"
	#endif
	#ifndef ACTIVITY_RANGE
		#define ACTIVITY_RANGE 	85.0f
	#endif
	#ifndef ACTIVITY_RESOLUTION
		#define ACTIVITY_RESOLUTION 	0.1f
	#endif
	#ifndef ACTIVITY_POWER
		#define ACTIVITY_POWER 	0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_SHAKE_SENSOR
	#ifndef SHAKE
		#define SHAKE 		"SHAKE"
		#define SHAKE_VENDER	"MTK"
	#endif
	#ifndef SHAKE_RANGE
		#define SHAKE_RANGE 	85.0f
	#endif
	#ifndef SHAKE_RESOLUTION
		#define SHAKE_RESOLUTION 	0.1f
	#endif
	#ifndef SHAKE_POWER
		#define SHAKE_POWER 	0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_PICK_UP_SENSOR
	#ifndef PICK_UP
		#define PICK_UP 		"PICK_UP"
		#define PICK_UP_VENDER	"MTK"
	#endif
	#ifndef PICK_UP_RANGE
		#define PICK_UP_RANGE 	85.0f
	#endif
	#ifndef PICK_UP_RESOLUTION
		#define PICK_UP_RESOLUTION 	0.1f
	#endif
	#ifndef PICK_UP_POWER
		#define PICK_UP_POWER 	0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_FACE_DOWN_SENSOR
	#ifndef FACE_DOWN
		#define FACE_DOWN 		"FACE_DOWN"
		#define FACE_DOWN_VENDER	"MTK"
	#endif
	#ifndef FACE_DOWN_RANGE
		#define FACE_DOWN_RANGE 	85.0f
	#endif
	#ifndef FACE_DOWN_RESOLUTION
		#define FACE_DOWN_RESOLUTION 	0.1f
	#endif
	#ifndef FACE_DOWN_POWER
		#define FACE_DOWN_POWER 	0.5f
	#endif
#endif


/*--------------------------------------------------------*/

struct sensor_t sSensorList[MAX_NUM_SENSOR] = 
{
#ifdef CUSTOM_KERNEL_ACCELEROMETER
	{  
		.name       = ACCELEROMETER,
		.vendor     = ACCELEROMETER_VENDER,
		.version    = 1,
		.handle     = ID_ACCELEROMETER+ID_OFFSET,
		.type       = SENSOR_TYPE_ACCELEROMETER,
		.maxRange   = ACCELEROMETER_RANGE,//32.0f,
		.resolution = ACCELEROMETER_RESOLUTION,//4.0f/1024.0f,
		.power      = ACCELEROMETER_POWER,//130.0f/1000.0f,
		.reserved   = {}
	},	
#endif

#ifdef CUSTOM_KERNEL_ALSPS
	{ 
		.name       = PROXIMITY,
		.vendor     = PROXIMITY_VENDER,
		.version    = 1,
		.handle     = ID_PROXIMITY+ID_OFFSET,
		.type       = SENSOR_TYPE_PROXIMITY,
		.maxRange   = PROXIMITY_RANGE,//1.00f,
		.resolution = PROXIMITY_RESOLUTION,//1.0f,
		.power      = PROXIMITY_POWER,//0.13f,
		.reserved   = {}
	},

	{ 
		.name       = LIGHT,
		.vendor     = LIGHT_VENDER,
		.version    = 1,
		.handle     = ID_LIGHT+ID_OFFSET,
		.type       = SENSOR_TYPE_LIGHT,
		.maxRange   = LIGHT_RANGE,//10240.0f,
		.resolution = LIGHT_RESOLUTION,//1.0f,
		.power      = LIGHT_POWER,//0.13f,
		.reserved   = {}
	},
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
	{ 
		.name       = GYROSCOPE,
		.vendor     = GYROSCOPE_VENDER,
		.version    = 1,
		.handle     = ID_GYROSCOPE+ID_OFFSET,
		.type       = SENSOR_TYPE_GYROSCOPE,
		.maxRange   = GYROSCOPE_RANGE,//34.91f,
		.resolution = GYROSCOPE_RESOLUTION,//0.0107f,
		.power      = GYROSCOPE_POWER,//6.1f,
		.reserved   = {}
	},
#endif	

#ifdef CUSTOM_KERNEL_MAGNETOMETER
	{ 
		.name       = ORIENTATION,
		.vendor     = ORIENTATION_VENDER,
		.version    = 1,
		.handle     = ID_ORIENTATION+ID_OFFSET,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = ORIENTATION_RANGE,//360.0f,
		.resolution = ORIENTATION_RESOLUTION,//1.0f,
		.power      = ORIENTATION_POWER,//0.25f,
		.reserved   = {}
	},

	{ 
		.name       = MAGNETOMETER,
		.vendor     = MAGNETOMETER_VENDER,
		.version    = 1,
		.handle     = ID_MAGNETIC+ID_OFFSET,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = MAGNETOMETER_RANGE,//600.0f,
		.resolution = MAGNETOMETER_RESOLUTION,//0.0016667f,
		.power      = MAGNETOMETER_POWER,//0.25f,
		.reserved   = {}
	}, 
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
	{ 
		.name       = PRESSURE,
		.vendor     = PRESSURE_VENDER,
		.version    = 1,
		.handle     = ID_PRESSURE+ID_OFFSET,
		.type       = SENSOR_TYPE_PRESSURE,
		.maxRange   = PRESSURE_RANGE,//360.0f,
		.resolution = PRESSURE_RESOLUTION,//1.0f,
		.power      = PRESSURE_POWER,//0.25f,
		.reserved   = {}
	},
#endif
	
#ifdef CUSTOM_KERNEL_TEMPURATURE
	{ 
		.name       = TEMPURATURE,
		.vendor     = TEMPURATURE_VENDER,
		.version    = 1,
		.handle     = ID_TEMPRERATURE+ID_OFFSET,
		.type       = SENSOR_TYPE_TEMPERATURE,
		.maxRange   = TEMPURATURE_RANGE,//600.0f,
		.resolution = TEMPURATURE_RESOLUTION,//0.0016667f,
		.power      = TEMPURATURE_POWER,//0.25f,
		.reserved   = {}
	}, 
#endif

#ifdef CUSTOM_KERNEL_STEP_COUNTER
	{ 
		.name       = STEP_COUNTER,
		.vendor     = STEP_COUNTER_VENDER,
		.version    = 1,
		.handle     = ID_STEP_COUNTER+ID_OFFSET,
		.type       = SENSOR_TYPE_STEP_COUNTER,
		.maxRange   = STEP_COUNTER_RANGE,//600.0f,
		.resolution = STEP_COUNTER_RESOLUTION,//0.0016667f,
		.power      = STEP_COUNTER_POWER,//0.25f,
		.reserved   = {}
	}, 
	{ 
		.name       = STEP_DETECTOR,
		.vendor     = STEP_DETECTOR_VENDER,
		.version    = 1,
		.handle     = ID_STEP_DETECTOR+ID_OFFSET,
		.type       = SENSOR_TYPE_STEP_DETECTOR,
		.maxRange   = STEP_DETECTOR_RANGE,//600.0f,
		.resolution = STEP_DETECTOR_RESOLUTION,//0.0016667f,
		.power      = STEP_DETECTOR_POWER,//0.25f,
		.reserved   = {}
	},
#endif

#ifdef CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR
	{ 
		.name		= SIGNIFICANT_MOTION,
		.vendor 	= SIGNIFICANT_MOTION_VENDER,
		.version	= 1,
		.handle 	= ID_SIGNIFICANT_MOTION+ID_OFFSET,
		.type		= SENSOR_TYPE_SIGNIFICANT_MOTION,
		.maxRange	= SIGNIFICANT_MOTION_RANGE,//600.0f,
		.resolution = SIGNIFICANT_MOTION_RESOLUTION,//0.0016667f,
		.power		= SIGNIFICANT_MOTION_POWER,//0.25f,
		.reserved	= {}
	},
#endif


#ifdef CUSTOM_KERNEL_IN_POCKET_SENSOR
	{ 
		.name		= IN_POCKET,
		.vendor 	= IN_POCKET_VENDER,
		.version	= 1,
		.handle 	= ID_IN_POCKET+ID_OFFSET,
		.type		= SENSOR_TYPE_IN_POCKET,
		.maxRange	= IN_POCKET_RANGE,//600.0f,
		.resolution	= IN_POCKET_RESOLUTION,//0.0016667f,
		.power		= IN_POCKET_POWER,//0.25f,
		.reserved	= {}
	},
#endif

#ifdef CUSTOM_KERNEL_PEDOMETER
	{ 
		.name		= PEDOMETER,
		.vendor 	= PEDOMETER_VENDER,
		.version	= 1,
		.handle 	= ID_PEDOMETER+ID_OFFSET,
		.type		= SENSOR_TYPE_PEDOMETER,
		.maxRange	= PEDOMETER_RANGE,//600.0f,
		.resolution	= PEDOMETER_RESOLUTION,//0.0016667f,
		.power		= PEDOMETER_POWER,//0.25f,
		.reserved	= {}
	},
#endif

#ifdef CUSTOM_KERNEL_ACTIVITY_SENSOR
	{ 
		.name		= ACTIVITY,
		.vendor 	= ACTIVITY_VENDER,
		.version	= 1,
		.handle 	= ID_ACTIVITY+ID_OFFSET,
		.type		= SENSOR_TYPE_ACTIVITY,
		.maxRange	= ACTIVITY_RANGE,//600.0f,
		.resolution	= ACTIVITY_RESOLUTION,//0.0016667f,
		.power		= ACTIVITY_POWER,//0.25f,
		.reserved	= {}
	},
#endif

#ifdef CUSTOM_KERNEL_SHAKE_SENSOR
	{ 
		.name		= SHAKE,
		.vendor 	= SHAKE_VENDER,
		.version	= 1,
		.handle 	= ID_SHAKE+ID_OFFSET,
		.type		= SENSOR_TYPE_SHAKE,
		.maxRange	= SHAKE_RANGE,//600.0f,
		.resolution	= SHAKE_RESOLUTION,//0.0016667f,
		.power		= SHAKE_POWER,//0.25f,
		.reserved	= {}
	},
#endif

#ifdef CUSTOM_KERNEL_PICK_UP_SENSOR
	{ 
		.name		= PICK_UP,
		.vendor 	= PICK_UP_VENDER,
		.version	= 1,
		.handle 	= ID_PICK_UP+ID_OFFSET,
		.type		= SENSOR_TYPE_PICK_UP,
		.maxRange	= PICK_UP_RANGE,//600.0f,
		.resolution	= PICK_UP_RESOLUTION,//0.0016667f,
		.power		= PICK_UP_POWER,//0.25f,
		.reserved	= {}
	},
#endif

#ifdef CUSTOM_KERNEL_FACE_DOWN_SENSOR
	{ 
		.name		= FACE_DOWN,
		.vendor 	= FACE_DOWN_VENDER,
		.version	= 1,
		.handle 	= ID_FACE_DOWN+ID_OFFSET,
		.type		= SENSOR_TYPE_FACE_DOWN,
		.maxRange	= FACE_DOWN_RANGE,//600.0f,
		.resolution	= FACE_DOWN_RESOLUTION,//0.0016667f,
		.power		= FACE_DOWN_POWER,//0.25f,
		.reserved	= {}
	},
#endif


};

