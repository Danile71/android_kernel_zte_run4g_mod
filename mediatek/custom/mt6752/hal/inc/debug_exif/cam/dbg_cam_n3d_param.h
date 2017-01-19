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

#ifndef _DBG_CAM_N3D_PARAM_H
#define _DBG_CAM_N3D_PARAM_H

#include "dbg_n3d_ae_param.h"
#include "dbg_n3d_awb_param.h"



//Common Parameter Structure
typedef enum
{
    N3D_TAG_VERSION = 0,
    
}DEBUG_N3D_TAG_T;


// Native3D debug info
#define N3D_DEBUG_TAG_SIZE     (N3D_AE_DEBUG_TAG_SIZE+N3D_AWB_DEBUG_TAG_SIZE)
#define N3D_DEBUG_TAG_VERSION  1


typedef struct DEBUG_N3D_INFO_S
{
    DEBUG_CAM_TAG_T Tag[N3D_DEBUG_TAG_SIZE];
} DEBUG_N3D_INFO_T;


#define DEBUG_N3D_AE_MODULE_ID          0x0001
#define DEBUG_N3D_AWB_MODULE_ID          0x0002

#define N3DAAATAG(module_id, tag, line_keep)   \
( (MINT32)                                  \
  ((MUINT32)(0x00000000) |                  \
   (MUINT32)((module_id & 0xff) << 24) |    \
   (MUINT32)((line_keep & 0x01) << 23) |    \
   (MUINT32)(tag & 0xffff))                 \
)


typedef struct
{
    N3D_AE_DEBUG_INFO_T  rAEDebugInfo;
    N3D_AWB_DEBUG_INFO_T rAWBDebugInfo;
} N3D_DEBUG_INFO_T;


#endif //_DBG_CAM_N3D_PARAM_H