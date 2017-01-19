#ifndef _DEBUG_ID_PARAM_H_
#define _DEBUG_ID_PARAM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DP Version
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define DEBUG_PARSER_VERSION        3

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CAM debug id
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define DEBUG_CAM_KEYID           0xF8F9FAFB
//
#define DEBUG_CAM_CMN_MID           0x5001
#define DEBUG_CAM_MF_MID            0x5002
#define DEBUG_CAM_N3D_MID           0x5003
#define DEBUG_CAM_SENSOR_MID        0x5004
#define DEBUG_CAM_RESERVE1_MID      0x5005
#define DEBUG_CAM_RESERVE2_MID      0x5006
#define DEBUG_CAM_RESERVE3_MID      0x5007


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 3A debug id
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AAA_DEBUG_KEYID           ( 0xF0F1F200 | DEBUG_PARSER_VERSION )


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP debug id
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define ISP_DEBUG_KEYID           0xF4F5F6F7


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SHAD TABLE debug id
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define DEBUG_SHAD_TABLE_KEYID    0xFCFDFEFF


#endif // _DEBUG_ID_PARAM_H_

