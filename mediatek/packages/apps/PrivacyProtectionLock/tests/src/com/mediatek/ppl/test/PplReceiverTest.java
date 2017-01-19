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

package com.mediatek.ppl.test;

import android.content.ComponentName;
import android.content.Intent;
import android.test.AndroidTestCase;
import android.test.mock.MockContext;
import android.util.Log;

import com.mediatek.ppl.PplReceiver;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.test.util.MockPplUtil;

public class PplReceiverTest extends AndroidTestCase {
    private static final String TAG = "PPL/PplReceiverTest";
    private PplReceiver mPplReceiver;
    private TestContext mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mPplReceiver = new PplReceiver();
        mContext = new TestContext();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testcase00() {
        MockPplUtil.formatLog(TAG, "test00");
        Intent intent = new Intent(Intent.ACTION_BOOT_COMPLETED);
        receiveIntent(intent);
    }

    public void testcase01() {
        MockPplUtil.formatLog(TAG, "test01");
        Intent intent = new Intent(PplService.ACTION_SIM_STATE_CHANGED);
        receiveIntent(intent);
    }

    /**
     * Check whether service is started and the intent received
     * */
    private void receiveIntent(Intent intent) {
        Log.i(TAG, "OnReceive " + intent);

        mPplReceiver.onReceive(mContext, intent);
        assertTrue(mContext.getmStateService());
        assertTrue(mContext.checkIntent(intent));
    }

    class TestContext extends MockContext {
        private Intent mIntent = null;
        private boolean mStateService = false;

        @Override
        public String getPackageName() {
            return "com.mediatek.ppl.test";
        }

        @Override
        public ComponentName startService(Intent intent) {
            mIntent = intent;
            mStateService = true;
            return new ComponentName("com.mediatek.ppl", "PplService");
        }

        public boolean checkIntent(Intent intent) {
            return mIntent.equals(intent);
        }

        public boolean getmStateService() {
            return mStateService;
        }
    }

}