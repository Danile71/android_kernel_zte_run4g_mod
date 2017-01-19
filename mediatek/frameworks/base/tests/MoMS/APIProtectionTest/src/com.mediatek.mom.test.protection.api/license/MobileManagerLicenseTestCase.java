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

package com.mediatek.mom.test.protection.api.license;

import android.test.AndroidTestCase;
import android.content.Context;
import com.mediatek.common.mom.IMobileManager;

public class MobileManagerLicenseTestCase extends AndroidTestCase {
    private IMobileManager mMobileManager = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMobileManager = (IMobileManager) mContext.getSystemService(Context.MOBILE_SERVICE);
        assertNotNull(mMobileManager);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        mMobileManager = null;
    }

    /**
     * The following APIs should be protected by enforceCallingUid()
     */
    public void testGetNotificationEnabledSetting() throws Exception {
        try {
            mMobileManager.setNotificationEnabledSetting("test package", false);
            fail("setNotificationEnabledSetting() should be protected by enforceCallingUid!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    /**
     * The following APIs should be protected by enforceCheckLicense()
     */
    public void testGetVersionName() throws Exception {
        try {
             mMobileManager.getVersionName();
             fail("getVersionName() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testClearAllSettings() throws Exception {
        try {
            mMobileManager.clearAllSettings();
            fail("clearAllSettings() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testClearPackageSettings() throws Exception {
        try {
            mMobileManager.clearPackageSettings("test package");
            fail("clearPackageSettings() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testAttach() throws Exception {
        try {
            mMobileManager.attach(null);
            fail("attach() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetInstalledPackages() throws Exception {
        try {
            mMobileManager.getInstalledPackages();
            fail("getInstalledPackages() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetPackageGrantedPermissions() throws Exception {
        try {
            mMobileManager.getPackageGrantedPermissions("test package");
            fail("getPackageGrantedPermissions() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetBootReceiverEnabledSetting() throws Exception {
        try {
            mMobileManager.setBootReceiverEnabledSetting("test package", false);
            fail("setBootReceiverEnabledSetting() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetBootReceiverEnabledSettings() throws Exception {
        try {
            mMobileManager.setBootReceiverEnabledSettings(null);
            fail("testSetBootReceiverEnabledSettings() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetBootReceiverList() throws Exception {
        try {
            mMobileManager.getBootReceiverList();
            fail("testGetBootReceiverList() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetBootReceiverEnabledSetting() throws Exception {
        try {
            mMobileManager.getBootReceiverEnabledSetting("test package");
            fail("getBootReceiverEnabledSetting() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testForceStopPackage() throws Exception {
        try {
            mMobileManager.forceStopPackage("test package");
            fail("forceStopPackage() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testInstallPackage() throws Exception {
        try {
            mMobileManager.installPackage(null, null);
            fail("installPackage() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testDeletePackage() throws Exception {
        try {
            mMobileManager.deletePackage("test package");
            fail("deletePackage() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testCancelNotification() throws Exception {
        try {
            mMobileManager.cancelNotification("test package");
            fail("cancelNotification() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetNotificationEnabledSetting() throws Exception {
        try {
            mMobileManager.setNotificationEnabledSetting("test package", false);
            fail("setNotificationEnabledSetting() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testRegisterNotificationListener() throws Exception {
        try {
            mMobileManager.registerNotificationListener(null);
            fail("registerNotificationListener() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetNotificationCache() throws Exception {
        try {
            mMobileManager.setNotificationCache(null);
            fail("setNotificationCache() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testEnableInterceptionController() throws Exception {
        try {
            mMobileManager.enableInterceptionController(false);
            fail("enableInterceptionController() should be protected by enforceCheckLicense!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    /**
     * The following APIs should be protected by enforceCheckAttachment()
     */
    public void testDetach() throws Exception {
        try {
            mMobileManager.detach();
            fail("detach() should be protected by enforceCheckAttachment!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testRegisterPermissionListener() throws Exception {
        try {
            mMobileManager.registerPermissionListener(null);
            fail("registerPermissionListener() should be protected by enforceCheckAttachment!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testEnablePermissionController() throws Exception {
        try {
            mMobileManager.enablePermissionController(false);
            fail("enablePermissionController() should be protected by enforceCheckAttachment!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetPermissionRecord() throws Exception {
        try {
            mMobileManager.setPermissionRecord(null);
            fail("setPermissionRecord() should be protected by enforceCheckAttachment!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetPermissionRecords() throws Exception {
        try {
            mMobileManager.setPermissionRecords(null);
            fail("setPermissionRecords() should be protected by enforceCheckAttachment!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSetPermissionCache() throws Exception {
        try {
            mMobileManager.setPermissionCache(null);
            fail("setPermissionCache() should be protected by enforceCheckAttachment!");
        } catch (SecurityException e) {
            // Expected
        }
    }
}
