#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_lens.h"

const NVRAM_LENS_PARA_STRUCT SENSOR_DRIVE_LENS_PARA_DEFAULT_VALUE =
{
    //Version
    NVRAM_CAMERA_LENS_FILE_VERSION,

    // Focus Range NVRAM
    {0, 1023},

    // AF NVRAM
    {
        // ------------------------------ sAF_Coef -----------------------------------------
        {
            {
                50,  // i4Offset
                20,  // i4NormalNum
                20,  // i4MacroNum
                0,   // i4InfIdxOffset
                0,   // i4MacroIdxOffset
                {
                    0,  20,  40,  60,  80, 110, 140, 180, 220, 270,
                    320, 370, 430, 490, 550, 610, 670, 750, 830, 910,
                    0,   0,   0,   0,   0,   0,   0,   0,   0,   0              
                }
            },
            30, // i4THRES_MAIN;
            20, // i4THRES_SUB;
            2,  // i4AFC_FAIL_CNT;                 
            0,  // i4FAIL_POS;

            1,  // i4INIT_WAIT;
            {500, 500, 500, 500, 500}, // i4FRAME_WAIT
            0,  // i4DONE_WAIT;                      
        },
        // ------------------------------- sVAFC_Coef --------------------------------------------
        {
            {
                50,  // i4Offset
                20,  // i4NormalNum
                20,  // i4MacroNum
                0,   // i4InfIdxOffset
                0,   // i4MacroIdxOffset          
                {
                    0,  20,  40,  60,  80, 110, 140, 180, 220, 270,
                    320, 370, 430, 490, 550, 610, 670, 750, 830, 910,
                    0,   0,   0,   0,   0,   0,   0,   0,   0,   0
                }                
            },
            30, // i4THRES_MAIN;
            20, // i4THRES_SUB;
            2,  // i4AFC_FAIL_CNT;         
            0,  // i4FAIL_POS;      
   
            1,  // i4INIT_WAIT;
            {500, 500, 500, 500, 500}, // i4FRAME_WAIT
            0,  // i4DONE_WAIT;            
        },
        // -------------------- sAF_TH ---------------------
        {
            8, // i4ISONum;
            {100,150,200,300,400,600,800,1600}, // i4ISO[ISO_MAX_NUM];
                            
            // SGG1~7
            {{20, 19, 19, 18, 18, 17, 16, 15},
             {29, 29, 28, 28, 27, 26, 26, 24},
             {43, 42, 42, 41, 41, 40, 39, 37},
             {62, 61, 61, 60, 60, 59, 58, 56},
             {88, 88, 88, 87, 87, 86, 85, 84},
             {126, 126, 126, 125, 125, 125, 124, 123},
             {180, 180, 180, 179, 179, 179, 179, 178}},
           
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];

            {50000,50000,50000,50000,50000,50000,50000,50000}, // i4MIN_TH[ISO_MAX_NUM];

            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
           
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
          
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
           
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];
        },
         // ------------------- sZSDAF_TH -----------------------------------
        {
            8, // i4ISONum;
            {100,150,200,300,400,600,800,1600}, // i4ISO[ISO_MAX_NUM];
                              
            // SGG1~7
            {{20, 19, 19, 18, 18, 17, 16, 15},
             {29, 29, 28, 28, 27, 26, 26, 24},
             {43, 42, 42, 41, 41, 40, 39, 37},
             {62, 61, 61, 60, 60, 59, 58, 56},
             {88, 88, 88, 87, 87, 86, 85, 84},
             {126, 126, 126, 125, 125, 125, 124, 123},
             {180, 180, 180, 179, 179, 179, 179, 178}},
           
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            
            {50000,50000,50000,50000,50000,50000,50000,50000}, // i4MIN_TH[ISO_MAX_NUM];
         
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
           
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];

            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];
        },
        // -------------------- sVID_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];       
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];            
        },
        // -------------------- sVID1_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];       
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                
        },
        // -------------------- sVID2_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];       
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                 
        },
        // -------------------- sIHDR_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];       
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                    
        },
        // -------------------- sREV1_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];       
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                    
        },
        // -------------------- sREV2_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];        
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                 
        },
        // -------------------- sREV3_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];        
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                   
        },
        // -------------------- sREV4_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];        
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                 
        },
        // -------------------- sREV5_AF_TH ---------------------
        {
            0, // i4ISONum;
            {0,0,0,0,0,0,0,0}, // i4ISO[ISO_MAX_NUM];        
            {{0, 0, 0, 0, 0, 0, 0, 0},// SGG1~7
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0},
             {0, 0, 0, 0, 0, 0, 0, 0}},
            {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
            {0, 0, 0, 0, 0, 0, 0, 0},// i4MIN_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4HW_TH[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
            {0,0,0,0,0,0,0,0} // i4HW_TH2[ISO_MAX_NUM];                   
        },

        // --- Common use ---        
        1,  // i4ReadOTP;               // 0:disable, 1:enable           
        3,  // i4StatGain;    
        30, // i4LV_THRES;         
        200,// i4InfPos;
        33, // i4FRAME_TIME    
        {0, 50, 150, 250, 350},     // back jump
        400,//i4BackJumpPos
                    
        20, // i4AFC_STEP_SIZE;
        18, // i4SPOT_PERCENT_W;       // AFC window location
        24, // i4SPOT_PERCENT_H;       // AFC window location
        0,  // i4CHANGE_CNT_DELTA;        

        1,  // i4AFS_STEP_MIN_ENABLE;
        4,  // i4AFS_STEP_MIN_NORMAL;
        4,  // i4AFS_STEP_MIN_MACRO;        

        5,  // i4FIRST_FV_WAIT;
        12, // i4FV_1ST_STABLE_THRES;        
        10000,  // i4FV_1ST_STABLE_OFFSET;
        6,  // i4FV_1ST_STABLE_NUM;                        
        6,  // i4FV_1ST_STABLE_CNT; 
        50, // i4FV_SHOCK_THRES;
        30000,  // i4FV_SHOCK_OFFSET;
        5,  // i4FV_VALID_CNT;
        20, // i4FV_SHOCK_FRM_CNT;
        5,  // i4FV_SHOCK_CNT;          

        // --- FaceAF ---
        80, // i4FDWinPercent;
        40, // i4FDSizeDiff;
	    15, // i4FD_DETECT_CNT;
	    5,  // i4FD_NONE_CNT; 
        
        // --- AFv1.1/AFv1.2 ---
        0,  // i4LeftSearchEnable;       //[0] 0:disable, 1:enable
        1,  // i4LeftJumpStep;           //[1] when disable left peak search, left step= 3 + i4LeftJumpStep
        0,  // No use
        0,  // i4AfDoneDelay;            //[3] AF done happen delay count
        0,  // i4VdoAfDoneDelay;         //[3] AF done happen delay count
        0,  // i4ZoomInWinChg;           //[4] enable AF window change with Zoom-in  
        0,  // i4SensorEnable;           //[5] AF use sensor lister => 0:disable, 1:enable
        70, // i4PostureComp;            //[6] post comp max offset => 0:disable, others:enable
        2,  // i4SceneMonitorLevel;      //[7] scenechange enhancement level => 0:original, 1~3:from stable to sensitive
        1,  // i4VdoSceneMonitorLevel;   //[7] scenechange enhancement level => 0:original, 1~3:from stable to sensitive

        // Scene Monitor 
        // {Type, Off, Thr{by level}, Cnt{by level}}        
        // Type: 1 is &&, 0 is ||
        // Off : value, min=0 is more sensitive
        // Thr : percentage(value for gyro/acce), min=0 is more sensitive 
        // Cnt : value, max=30 is more stable
        // preview params
        {1,                                       // FV 0:disable, 1:enable
         0,  100, { 40, 40, 30}, { 13, 10, 10},   //    chg; chgT lower to sensitive, chgN bigger to stable   
         1, 5000, { 15, 15, 15}, { 20, 15, 10} }, //    stb; stbT should fix, stbN bigger to stable         
        {1,                                       // GS 0:disable, 1:enable
         0,   15, { 20, 20, 15}, { 28, 25, 25},   //    chg; chgT lower to sensitive, chgN bigger to stable
         1,    5, {  5,  5,  5}, { 12, 10,  7} }, //    stb  
        {1,                                       // AEB 0:disable, 1:enable
         0,   15, { 30, 30, 25}, { 33, 30, 30},   //    chg
         1,    5, { 10, 10, 10}, { 12, 10,  7} }, //    stb           
        {1,                                       // GYRO 0:disable, 1:enable
         0,    0, { 40, 40, 20}, {  5,  3,  3},   //    chg; off=0
         1,    0, { 10, 10, 10}, { 12, 10,  7} }, //    stb; off=0         
        {1,                                       // ACCE 0:disable, 1:enable
         0,    0, { 80, 80, 60}, { 15, 12, 12},   //    chg; off=0   
         1,    0, { 50, 50, 50}, { 12, 10,  7} }, //    stb; off=0      
        // video params
        {1,                                       // vdo FV
         0,  100, { 40, 40, 30}, { 13, 10, 10},   //    chg; chgT lower to sensitive, chgN bigger to stable   
         1, 5000, { 15, 15, 15}, { 20, 15, 10} }, //    stb; stbT should fix, stbN bigger to stable         
        {1,                                       // vdo GS
         0,   15, { 20, 20, 15}, { 28, 25, 25},   //    chg; chgT lower to sensitive, chgN bigger to stable
         1,    5, {  5,  5,  5}, { 15, 13, 10} }, //    stb            
        {1,                                       // vdo AEB
         0,   15, { 30, 30, 25}, { 33, 30, 30},   //    chg
         1,    5, { 10, 10, 10}, { 15, 13, 10} }, //    stb            
        {1,                                       // vdo GYRO
         0,    0, { 40, 40, 20}, {  7,  5,  5},   //    chg; video mode cnt > normal mode cnt for stable
         1,    0, { 10, 10, 10}, { 15, 13, 10} }, //    stb; video mode cnt > normal mode cnt for stable  
        {1,                                       // vdp ACCE 
         0,    0, { 80, 80, 60}, { 15, 12, 12},   //    chg; video mode cnt > normal mode cnt for stable  
         1,    0, { 50, 50, 50}, { 13, 13, 10} }, //    stb; video mode cnt > normal mode cnt for stable  

        // --- AFv2.0 ---     
        0,  // i4FvExtractEnable;           // 0:disable, 1:enable (for faceAF)
        30, // i4FvExtractThr               // percentage, fixed
        
        0,  // i4DampingCompEnable;         // 0:disable, 1:enable
        {0, 10, 20, 30, 40, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // { step1, step2, ... , step 15(atmost) }
        {
            {  0, -3, -5, -7, -10, -13, 0, 0, 0, 0, 0, 0, 0, 0, 0},// prerun0:{rComp1, rComp2, ..., rComp15(atmost)}
            {  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // prerun1:{rComp1, rComp2, ..., rComp15(atmost)}
        },
        {
            {  0,  3,  5,  7,  10,  13, 0, 0, 0, 0, 0, 0, 0, 0, 0},// prerun0:{lComp1, lComp2, ..., lComp15(atmost)}
            {  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // prerun1:{lComp1, lComp2, ..., lComp15(atmost)}
        },
        
        0,  // i4DirSelectEnable;           // 0:disable, 1:enable
        -1, // i4InfDir;                    // 1:right, -1:left, 2:reverse, 0:keep
        -1, // i4MidDir;                    // "
        -1, // i4MacDir;                    // "
        40, // i4RatioInf;                  // below % of tableLength belongs to INF
        70, // i4RatioMac;                  // above % of tableLength belongs to MAC ; should be larger than i4RatioInf                
        0,  // i4StartBoundEnable;          // 0:disable, 1:enable
        3,  // i4StartCamCAF;               // 0:keep current, -1:force to INF, X:set to boundary if within X steps
        3,  // i4StartCamTAF;               // "
        0,  // i4StartVdoCAF;               // "
        0,  // i4StartVdoTAF;               // "                          
        0,  // i4FpsRemapTblEnable;         // 0:disable, 1:enable
        {10, 12}, // i4FpsThres[2];        // two fps threshold {very-low, low}
        {1, 0},   // i4TableClipPt[2];     // clip pts at macro side
        {80, 90}, // i4TableRemapPt[2];    // percentage of new/org table length

        // --- Easytuning ---       
        {0},// i4EasyTuning[100]

        // --- DepthAF ---          
        {0},// i4DepthAF[500]
        
        // --- reserved ---                 
        {0} // i4Coefs[100];
    },
    {0}
};

UINT32 SensorDrive_getDefaultData(VOID *pDataBuf, UINT32 size)
{
    UINT32 dataSize = sizeof(NVRAM_LENS_PARA_STRUCT);

    if ((pDataBuf == NULL) || (size < dataSize))
    {
        return 1;
    }

    // copy from Buff to global struct
    memcpy(pDataBuf, &SENSOR_DRIVE_LENS_PARA_DEFAULT_VALUE, dataSize);

    return 0;
}

PFUNC_GETLENSDEFAULT pSensorDrive_getDefaultData = SensorDrive_getDefaultData;

