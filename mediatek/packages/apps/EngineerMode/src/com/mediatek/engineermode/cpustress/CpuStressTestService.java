/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.cpustress;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;
import com.mediatek.xlog.Xlog;

import java.io.File;

public class CpuStressTestService extends Service {

    private static final String TAG = "EM/CpuStressTestService";

    protected static final String THERMAL_ETC_FILE = "/etc/.tp/.ht120.mtc";
    private static final int INDEX_TEST_APMCU = 1;
    private static final int INDEX_TEST_VIDEOCODEC = 2;
    private static final int INDEX_TEST_BACKUP = 3;
    private static final int INDEX_TEST_RESTORE = 4;
    private static final int INDEX_TEST_FORCE_CORES = 5;
    private static final String HANDLER_THREAD_NAME_APMCU = "ApMcu";
    private static final String HANDLER_THREAD_NAME_VIDEO = "VideoCodec";
    private static final String HANDLER_THREAD_NAME_BACKUPRESTORE = "BackupRestore";
    
    private static final String KEY_TEST_REQUEST = "TEST_REQUEST";
    public static final int FORCE_CORE_BIT_LEN = 4;
    public static final int CPU_ARCH_NORMAL = 0;
    public static final int CPU_ARCH_BIG_LITTLE = 3;
    public static final int CPU_BITS_32 = 32;
    public static final int CPU_BITS_64 = 64;
    public static final int MODE_BACKUP_RESTORE = 1;
    public static final int MODE_CHECK_CUSTOM = 2;
    public static final int MODE_CUSTOM_V2 = 3;
    public static final int INDEX_ENABLE_TEST_MODE = 1;
    public static final int INDEX_DISABLE_TEST_MODE = 2;
    public static final int INDEX_CHECK_CUSTOM = 3;

    public static final String VALUE_RUN = "run";
    public static final String VALUE_LOOPCOUNT = "loopcount";
    public static final String VALUE_ITERATION = "iteration";
    public static final String VALUE_MASK = "mask";
    public static final String VALUE_RESULT = "result";
    public static final String RESULT_NEON = "result_neon";
    public static final String RESULT_PASS_NEON = "result_pass_neon";
    public static final String RESULT_CA9 = "result_ca9";
    public static final String RESULT_PASS_CA9 = "result_pass_ca9";
    public static final String RESULT_DHRY = "result_dhry";
    public static final String RESULT_PASS_DHRY = "result_pass_dhry";
    public static final String RESULT_MEMCPY = "result_memcpy";
    public static final String RESULT_PASS_MEMCPY = "result_pass_memcpy";
    public static final String RESULT_FDCT = "result_fdct";
    public static final String RESULT_PASS_FDCT = "result_pass_fdct";
    public static final String RESULT_IMDCT = "result_imdct";
    public static final String RESULT_PASS_IMDCT = "result_pass_imdct";
    public static final String RESULT_VIDEOCODEC = "result_video_codec";
    public static final String RESULT_PASS_VIDEOCODEC = "result_pass_video_codec";

    private static final String PASS = "PASS";
    private static final String FAIL = "FAIL";
    private static final String SKIP = "is powered off";
    private static final String PASS_89 = "Frame #1950";
    private static final String SKIP_89 = "Frame #";
    private static final String RESULT_ERROR = "ERROR";
    private static final String TAG_NO_TEST = "$[NO_TEST)#";

    private static final int TIME_DELAYED = 100;
    private static final long LOOPCOUNT_DEFAULT_VALUE = 99999999;
    protected static final int CORE_NUM_MASK = 48;
    private static final String CPU_1_ONLINE_PATH = "/sys/devices/system/cpu/cpu1/online";
    private static final String CPU_2_ONLINE_PATH = "/sys/devices/system/cpu/cpu2/online";
    private static final String CPU_3_ONLINE_PATH = "/sys/devices/system/cpu/cpu3/online";
    private static final String CPU_7_ONLINE_PATH = "/sys/devices/system/cpu/cpu7/online";

    private static final String RESULT_SEPARATE = ";";

    public static final int CORE_NUMBER_8 = 8;
    public static final int CORE_NUMBER_4 = 4;
    public static final int CORE_NUMBER_3 = 3;
    public static final int CORE_NUMBER_2 = 2;
    public static final int CORE_NUMBER_1 = 1;

    public static int sCoreNumber = 0;
    public static int sCpuArch = CPU_ARCH_NORMAL;
    public static int sCpuBits = CPU_BITS_32;
    protected static boolean sIsThermalSupport = false;
    protected static boolean sIsThermalDisabled = false;
    protected static int sIndexMode = 0;

    private boolean mTestClockSwitchRunning = false;

    protected ICpuStressTestComplete mTestClass = null;

    private WakeLock mWakeLock = null;

    private final StressTestBinder mBinder = new StressTestBinder();
    private final HandlerThread mHandlerThreadApMcu = new HandlerThread(
            HANDLER_THREAD_NAME_APMCU);
    private final HandlerThread mHandlerThreadVideoCodec = new HandlerThread(
            HANDLER_THREAD_NAME_VIDEO);
    private final HandlerThread mHandlerThreadBackupRestore = new HandlerThread(
            HANDLER_THREAD_NAME_BACKUPRESTORE);
    private HandlerApMcu mHandlerApMcu = null;
    private HandlerVideoCodec mHandlerVideoCodec = null;
    private HandlerBackupRestore mHandlerBackupRestore = null;
    
    private CpuTestRequest mForceCoreReqRecords = null;
    private static int sCpuTestMode;
    private TestDataSet mApmcuTestDatas = null;
    private TestDataSet mSwVideoTestDatas = null;
    
    static {
        initCpuInfo();
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }
    
    public static int getCpuTestMode() {
        return sCpuTestMode;
    }
    
    private static void initCpuInfo() {
        sCoreNumber = coreNum();
        sCpuArch = CPU_ARCH_NORMAL;
        sCpuBits = CPU_BITS_32;
        sCpuTestMode = MODE_BACKUP_RESTORE;
        if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6595_SUPPORT)) {
            if (sCoreNumber == 8) {
                sCpuTestMode = MODE_CHECK_CUSTOM;
                sCpuArch = CPU_ARCH_BIG_LITTLE;
            }
        }
        if (ChipSupport.isCurrentChipEquals(ChipSupport.MTK_6752_SUPPORT)) {
            sCpuBits = CPU_BITS_64;
            sCpuTestMode = MODE_CUSTOM_V2;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        int coreNumber = sCoreNumber;
        sIsThermalSupport = new File(THERMAL_ETC_FILE).exists();
        mWakeLock = new WakeLock();
        try {
            mHandlerThreadApMcu.start();
            mHandlerThreadVideoCodec.start();
            mHandlerThreadBackupRestore.start();
            mHandlerApMcu = new HandlerApMcu(mHandlerThreadApMcu.getLooper());
            mHandlerVideoCodec = new HandlerVideoCodec(mHandlerThreadVideoCodec
                    .getLooper());
            mHandlerBackupRestore = new HandlerBackupRestore(
                    mHandlerThreadBackupRestore.getLooper());
        } catch (IllegalThreadStateException e) {
            Xlog.w(TAG, "Handler thread IllegalThreadStateException: "
                    + e.getMessage());
            Toast.makeText(this,
                    R.string.hqa_cpustress_test_toast_threadhandler_error,
                    Toast.LENGTH_LONG).show();
        }
        Xlog.i(TAG, "Core Number: " + coreNumber);
        runCmdInNative(AFMFunctionCallEx.FUNCTION_EM_CPU_STRESS_TEST_INIT, 0);
        mForceCoreReqRecords = createCpuTestRequest(FORCE_CORE_BIT_LEN);
        mApmcuTestDatas = new TestDataSet(ApMcu.class.getSimpleName());
        mApmcuTestDatas.leftTestCount = LOOPCOUNT_DEFAULT_VALUE;
        mApmcuTestDatas.addPatchTestData(ApMcu.APMCU_TEST_INDEXS, sCoreNumber);
        mSwVideoTestDatas = new TestDataSet(SwVideoCodec.class.getSimpleName());
        mSwVideoTestDatas.leftTestCount = LOOPCOUNT_DEFAULT_VALUE;
        mSwVideoTestDatas.addTestData(new TestData(SwVideoCodec.INDX_SWCODEC, sCoreNumber));
    }
    
    CpuTestRequest getForceCoreReqRecord(boolean enabled) {
        return getForceCoreReq(mForceCoreReqRecords, enabled);
    }
    
    /**
     *  get Enable or disable request from provided request
     *  note: the request should only record enable test core; 
     *  so if enabled is true; return itself
     *  otherwise, set the enabled test core to disable 
     *  and return a new request with disable record
     * @param request
     *     provided request
     * @param enabled
     *     whether enabled or not
     * @return
     *     return enabled or disabled request
     */
    CpuTestRequest getForceCoreReq(CpuTestRequest request, boolean enabled) {
        CpuTestRequest targetReq = null;
        if (enabled) {
            targetReq = request;
        } else {
            if (!request.isSetCpuTestCore()) {
                targetReq = request;
            } else {
                targetReq = createCpuTestRequest(FORCE_CORE_BIT_LEN);
                for (int i = 0; i < sCoreNumber; i++) {
                    int val = request.getCpuTestCore(i);
                    if (val > 0 && val < CpuStressTest.DISABLE_FORCE_CORE) {
                        targetReq.setCpuTestCore(i, CpuStressTest.DISABLE_FORCE_CORE);
                    }
                }
            }
        }
        return targetReq;
    }
    
    CpuTestRequest createCpuTestRequest(int bitLen) {
        CpuTestRequest request = new CpuTestRequest(sCoreNumber, sCpuArch, sCpuBits, bitLen);
        return request;
    }
    
    @Override
    public void onDestroy() {
        restore(sIndexMode);
        mHandlerThreadApMcu.quit();
        mHandlerThreadVideoCodec.quit();
        mHandlerThreadBackupRestore.quit();
        mWakeLock.release();
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Xlog.v(TAG, "Enter onStartCommand");
        return START_NOT_STICKY;
    }
    
    public void startApmcuTest(TestDataSet testDataSet) {
        Xlog.v(TAG, "Enter startApmcuTest");
        if (mTestClass instanceof ApMcu) {
            Xlog.v(TAG, "startTest for ApMcu");
            if (mApmcuTestDatas.running) {
                Xlog.v(TAG, "ApMpu test is running");
                return;
            }
            mApmcuTestDatas.running = true;
            mApmcuTestDatas.wantStop = false;
            for (TestData testData : mApmcuTestDatas.getTestDataCollect()) {
                testData.testedCount = 0;
                testData.passedCount = 0;
                testData.clearTestResult();
            }
            updateWakeLock();
            mHandlerApMcu.sendEmptyMessage(INDEX_TEST_APMCU);
        } else {
            Xlog.d(TAG, "invalid testClass:" + mTestClass.getClass().getName());
        }
    }

    /**
     * Start test use configuration data
     * 
     * @param data
     *            Test configuration data
     */
    public void startTest(Bundle data) {
        Xlog.v(TAG, "Enter startTest");
        if (mTestClass instanceof ClockSwitch) {
            Xlog.v(TAG, "startTest for ClockSwitch");
            mTestClockSwitchRunning = true;
            updateWakeLock();
        }
    }
    
    public void startSwCodecTest() {
        Xlog.d(TAG, "Enter startSwCodecTest");
        if (!(mTestClass instanceof SwVideoCodec)) {
            throw new RuntimeException("Wrongly use this api, only for SwVideoCodec");
        }
        mSwVideoTestDatas.running = true;
        mSwVideoTestDatas.wantStop = false;
        for (TestData testData : mSwVideoTestDatas.getTestDataCollect()) {
            testData.passedCount = 0;
            testData.testedCount = 0;
            testData.clearTestResult();
        }
        updateWakeLock();
        mHandlerVideoCodec.sendEmptyMessage(INDEX_TEST_VIDEOCODEC);
    }

    /**
     * Invoked when press "stop" button to stop the test
     */
    public void stopTest() {
        Xlog.v(TAG, "Enter stopTest, testObject is: " + mTestClass);
        if (mTestClass instanceof ApMcu) {
            Xlog.v(TAG, "stopTest for ApMcu");
            // bRunApMcu = false;
            mApmcuTestDatas.wantStop = true;
        } else if (mTestClass instanceof SwVideoCodec) {
            Xlog.v(TAG, "stopTest for SwVideoCodec");
            // bRunVideoCodec = false;
            mSwVideoTestDatas.wantStop = true;
        } else if (mTestClass instanceof ClockSwitch) {
            Xlog.v(TAG, "stopTest for ClockSwitch");
            mTestClockSwitchRunning = false;
        }
        // updateWakeLock();
    }

    /**
     * Do backup and restore
     * 
     * @param index
     *            Index to do different branch
     */
    private void doBackupRestore(int index) {
        Xlog.v(TAG, "Enter doBackupRestore: " + index);
        CpuTestRequest request = createCpuTestRequest(0);
        request.setTestData(new int[]{MODE_BACKUP_RESTORE, index});
        int[] packet = request.packRequest(CpuTestRequest.FLAG_PACK_TEST_DATA);
        String result = runCmdInNative(
                AFMFunctionCallEx.FUNCTION_EM_CPU_STRESS_TEST_BACKUP, packet.length, packet);
        Xlog.v(TAG, "doBackupRestore: " + result);
    }
    
    private void doForceCoresTest(CpuTestRequest request) {
        Xlog.d(TAG, "start doForceCoresTest" );
        Xlog.d(TAG, " request:" + request.toString());
        int[] packet = request.packRequest(CpuTestRequest.FLAG_PACK_ALL);
        String result = runCmdInNative(AFMFunctionCallEx.FUNCTION_EM_CPU_STRESS_TEST_BACKUP, packet.length, packet);
        Xlog.d(TAG, "doForceCoresTest:" + result);
    }

    /**
     * Do ApMcu test, invoked in #HandlerThread
     */
    private void doApMcuTest() {
        Xlog.v(TAG, "enter doApMpuTest");
        int testCoreNumber = CORE_NUMBER_1;
        switch (sIndexMode) {
        case CpuStressTest.INDEX_SINGLE:
            testCoreNumber = CORE_NUMBER_1;
            break;
        case CpuStressTest.INDEX_DUAL:
            testCoreNumber = CORE_NUMBER_2;
            break;
        case CpuStressTest.INDEX_TRIPLE:
            testCoreNumber = CORE_NUMBER_3;
            break;
        case CpuStressTest.INDEX_QUAD:
            testCoreNumber = CORE_NUMBER_4;
            break;
        case CpuStressTest.INDEX_OCTA:
            testCoreNumber = CORE_NUMBER_8;
            break;
        case CpuStressTest.INDEX_TEST:
            testCoreNumber = sCoreNumber;
            break;
        default:
            break;
        }

        TestDataSet testDataSet = mApmcuTestDatas;
        for (int i = 0; i < ApMcu.APMCU_TEST_INDEXS.length; i++) {
            int index = ApMcu.APMCU_TEST_INDEXS[i];
            TestData testData = testDataSet.getTestData(index);
            testData.clearTestResult();
            Xlog.d(TAG, "doApMcuTest: index:" + index + " enabled:" + testData.enabled);
            if (testData.enabled) {
                doApMcuTest(index, testCoreNumber);
            }
        }
    }

    /**
     * Do ApMcu test
     * 
     * @param index
     *            Different test branch
     */
    private void doApMcuTest(int index, int coreNumber) {
        Xlog.v(TAG, "doApMpuTest index is: " + index);
        int mode = getCpuTestMode();
        int[] packs = null;
        
        if (mode == MODE_BACKUP_RESTORE) {
            CpuTestRequest request = createCpuTestRequest(0);
            request.setTestData(new int[]{MODE_BACKUP_RESTORE, index, coreNumber});
            packs = request.packRequest(CpuTestRequest.FLAG_PACK_TEST_DATA);
        } else if (mode == MODE_CHECK_CUSTOM || mode == MODE_CUSTOM_V2) {
            CpuTestRequest request = getForceCoreReqRecord(true).copy(CpuTestRequest.FLAG_PACK_CPU_INFO);
            request.setTestData(new int[]{sCpuTestMode, index});
            packs = request.packRequest(CpuTestRequest.FLAG_PACK_ALL);
            Xlog.d(TAG, " DUMP reqeust:" +request.toString());
        } else {
            throw new RuntimeException("doApMcuTest unhandled mode:" + mode);
        }
        String response = runCmdInNative(
                AFMFunctionCallEx.FUNCTION_EM_CPU_STRESS_TEST_APMCU, packs.length, packs);
        Xlog.v(TAG, "doApMcuTest response: " + response);
        if (null == response) {
            return;
        }
        handleApmcuTestResult(response, index);
    }
    
    private void handleApmcuTestResult(String response, int index) {
        Xlog.d(TAG, "handleApmcuTestResult: index:" + index);
        TestData testData = mApmcuTestDatas.getTestData(index);
        if (testData == null) {
            Xlog.d(TAG, "handleApmcuTestResult: unknown index:" + index);
            return;
        }
        testData.testedCount++;
        if (parseApMcuTestResult(response, testData)) {
            testData.passedCount++;
        }

    }

    private boolean parseApMcuTestResult(String response, TestData testData) {
        String[] result = response.split(RESULT_SEPARATE, -1);
        
        boolean bPass = true;
        StringBuilder sb = new StringBuilder();
        sb.append("result len:").append(result.length);
        if (result.length <= 0) {
            for (int i = 0; i < sCoreNumber; i++) {
                testData.setTestResult(i, TestData.TEST_FAIL);
            }
            bPass = false;
        } else {
            for (int i = 0; i < result.length; i++) {
                sb.append("\n[").append(i).append("]=").append(result[i]);
                if (result[i].contains(PASS)) {
                    testData.setTestResult(i, TestData.TEST_PASS);
                } else if (result[i].contains(SKIP)) {
                    testData.setTestResult(i, TestData.TEST_PASS);
                } else if (result[i].contains(TAG_NO_TEST)) {
                    testData.setTestResult(i, TestData.TEST_NO_TEST);
                } else {
                    bPass = false;
                    testData.setTestResult(i, TestData.TEST_FAIL);
                }
            }
        }
        Xlog.d(TAG, "parse result:" + sb.toString());
        return bPass;
    }

    private String runCmdInNative(int index, int paramNum, int... param) {
        StringBuilder build = new StringBuilder();
        AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
        boolean result = functionCall.startCallFunctionStringReturn(index);
        functionCall.writeParamNo(paramNum);
        for (int i : param) {
            functionCall.writeParamInt(i);
        }
        if (result) {
            FunctionReturn r;
            do {
                r = functionCall.getNextResult();
                if (r.mReturnString.isEmpty()) {
                    break;
                }
                build.append(r.mReturnString);
            } while (r.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
            if (r.mReturnCode == AFMFunctionCallEx.RESULT_IO_ERR) {
                Xlog.d(TAG, "AFMFunctionCallEx: RESULT_IO_ERR");
                build.replace(0, build.length(), RESULT_ERROR);
            }
        } else {
            Xlog.d(TAG, "AFMFunctionCallEx return false");
            build.append(RESULT_ERROR);
        }
        return build.toString();
    }

    /**
     * Do video codec test
     */
    private void doVideoCodecTest() {
        Xlog.v(TAG, "enter doVideoCodecTest");
        int mode = getCpuTestMode();
        int[] packs = null;
        String response = "";
        int testCoreNumber = -1;
        switch (sIndexMode) {
        case CpuStressTest.INDEX_SINGLE:
            testCoreNumber = CORE_NUMBER_1;
            break;
        case CpuStressTest.INDEX_DUAL:
            testCoreNumber = CORE_NUMBER_2;
            break;
        case CpuStressTest.INDEX_TRIPLE:
            testCoreNumber = CORE_NUMBER_3;
            break;
        case CpuStressTest.INDEX_QUAD:
            testCoreNumber = CORE_NUMBER_4;
            break;
        case CpuStressTest.INDEX_OCTA:
            testCoreNumber = CORE_NUMBER_8;
            break;
        case CpuStressTest.INDEX_TEST:
            testCoreNumber = sCoreNumber;
            break;
        case CpuStressTest.INDEX_CUSTOM:
            testCoreNumber = sCoreNumber;
            break;
        default:
            break;
        }

        if (-1 == testCoreNumber) {
            Xlog.d(TAG, "doVideoCodecTest Invalid testCoreNumber:" + testCoreNumber);
            return;
        }
        if (mode == MODE_BACKUP_RESTORE) {
            CpuTestRequest request = createCpuTestRequest(0);
            request.setTestData(new int[]{MODE_BACKUP_RESTORE, testCoreNumber, 1});
            packs = request.packRequest(CpuTestRequest.FLAG_PACK_TEST_DATA);
            Xlog.d(TAG, "doVideoCodecTest DUMP reqeust:" + request.toString());
        } else if (mode == MODE_CHECK_CUSTOM || mode == MODE_CUSTOM_V2) {
            CpuTestRequest request = getForceCoreReqRecord(true).copy(CpuTestRequest.FLAG_PACK_CPU_INFO);
            request.setTestData(new int[]{sCpuTestMode});
            packs = request.packRequest(CpuTestRequest.FLAG_PACK_ALL);
            Xlog.d(TAG, "doVideoCodecTest DUMP reqeust:" + request.toString());
        } else {
            throw new RuntimeException("doVideoCodecTest unhandled mode:" + mode);
        }
        response = runCmdInNative(
                AFMFunctionCallEx.FUNCTION_EM_CPU_STRESS_TEST_SWCODEC, packs.length, packs);
        Xlog.v(TAG, "doVideoCodecTest response: " + response);
        if (null == response) {
            return;
        }
        
        TestData testData = mSwVideoTestDatas.getTestData(SwVideoCodec.INDX_SWCODEC);
        testData.testedCount++;
        String[] resultArray = response.split(RESULT_SEPARATE, -1);
        StringBuilder sb = new StringBuilder();
        sb.append("result len:").append(resultArray.length);
        boolean bPass = true;
        if (resultArray.length > 0) {
                String passTag = PASS_89;
                String skipTag = SKIP_89;
                if (!ChipSupport.isCurrentChipHigher(ChipSupport.MTK_6589_SUPPORT, true)) {
                    passTag = PASS;
                    skipTag = SKIP;
                }
                for (int i = 0; i < resultArray.length; i++) {
                    sb.append("\n[").append(i).append("]=").append(resultArray[i]);
                    if (resultArray[i].contains(passTag)) {
                        testData.setTestResult(i, TestData.TEST_PASS);
                    } else if (resultArray[i].contains(skipTag)) {
                        testData.setTestResult(i, TestData.TEST_SKIP);
                    } else if (resultArray[i].contains(TAG_NO_TEST)) {
                        testData.setTestResult(i, TestData.TEST_NO_TEST);
                    } else {
                        testData.setTestResult(i, TestData.TEST_FAIL);
                        bPass = false;
                    }
                }
            
        } else {
            for (int i = 0; i < sCoreNumber; i++) {
                testData.setTestResult(i, TestData.TEST_FAIL);
            }
            bPass = false;
        }
        Xlog.d(TAG, "Result Array:" + sb.toString());
        if (bPass) {
            testData.passedCount++;
        }
    }
    
    public TestDataSet getTestDataSet(String name) {
        if (mApmcuTestDatas.getTestName().equals(name)) {
            return mApmcuTestDatas;
        } else if (mSwVideoTestDatas.getTestName().equals(name)) {
            return mSwVideoTestDatas;
        } else {
            Xlog.d(TAG, "unknown TestDataSet name:" + name);
        }

        return null;
    }

    /**
     * Check whether has test running
     * 
     * @return True if there is test running
     */
    public boolean isTestRun() {
        return mApmcuTestDatas.running || mSwVideoTestDatas.running
                || mTestClockSwitchRunning;
    }

    /**
     * Check whether clock switch test is running
     * 
     * @return True if clock switch test is running
     */
    public boolean isClockSwitchRun() {
        return mTestClockSwitchRunning;
    }

    /**
     * Get CPU core numbers
     * 
     * @return CPU core number
     */
    private static int coreNum() {
        if (new File(CPU_7_ONLINE_PATH).exists()) {
            return CORE_NUMBER_8;
        } else if (new File(CPU_3_ONLINE_PATH).exists()) {
            return CORE_NUMBER_4;
        } else if (new File(CPU_2_ONLINE_PATH).exists()) {
            return CORE_NUMBER_3;
        } else if (new File(CPU_1_ONLINE_PATH).exists()) {
            return CORE_NUMBER_2;
        } else {
            return CORE_NUMBER_1;
        }
    }

    public class StressTestBinder extends Binder {

        /**
         * Binder
         * 
         * @return #CpuStressTestService
         */
        CpuStressTestService getService() {
            return CpuStressTestService.this;
        }
    }

    public interface ICpuStressTestComplete {

        /**
         * Invoked when need to update test result
         */
        void onUpdateTestResult();
    }

    /**
     * Backup/restore CPU status
     * 
     * @param indexDefault
     *            Test mode index
     */
    public void setIndexMode(int indexDefault) {
        Xlog.v(TAG, "setIndexMode: " + indexDefault + " sIndexMode: "
                + sIndexMode);
        if (indexDefault == sIndexMode) {
            return;
        }
        if (sCpuTestMode == MODE_BACKUP_RESTORE) {
            if (0 == sIndexMode) {
                backup(indexDefault);
            } else if (0 == indexDefault) {
                restore(sIndexMode);
            } else {
                restore(sIndexMode);
                backup(indexDefault);
            }
        }
        synchronized (this) {
            sIndexMode = indexDefault;
        }
    }
    
    public void requestCustomForceCores(CpuTestRequest request) {
        Xlog.d(TAG, "requestCustomForceCores: new request:" + request);
        if (mForceCoreReqRecords.equals(request, CpuTestRequest.FLAG_PACK_CPU_INFO)) {
            Xlog.d(TAG, "new request equals mForceCoreReqRecords, discard it");
            return;
        }
        // first disable the last request
        CpuTestRequest lastDisableReq = getForceCoreReqRecord(false);
        if (lastDisableReq.isSetCpuTestCore()) {
            lastDisableReq.setTestData(new int[]{sCpuTestMode, CpuStressTestService.INDEX_CHECK_CUSTOM});
            setForceCoreReq(lastDisableReq);
        }
        // update record & enable current request
        mForceCoreReqRecords = request.copy(CpuTestRequest.FLAG_PACK_CPU_INFO);
        if (request.isSetCpuTestCore()) {
            request.setTestData(new int[]{sCpuTestMode, CpuStressTestService.INDEX_CHECK_CUSTOM});
            setForceCoreReq(request);
        }
    }
    
    public void requestTestManner(int index, boolean enabled) {
        Xlog.d(TAG, "requestTestManner index:" + index + " enabled:" + enabled);
        if (index == CpuStressTest.INDEX_TEST) {
            CpuTestRequest record = getForceCoreReqRecord(true);
            if (enabled) {
                for (int i = 0; i < sCoreNumber; i++) {
                    record.setCpuTestCore(i, CpuStressTest.FORCE_CORE_RUN);
                }
            } else {
                record.clearCpuTestCore();
            }
            CpuTestRequest request = createCpuTestRequest(0);
            int val = INDEX_DISABLE_TEST_MODE;
            if (enabled) {
                val = INDEX_ENABLE_TEST_MODE;
            }
            request.setTestData(new int[]{sCpuTestMode, val});
            setForceCoreReq(request);
        } else if (index == CpuStressTest.INDEX_CUSTOM) {
            if (!enabled) {
                // disable the last custom request
                CpuTestRequest request = getForceCoreReqRecord(false);
                if (request.isSetCpuTestCore()) {
                    request.setTestData(new int[]{sCpuTestMode, 
                            CpuStressTestService.INDEX_CHECK_CUSTOM});
                    setForceCoreReq(request);

                }
                getForceCoreReqRecord(true).clearCpuTestCore();
            }
        }
    }
    
    public void setForceCoreReq(CpuTestRequest request) {
        Xlog.d(TAG, "enter setForceCoreReq");
        Message msg = mHandlerBackupRestore.obtainMessage(INDEX_TEST_FORCE_CORES);
        msg.obj = request;
        mHandlerBackupRestore.sendMessage(msg);
    }

    /**
     * Backup CPU status
     * 
     * @param index
     *            Test mode index
     */
    private void backup(int index) {
        Xlog.v(TAG, "Enter backup: " + index);
        Message m = mHandlerBackupRestore.obtainMessage(INDEX_TEST_BACKUP);
        m.arg1 = index + CpuStressTest.TEST_BACKUP;
        mHandlerBackupRestore.sendMessage(m);
    }

    /**
     * Restore CPU status
     * 
     * @param index
     *            Test mode index
     */
    private void restore(int index) {
        Xlog.v(TAG, "Enter restore: " + index);
        Message m = mHandlerBackupRestore.obtainMessage(INDEX_TEST_RESTORE);
        m.arg1 = index + CpuStressTest.TEST_RESTORE;
        mHandlerBackupRestore.sendMessage(m);
    }

    /**
     * Update wake lock, auto acquire or release lock by test running
     */
    public void updateWakeLock() {
        synchronized (this) {
            if (isTestRun()) {
                mWakeLock.acquire(this);
            } else {
                mWakeLock.release();
            }
        }
    }

    class HandlerApMcu extends Handler {

        HandlerApMcu(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            Xlog.v(TAG, "mTestHandlerApMcu receive msg: " + msg.what);
            if (INDEX_TEST_APMCU == msg.what) {
                if (mApmcuTestDatas.leftTestCount <= 0) {
                    mApmcuTestDatas.leftTestCount = 0;
                    mApmcuTestDatas.running = false;
                    updateWakeLock();
                    removeMessages(INDEX_TEST_APMCU);
                } else {
                    if (mApmcuTestDatas.wantStop) {
                        mApmcuTestDatas.running = false;
                        mApmcuTestDatas.wantStop = false;
                        removeMessages(INDEX_TEST_APMCU);
                        updateWakeLock();
                    } else {
                        // lLoopCountApMcu--;
                        doApMcuTest();
                        sendEmptyMessageDelayed(INDEX_TEST_APMCU, TIME_DELAYED);
                    }
                }
            }

            if (mApmcuTestDatas.running && mApmcuTestDatas.leftTestCount > 0) {
                mApmcuTestDatas.leftTestCount--;
            }
            if (null != mTestClass) {
                if (mTestClass instanceof ApMcu
                        || mTestClass instanceof CpuStressTest) {
                    mTestClass.onUpdateTestResult();
                }
            }
            super.handleMessage(msg);
        }
    };

    class HandlerVideoCodec extends Handler {

        HandlerVideoCodec(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            Xlog.v(TAG, "mTestHandlerVideoCodec receive msg: " + msg.what);
            if (INDEX_TEST_VIDEOCODEC == msg.what) {
                if (mSwVideoTestDatas.leftTestCount <= 0) {
                    mSwVideoTestDatas.leftTestCount = 0;
                    mSwVideoTestDatas.running = false;
                    updateWakeLock();
                    removeMessages(INDEX_TEST_VIDEOCODEC);
                } else {
                    if (mSwVideoTestDatas.wantStop) {
                        mSwVideoTestDatas.running = false;
                        mSwVideoTestDatas.wantStop = false;
                        removeMessages(INDEX_TEST_VIDEOCODEC);
                        updateWakeLock();
                    } else {
                        // lLoopCountVideoCodec--;
                        doVideoCodecTest();
                        sendEmptyMessageDelayed(INDEX_TEST_VIDEOCODEC,
                                TIME_DELAYED);
                    }
                }
            }
            if (mSwVideoTestDatas.leftTestCount > 0 && mSwVideoTestDatas.running) {
                mSwVideoTestDatas.leftTestCount--;
            }
            if (null != mTestClass) {
                if (mTestClass instanceof SwVideoCodec
                        || mTestClass instanceof CpuStressTest) {
                    mTestClass.onUpdateTestResult();
                }
            }
            super.handleMessage(msg);
        }
    };

    class HandlerBackupRestore extends Handler {

        HandlerBackupRestore(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            Xlog.v(TAG, "mTestHandlerBackupRestore receive msg: " + msg.what);
            switch (msg.what) {
            case INDEX_TEST_BACKUP:
            case INDEX_TEST_RESTORE:
                doBackupRestore(msg.arg1);
                break;
            case INDEX_TEST_FORCE_CORES:
                Object data = msg.obj;
                if ((data != null) && (data instanceof CpuTestRequest)) {
                    doForceCoresTest((CpuTestRequest)data);
                }
                break;
            default:
                break;
            }
            if (null != mTestClass) {
                mTestClass.onUpdateTestResult();
            }
            super.handleMessage(msg);
        }
    };

    static class WakeLock {
        private PowerManager.WakeLock mScreenWakeLock = null;
        private PowerManager.WakeLock mCpuWakeLock = null;

        /**
         * Acquire CPU wake lock
         * 
         * @param context
         *            Global information about an application environment
         */
        void acquireCpuWakeLock(Context context) {
            Xlog.v(TAG, "Acquiring cpu wake lock");
            if (mCpuWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mCpuWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mCpuWakeLock.acquire();
        }

        /**
         * Acquire screen wake lock
         * 
         * @param context
         *            Global information about an application environment
         */
        void acquireScreenWakeLock(Context context) {
            Xlog.v(TAG, "Acquiring screen wake lock");
            if (mScreenWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mScreenWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mScreenWakeLock.acquire();
        }

        /**
         * Acquire wake lock
         * 
         * @param context
         *            Global information about an application environment
         */
        void acquire(Context context) {
            acquireScreenWakeLock(context);
            // acquireCpuWakeLock(context);
        }

        /**
         * Release wake lock
         */
        void release() {
            Xlog.v(TAG, "Releasing wake lock");
            if (mCpuWakeLock != null) {
                mCpuWakeLock.release();
                mCpuWakeLock = null;
            }
            if (mScreenWakeLock != null) {
                mScreenWakeLock.release();
                mScreenWakeLock = null;
            }
        }
    }
}
