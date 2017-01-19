/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 04 22 2013 kh.hung
 * [ALPS00580843] [MT6572tdv1_phone[lca]][music][Symbio][Free test] 音?播放器播放音?的?音?大?小，不?定
 * Use default DRC setting.
 * 
 * Review: http://mtksap20:8080/go?page=NewReview&reviewid=59367
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_MUSIC_DRC_DEFAULT_H
#define AUDIO_MUSIC_DRC_DEFAULT_H

#define BES_LOUDNESS_MUSICDRC_L_HPF_FC          0
#define BES_LOUDNESS_MUSICDRC_L_HPF_ORDER       0
#define BES_LOUDNESS_MUSICDRC_L_BPF_FC          0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_MUSICDRC_L_BPF_BW          0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_MUSICDRC_L_BPF_GAIN        0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_MUSICDRC_L_LPF_FC          0
#define BES_LOUDNESS_MUSICDRC_L_LPF_ORDER       0
#define BES_LOUDNESS_MUSICDRC_R_HPF_FC          0
#define BES_LOUDNESS_MUSICDRC_R_HPF_ORDER       0
#define BES_LOUDNESS_MUSICDRC_R_BPF_FC          0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_MUSICDRC_R_BPF_BW          0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_MUSICDRC_R_BPF_GAIN        0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_MUSICDRC_R_LPF_FC          0
#define BES_LOUDNESS_MUSICDRC_R_LPF_ORDER       0

#define BES_LOUDNESS_MUSICDRC_SEP_LR_FILTER     0
	
#define BES_LOUDNESS_MUSICDRC_WS_GAIN_MAX       0
#define BES_LOUDNESS_MUSICDRC_WS_GAIN_MIN       0
#define BES_LOUDNESS_MUSICDRC_FILTER_FIRST      0

#define BES_LOUDNESS_MUSICDRC_NUM_BANDS      8
#define BES_LOUDNESS_MUSICDRC_FLT_BANK_ORDER 0
#define BES_LOUDNESS_MUSICDRC_DRC_DELAY      0
#define BES_LOUDNESS_MUSICDRC_CROSSOVER_FREQ 300,500,1000,2000,4000,8000,16000
#define BES_LOUDNESS_MUSICDRC_SB_MODE        0,0,0,0,0,0,0,0
#define BES_LOUDNESS_MUSICDRC_SB_GAIN        -15360,-8960,-3840,0,0,0,0,0
#define BES_LOUDNESS_MUSICDRC_GAIN_MAP_IN    \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0,                  \
        -15360,-12800,-10240,-7680,0
#define BES_LOUDNESS_MUSICDRC_GAIN_MAP_OUT   \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0,                  \
        6144,6144,6144,6144,0
#define BES_LOUDNESS_MUSICDRC_ATT_TIME       \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164,                  \
        164,164,164,164,164,164
#define BES_LOUDNESS_MUSICDRC_REL_TIME       \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400,                  \
        16400,16400,16400,16400,16400,16400
#define BES_LOUDNESS_MUSICDRC_HYST_TH        \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256,                  \
        256,256,256,256,256,256

#define BES_LOUDNESS_MUSICDRC_LIM_TH     0x7FFF
#define BES_LOUDNESS_MUSICDRC_LIM_GN     0x7FFF
#define BES_LOUDNESS_MUSICDRC_LIM_CONST  4
#define BES_LOUDNESS_MUSICDRC_LIM_DELAY  0

#endif
