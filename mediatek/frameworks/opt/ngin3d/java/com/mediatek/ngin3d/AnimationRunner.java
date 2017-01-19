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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.ngin3d;

import android.util.Log;
import com.mediatek.ngin3d.animation.MasterClock;

public class AnimationRunner extends Thread {
    private static final String TAG = "AnimationRunner";
    private static final AnimationRunnerManager ANIMATION_RUNNER_MANAGER = new AnimationRunnerManager();
    private final Stage mStage;
    private boolean mShouldExit;

    public AnimationRunner(Stage stage) {
        super();
        mStage = stage;
    }

    @Override
    public void run() {
        setName("AnimationRunner " + getId());
        Log.i(TAG, "starting tid=" + getId());

        try {
            guardedRun();
        } catch (InterruptedException e) {
            // fall thru and exit normally
            e.printStackTrace();
        } finally {
            Log.i(TAG, "threadExiting tid=" + getId());
            ANIMATION_RUNNER_MANAGER.threadExiting(this);
        }
    }

    private void guardedRun() throws InterruptedException {
        while (!mShouldExit) {

            // Tick the clock to do animation and make Stage dirty
            MasterClock.getDefault().tick();

            // Apply transaction for animations.
            Transaction.applyOperations();

            // Check stage is dirty or is there any animation running.
            mStage.isAnimationStarted();
            synchronized (ANIMATION_RUNNER_MANAGER) {
                ANIMATION_RUNNER_MANAGER.wait();
            }
        }
    }

    public void requestRender() {
        synchronized (ANIMATION_RUNNER_MANAGER) {
            ANIMATION_RUNNER_MANAGER.notifyAll();
        }
    }

    public void requestExit() {
        mShouldExit = true;
        synchronized (ANIMATION_RUNNER_MANAGER) {
            ANIMATION_RUNNER_MANAGER.notifyAll();
        }
    }

    public static class AnimationRunnerManager {
        private static final String TAG = "AnimationRunnerManager";
        @SuppressWarnings("PMD")
        public void threadExiting(AnimationRunner runner) {
            synchronized (this) {
                Log.i(TAG, "exiting tid=" + runner.getId());

                notifyAll();
            }
        }

    }
}