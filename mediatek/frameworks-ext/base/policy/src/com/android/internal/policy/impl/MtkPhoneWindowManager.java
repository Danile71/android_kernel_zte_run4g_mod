/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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


package com.android.internal.policy.impl;

import android.content.Context;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Message;
import android.util.Slog;
import android.view.IWindowManager;
import android.view.WindowManager;

import com.mediatek.common.featureoption.FeatureOption;

import static android.view.WindowManager.LayoutParams.*;

/**
 * WindowManagerPolicy implementation for the Android phone UI.  This
 * introduces a new method suffix, Lp, for an internal lock of the
 * PhoneWindowManager.  This is used to protect some internal state, and
 * can be acquired with either the Lw and Li lock held, so has the restrictions
 * of both of those when held.
 */
public class MtkPhoneWindowManager extends PhoneWindowManager {
    static final String TAG = "WindowManager";

    static boolean DEBUG = false;
    static boolean DEBUG_LAYOUT = true;

    private static final int MSG_MTK_POLICY = 1000;
    private static final int MSG_ENABLE_FLOATING_MONITOR = MSG_MTK_POLICY + 0;
    private static final int MSG_DISABLE_FLOATING_MONITOR = MSG_MTK_POLICY + 1;

    Handler mHandler;

    boolean mIsDefaultDisplay;
    int mDisplayWidth, mDisplayHeight, mDisplayRotation;

    /** {@inheritDoc} */
    @Override
    public void init(Context context, IWindowManager windowManager,
            WindowManagerFuncs windowManagerFuncs) {
        super.init(context, windowManager, windowManagerFuncs);

        mMtkPhoneWindowUtility = new MtkPhoneWindowUtility(context, windowManagerFuncs);
    }

    @Override
    public void beginLayoutLw(boolean isDefaultDisplay, int displayWidth, int displayHeight,
                              int displayRotation) {
        super.beginLayoutLw(isDefaultDisplay, displayWidth, displayHeight, displayRotation);
        mIsDefaultDisplay = isDefaultDisplay;
        mDisplayWidth = displayWidth;
        mDisplayHeight = displayHeight;
        mDisplayRotation = displayRotation;
    }

    /** {@inheritDoc} */
    @Override
    public void layoutWindowLw(WindowState win, WindowManager.LayoutParams attrs,
            WindowState attached) {
        if (win == null) {
            return;
        }

        if (!win.isFloatingWindow()) {
            super.layoutWindowLw(win, attrs, attached);
        } else if (mIsDefaultDisplay) {
            final int fl = attrs.flags;
            final int sim = attrs.softInputMode;
            final int sysUiFl = win.getSystemUiVisibility();
            final int adjust = sim & SOFT_INPUT_MASK_ADJUST;

            final Rect pf = mTmpParentFrame;
            final Rect df = mTmpDisplayFrame;
            final Rect of = mTmpOverscanFrame;
            final Rect cf = mTmpContentFrame;
            final Rect vf = mTmpVisibleFrame;
            final Rect dcf = mTmpDecorFrame;
            Rect floatRegion = win.getFloatingRect(mDisplayRotation,
                    mDisplayWidth, mDisplayHeight);

            if (floatRegion == null) {
                return;
            }

            Slog.d(TAG, "layoutWindowLw floating win = " + win);

            int yOffset = 0, xOffset = 0;
            if (win.isFocusStackBox()) {
                if ((mContentBottom < mStableBottom) && (mContentBottom < floatRegion.bottom)) {
                    yOffset += mContentBottom - floatRegion.bottom;
                }
                if ((floatRegion.top + yOffset) < mStableTop) {
                    yOffset += mStableTop - (floatRegion.top + yOffset);
                }
                if (yOffset != 0) {
                    if (floatRegion.left < mStableLeft) {
                        xOffset += mStableLeft - floatRegion.left;
                    } else if (floatRegion.right > mStableRight) {
                        xOffset += mStableRight - floatRegion.right;
                    }
                }
            }
            win.adjustFloatingRect(xOffset, yOffset);

            pf.left = df.left = of.left = cf.left = vf.left =
                    floatRegion.left + xOffset;
            pf.top = df.top = of.top = cf.top = vf.top = floatRegion.top + yOffset;
            pf.right = df.right = of.right = cf.right = vf.right =
                    floatRegion.right + xOffset;
            pf.bottom = df.bottom = of.bottom = cf.bottom = vf.bottom =
                    floatRegion.bottom + yOffset;

            if(attached != null){
                pf.set((fl & FLAG_LAYOUT_IN_SCREEN) == 0
                    ? attached.getFrameLw() : df);
            }
			
            if (mCurrentInputMethodTargetWindow == win) {
                pf.bottom = Math.min(mContentBottom, pf.bottom);
                if (adjust != SOFT_INPUT_ADJUST_RESIZE) {
                    cf.right = mDockRight;
                    cf.bottom = mDockBottom;
                } else {
                    cf.right = mContentRight;
                    cf.bottom = mContentBottom;
                }

                if (adjust != SOFT_INPUT_ADJUST_NOTHING) {
                    vf.right = mCurRight;
                    vf.bottom = mCurBottom;
                } else {
                    vf.set(cf);
                }
            }

            if (true) Slog.v(TAG, "Compute float frame " + attrs.getTitle()
                    + ": sim=#" + Integer.toHexString(sim)
                    + " attach=" + attached + " type=" + attrs.type
                    + String.format(" flags=0x%08x", fl)
                    + " pf=" + pf.toShortString() + " df=" + df.toShortString()
                    + " of=" + of.toShortString()
                    + " cf=" + cf.toShortString() + " vf=" + vf.toShortString()
                    + " dcf=" + dcf.toShortString() + " floatRegion=" + floatRegion.toShortString());

            win.computeFrameLw(pf, df, of, cf, vf, dcf);
        }
    }

    /** {@inheritDoc} */
    @Override
    public void applyPostLayoutPolicyLw(WindowState win,
                                WindowManager.LayoutParams attrs) {
        /// Floating window can't be the top full screen.
        if (win != null && win.isFloatingWindow()){
            Slog.d(TAG, "floating windod skips applyPostLayoutPolicyLw ");
            return;
        }

        super.applyPostLayoutPolicyLw(win, attrs);

    }

    /** {@inheritDoc} */
    @Override
    public void finishLayoutLw() {
        super.finishLayoutLw();
        if (FeatureOption.MTK_MULTI_WINDOW_SUPPORT
                && mMtkPhoneWindowUtility != null) {
            mMtkPhoneWindowUtility.updateRect(mContentLeft,
                    mContentTop, mContentRight, mContentBottom);
        }
        return;
    }

    public int focusChangedLw(WindowState lastFocus, WindowState newFocus) {
        int resVal;

        /// M: When the focus window is belong to the floating window, enable
        /// the floating monitor. Otherwise, disable it. @{
        if (FeatureOption.MTK_MULTI_WINDOW_SUPPORT) {
            mMtkPhoneWindowUtility.updateFocus2FloatMonitor(newFocus);
        }
        /// @}

        resVal = super.focusChangedLw(lastFocus, newFocus);
        if (resVal != 0) {
            return resVal;
        }

        return 0;
    }

    /// M: Init mMtkPhoneWindowUtility as null
    MtkPhoneWindowUtility mMtkPhoneWindowUtility = null;

    /// @}
}
