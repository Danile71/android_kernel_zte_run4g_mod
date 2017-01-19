#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_lens.h"

const NVRAM_LENS_PARA_STRUCT GAF008AF_LENS_PARA_DEFAULT_VALUE =
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
                155, // i4Offset
                12,  // i4NormalNum
                12,  // i4MacroNum
                0,   // i4InfIdxOffset
                0,   // i4MacroIdxOffset          
                {
                    0,   10,  30,  60,  95,  135, 175, 215, 255, 295, 
                    340, 385, 0,   0,   0,   0,   0,   0,   0,   0,
                    0,   0,   0,   0,   0,   0,   0,   0,   0,   0
                }
            },
            15, // i4THRES_MAIN;
            10, // i4THRES_SUB;      
            1,  // i4AFC_FAIL_CNT;                 
            0,  // i4FAIL_POS;

            4,  // i4INIT_WAIT;
            {500, 500, 500, 500, 500}, // i4FRAME_WAIT
            0,  // i4DONE_WAIT;         
        },                             
        // ------------------------------- sVAFC_Coef --------------------------------------------
        {
            {
                155, // i4Offset
                16,  // i4NormalNum
                16,  // i4MacroNum
                0,   // i4InfIdxOffset
                0,   // i4MacroIdxOffset          
                {
                    0,   10,  30,  50,  70,  95,  120, 145, 175, 205, 
                    235, 265, 295, 325, 355, 385, 0,   0,   0,   0,
                    0,   0,   0,   0,   0,   0,   0,   0,   0,   0
                }
            },
            15, // i4THRES_MAIN;
            10, // i4THRES_SUB;            
            1,  // i4AFC_FAIL_CNT;         
            -1, // i4FAIL_POS;

            4,  // i4INIT_WAIT;
            {  0,  0, 100, 500, 500}, // i4FRAME_WAIT
            0,  // i4DONE_WAIT;                        
        },
        // -------------------- sAF_TH ---------------------
        {
            8, // i4ISONum;
            {100, 150, 200, 300, 400, 600, 800, 1600},
                  
            // SGG1~7
            {{21, 21, 21, 20, 20, 19, 19, 18},
             {30, 30, 30, 30, 29, 29, 28, 27},
             {44, 44, 43, 43, 43, 42, 42, 40},
             {63, 62, 62, 62, 62, 61, 61, 60},
             {89, 89, 89, 89, 88, 88, 88, 87},
             {127, 127, 127, 127, 126, 126, 126, 125},
             {181, 180, 180, 180, 180, 180, 180, 179}},
          
            // horizontal FV baseline
            {0, 0, 0, 0, 0, 0, 0, 0},
           
            // horizontal FV min. threshold
            {2000, 2000, 2000, 1000, 1000, 1000, 800, 800},

            // horizontal FV threshold
            {2, 2, 2, 2, 3, 3, 4, 4},//{3, 3, 3, 4, 4, 5, 6, 8},//{4,4,5,6,7,7,8,12},

            // vertical FV baseline
            {0, 0, 0, 0, 0, 0, 0, 0},
           
            // vertical FV min. threshold
            {0, 0, 0, 0, 0, 0, 0, 0},

            // vertical FV threshold
            {2, 2, 2, 2, 3, 3, 4, 4},//{3, 4, 4, 5, 5, 6, 6, 6},//{6, 6, 6, 8, 8, 8, 10, 14},//{9,9,9,9,10,10,11,13},
        },
        // ------------------- sZSDAF_TH -----------------------------------
        {
            8, // i4ISONum;
           {100, 150, 200, 300, 400, 600, 800, 1600},
                   
            // SGG1~7
           {{20, 19, 19, 18, 18, 17, 16, 14},
            {29, 29, 28, 28, 27, 26, 25, 23},
            {43, 42, 42, 41, 40, 40, 39, 36},
            {62, 61, 61, 60, 60, 59, 58, 56},
            {88, 88, 88, 87, 87, 86, 85, 83},
            {126, 126, 126, 125, 125, 124, 124, 122},
            {180, 180, 180, 179, 179, 179, 179, 177}},
                       
           // horizontal FV baseline
           {0, 0, 0, 0, 0, 0, 0, 0},
                      
           // horizontal FV min. threshold
           {2000, 2000, 2000, 1000, 1000, 1000, 1000, 1000},
                   
           // horizontal FV threshold
           {4, 4, 4, 5, 5, 5, 7, 7},//{4, 5, 6, 7, 8, 10, 11, 16},//{7,7,8,9,10,11,12,15},

           // vertical FV baseline
           {0, 0, 0, 0, 0, 0, 0, 0},
            
           // vertical FV min. threshold
           {0, 0, 0, 0, 0, 0, 0, 0},
                   
           // vertical FV threshold
           {4, 4, 4, 5, 5, 5, 7, 7},//{4, 4, 5, 6, 7, 7, 8, 9},//{8, 9, 10, 12, 12, 14, 18, 26},//{11,12,14,17,20,22,24,25},
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
        1,  // i4ReadOTP; 				// 0: disable, 1:enable           
        3,  // i4StatGain;    
        0,  // i4LV_THRES;         
        170,// i4InfPos;
        33, // i4FRAME_TIME    
        {0, 100, 200, 350, 500},     // back jump
        500,//i4BackJumpPos

        20, //i4AFC_STEP_SIZE;
        18, // i4SPOT_PERCENT_W;        // AFC window location
        24, // i4SPOT_PERCENT_H;        // AFC window location
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
        30000, // i4FV_SHOCK_OFFSET;
        5,  // i4FV_VALID_CNT;
        20, // i4FV_SHOCK_FRM_CNT;
        5,  // i4FV_SHOCK_CNT;          

        // --- FaceAF ---
        90, // i4FDWinPercent;
        40, // i4FDSizeDiff;
        20, // i4FD_DETECT_CNT;
        5,  // i4FD_NONE_CNT; 

        // --- AFv1.1/AFv1.2 ---
        1,  // i4LeftSearchEnable;       //[0] 0:disable, 1:enable
        0,  // i4LeftJumpStep;           //[1] when disable left peak search, left step= 3 + i4LeftJumpStep
        0,  // No use
        0,  // i4AfDoneDelay;            //[3] AF done happen delay count
        0,  // i4VdoAfDoneDelay;         //[3] AF done happen delay count
        0,  // i4ZoomInWinChg;           //[4] enable AF window change with Zoom-in  
        1,  // i4SensorEnable;           //[5] AF use sensor lister => 0:disable, 1:enable
        80, // i4PostureComp;            //[6] post comp max offset => 0:disable, others:enable
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
         1, 50000, { 25, 25, 25}, { 20, 15, 10} },//    stb; stbT should fix, stbN bigger to stable         
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
         1, 50000, { 20, 20, 20}, { 20, 15, 10} },//    stb; stbT should fix, stbN bigger to stable         
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
        1,  // i4FvExtractEnable;           // 0:disable, 1:enable (for faceAF)
        40, // i4FvExtractThr               // percentage, fixed
        
        1,  // i4DampingCompEnable;         // 0:disable, 1:enable
        {7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 0, 0, 0, 0, 0},        // { step1, step2, ... , step 15(atmost) }
        {
            {-2,-4,-4,-6,-5,-5,-13,-8,-10,-20, 0, 0, 0, 0, 0},// prerun0:{rComp1, rComp2, ..., rComp15(atmost)}
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}   // prerun1:{rComp1, rComp2, ..., rComp15(atmost)}
        },
        {
            {2,4,4,6,5,5,13,8,10,20, 0, 0, 0, 0, 0},          // prerun0:{lComp1, lComp2, ..., lComp15(atmost)}
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}   // prerun1:{lComp1, lComp2, ..., lComp15(atmost)}
        },

        1,  // i4DirSelectEnable;           // 0:disable, 1:enable
        -1, // i4InfDir;                    // 1:right, -1:left, 2:reverse, 0:keep
        -1, // i4MidDir;                    // "
        -1, // i4MacDir;                    // "
        40, // i4RatioInf;                  // below % of tableLength belongs to INF
        70, // i4RatioMac;                  // above % of tableLength belongs to MAC ; should be larger than i4RatioInf                
        1,  // i4StartBoundEnable;          // 0:disable, 1:enable
        3,  // i4StartCamCAF;               // 0:keep current, -1:force to INF, X:set to boundary if within X steps
        3,  // i4StartCamTAF;               // "
        0,  // i4StartVdoCAF;               // "
        0,  // i4StartVdoTAF;               // "                          
        1,  // i4FpsRemapTblEnable;         // 0:disable, 1:enable
        {10, 12}, // i4FpsThres[2];        // two fps threshold {very-low, low}
        {0, 0},   // i4TableClipPt[2];     // clip pts at macro side
        {80, 90}, // i4TableRemapPt[2];    // percentage of new/org table length

        // --- Easytuning ---       
        {0}, // i4EasyTuning[100]

        // --- DepthAF ---          
        {
            12,     // [0] 0:use i4DampingRdirComp/i4DampingLdirComp, N: use following sDpComp tbl, N is also max shutterIdx           
            120,    // [1] shutter_base, shutter = idx * 1/shutter_base (sec)
            10,     // [2] sDpComp_tbl_size
            30, 70, 100, // [3]~[5] position ratio[%]

            // shutterIdx=1, ratio[3], calibration table right & left
            1, 
            100, 100, 100, 
              0,  0,  0,  0, -1, -2,  0, -7, -1,  0,    // Right comp 1 ~ comp sDpComp_tbl_size
              0,  0,  0,  2,  1,  2,  2,  1,  0,  3,    // Left comp 1 ~ comp sDpComp_tbl_size
            // shutterIdx=2
            2, 
            100, 100, 100, 
              0,  0,  0,  0, -1, -2,  0, -7, -1,  0,    
              0,  0,  0,  2,  1,  2,  2,  1,  0,  3,    
            3, 
            100, 100, 100,
             -2, -5, -8, -9,-13,-15,-12,-33,-27,-16,
              2,  4,  6, 11, 14, 18, 16, 16, 20, 48,
            4, 
            100, 100, 100,
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,
            5,  
             80,  80,  80, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,            
            6,  
             66,  66,  66, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,            
            7,  
             57,  57,  57, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,            
            8,  
             50,  50,  50, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,            
            9,  
             44,  44,  44, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,            
            10, 
             40,  40,  40, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,                                         
            11, 
             36,  36,  36, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,                                                       
            // shutterIdx=maxShutterIdx  
            12, 
             33,  33,  33, 
             -3, -7,-12,-12,-19,-21,-29,-40,-52,-26,
              3,  7,  9, 17, 20, 25, 29, 23, 28, 60,                           
        }, // i4DepthAF[500]
        
        // --- reserved ---                 
        {0, // [0] InclineReject: 0 is disable, 1 is enable
        }  // i4Coefs[100];
    },
    {0}
};

UINT32 GAF008AF_getDefaultData(VOID *pDataBuf, UINT32 size)
{
    UINT32 dataSize = sizeof(NVRAM_LENS_PARA_STRUCT);

    if ((pDataBuf == NULL) || (size < dataSize))
    {
        return 1;
    }

    // copy from Buff to global struct
    memcpy(pDataBuf, &GAF008AF_LENS_PARA_DEFAULT_VALUE, dataSize);

    return 0;
}

PFUNC_GETLENSDEFAULT pGAF008AF_getDefaultData = GAF008AF_getDefaultData;

