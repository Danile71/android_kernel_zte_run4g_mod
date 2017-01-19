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

#ifndef _DBG_CAM_RESERVEA_PARAM_H
#define _DBG_CAM_RESERVEA_PARAM_H

//ReserveA Parameter Structure
typedef enum
{
    RESERVEA_TAG_VERSION = 0,
    /* add tags here */
    
    RESERVEA_TAG_END
}DEBUG_RESERVEA_TAG_T;

// TEST_A debug info
#define RESERVEA_DEBUG_TAG_SIZE     (RESERVEA_TAG_END+10)
#define RESERVEA_DEBUG_TAG_VERSION  (0)


struct DEBUG_RESERVEA_INFO_T {
    DEBUG_CAM_TAG_T Tag[RESERVEA_DEBUG_TAG_SIZE];
};


#endif //_DBG_CAM_TESTA_PARAM_H
