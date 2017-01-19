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

package com.mediatek.keyguard.plugin;

import android.app.AppGlobals ;
import android.app.KeyguardManager;
import android.content.Context;
import android.content.pm.IPackageManager ;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.RemoteException;
import android.os.SystemProperties ;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManagerPolicy.OnKeyguardExitResult;
import android.view.WindowManager;
import android.view.WindowManagerGlobal;

import com.android.internal.policy.impl.keyguard.DgilForLockscreen;

import com.mediatek.keyguard.ext.DefaultLockScreenExt;
import com.mediatek.keyguard.ext.ILockScreenExt;
import com.mediatek.xlog.Xlog;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

public class OP03LockScreenExt extends DefaultLockScreenExt implements DgilForLockscreen.Callback {
    private static final String TAG = "LockScreenExt";
    private Context mContext = null;
		private DgilForLockscreen mDgilForLockscreen = null;
		
		
		public OP03LockScreenExt() {
				Xlog.d(TAG, "Default Constructor called ");
		}
		
		public OP03LockScreenExt(Context context) {
				Xlog.d(TAG, "Constructor called with argument");
		    mContext = context;
		}
		
		@Override
    public void unlock() {
        Xlog.d(TAG, "DgilLayout ask to unlock");
        KeyguardManager mKeyguardManager;
        mKeyguardManager = (KeyguardManager) mContext.getSystemService(Context.KEYGUARD_SERVICE);
        if (!mKeyguardManager.isKeyguardSecure()) {
            Xlog.d(TAG, "verify unlock called to framework");
            try {
                WindowManagerGlobal.getWindowManagerService().dismissKeyguard();
            } catch (RemoteException e) {
                Xlog.d(TAG, "Exception found in Unlock");                            
            }
            notifyUnlockedScreen();
            
            
        } 
    }
    
    public boolean disableSearch(Context context) {
        Xlog.d(TAG, "disableSearch called");
        try {
            boolean isInstalled = false;
            PackageManager pm = context.getPackageManager();
            isInstalled = pm.getApplicationInfo("com.orange.lockscreen", 0) != null;
            Xlog.d(TAG, "isInstalled "+isInstalled);
            return isInstalled;
        } catch (NameNotFoundException e) {
            Xlog.d(TAG, "returning false");
            return false;
        }
    }
    
		public void notifyUnlockedScreen() {
				Xlog.d(TAG, "notifyUnlockedScreen called");
				if(mDgilForLockscreen!=null) {
            mDgilForLockscreen.notifyUnlockedScreen();
        }
		}
		
		public void initDgilLayout(int currentMode, int mode, ViewGroup hostView ) {              
        Xlog.d(TAG, "initDgilLayout called");
        if (currentMode != mode) {
            try {
                 if ("eng".equals(SystemProperties.get("ro.build.type"))) {
                    IPackageManager ipm = AppGlobals.getPackageManager();
                    try {
                        //creating cache for Orange Lockscreen
                        ipm.enforceDexOpt("com.orange.lockscreen");
                     } catch (RemoteException e) {
                         Xlog.e(TAG, "performDexOpt exception occur");
                     }
                }

                Context lockscreenContext = mContext.createPackageContext("com.orange.lockscreen", Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                Class<?> lmClass = lockscreenContext.getClassLoader().loadClass("com.orange.lockscreen.LockscreenManager");
                Xlog.d(TAG, "got orange class "+lmClass);
                Method method = lmClass.getDeclaredMethod("getDgilForLockscreen", Context.class, DgilForLockscreen.Callback.class);
                Xlog.d(TAG, "got orange method "+method);
                mDgilForLockscreen = (DgilForLockscreen) method.invoke(null, lockscreenContext, this);
							  Xlog.d(TAG, "got orange interface instance"+mDgilForLockscreen);
							  
                if (mDgilForLockscreen != null) {
                    List<View> children = new ArrayList<View>();
                    for (int i = 0; i < hostView.getChildCount(); i++) {
                        View child = hostView.getChildAt(i);
                        hostView.removeView(child);
                        children.add(child);
                    }
                    ViewGroup dgilLayout = mDgilForLockscreen.getDgillayout();
                    Xlog.d(TAG, "Got dgil layout " + dgilLayout);
                    hostView.addView(dgilLayout);
                    for (View child : children) {
                        dgilLayout.addView(child);
                    }
                    Xlog.d(TAG, "view added " +dgilLayout );
                }

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
