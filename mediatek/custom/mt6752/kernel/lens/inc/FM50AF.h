#ifndef _FM50AF_H
#define _FM50AF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define FM50AF_MAGIC 'A'
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
} stFM50AF_MotorInfo;

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
}stRUMBAAF_MotorMETAInfo;
#endif


//Structures
typedef struct {
//if have ois hw, disable bit
bool bOIS_disable;
//if have ois hw, gain setting
unsigned long u4ois_gain;
//if have ois hw, freq setting
unsigned long u4ois_freq;
//if have ois hw, other setting1
unsigned long u4ois_setting1;
//if have ois hw, other setting2
unsigned long u4ois_setting2;
} stFM50AF_MotorPara;


//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define FM50AFIOC_G_MOTORINFO _IOR(FM50AF_MAGIC,0,stFM50AF_MotorInfo)

#define FM50AFIOC_T_MOVETO _IOW(FM50AF_MAGIC,1,unsigned long)

#define FM50AFIOC_T_SETINFPOS _IOW(FM50AF_MAGIC,2,unsigned long)

#define FM50AFIOC_T_SETMACROPOS _IOW(FM50AF_MAGIC,3,unsigned long)

#ifdef LensdrvCM3
#define FM50AFIOC_G_MOTORMETAINFO _IOR(FM50AF_MAGIC,4,stFM50AF_MotorMETAInfo)
#endif

#define FM50AFIOC_T_SETPARA _IOW(FM50AF_MAGIC,5,unsigned long)

#else
#endif
