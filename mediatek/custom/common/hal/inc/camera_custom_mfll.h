#ifndef _CAMERA_CUSTOM_MFLL_H_
#define _CAMERA_CUSTOM_MFLL_H_

#include "camera_custom_types.h"	// For MUINT*/MINT*/MVOID/MBOOL type definitions.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
// For MFLL Customer Parameters

// [Best Shot Selection (BSS)]
//     - When CUST_MFLL_ENABLE_BSS_FOR_MFLL==0,
//          Use the first image as base image for blending.
//          The advantage is shutter lag has been minimized.
//          The drawback is final image has higher motion blur (cause by handshack)possibility .
//     - When CUST_MFLL_ENABLE_BSS_FOR_MFLL==1 (recommended),
//          Use the image with the highest sharpness as base image.
//          The advantage is the motion blur (cause by handshack) has been minimized.
//          The drawback is a longer shutter lag (average ~+100ms)
#define CUST_MFLL_ENABLE_BSS_FOR_MFLL	1

//     - When CUST_MFLL_ENABLE_BSS_FOR_AIS==0,
//          Use the first image as base image for blending.
//          The advantage is shutter lag has been minimized.
//          The drawback is final image has higher motion blur (cause by handshack)possibility .
//     - When CUST_MFLL_ENABLE_BSS_FOR_AIS==1 (recommended),
//          Use the image with the highest sharpness as base image.
//          The advantage is the motion blur (cause by handshack) has been minimized.
//          The drawback is a longer shutter lag (average ~+100ms)
#define CUST_MFLL_ENABLE_BSS_FOR_AIS	1

//     - how many rows are skipped during processing 
//          recommand range: >=8
//          recommand value: 8
//          larger scale factor cause less accurate but faster execution time.
#define CUST_MFLL_BSS_SCALE_FACTOR      8

//     - minimum edge response
//          recommand range: 10~30
//          recommand value: 20
//          larger th0 cause better noise resistence but may miss real edges.
#define CUST_MFLL_BSS_CLIP_TH0          20

//     - maximum edge response
//          recommand range: 50~120
//          recommand value: 100
//          larger th1 will suppress less high contrast edges
#define CUST_MFLL_BSS_CLIP_TH1          100

//     - tri-pod/static scene detection
//          recommand range: 0~10
//          recommand value: 10
//          larger zero cause more scene will be considered as static
#define CUST_MFLL_BSS_ZERO              10

// [EIS]
//     - CUST_MFLL_EIS_TRUST_TH,
//          The threashold of the trust value of EIS statistic.
//          If the trust value of EIS statistic lower than this threshold, the GMV calculation process willn't include this block.
//          Use higher CUST_MFLL_EIS_TRUST_TH will reduce the possibility of artifact, but increase the possibility of single frame output too.
//          p.s. single frame outout - when the motion between frames are too large to be accepted, MFLL/AIS willn't take effect for avoiding artifact.
#define CUST_MFLL_EIS_TRUST_TH    100
//     - CUST_MFLL_EIS_OP_STEP_H,
//          reserved only, never modify this
#define CUST_MFLL_EIS_OP_STEP_H   1
//     - CUST_MFLL_EIS_OP_STEP_V,
//          reserved only, never modify this
#define CUST_MFLL_EIS_OP_STEP_V   1



#endif	// _CAMERA_CUSTOM_MFLL_H_

