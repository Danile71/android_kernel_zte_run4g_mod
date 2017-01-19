
package com.mediatek.mom.test.function.pmc;

public class EmulatorManagerOps{
    // Basic Information
    public static final String EMUMGR_CLASS = "com.mediatek.mom.test.app.mgremu.ManagerEmulator";
    public static final String EMUMGR_PACKAGE = "com.mediatek.mom.test.app.mgremu";
    public static final String APK_LOCATION = "/sdcard/MoMS";
    public static final String CTA_APK = "MTK_CtaTestAPK_KK.apk";
    public static final String CTA_PACKAGE = "com.mediatek.cta";
    public static final String RESULT_INTENT = "com.mediatek.mom.test.app.mgremu.operation.done";

    // Result code
    public static final int RESULT_NONE = -1;
    public static final int RESULT_SUCCESS = 1;
    public static final int RESULT_FAILED = 0;
    public static final int RESULT_WAIT = -999;
    public static final int RESULT_TIMEOUT = -998;

    // Emualted manager operations
    public static final int MGR_OP_NONE = 0;
    public static final int MGR_OP_CB_TIMEOUT = -1;
    // Permission Controller (PMC)
    public static final int MGR_OP_PMC_ATTACH = 1;  
    public static final int MGR_OP_PMC_ATTACH_NULL_CB = 2;
    public static final int MGR_OP_PMC_DETACH = 3;
    public static final int MGR_OP_PMC_REGISTER_CB = 4;
    public static final int MGR_OP_PMC_ENABLE_CONTROLLER = 5;
    public static final int MGR_OP_PMC_SET_RECORD = 6;
    public static final int MGR_OP_PMC_GET_RECORD = 7;
    public static final int MGR_OP_PMC_GET_INSTALLED_PACKAGES = 8;
    public static final int MGR_OP_PMC_CHECK_PERMISSION = 9;
    public static final int MGR_OP_PMC_CHECK_PERMISSION_ASYNC = 10;
    public static final int MGR_OP_PMC_CB_CONNECTION_ENDED = 11;
    public static final int MGR_OP_PMC_CB_CONNECTION_RESUME = 12;
    public static final int MGR_OP_PMC_CB_PERMISSION_CHECK = 13;

}
