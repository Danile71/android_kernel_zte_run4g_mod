/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.dialer.plugin.calllog;

import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Phone;

import com.mediatek.dialer.PhoneCallDetailsEx;

/**
 * The details of a phone call to be shown in the UI.
 */
public class OP09PhoneCallDetails extends PhoneCallDetailsEx {

    public static final int DISPLAY_TYPE_DEFAULT_CALL = 1;
    public static final int DISPLAY_TYPE_NEW_CALL = 2;
    public static final int DISPLAY_TYPE_TEXT = 3;

    public final int displayType;

    /**
     * Create the details for a call with a number not associated with a
     * contact.
     */
    public OP09PhoneCallDetails(CharSequence number, CharSequence formattedNumber, String countryIso,
            String geocode, int callType, long date, long duration, int simId, int vtCall,
            int callCount, String ipPrefix, int displayType) {
        super(number, formattedNumber, countryIso, geocode, callType, date, duration, "", 0, "",
                null, null, simId, vtCall, callCount, ipPrefix);
        this.displayType = displayType;
    }

    /** Create the details for a call with a number associated with a contact. */
    public OP09PhoneCallDetails(CharSequence number, CharSequence formattedNumber, String countryIso,
            String geocode, int callType, long date, long duration, CharSequence name,
            int numberType, CharSequence numberLabel, Uri contactUri, Uri photoUri, int simId,
            int vtCall, int callCount, String ipPrefix, int displayType) {
        super(number, formattedNumber, countryIso, geocode, callType, date, duration, name,
              numberType, numberLabel, contactUri, photoUri, simId, vtCall, callCount, ipPrefix);
        this.displayType = displayType;
    }

    public OP09PhoneCallDetails(PhoneCallDetailsEx phoneCallDetails, int displayType) {
        this(phoneCallDetails.number, phoneCallDetails.formattedNumber, phoneCallDetails.countryIso,
             phoneCallDetails.geocode, phoneCallDetails.callType, phoneCallDetails.date, phoneCallDetails.duration,
             phoneCallDetails.name, phoneCallDetails.numberType, phoneCallDetails.numberLabel,
             phoneCallDetails.contactUri, phoneCallDetails.photoUri, phoneCallDetails.simId,
             phoneCallDetails.vtCall, phoneCallDetails.callCount, phoneCallDetails.ipPrefix,
             displayType);
    }

    public OP09PhoneCallDetails(int displayType) {
        this(null, null, null, null, -1, -1l, -1l, -1, -1, -1, null, displayType);
    }

}