/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.settings.plugin;

import android.app.IntentService;
import android.content.Intent;
import android.net.Uri;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.xlog.Xlog;

/**
 * When user press <code>FreeService</code>,
 * the number "+86 18918910000" will be dialed directly.
 * Note:
 * Even the default SIM for "Voice call" is another non-CDMA SIM,
 * this free service number is always dialed out by CDMA SIM card.
 */
public class FreeService extends IntentService {

    private static final String TAG = "FreeService";

    public static final int NO_SIM_ERROR = 0;
    public static final int ONE_CDMA = 1;
    public static final int ONE_GSM = 2;
    public static final int TWO_SIM = 3;
    public static final String SIM_INFO = "SIM_INFO";
    public static final String EXTRA_SLOT_ID = "com.android.phone.extra.slot";

    private static final String FREE_SERVICE_URI = "tel:+8618918910000";

    /**
     * Creates a new <code>FreeService</code> instance.
     *
     */
    public FreeService() {
        super("FreeService");
        Xlog.v(TAG, "FreeService Constructor is called.");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onHandleIntent(Intent intent) {
        Xlog.v(TAG, "onHandleIntent method is called.");
    Intent newIntent = new Intent(Intent.ACTION_CALL, Uri.parse(FREE_SERVICE_URI));
    newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    int simStatus = intent.getIntExtra(SIM_INFO, NO_SIM_ERROR);
    if (simStatus == ONE_CDMA || simStatus == TWO_SIM) {
        // It's normal status, calling by SIM1 CDMA
        newIntent.putExtra(EXTRA_SLOT_ID, PhoneConstants.GEMINI_SIM_1);
    } else if (simStatus == ONE_GSM) {
        // Only GSM Sim inserted into Slot2, calling by SIM2 GSM
        newIntent.putExtra(EXTRA_SLOT_ID, PhoneConstants.GEMINI_SIM_2);
    } else { // NO_SIM_ERROR
        Xlog.v(TAG, "ERROR! No sim detected!");
    }

        startActivity(newIntent);
        Xlog.v(TAG, "Dialing Successfully!");
    }

}
