
package com.mediatek.mom.test.function.rec;

public class EmulatorManagerOps{
    // Basic Information
    public static final String EMUMGR_CLASS = "com.mediatek.mom.test.app.mgremu.ManagerEmulator";
    public static final String EMUMGR_PACKAGE = "com.mediatek.mom.test.app.mgremu";
    public static final String APK_LOCATION = "/sdcard/MoMS";
    public static final String CTA_APK = "MTK_CtaTestAPK_KK.apk";
    public static final String CTA_PACKAGE = "com.mediatek.cta";
    public static final String CTA_CLASS = "com.mediatek.cta.CtaActivity";
    public static final String RESULT_INTENT = "com.mediatek.mom.test.app.mgremu.operation.done";

    // Result code
    public static final int RESULT_NONE = -1;
    public static final int RESULT_SUCCESS = 1;
    public static final int RESULT_FAILED = 0;

    // Emualted manager operations
    public static final int MGR_OP_NONE = 0;
    public static final int MGR_OP_REC_OFFSET = 100;
    public static final int MGR_OP_REC_GET_RECEIVER_LIST = MGR_OP_REC_OFFSET;
    public static final int MGR_OP_REC_SET_RECORD = MGR_OP_REC_OFFSET + 1;
    public static final int MGR_OP_REC_GET_RECORD = MGR_OP_REC_OFFSET + 2;
    public static final int MGR_OP_REC_FILTER_RECEIVER = MGR_OP_REC_OFFSET + 3;
    public static final int MGR_OP_REC_START_MONITOR = MGR_OP_REC_OFFSET + 4;
    public static final int MGR_OP_REC_STOP_MONITOR = MGR_OP_REC_OFFSET + 5;
}
