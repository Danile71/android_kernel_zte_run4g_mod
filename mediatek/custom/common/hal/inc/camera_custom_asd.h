#ifndef _ASD_CONFIG_H
#define _ASD_CONFIG_H

typedef struct
{
    unsigned char u1TimeWeightType;
    unsigned char u1TimeWeightRange;    
    unsigned char u1ScoreThrNight;
    unsigned char u1ScoreThrBacklit;
    unsigned char u1ScoreThrPortrait;
    unsigned char u1ScoreThrLandscape;
}ASD_Customize_PARA1;

typedef struct
{
	short int s2IdxWeightBlAe;
    short int s2IdxWeightBlScd;    
	short int s2IdxWeightLsAe;        
  	short int s2IdxWeightLsAwb;
    short int s2IdxWeightLsAf;    
    short int s2IdxWeightLsScd;
    short int s2EvLoThrNight;
    short int s2EvHiThrNight;
    short int s2EvLoThrOutdoor;
    short int s2EvHiThrOutdoor;
    bool boolBacklitLockEnable;
    short int s2BacklitLockEvDiff;  
}ASD_Customize_PARA2;


void get_asd_CustomizeData1(ASD_Customize_PARA1  *ASDDataOut1);
void get_asd_CustomizeData2(ASD_Customize_PARA2  *ASDDataOut2);
	
#endif /* _FD_CONFIG_H */

