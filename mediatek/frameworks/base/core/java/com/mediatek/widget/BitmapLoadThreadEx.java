/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.widget;

import android.content.Context;
import android.os.Process;
import android.util.Log;
import android.widget.AbsListView.OnScrollListener;


public final class BitmapLoadThreadEx extends Thread {
    private static final String TAG = "BitmapLoadThreadEx";
    public boolean mIsLoading;
    private Context loadThreadContext;
    private boolean doLoop = true;
    public BitmapLoadThreadEx(Context context) {
        super("TextureLoad");
        loadThreadContext = context;
        
    }
    public final void stopLoop () {
        doLoop = false;
    }
    @Override
    public void run() {
        Process.setThreadPriority(Process.THREAD_PRIORITY_LESS_FAVORABLE);
        DequeEx<DataAdapterEx> inputQueue = QuickContactBadgeEx.sLoadInputQueue;
        try {
            while (doLoop) {
                // Pop the next texture from the input queue.
                DataAdapterEx adapter = null;
                synchronized (inputQueue) {
                    while ((adapter = inputQueue.pollFirst()) == null) {
                        if (!doLoop) this.interrupt(); 
                        inputQueue.wait();
                    }
                }
                if (QuickContactBadgeEx.DEBUG) Log.v(TAG, "run() pollFirst=" + adapter + ", inputQueue size=" + inputQueue.size() +" loadThreadContext="+loadThreadContext);
                if (QuickContactBadgeEx.sMavLoadThread != this)
                    mIsLoading = true;
                // Load the texture bitmap.
                if (adapter.getState() == DataAdapterEx.STATE_RELEASE_ALL_FRAME) {
                    adapter.freeFrame(); 
                    continue;
                }
                MpoDecodeHelper.load(adapter);
                if (QuickContactBadgeEx.mCount < adapter.getmFrameCount()) {
                    QuickContactBadgeEx.mCount = adapter.getmFrameCount();
                }
                if (adapter.getState() == DataAdapterEx.STATE_LOADED_MARK_FRAME && !adapter.isInterrupte()) {
                    inputQueue.addLast(adapter);
                } else if (adapter.getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME && !adapter.isInterrupte()) {
                    if (QuickContactBadgeEx.sMavRenderThread != null) {
                        QuickContactBadgeEx.sMavRenderThread.setRenderRequester(QuickContactBadgeEx.isOnIdleState());
                    }
                }

                mIsLoading = false;
            }
        } catch (InterruptedException e) {
            // Terminate the thread.
        }
    }
}

