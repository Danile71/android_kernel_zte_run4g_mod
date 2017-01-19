#ifndef _EIS_CONFIG_H_
#define _EIS_CONFIG_H_

typedef int          MINT32;
typedef unsigned int MUINT32;
typedef float        MFLOAT;


typedef enum
{
    CUSTOMER_EIS_SENSI_LEVEL_HIGH = 0,
    CUSTOMER_EIS_SENSI_LEVEL_NORMAL = 1,
    CUSTOMER_EIS_SENSI_LEVEL_ADVTUNE = 2
}Customize_EIS_SENSI;

typedef enum
{
    ABSOLUTE_HIST,
    SMOOTH_HIST    
} Customize_EIS_VOTE_METHOD_ENUM;

typedef enum
{
    EIS_WARP_METHOD_6_COEFF = 0,        // 6 coefficient
    EIS_WARP_METHOD_4_COEFF = 1,        // 4 coefficient
    EIS_WARP_METHOD_6_4_ADAPTIVE = 2,   // 6/4 adaptive (default)
    EIS_WARP_METHOD_2_COEFF = 3         //  2 coefficient
} Customize_WARP_METHOD_ENUM;


typedef struct
{
    Customize_EIS_SENSI sensitivity;
    MUINT32 filter_small_motion;    // 0 or 1
    MUINT32 adv_shake_ext;          // 0 or 1
    MFLOAT  stabilization_strength; // 0.5~0.95
    MUINT32 new_tru_th;             // 0~100
    MUINT32 vot_th;                 // 1~16
    MUINT32 votb_enlarge_size;      // 0~1280
    MUINT32 min_s_th;               // 10~100
    MUINT32 vec_th;                 // 0~11   should be even
    MUINT32 spr_offset;             //0 ~ MarginX/2
    MUINT32 spr_gain1;              // 0~127
    MUINT32 spr_gain2;              // 0~127
    MUINT32 gmv_pan_array[4];       //0~5
    MUINT32 gmv_sm_array[4];        //0~5
    MUINT32 cmv_pan_array[4];       //0~5
    MUINT32 cmv_sm_array[4];        //0~5
    
    Customize_EIS_VOTE_METHOD_ENUM vot_his_method; //0 or 1
    MUINT32 smooth_his_step; // 2~6
    MUINT32 eis_debug;
}EIS_Customize_Para_t;

typedef struct
{
    Customize_WARP_METHOD_ENUM warping_mode;
    MINT32 search_range_x;      // 32~64
    MINT32 search_range_y;      // 32~64
    MINT32 crop_ratio;          // 10~40
    MFLOAT stabilization_strength;  // 0.5~0.95
}EIS_PLUS_Customize_Para_t;


void get_EIS_CustomizeData(EIS_Customize_Para_t *a_pDataOut);
void get_EIS_PLUS_CustomizeData(EIS_PLUS_Customize_Para_t *a_pDataOut);
bool GetEisLinkWithDfr();
	
#endif /* _EIS_CONFIG_H */

