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

import android.os.Parcel;
import android.os.Parcelable;

import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.H264Config;

/**
 * Video codec
 * 
 * @author Jean-Marc AUFFRET
 */
public class VideoCodec implements Parcelable {
	/**
	 * Video encoding
	 */
	private String encoding;
	
	/**
	 * Payload
	 */
	private int payload;
	
	/**
	 * Clock rate
	 */
	private int clockRate;
	
	/**
	 * Frame rate
	 */
	private int frameRate;
	
	/**
	 * Bit rate
	 */
	private int bitRate;

	/**
	 * Screen width
	 */
	private int width;
	
	/**
	 * Screen height
	 */
	private int height;

	/**
	 * Video parameters
	 */
	private String parameters;
	
	public static final String TAG = "VideoCodec";
	
    /**
     * Constructor
     *
     * @param encoding Video encoding
     * @param payload Payload
     * @param clockRate Clock rate
     * @param framerate Frame rate
     * @param bitRate Bit rate
     * @param width Video width
     * @param height Video height
     * @param parameters Codec parameters
     * @hide
     */
    public VideoCodec(String encoding, int payload, int clockRate, int frameRate, int bitRate, int width, int height, String parameters) {
    	Logger.i(TAG, "VideoCodec entry" + "encoding" + encoding + "payload" + "clockRate" + clockRate + "frameRate" + frameRate +"bitRate" + 
        bitRate + "width" + width + "height" + height +  "parameters" + parameters);
    	this.encoding = encoding;
    	this.payload = payload;
    	this.clockRate = clockRate;
    	this.frameRate = frameRate;
    	this.bitRate = bitRate;
    	this.width = width;
    	this.height = height;
    	this.parameters = parameters;
    }
    
    /**
	 * Constructor
	 * 
	 * @param source Parcelable source
     * @hide
	 */
	public VideoCodec(Parcel source) {
		this.encoding = source.readString();
    	this.payload = source.readInt();
    	this.clockRate = source.readInt();
    	this.frameRate = source.readInt();
    	this.bitRate = source.readInt();
    	this.width = source.readInt();
    	this.height = source.readInt();
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
    	dest.writeInt(clockRate);
    	dest.writeInt(frameRate);
    	dest.writeInt(bitRate);
    	dest.writeInt(width);
    	dest.writeInt(height);
    	dest.writeString(parameters);
    }
    
    /**
     * Parcelable creator
     * 
     * @hide
     */
    public static final Parcelable.Creator<VideoCodec> CREATOR
            = new Parcelable.Creator<VideoCodec>() {
        public VideoCodec createFromParcel(Parcel source) {
            return new VideoCodec(source);
        }

        public VideoCodec[] newArray(int size) {
            return new VideoCodec[size];
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
     * Returns the codec clock rate (e.g. 90000)
     * 
     * @return Clock rate
     */
    public int getClockRate() {
    	Logger.i(TAG, "getClockRate value " + clockRate);
    	return clockRate;
    }
    
    /**
     * Returns the codec frame rate (e.g. 10)
     * 
     * @return Frame rate
     */
    public int getFrameRate() {
    	Logger.i(TAG, "getFrameRate value " + frameRate);
    	return frameRate;
    }
    
    /**
     * Returns the codec bit rate (e.g. 64000)
     * 
     * @return Bit rate
     */
    public int getBitRate() {
    	Logger.i(TAG, "getBitRate value " + bitRate);
    	return bitRate;
    }
    
    /**
     * Returns the video width (e.g. 176)
     * 
     * @return Video width
     */
    public int getVideoWidth() {
    	Logger.i(TAG, "getVideoWidth value " + width);
    	return width;
    }
    
    /**
     * Returns the video height (e.g. 144)
     * 
     * @return Video height
     */
    public int getVideoHeight() {
    	Logger.i(TAG, "getVideoHeight value " + height);
    	return height;
    }
    
    /**
     * Returns the list of codec parameters (e.g. profile-level-id, packetization-mode).
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
    	Logger.i(TAG, "getParameter value " + value);
    	return value;    	
    }

    /**
     * Compare codec
     *
     * @param codec Codec to compare
     * @return Returns True if codecs are equals, else returns False
     */
    public boolean compare(VideoCodec codec) {
        boolean ret = false;
        if (getEncoding().equalsIgnoreCase(codec.getEncoding()) 
                && (getVideoWidth() == codec.getVideoWidth() || getVideoWidth() == 0 || codec.getVideoWidth() == 0)
                && (getVideoHeight() == codec.getVideoHeight() || getVideoHeight() == 0 || codec.getVideoHeight() == 0)) {
            if (getEncoding().equalsIgnoreCase(H264Config.CODEC_NAME)) {
                if (H264Config.getCodecProfileLevelId(getParameters()).compareToIgnoreCase(H264Config.getCodecProfileLevelId(codec.getParameters())) == 0) {
                    ret =  true;
                }
            } else {
                if (getParameters().equalsIgnoreCase(codec.getParameters())) {
                    ret = true;
                }
            }
        }
        Logger.i(TAG, "compare value " + ret);
        return ret;
    }
}
