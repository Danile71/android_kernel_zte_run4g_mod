#ifndef _AK7348AF_H
#define _AK7348AF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define AK7348AF_MAGIC 'A'
//IOCTRL(inode * ,file * ,cmd ,arg )


//Structures
typedef struct {
//current position
unsigned long u4CurrentPosition;
//macro position
unsigned long u4MacroPosition;
//Infiniti position
unsigned long u4InfPosition;
//Motor Status
bool          bIsMotorMoving;
//Motor Open?
bool          bIsMotorOpen;
//Support SR?
bool          bIsSupportSR;
} stAK7348AF_MotorInfo;


//Structures
//#define LensdrvCM3
#ifdef LensdrvCM3
typedef struct {
	//APERTURE , won't be supported on most devices.
	float Aperture;
	//FILTER_DENSITY, won't be supported on most devices.
	float FilterDensity;
	//FOCAL_LENGTH, lens optical zoom setting. won't be supported on most devices.
	float FocalLength;
	//FOCAL_DISTANCE, current focus distance, lens to objects.
	float FocalDistance;
	//OPTICAL_STABILIZATION_MODE 
	u16 u4OIS_Mode;
	//FACING
	u16 Facing;
	//Optical axis angle,  optical axis is perpendicular to LCM, usually is  {0,0}.
	float OpticalAxisAng[2];
	//Optical axis position (mm),  usually is  {0,0,0}.
	float Position[3];
	//Focus range, DOF, 
	float FocusRange;
	//State
	u16 State;
	//INFO
	float InfoAvalibleMinFocusDistance;
	float InfoAvalibleApertures;
	float InfoAvalibleFilterDensity;
	u16 InfoAvalibleOptStabilization;
	float InfoAvalibleFocalLength;
	float InfoAvalibleHypeDistance;
}stAK7348AF_MotorMETAInfo;
#endif

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"             
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define AK7348AFIOC_G_MOTORINFO _IOR(AK7348AF_MAGIC,0,stAK7348AF_MotorInfo)

#define AK7348AFIOC_T_MOVETO _IOW(AK7348AF_MAGIC,1,unsigned long)

#define AK7348AFIOC_T_SETINFPOS _IOW(AK7348AF_MAGIC,2,unsigned long)

#define AK7348AFIOC_T_SETMACROPOS _IOW(AK7348AF_MAGIC,3,unsigned long)
#ifdef LensdrvCM3
#define AK7348AFIOC_G_MOTORMETAINFO _IOR(AK7348AF_MAGIC,4,stAK7348AF_MotorMETAInfo)
#endif

#else
#endif
