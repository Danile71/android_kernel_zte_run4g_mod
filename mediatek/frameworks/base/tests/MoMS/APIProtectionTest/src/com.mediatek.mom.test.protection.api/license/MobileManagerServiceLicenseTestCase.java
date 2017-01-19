/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.mom.test.protection.api;

import android.test.AndroidTestCase;
import android.content.Context;
import android.os.IBinder;
import android.os.ServiceManager;
import com.mediatek.common.mom.IMobileManagerService;

public class MobileManagerServiceLicenseTestCase extends AndroidTestCase {
    private IMobileManagerService mMobileManagerService = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        IBinder binder = (IBinder) ServiceManager.getService(Context.MOBILE_SERVICE);
        assertNotNull(binder);
        mMobileManagerService = (IMobileManagerService)IMobileManagerService.Stub.asInterface(binder);
        assertNotNull(mMobileManagerService);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        mMobileManagerService = null;
    }

    /**
     * The following APIs should be protected by enforceCallingUid()
     */
    public void testTriggerManagerApListener() throws Exception {
        try {
            mMobileManagerService.triggerManagerApListener(0, null, 0);
            fail("triggerManagerApListener() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testTriggerManagerApListenerAsync() throws Exception {
        try {
            mMobileManagerService.triggerManagerApListenerAsync(0, null, 0, null);
            fail("triggerManagerApListenerAsync() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetUserConfirmTime() throws Exception {
        try {
            mMobileManagerService.getUserConfirmTime(0, 1000);
            fail("getUserConfirmTime() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetInterceptionEnabledSetting() throws Exception {
        try {
            mMobileManagerService.getInterceptionEnabledSetting();
            fail("getInterceptionEnabledSetting() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testFilterReceiver() throws Exception {
        try {
            mMobileManagerService.filterReceiver(null, null, 0);
            fail("testFilterReceiver() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testStartMonitorBootReceiver() throws Exception {
        try {
            mMobileManagerService.startMonitorBootReceiver(null);
            fail("testStartMonitorBootReceiver() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testStopMonitorBootReceiver() throws Exception {
        try {
            mMobileManagerService.stopMonitorBootReceiver(null);
            fail("testStopMonitorBootReceiver() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }
}
