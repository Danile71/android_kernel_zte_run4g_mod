/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_hcf_default.h
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
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_HCF_DEFAULT_H
#define AUDIO_HCF_DEFAULT_H
#if defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V5)
#define BES_LOUDNESS_HCF_L_HPF_FC       0
#define BES_LOUDNESS_HCF_L_HPF_ORDER    0
#define BES_LOUDNESS_HCF_L_BPF_FC       0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_L_BPF_BW       0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_L_BPF_GAIN     0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8
#define BES_LOUDNESS_HCF_L_LPF_FC       0
#define BES_LOUDNESS_HCF_L_LPF_ORDER    0
#define BES_LOUDNESS_HCF_R_HPF_FC       0
#define BES_LOUDNESS_HCF_R_HPF_ORDER    0
#define BES_LOUDNESS_HCF_R_BPF_FC       0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_R_BPF_BW       0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_R_BPF_GAIN     0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8, 0 << 8
#define BES_LOUDNESS_HCF_R_LPF_FC       0
#define BES_LOUDNESS_HCF_R_LPF_ORDER    0

#define BES_LOUDNESS_HCF_SEP_LR_FILTER  0

#define BES_LOUDNESS_HCF_WS_GAIN_MAX    0
#define BES_LOUDNESS_HCF_WS_GAIN_MIN    0
#define BES_LOUDNESS_HCF_FILTER_FIRST   0

#define BES_LOUDNESS_HCF_NUM_BANDS      0
#define BES_LOUDNESS_HCF_FLT_BANK_ORDER 0
#define BES_LOUDNESS_HCF_DRC_DELAY      0
#define BES_LOUDNESS_HCF_CROSSOVER_FREQ 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_SB_MODE        0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_SB_GAIN        0, 0, 0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_GAIN_MAP_IN    \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_GAIN_MAP_OUT   \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0,                  \
        0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_ATT_TIME       \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_REL_TIME       \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0
#define BES_LOUDNESS_HCF_HYST_TH        \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0,               \
        0, 0, 0, 0, 0, 0

#define BES_LOUDNESS_HCF_LIM_TH     0
#define BES_LOUDNESS_HCF_LIM_GN     0
#define BES_LOUDNESS_HCF_LIM_CONST  0
#define BES_LOUDNESS_HCF_LIM_DELAY  0

#elif defined(MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4)
    /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 

	#define BES_LOUDNESS_HCF_HSF_COEFF \
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
\
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 

   

    /* Compensation Filter BPF coeffs: default all pass filter      */ 
    #define BES_LOUDNESS_HCF_BPF_COEFF \
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
\
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
\
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
\    
 	0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \         
\    
 	0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \     
\    
 	0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \
\    
 	0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \    
\
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000, \ 
    0x00000000,0x00000000,0x00000000     
    
    #define BES_LOUDNESS_HCF_LPF_COEFF \
    0x00000000, 0x00000000, 0x00000000,\ 
	0x00000000, 0x00000000, 0x00000000,\ 
	0x00000000, 0x00000000, 0x00000000,\ 
	0x00000000, 0x00000000, 0x00000000,\ 
	0x00000000, 0x00000000, 0x00000000,\ 
	0x00000000, 0x00000000, 0x00000000 

    #define BES_LOUDNESS_HCF_WS_GAIN_MAX  0x399A
           
    #define BES_LOUDNESS_HCF_WS_GAIN_MIN  0x2666
           
    #define BES_LOUDNESS_HCF_FILTER_FIRST  0
           
    #define BES_LOUDNESS_HCF_GAIN_MAP_IN \
    -45, -35, -19, -18,  0
   
    #define BES_LOUDNESS_HCF_GAIN_MAP_OUT \            
    0, 12, 12, 12, 0

	#define BES_LOUDNESS_HCF_ATT_TIME	164
	#define BES_LOUDNESS_HCF_REL_TIME	16400
#else
/* Compensation Filter HSF coeffs: default all pass filter       */
/* BesLoudness also uses this coeffs    */
#define BES_LOUDNESS_HCF_HSF_COEFF \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0

/* Compensation Filter BPF coeffs: default all pass filter      */
#define BES_LOUDNESS_HCF_BPF_COEFF \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    \     
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    \     
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    \     
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0, \
    0, 0, 0

#define BES_LOUDNESS_HCF_DRC_FORGET_TABLE \
    0, 0, \
    0, 0, \
    0, 0, \
    0, 0, \
    0, 0, \
    0, 0, \
    0, 0, \
    0, 0, \
    0, 0

#define BES_LOUDNESS_HCF_WS_GAIN_MAX  0

#define BES_LOUDNESS_HCF_WS_GAIN_MIN  0

#define BES_LOUDNESS_HCF_FILTER_FIRST  0

#define BES_LOUDNESS_HCF_GAIN_MAP_IN \
    0, 0, 0, 0, 0

#define BES_LOUDNESS_HCF_GAIN_MAP_OUT \
    0, 0, 0, 0, 0

#endif

#endif
