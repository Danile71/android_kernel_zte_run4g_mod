/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/
package org.gsma.joyn.ipcall;

import com.mediatek.rcse.api.Logger;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * IP call service configuration
 * 
 * @author Jean-Marc AUFFRET
 */
public class IPCallServiceConfiguration implements Parcelable {
	/**
	 * Voice call breakout
	 */
	private boolean voiceBreakout;
	
	public static final String TAG = "IPCallServiceConfiguration";
	
	/**
	 * Constructor
	 * 
	 * @param voiceBreakout Voice call breakout
     * @hide
	 */
	public IPCallServiceConfiguration(boolean voiceBreakout) {
		Logger.i(TAG, "IPCallServiceConfiguration entry" + voiceBreakout);
		this.voiceBreakout = voiceBreakout;
    }	
	
	/**
	 * Constructor
	 * 
	 * @param source Parcelable source
     * @hide
	 */
	public IPCallServiceConfiguration(Parcel source) {
		this.voiceBreakout = source.readInt() != 0;
    }

	/**
	 * Describe the kinds of special objects contained in this Parcelable's
	 * marshalled representation
	 * 
	 * @return Integer
     * @hide
	 */
	public int describeContents() {
        return 0;
    }

	/**
	 * Write parcelable object
	 * 
	 * @param dest The Parcel in which the object should be written
	 * @param flags Additional flags about how the object should be written
     * @hide
	 */
    public void writeToParcel(Parcel dest, int flags) {
    	dest.writeInt(voiceBreakout ? 1 : 0);
    }

    /**
     * Parcelable creator
     * 
     * @hide
     */
    public static final Parcelable.Creator<IPCallServiceConfiguration> CREATOR
            = new Parcelable.Creator<IPCallServiceConfiguration>() {
        public IPCallServiceConfiguration createFromParcel(Parcel source) {
            return new IPCallServiceConfiguration(source);
        }

        public IPCallServiceConfiguration[] newArray(int size) {
            return new IPCallServiceConfiguration[size];
        }
    };	

    /**
	 * Is voice call breakout activated. It returns True if the service can reach
	 * any user, else returns False if only joyn users supporting the IP call
	 * capability may be called.
	 * 
	 * @return Boolean 
	 */
	public boolean isVoiceCallBreakout() {
		Logger.i(TAG, "isVoiceCallBreakout value " + voiceBreakout);
		return voiceBreakout;
	}
}
