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

#ifndef _DBG_CAM_RESERVEC_PARAM_H
#define _DBG_CAM_RESERVEC_PARAM_H

//ReserveC Parameter Structure
typedef enum
{
    RESERVEC_TAG_VERSION = 0,
    /* add tags here */
    
    RESERVEC_TAG_END
}DEBUG_RESERVEC_TAG_T;

// TEST_C debug info
#define RESERVEC_DEBUG_TAG_SIZE     (RESERVEC_TAG_END+10)
#define RESERVEC_DEBUG_TAG_VERSION  (0)

//gmv
#define MF_MAX_FRAME            8
enum {MF_GMV_DEBUG_TAG_GMV_X
    , MF_GMV_DEBUG_TAG_GMV_Y
    , MF_GMV_DEBUG_TAG_ITEM_SIZE
    };
#define MF_GMV_DEBUG_TAG_SIZE               (MF_GMV_DEBUG_TAG_ITEM_SIZE)

//eis
#define MF_EIS_DEBUG_TAG_WINDOW             32
enum {MF_EIS_DEBUG_TAG_MV_X    
    , MF_EIS_DEBUG_TAG_MV_Y
    , MF_EIS_DEBUG_TAG_TRUST_X
    , MF_EIS_DEBUG_TAG_TRUST_Y
    , MF_EIS_DEBUG_TAG_ITEM_SIZE
    };
#define MF_EIS_DEBUG_TAG_SIZE               (MF_EIS_DEBUG_TAG_WINDOW*MF_EIS_DEBUG_TAG_ITEM_SIZE)

struct DEBUG_RESERVEC_INFO_T {
    const MUINT32 count;
    const MUINT32 gmvCount;
    const MUINT32 eisCount;
    const MUINT32 gmvSize;
    const MUINT32 eisSize;
    MINT32  gmvData[MF_MAX_FRAME][MF_GMV_DEBUG_TAG_ITEM_SIZE];
    MUINT32 eisData[MF_MAX_FRAME][MF_EIS_DEBUG_TAG_WINDOW][MF_EIS_DEBUG_TAG_ITEM_SIZE];

    DEBUG_RESERVEC_INFO_T() : count(2)  //gmvCount + eisCount
                            //
                            , gmvCount(MF_MAX_FRAME)
                            , eisCount(MF_MAX_FRAME)
                            //
                            , gmvSize(MF_GMV_DEBUG_TAG_SIZE)
                            , eisSize(MF_EIS_DEBUG_TAG_SIZE)
    {
        
    }
};


#endif //_DBG_CAM_TESTC_PARAM_H
