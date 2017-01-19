#ifndef _DBG_AAA_PARAM_H_
#define _DBG_AAA_PARAM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 3A debug info
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AAATAG(module_id, tag, line_keep)   \
( (MINT32)                                  \
  ((MUINT32)(0x00000000) |                  \
   (MUINT32)((module_id & 0xff) << 24) |    \
   (MUINT32)((line_keep & 0x01) << 23) |    \
   (MUINT32)(tag & 0xffff))                 \
)

#define MODULE_NUM(total_module, tag_module)      \
((MINT32)                                         \
 ((MUINT32)(0x00000000) |                         \
  (MUINT32)((total_module & 0xff) << 16) |        \
  (MUINT32)(tag_module & 0xff))                   \
)

typedef struct
{
    MUINT32 u4FieldID;
    MUINT32 u4FieldValue;
}  AAA_DEBUG_TAG_T;

#include "dbg_ae_param.h"
#include "dbg_af_param.h"
#include "dbg_awb_param.h"
#include "dbg_flash_param.h"
#include "dbg_flicker_param.h"
#include "dbg_shading_param.h"
//
#define AAA_DEBUG_AE_MODULE_ID          0x6001
#define AAA_DEBUG_AF_MODULE_ID          0x6002
#define AAA_DEBUG_AWB_MODULE_ID         0x6003
#define AAA_DEBUG_FLASH_MODULE_ID       0x6004
#define AAA_DEBUG_FLICKER_MODULE_ID     0x6005
#define AAA_DEBUG_SHADING_MODULE_ID     0x6006
#define AAA_DEBUG_AWB_DATA_MODULE_ID    0x6007
#define AAA_DEBUG_AE_PLINE_MODULE_ID    0x6008
#define AAA_DEBUG_SHADTBL2_MODULE_ID    0x6009
//

typedef struct
{
    struct Header
    {
        MUINT32  u4KeyID;
        MUINT32  u4ModuleCount;
        MUINT32  u4AEDebugInfoOffset;
        MUINT32  u4AFDebugInfoOffset;
        MUINT32  u4AWBDebugInfoOffset;
        MUINT32  u4FlashDebugInfoOffset;
        MUINT32  u4FlickerDebugInfoOffset;
        MUINT32  u4ShadingDebugInfoOffset;
        MUINT32  u4AWBDebugDataOffset;
        MUINT32  u4AEPlineDataOffset;
        MUINT32  u4ShadingTableOffset;
    } hdr;

    AE_DEBUG_INFO_T       rAEDebugInfo;
    AF_DEBUG_INFO_T       rAFDebugInfo;
    AWB_DEBUG_INFO_T      rAWBDebugInfo;
    FLASH_DEBUG_INFO_T    rFlashDebugInfo;
    FLICKER_DEBUG_INFO_T  rFlickerDebugInfo;
    SHADING_DEBUG_INFO_T  rShadigDebugInfo;
    AWB_DEBUG_DATA_T      rAWBDebugData;
    AE_PLINE_DEBUG_INFO_T rAEPlineDebugInfo;
    //SHAD Table
    DEBUG_SHAD_ARRAY_2_T  rShadingRestTbl;

} AAA_DEBUG_INFO_T;

#endif // _DBG_AAA_PARAM_H_

