/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_PERFSURFACENATIVE_H
#define ANDROID_PERFSURFACENATIVE_H

__BEGIN_DECLS

enum {
    SCN_NONE           = 0,
    SCN_APP_SWITCH     = 1,
    SCN_APP_ROTATE     = 2,
    SCN_SW_CODEC       = 3,
    SCN_SW_CODEC_BOOST = 4,
    SCN_APP_TOUCH      = 5,
    SCN_DONT_USE1      = 6,
} PERFSURFACENATIVE_SCN_T;

enum {
    STATE_PAUSED    = 0,
    STATE_RESUMED   = 1,
    STATE_DESTROYED = 2,
    STATE_DEAD      = 3,
} PERFSURFACENATIVE_STATE_T;

enum {
    CMD_GET_CPU_FREQ_LEVEL_COUNT        = 0,
    CMD_GET_CPU_FREQ_LITTLE_LEVEL_COUNT = 1,
    CMD_GET_CPU_FREQ_BIG_LEVEL_COUNT    = 2,
    CMD_GET_GPU_FREQ_LEVEL_COUNT        = 3,
    CMD_GET_MEM_FREQ_LEVEL_COUNT        = 4,
} PERFSURFACENATIVE_CMD_GET_T;

enum {
    CMD_SET_CPU_CORE_MIN            = 0,
    CMD_SET_CPU_CORE_MAX            = 1,
    CMD_SET_CPU_CORE_BIG_LITTLE_MIN = 2,
    CMD_SET_CPU_CORE_BIG_LITTLE_MAX = 3,
    CMD_SET_CPU_FREQ_MIN            = 4,
    CMD_SET_CPU_FREQ_MAX            = 5,
    CMD_SET_CPU_FREQ_BIG_LITTLE_MIN = 6,
    CMD_SET_CPU_FREQ_BIG_LITTLE_MAX = 7,
    CMD_SET_GPU_FREQ_MIN            = 8,
    CMD_SET_GPU_FREQ_MAX            = 9,
    CMD_SET_MEM_FREQ_MIN            = 10,
    CMD_SET_MEM_FREQ_MAX            = 11,
} PERFSURFACENATIVE_CMD_SET_T;

extern void PerfServiceNative_boostEnable(int scenario);
extern void PerfServiceNative_boostDisable(int scenario);
extern void PerfServiceNative_boostEnableTimeout(int scenario, int timeout);

extern int  PerfServiceNative_userReg(int scn_core, int scn_freq);
extern int  PerfServiceNative_userRegBigLittle(int scn_core_big, int scn_freq_big, int scn_core_little, int scn_freq_little);
extern void PerfServiceNative_userUnreg(int handle);

extern int  PerfServiceNative_userGetCapability(int cmd);

extern int  PerfServiceNative_userRegScn();
extern void PerfServiceNative_userRegScnConfig(int handle, int cmd, int param_1, int param_2, int param_3, int param_4);
extern void PerfServiceNative_userUnregScn(int handle);

extern void PerfServiceNative_userEnable(int handle);
extern void PerfServiceNative_userDisable(int handle);
extern void PerfServiceNative_userEnableTimeout(int handle, int timeout);

extern void PerfServiceNative_userResetAll(void);
extern void PerfServiceNative_userDisableAll(void);

extern void PerfServiceNative_dumpAll(void);

extern void PerfServiceNative_setFavorPid(int pid);


__END_DECLS

#endif // ANDROID_PERFSURFACENATIVE_H
