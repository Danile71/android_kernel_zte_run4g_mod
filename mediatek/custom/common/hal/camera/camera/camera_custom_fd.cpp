#include "camera_custom_fd.h"

void get_fd_CustomizeData(FD_Customize_PARA  *FDDataOut)
{    
    FDDataOut->FDThreadNum = 1;
    FDDataOut->FDThreshold = 256;
    FDDataOut->MajorFaceDecision = 1;
    FDDataOut->OTRatio = 1088;
    FDDataOut->SmoothLevel = 5;
    FDDataOut->FDSkipStep = 1;
    FDDataOut->FDRectify = 10;
    FDDataOut->FDRefresh = 3;
    FDDataOut->SDThreshold = 69;
    FDDataOut->SDMainFaceMust = 1;
    FDDataOut->SDMaxSmileNum = 3;
    FDDataOut->GSensor = 1;
    FDDataOut->FDModel = 1;
    FDDataOut->FDSizeRatio = 0.0; //Float: 0~1 (filter ratio: face size/image hieght)
    FDDataOut->OTFlow = 1;  //0:Original Flow (FDRefresh:60)  , 1:New Flow (FDRefresh:3)
}


