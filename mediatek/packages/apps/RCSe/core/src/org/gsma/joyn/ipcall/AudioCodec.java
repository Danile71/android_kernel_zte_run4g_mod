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

import java.util.ArrayList;
import java.util.Arrays;

import com.mediatek.rcse.api.Logger;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Audio codec
 * 
 * @author Jean-Marc AUFFRET
 */
public class AudioCodec implements Parcelable {
	/**
	 * Audio encoding
	 */
	private String encoding;
	
	/**
	 * Payload
	 */
	private int payload;
	
	/**
	 * Sample rate
	 */
	private int sampleRate;

	/**
	 * Audio parameters
	 */
	private String parameters;
	
	public static final String TAG = "AudioCodec";
	
    /**
     * Constructor
     *
     * @param encoding Audio encoding
     * @param payload Payload
     * @param sampleRate Sample rate
     * @param parameters Codec parameters
     * @hide
     */
    public AudioCodec(String encoding, int payload, int sampleRate, String parameters) {
    	Logger.i(TAG, "AudioCodec entry " + "encoding-" + encoding + "payload-" + "sampleRate-" + sampleRate + "parameters-" + parameters);
    	this.encoding = encoding;
    	this.payload = payload;
    	this.sampleRate = sampleRate;
    	this.parameters = parameters;
    }
    
    /**
	 * Constructor
	 * 
	 * @param source Parcelable source
     * @hide
	 */
	public AudioCodec(Parcel source) {
		this.encoding = source.readString();
    	this.payload = source.readInt();
    	this.sampleRate = source.readInt();
		this.parameters = source.readString();
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
    	dest.writeString(encoding);
    	dest.writeInt(payload);
    	dest.writeInt(sampleRate);
    	dest.writeString(parameters);
    }
    
    /**
     * Parcelable creator
     * 
     * @hide
     */
    public static final Parcelable.Creator<AudioCodec> CREATOR
            = new Parcelable.Creator<AudioCodec>() {
        public AudioCodec createFromParcel(Parcel source) {
            return new AudioCodec(source);
        }

        public AudioCodec[] newArray(int size) {
            return new AudioCodec[size];
        }
    };	

    /**
    * Returns the encoding name (e.g. H264)
    * 
    * @return Encoding name
    */
    public String getEncoding() {
    	Logger.i(TAG, "getEncoding value " + encoding);
    	return encoding;
    }
    
    /**
     * Returns the codec payload type (e.g. 96)
     * 
     * @return Payload type
     */
    public int getPayloadType() {
    	Logger.i(TAG, "getPayloadType value " + payload);
    	return payload;
    }
    
    /**
     * Returns the codec sample rate (e.g. 15)
     * 
     * @return Clock rate
     */
    public int getSampleRate() {
    	Logger.i(TAG, "getSampleRate value " + sampleRate);
    	return sampleRate;
    }
    
    /**
     * Returns the list of codec parameters (e.g. packetization-mode).
     * Parameters are are semicolon separated.
     * 
     * @return Parameters
     */
    public String getParameters() {
    	Logger.i(TAG, "getParameters value " + parameters);
    	return parameters;    	
    }

    /**
     * Returns the value of codec parameter key (e.g. profile-level-id, packetization-mode).
     *  
     * @return Parameters
     */
    public String getParameter(String key) {
    	String value = null;    	
    	String[] parameters = getParameters().split(",");
    	ArrayList<String> codecparams = new ArrayList<String>(Arrays.asList(parameters));
    	for(int i =0; i< codecparams.size() ; i++)
    	{
    		if(codecparams.get(i).startsWith(key))
    		{
    			value = codecparams.get(i).substring(key.length() + 1);
    		}
    	}
    	Logger.i(TAG, "getParameter key - "+ key + " value -" + value);
    	return value;    	
    }

    /**
     * Compare codec encodings
     *
     * @param codec Codec to compare
     * @return True if codecs are equals
     */
    public boolean compare(AudioCodec codec) {
    	boolean ret = false;
        if (getEncoding().equalsIgnoreCase(codec.getEncoding())) {
        	ret = true;
        }
        Logger.i(TAG, "compare value " + ret);
        return ret;
    }
}
