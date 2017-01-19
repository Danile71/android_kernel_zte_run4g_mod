#include "camera_custom_asd.h"

void get_asd_CustomizeData1(ASD_Customize_PARA1  *ASDDataOut1)
{
    ASDDataOut1->u1TimeWeightType = 1;       /*0~1 */
    ASDDataOut1->u1TimeWeightRange = 10;     /*1~10 */
    ASDDataOut1->u1ScoreThrNight = 60;       /* 40~100 */
    ASDDataOut1->u1ScoreThrBacklit = 50;     /* 40~100 */
    ASDDataOut1->u1ScoreThrPortrait = 50;    /* 40~100 */
    ASDDataOut1->u1ScoreThrLandscape = 55;   /* 40~100 */
}

void get_asd_CustomizeData2(ASD_Customize_PARA2  *ASDDataOut2)
{
    ASDDataOut2->s2IdxWeightBlAe = 1;        /*0~10 */
    ASDDataOut2->s2IdxWeightBlScd = 8;       /*0~10 */
    ASDDataOut2->s2IdxWeightLsAe = 1;        /*0~10 */
    ASDDataOut2->s2IdxWeightLsAwb = 1;       /*0~10 */
    ASDDataOut2->s2IdxWeightLsAf = 1;        /*0~10 */
    ASDDataOut2->s2IdxWeightLsScd = 2;       /*0~10 */
    ASDDataOut2->s2EvLoThrNight = 50;        /* 50 -> Lv=5 */
    ASDDataOut2->s2EvHiThrNight = 80;        /* 70 -> Lv=7 */
    ASDDataOut2->s2EvLoThrOutdoor = 90;      /*110 -> Lv=11 */
    ASDDataOut2->s2EvHiThrOutdoor = 130;     /*140 -> Lv=14 */
    ASDDataOut2->boolBacklitLockEnable = true;
    ASDDataOut2->s2BacklitLockEvDiff = -10;  /* -50~0 */
}

