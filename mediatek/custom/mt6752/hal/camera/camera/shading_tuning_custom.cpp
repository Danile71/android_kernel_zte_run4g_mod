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

#include "camera_custom_types.h"
#include "shading_tuning_custom.h"

/*
 *  disable sl2a & sl2b. enable slb2 for ais stage3/4, disable for other cases
 *  make sure shading_tuning_custom.cpp & isp_tuning_custom.cpp use the same value
 */
#define TUNING_FOR_AIS  1

MINT32 isEnableSL2(MUINT32 u4SensorDev)
{
    if(TUNING_FOR_AIS) {
        return 1;        
    }
    return 0;
}

MINT32 isByp123ToNvram(void)
{
    return 1;
}

MINT32 isLscTransformFromCapture(MUINT32 u4SensorDev)
{
    return 0;
}

