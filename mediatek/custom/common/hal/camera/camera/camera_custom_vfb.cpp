#include "camera_custom_vfb.h"

void get_VFB_CustomizeData(VFB_Customize_PARA_STRUCT *a_pDataOut)
{    
    a_pDataOut->temporal_smooth_level = 9;
    a_pDataOut->lost_face_timeout = 1800; // number of frames to wait until the face re-appears in extreme mode
    a_pDataOut->sort_face_weight  = 5;
    a_pDataOut->ruddy_level = 6;
}


