package com.mediatek.common.perfservice;

//import android.os.Bundle;

public interface IPerfServiceWrapper {

    public static final int SCN_NONE       = 0;
    public static final int SCN_APP_SWITCH = 1; /* apply for both launch/exit */
    public static final int SCN_APP_ROTATE = 2;
    public static final int SCN_SW_CODEC   = 3;
    public static final int SCN_SW_CODEC_BOOST = 4;
    public static final int SCN_APP_TOUCH      = 5;
    public static final int SCN_DONT_USE1      = 6;

    public static final int STATE_PAUSED    = 0;
    public static final int STATE_RESUMED   = 1;
    public static final int STATE_DESTROYED = 2;
    public static final int STATE_DEAD      = 3;

    public static final int CMD_GET_CPU_FREQ_LEVEL_COUNT        = 0;
    public static final int CMD_GET_CPU_FREQ_LITTLE_LEVEL_COUNT = 1;
    public static final int CMD_GET_CPU_FREQ_BIG_LEVEL_COUNT    = 2;
    public static final int CMD_GET_GPU_FREQ_LEVEL_COUNT        = 3;
    public static final int CMD_GET_MEM_FREQ_LEVEL_COUNT        = 4;

    public static final int CMD_SET_CPU_CORE_MIN            = 0;
    public static final int CMD_SET_CPU_CORE_MAX            = 1;
    public static final int CMD_SET_CPU_CORE_BIG_LITTLE_MIN = 2;
    public static final int CMD_SET_CPU_CORE_BIG_LITTLE_MAX = 3;
    public static final int CMD_SET_CPU_FREQ_MIN            = 4;
    public static final int CMD_SET_CPU_FREQ_MAX            = 5;
    public static final int CMD_SET_CPU_FREQ_BIG_LITTLE_MIN = 6;
    public static final int CMD_SET_CPU_FREQ_BIG_LITTLE_MAX = 7;
    public static final int CMD_SET_GPU_FREQ_MIN            = 8;
    public static final int CMD_SET_GPU_FREQ_MAX            = 9;
    public static final int CMD_SET_MEM_FREQ_MIN            = 10;
    public static final int CMD_SET_MEM_FREQ_MAX            = 11;

    public void boostEnable(int scenario);
    public void boostDisable(int scenario);
    public void boostEnableTimeout(int scenario, int timeout);
    public void notifyAppState(String packName, String className, int state);

    public int  userReg(int scn_core, int scn_freq);
    public int  userRegBigLittle(int scn_core_big, int scn_freq_big, int scn_core_little, int scn_freq_little);
    public void userUnreg(int handle);

    public int  userGetCapability(int cmd);

    public int  userRegScn();
    public void userRegScnConfig(int handle, int cmd, int param_1, int param_2, int param_3, int param_4);
    public void userUnregScn(int handle);

    public void userEnable(int handle);
    public void userEnableTimeout(int handle, int timeout);
    public void userDisable(int handle);

    public void userResetAll();
    public void userDisableAll();

    public void dumpAll();

    public void setFavorPid(int pid);
}
