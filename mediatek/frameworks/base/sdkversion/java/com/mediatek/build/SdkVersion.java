/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.build;

import android.os.SystemProperties;

/**
 * This class gets MediaTek SDK version (API level) of the application and device, 
 * and reports on whether the API level of the SDK used to create the application is compatible with the API level on the device.
 */
public final class SdkVersion {
    /**
    * MediaTek SDK version on application.
    */
    private static final int APP_SDK_VERSION = 3;
    private SdkVersion() {
       // private constructor;
    }
    /**
    * Reports on whether the MediaTek SDK version used to create the application is compatible with the device's platform.
    * Where true is returned the device offer support for the same or higher MediaTek SDK version, where false is returned 
    * the application’s MediaTek SDK version is higher than the device’s platform. Where false is returned, the application may still be 
    * compatible with the device, but the MediaTek Compatibility API will need to be used to check for the availability of 
    * APIs used in the application. See the <a href="http://labs.mediatek.com/fileMedia/download/5f1471ad-ff1d-4fea-9828-0bc69c63b3e3"> Android Developer’s Guide</a>
    * for more information on the MediaTek Compatibility API.
    *
    * @return true or false.
    */
    public static boolean isCompatible() {
       return (getAppVersion() > getPlatformVersion()) ? false : true;
    }
    /**
    * Gets MediaTek SDK version of the application.
    *
    * @return MediaTek SDK version of the application.
    */
    public static int getAppVersion() {
       return APP_SDK_VERSION;
    }
    /**
    * Gets MediaTek SDK version of the device's platform.
    *
    * @return MediaTek SDK version of the device's platform.
    */
    public static int getPlatformVersion() {
       return Integer.valueOf(SystemProperties.get("ro.mediatek.version.sdk"));
    }
}
