/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _DBG_CAM_PARAM_H
#define _DBG_CAM_PARAM_H

typedef struct
{
    MUINT32 u4FieldID;
    MUINT32 u4FieldValue;
} DEBUG_CAM_TAG_T;

#include "dbg_cam_common_param.h"
#include "dbg_cam_mf_param.h"
#include "dbg_cam_n3d_param.h"
#include "dbg_cam_sensor_param.h"
#include "dbg_cam_reservea_param.h"
#include "dbg_cam_reserveb_param.h"
#include "dbg_cam_reservec_param.h"

// dbgCAM debug info
#define CAMTAG(module_id, tag, line_keep)   \
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
//
#define DEBUF_CAM_TOT_MODULE_NUM    7 //should be modified
#define DEBUF_CAM_TAG_MODULE_NUM    6 //should be modified

typedef struct DEBUG_CAM_INFO_S
{
    struct Header
    {
        MUINT32  u4KeyID;
        MUINT32  u4ModuleCount;
        MUINT32  u4DbgCMNInfoOffset;
        MUINT32  u4DbgMFInfoOffset;
        MUINT32  u4DbgN3DInfoOffset;
        MUINT32  u4DbgSENSORInfoOffset;
        MUINT32  u4DbgRESERVEAInfoOffset;
        MUINT32  u4DbgRESERVEBInfoOffset;
        MUINT32  u4DbgRESERVECInfoOffset;
    } hdr;

    DEBUG_CMN_INFO_T        rDbgCMNInfo;
    DEBUG_MF_INFO_T         rDbgMFInfo;
    DEBUG_N3D_INFO_T        rDbgN3DInfo;
    DEBUG_SENSOR_INFO_T     rDbgSENSORInfo;
    DEBUG_RESERVEA_INFO_T   rDbgRESERVEAInfo;
    DEBUG_RESERVEB_INFO_T   rDbgRESERVEBInfo;
    DEBUG_RESERVEC_INFO_T   rDbgRESERVECInfo;

} DEBUG_CAM_INFO_T;


#endif  //_DBG_CAM_PARAM_H

