#ifndef _CAMERA_CUSTOM_AEPTABLE_H_
#define _CAMERA_CUSTOM_AEPTABLE_H_

#define MAX_PLINE_TABLE 50
#define MAX_PLINE_MAP_TABLE 30

// AE mode definition ==> need to change the AE Pline table
typedef enum                           
{
    LIB3A_AE_SCENE_UNSUPPORTED           = -1,
    LIB3A_AE_SCENE_OFF                            =  0,            // disable AE
    LIB3A_AE_SCENE_AUTO                          =  1,           // auto mode   full auto ,EV ISO LCE .. is inactive
    LIB3A_AE_SCENE_NIGHT                         =  2,           // preview Night Scene mode
    LIB3A_AE_SCENE_ACTION                       =  3,           // AE Action mode
    LIB3A_AE_SCENE_BEACH                         =  4,           // AE beach mode
    LIB3A_AE_SCENE_CANDLELIGHT             =  5,           // AE Candlelight mode
    LIB3A_AE_SCENE_FIREWORKS                 = 6,           // AE firework mode
    LIB3A_AE_SCENE_LANDSCAPE                 = 7,           // AE landscape mode
    LIB3A_AE_SCENE_PORTRAIT                   = 8,            // AE portrait mode
    LIB3A_AE_SCENE_NIGHT_PORTRAIT       = 9,           // AE night portrait mode
    LIB3A_AE_SCENE_PARTY                          = 10,           // AE party mode
    LIB3A_AE_SCENE_SNOW                           = 11,           // AE snow mode
    LIB3A_AE_SCENE_SPORTS                        = 12,           // AE sport mode
    LIB3A_AE_SCENE_STEADYPHOTO             = 13,           // AE steadyphoto mode
    LIB3A_AE_SCENE_SUNSET                        = 14,           // AE sunset mode
    LIB3A_AE_SCENE_THEATRE                      = 15,           // AE theatre mode
    LIB3A_AE_SCENE_ISO_ANTI_SHAKE        = 16,           // AE ISO anti shake mode
    LIB3A_AE_SCENE_BACKLIGHT                 = 17,           // ADD BACKLIGHT MODE
    LIB3A_AE_SCENE_ISO100           = 100,
    LIB3A_AE_SCENE_ISO200           = 101,
    LIB3A_AE_SCENE_ISO400           = 102,
    LIB3A_AE_SCENE_ISO800           = 103,
    LIB3A_AE_SCENE_ISO1600         = 104,
} LIB3A_AE_SCENE_T;

// AE ISO speed
typedef enum
{
    LIB3A_AE_ISO_SPEED_UNSUPPORTED =     -1,
    LIB3A_AE_ISO_SPEED_AUTO                =      0,
    LIB3A_AE_ISO_SPEED_50                     =    50,
    LIB3A_AE_ISO_SPEED_100                   =    100,
    LIB3A_AE_ISO_SPEED_150                   =    150,    
    LIB3A_AE_ISO_SPEED_200                   =    200,
    LIB3A_AE_ISO_SPEED_300                   =    300,    
    LIB3A_AE_ISO_SPEED_400                   =    400,
    LIB3A_AE_ISO_SPEED_600                   =    600,    
    LIB3A_AE_ISO_SPEED_800                   =    800,
    LIB3A_AE_ISO_SPEED_1200                 =   1200,     
    LIB3A_AE_ISO_SPEED_1600                 =   1600, 
    LIB3A_AE_ISO_SPEED_2400                 =   2400, 
    LIB3A_AE_ISO_SPEED_3200                 =   3200,
    LIB3A_AE_ISO_SPEED_MAX = LIB3A_AE_ISO_SPEED_3200
}LIB3A_AE_ISO_SPEED_T;

//AE Parameter Structure
typedef enum
{
    AETABLE_RPEVIEW_AUTO = 0,     // default 60Hz
    AETABLE_CAPTURE_AUTO,
    AETABLE_VIDEO_AUTO,
    AETABLE_VIDEO1_AUTO,
    AETABLE_VIDEO2_AUTO,
    AETABLE_CUSTOM1_AUTO, 
    AETABLE_CUSTOM2_AUTO,  
    AETABLE_CUSTOM3_AUTO, 
    AETABLE_CUSTOM4_AUTO, 
    AETABLE_CUSTOM5_AUTO,    
    AETABLE_VIDEO_NIGHT,    
    AETABLE_CAPTURE_ISO50,
    AETABLE_CAPTURE_ISO100,
    AETABLE_CAPTURE_ISO200,
    AETABLE_CAPTURE_ISO400,
    AETABLE_CAPTURE_ISO800,
    AETABLE_CAPTURE_ISO1600,
    AETABLE_CAPTURE_ISO3200,
    AETABLE_STROBE, 
    AETABLE_SCENE_INDEX1,                           // for mode used of capture
    AETABLE_SCENE_INDEX2,
    AETABLE_SCENE_INDEX3,
    AETABLE_SCENE_INDEX4,
    AETABLE_SCENE_INDEX5,
    AETABLE_SCENE_INDEX6,
    AETABLE_SCENE_INDEX7,
    AETABLE_SCENE_INDEX8,
    AETABLE_SCENE_INDEX9,
    AETABLE_SCENE_INDEX10,
    AETABLE_SCENE_INDEX11,
    AETABLE_SCENE_INDEX12,
    AETABLE_SCENE_INDEX13,
    AETABLE_SCENE_INDEX14,
    AETABLE_SCENE_INDEX15,
    AETABLE_SCENE_INDEX16,
    AETABLE_SCENE_INDEX17,
    AETABLE_SCENE_INDEX18,
    AETABLE_SCENE_INDEX19,
    AETABLE_SCENE_INDEX20,
    AETABLE_SCENE_INDEX21,
    AETABLE_SCENE_INDEX22,
    AETABLE_SCENE_INDEX23,
    AETABLE_SCENE_INDEX24,
    AETABLE_SCENE_INDEX25,
    AETABLE_SCENE_INDEX26,
    AETABLE_SCENE_INDEX27,
    AETABLE_SCENE_INDEX28,
    AETABLE_SCENE_INDEX29,
    AETABLE_SCENE_INDEX30,
    AETABLE_SCENE_INDEX31,
    AETABLE_SCENE_MAX    
}eAETableID;

typedef struct	strEvSetting
{
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4AfeGain;           //!<: raw gain
    MUINT32 u4IspGain;           //!<: sensor gain
    MUINT8  uIris;                    //!<: Iris
    MUINT8  uSensorMode;      //!<: sensor mode
    MUINT8  uFlag;                   //!<: flag to indicate hysteresis ...
//    MUINT8  uLV;                        //!<: LV avlue , in ISO 100 condition  LV=TV+AV
}strEvSetting;

typedef struct
{
   strEvSetting sPlineTable[200];   // Pline structure table
} strEvPline;

typedef struct
{
   eAETableID   eID;
   MUINT32       u4TotalIndex;      //preview table Tatal index
   MINT32        i4StrobeTrigerBV;  // Strobe triger point in strobe auto mode
   MINT32        i4MaxBV;
   MINT32        i4MinBV;
   MINT32        i4EffectiveMaxBV;
   MINT32        i4EffectiveMinBV;
   LIB3A_AE_ISO_SPEED_T   ISOSpeed;
   strEvPline sTable60Hz;   // 50Hz structure table
   strEvPline sTable50Hz;   // 60Hz structure table
   strEvPline *pCurrentTable;      // current structure table
}strAETable;

typedef struct
{
    LIB3A_AE_SCENE_T eAEScene;
    eAETableID ePLineID[11];
} strAEPLineMapping;

typedef struct
{
    strAETable    sPlineTable[MAX_PLINE_TABLE];    //AE Pline Table
}strAEPLineTable;

typedef struct
{
    MUINT16 u2Index;
    MUINT16 u2MaxFrameRate;
    MUINT16 u2MinFrameRate;
    MUINT16 u2MinGain_x1024;
    MUINT16 u2MaxGain_x1024;
}strAEPLineTableIndex;

typedef struct
{
    eAETableID ePLineID;
    MINT32       i4EffectiveMaxBV;
    MINT32       i4EffectiveMinBV;    
    strAEPLineTableIndex    PLineIndex[15];    //AE Pline Table
}strAEPLineNumInfo;

typedef struct
{
    MINT32 i4IsUpdate;
    strAEPLineNumInfo    PLineNumInfo[MAX_PLINE_TABLE];    //AE Pline Table
}strAEPLineInfomation;

typedef struct
{
    MUINT16 u2TotalNum;
    MUINT16 u2SensorGainStep[255][2];    //AE Pline Table
}strAEPLineGainList;

typedef struct
{
    strAEPLineMapping    sAESceneMapping[MAX_PLINE_MAP_TABLE];   // Get PLine ID for different AE mode
}strAESceneMapping;

typedef struct
{
    strAESceneMapping    sAEScenePLineMapping;   // Get PLine ID for different AE mode
    strAEPLineTable AEPlineTable;
    strAEPLineInfomation AEPlineInfo;
    strAEPLineGainList AEGainList;
} AE_PLINETABLE_T, *PAE_PLINETABLE_STRUCT; 

// for AE algorithm used.
typedef struct
{
    LIB3A_AE_SCENE_T eAEScene;
    eAETableID ePLineID[11];
} AE_PLINEMAPPINGTABLE_T, *PAE_PLINEMAPPINGTABLE; 

#endif // _CAMERA_CUSTOM_AEPTABLE_H_

