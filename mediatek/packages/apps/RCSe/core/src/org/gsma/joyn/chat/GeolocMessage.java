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
package org.gsma.joyn.chat;

import java.util.Date;

import com.mediatek.rcse.api.Logger;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Geoloc push message
 *  
 * @author Jean-Marc AUFFRET
 */
public class GeolocMessage extends ChatMessage implements Parcelable {
	/**
	 * MIME type
	 */
	public final static String MIME_TYPE = "application/geoloc"; 

	public static final String TAG = "TAPI-GeolocMessage";
	/**
	 * Geoloc info
	 */
	private Geoloc geoloc = null;
		
    /**
     * Constructor for outgoing message
     * 
     * Constructor for outgoing message
     * 
     * @param messageId Message Id
     * @param contact Contact
     * @param geoloc Geolocation info
     * @param receiptAt Receipt date
     * @param displayedReportRequested Flag indicating if a displayed report is requested
     * @hide
	 */
	public GeolocMessage(String messageId, String remote, Geoloc geoloc, Date receiptAt, boolean imdnDisplayedRequested) {
		super(messageId, remote, null, receiptAt, imdnDisplayedRequested,null);
		Logger.i(TAG, "GeolocMessage entry geoloc =" + geoloc);
		this.geoloc = geoloc;
	}
	
	/**
	 * Constructor
	 * 
	 * @param source Parcelable source
     * @hide
	 */
	public GeolocMessage(Parcel source) {
		super(source);
		
		this.geoloc = new Geoloc(source);
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
    	super.writeToParcel(dest, flags);
    	
    	geoloc.writeToParcel(dest, flags);
    }

    /**
     * Parcelable creator
     * 
     * @hide
     */
    public static final Parcelable.Creator<GeolocMessage> CREATOR
            = new Parcelable.Creator<GeolocMessage>() {
        public GeolocMessage createFromParcel(Parcel source) {
            return new GeolocMessage(source);
        }

        public GeolocMessage[] newArray(int size) {
            return new GeolocMessage[size];
        }
    };	

    /**
	 * Get geolocation info
	 * 
	 * @return Geoloc object
	 * @see Geoloc
	 */
	public Geoloc getGeoloc() {
		return geoloc;
	}
}
