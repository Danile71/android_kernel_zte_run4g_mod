#ifndef _AF_PARAM_H
#define _AF_PARAM_H

#define AF_WIN_NUM_SPOT  1
#define MAX_AF_HW_WIN   37

#define FD_WIN_NUM     15

#define PATH_LENGTH    100

#define AFEXTENDCOEF

typedef struct
{
    MINT32 i4X;
    MINT32 i4Y;
    MINT32 i4W;
    MINT32 i4H;
    MINT32 i4Info;
    
} AREA_T;

typedef struct 
{
    MINT32  i4Count;
    MINT32  i4Score;
    AREA_T  sRect[AF_WIN_NUM_SPOT];    
} AF_AREA_T;

typedef struct
{
    MINT32 i4SGG_GAIN;
    MINT32 i4SGG_GMR1;
    MINT32 i4SGG_GMR2;
    MINT32 i4SGG_GMR3;
    MINT32 i4SGG_GMR4;
    MINT32 i4SGG_GMR5;
    MINT32 i4SGG_GMR6;
    MINT32 i4SGG_GMR7;
    MINT32 AF_DECI_1;
    MINT32 AF_ZIGZAG;
    MINT32 AF_ODD;
    MINT32 AF_FILT1[12];
    MINT32 AF_FILT2[4];
    MINT32 AF_TH[2];
    MINT32 AF_THEX;

} AF_CONFIG_T;


#define ISO_MAX_NUM     (8)
typedef struct
{
    MINT32 i4ISONum;
    MINT32 i4ISO[ISO_MAX_NUM];
    MINT32 i4GMR[7][ISO_MAX_NUM];
    MINT32 i4FV_DC[ISO_MAX_NUM];
    MINT32 i4MIN_TH[ISO_MAX_NUM];
    MINT32 i4HW_TH[ISO_MAX_NUM];
    MINT32 i4FV_DC2[ISO_MAX_NUM];
    MINT32 i4MIN_TH2[ISO_MAX_NUM];
    MINT32 i4HW_TH2[ISO_MAX_NUM];
} CustAF_THRES_T;

typedef struct
{
    MINT32 i4ImageWidth;
    MINT32 i4ImageHeight;
    MINT32 i4SensorID;
    CustAF_THRES_T Coef;
}AF_Extend_Coef_T;

typedef struct
{
    MINT32 AF_WINX[6];
    MINT32 AF_WINY[6];
    MINT32 AF_XSIZE;
    MINT32 AF_YSIZE;
    MINT32 AF_WINXE;
    MINT32 AF_WINYE;
    MINT32 AF_SIZE_XE;
    MINT32 AF_SIZE_YE;

} AF_WIN_CONFIG_T;


typedef struct
{
    MUINT32 u4Stat24;
    MUINT32 u4StatV;

} AF_HW_STAT_SINGLE_T;

typedef struct
{
    AF_HW_STAT_SINGLE_T sStat[MAX_AF_HW_WIN];

} AF_HW_STAT_T;

typedef struct
{
    MINT64 i8Stat24;
    MINT64 i8StatFL;
    MINT64 i8StatV;

} AF_STAT_T;

typedef struct
{
    MINT64 i8StatH[MAX_AF_HW_WIN-1];
    MINT64 i8StatV[MAX_AF_HW_WIN-1];
    MBOOL  bValid;
} AF_FULL_STAT_T;

typedef struct
{
    MINT32 i4CurrentPos;        //current position
    MINT32 i4MacroPos;          //macro position
    MINT32 i4InfPos;            //Infiniti position
    MINT32 bIsMotorMoving;      //Motor Status
    MINT32 bIsMotorOpen;        //Motor Open?
    MINT32 bIsSupportSR;

} LENS_INFO_T;

// AF v2.0
typedef struct
{
    MINT32      i4IsZSD;
    MINT32      i4IsVDO;
    MINT32      i4IsIHDR;
    MINT32      i4IsRevMode1;
    MINT32      i4IsRevMode2;    
    MINT32      i4IsRevMode3;
    MINT32      i4IsRevMode4;    
    MINT32      i4IsRevMode5;  
    MINT32      i4IsVDO1;
    MINT32      i4IsVDO2;
    MINT32      i4IsAEStable;
    MINT64      i8GSum;
    MINT32      i4ISO; 
    MINT32      i4SceneLV;
    MINT32      i4ShutterValue;
    MINT32      i4FullScanStep;
    AF_AREA_T   sAFArea;
    AF_STAT_T   sAFStat;
    MINT32      i4IsFlashFrm;           // PL detect    
    MINT32      i4AEBlockAreaYCnt;      // PL detect    
    MUINT8      *pAEBlockAreaYvalue;    // PL detect        
    LENS_INFO_T sLensInfo;
    AREA_T      sEZoom;

} AF_INPUT_T;

typedef struct
{
    MINT32      i4IsAFDone;
    MINT32      i4IsFocused;
    MINT32      i4IsMonitorFV;
    MINT32      i4AFBestPos;
    MINT32      i4AFPos;    
    MINT32      i4FDDetect;  
    MINT64      i8AFValue;
    AF_CONFIG_T sAFStatConfig;
    AF_AREA_T   sAFArea;

} AF_OUTPUT_T;

typedef struct
{
    MINT32  i4AFS_MODE;    //0 : singleAF, 1:smoothAF
    MINT32  i4AFC_MODE;    //0 : singleAF, 1:smoothAF    
    MINT32  i4VAFC_MODE;   //0 : singleAF, 1:smoothAF    
} AF_PARAM_T;

typedef struct
{
    MINT32  i4Num;
    MINT32  i4Pos[PATH_LENGTH];

} AF_STEP_T;

typedef enum
{
    AF_MARK_NORMAL = 0,
    AF_MARK_OK,
    AF_MARK_FAIL,
    AF_MARK_NONE

} AF_MARK_T;

// AF info for ISP tuning
typedef struct
{
    MINT32 i4AFPos; // AF position
    
} AF_INFO_T;

#endif

