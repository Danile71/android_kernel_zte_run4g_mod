#include "camera_custom_eis.h"

void get_EIS_CustomizeData(EIS_Customize_Para_t *a_pDataOut)
{    
    a_pDataOut->sensitivity   = CUSTOMER_EIS_SENSI_LEVEL_ADVTUNE;
    a_pDataOut->filter_small_motion = 0;    // 0 or 1
    a_pDataOut->adv_shake_ext = 1;          // 0 or 1
    a_pDataOut->stabilization_strength = 0.9;   // 0.5~0.95
    a_pDataOut->new_tru_th = 25;            // 0~100
    a_pDataOut->vot_th = 4;                 // 1~16
    a_pDataOut->votb_enlarge_size = 0;      // 0~1280
    a_pDataOut->min_s_th = 40;              // 10~100
    a_pDataOut->vec_th = 0;                 // 0~11   should be even
    a_pDataOut->spr_offset = 0;             // 0 ~ MarginX/2
    a_pDataOut->spr_gain1 = 0;              // 0~127
    a_pDataOut->spr_gain2 = 0;              // 0~127
    a_pDataOut->gmv_pan_array[0] = 0;       // 0~5
    a_pDataOut->gmv_pan_array[1] = 0;       // 0~5
    a_pDataOut->gmv_pan_array[2] = 0;       // 0~5
    a_pDataOut->gmv_pan_array[3] = 1;       // 0~5
    
    a_pDataOut->gmv_sm_array[0] = 0;    //0~5
    a_pDataOut->gmv_sm_array[1] = 0;    //0~5
    a_pDataOut->gmv_sm_array[2] = 0;    //0~5
    a_pDataOut->gmv_sm_array[3] = 1;    //0~5

    a_pDataOut->cmv_pan_array[0] = 0;   //0~5
    a_pDataOut->cmv_pan_array[1] = 0;   //0~5
    a_pDataOut->cmv_pan_array[2] = 0;   //0~5
    a_pDataOut->cmv_pan_array[3] = 1;   //0~5
    
    a_pDataOut->cmv_sm_array[0] = 0;    //0~5
    a_pDataOut->cmv_sm_array[1] = 1;    //0~5
    a_pDataOut->cmv_sm_array[2] = 2;    //0~5
    a_pDataOut->cmv_sm_array[3] = 4;    //0~5
    
    a_pDataOut->vot_his_method = ABSOLUTE_HIST; //ABSOLUTE_HIST or SMOOTH_HIST
    a_pDataOut->smooth_his_step = 3; // 2~6
    a_pDataOut->eis_debug = 0;
}

void get_EIS_PLUS_CustomizeData(EIS_PLUS_Customize_Para_t *a_pDataOut)
{
    a_pDataOut->warping_mode = EIS_WARP_METHOD_6_4_ADAPTIVE;
    a_pDataOut->search_range_x = 64;        // 32~64
    a_pDataOut->search_range_y = 64;        // 32~64
    a_pDataOut->crop_ratio     = 20;        // 10~40
    a_pDataOut->stabilization_strength = 0.9;   // 0.5~0.95
}

bool GetEisLinkWithDfr()
{
    return true;    // true : dynamic frame rate off when eis on; false : dynamic frame rate on when eis on
}

