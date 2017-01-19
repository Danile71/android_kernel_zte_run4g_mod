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

package com.orangelabs.rcs.provider.sharing;

import java.util.Calendar;

import org.gsma.joyn.ish.ImageSharing;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;

import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Rich call history
 * 
 * @author Jean-Marc AUFFRET
 */
public class RichCallHistory {
	/**
	 * Current instance
	 */
	private static RichCallHistory instance = null;

	/**
	 * Content resolver
	 */
	private ContentResolver cr;
	
	/**
	 * Database URI for image sharing
	 */
	private Uri ishDatabaseUri = ImageSharingData.CONTENT_URI;
	
	/**
	 * Database URI for video sharing
	 */
	private Uri vshDatabaseUri = VideoSharingData.CONTENT_URI;
	
	/**
	 * The logger
	 */
	private Logger logger = Logger.getLogger(this.getClass().getName());
	
	/**
	 * Create instance
	 * 
	 * @param ctx Context
	 */
	public static synchronized void createInstance(Context ctx) {
		if (instance == null) {
			instance = new RichCallHistory(ctx);
		}
	}
	
	/**
	 * Returns instance
	 * 
	 * @return Instance
	 */
	public static RichCallHistory getInstance() {
		return instance;
	}
	
	/**
     * Constructor
     * 
     * @param ctx Application context
     */
	private RichCallHistory(Context ctx) {
		super();
		
        this.cr = ctx.getContentResolver();
    }
	
	/*--------------------- Video sharing methods ----------------------*/
	
	/**
	 * Add a new video sharing in the call history 
	 * 
	 * @param contact Remote contact
	 * @param sessionId Session ID
	 * @param direction Call event direction
	 * @param content Shared content
	 * @param status Call status
	 */
	public Uri addVideoSharing(String contact, String sessionId, int direction, MmContent content, int status) {
		if(logger.isActivated()){
			logger.debug("Add new video sharing for contact " + contact + ": session=" + sessionId + ", status=" + status);
		}

		contact = PhoneUtils.extractNumberFromUri(contact);
		ContentValues values = new ContentValues();
		values.put(VideoSharingData.KEY_SESSION_ID, sessionId);
		values.put(VideoSharingData.KEY_CONTACT, contact);
		values.put(VideoSharingData.KEY_DIRECTION, direction);
		values.put(VideoSharingData.KEY_STATUS, status);
		values.put(VideoSharingData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
		values.put(VideoSharingData.KEY_DURATION, 0);
		return cr.insert(vshDatabaseUri, values);
	}

	/**
	 * Update the video sharing status
	 * 
	 * @param sessionId Session ID of the entry
	 * @param status New status
	 */
	public void setVideoSharingStatus(String sessionId, int status) {
		if (logger.isActivated()) {
			logger.debug("Update video sharing status of session " + sessionId + " to " + status);
		}
		ContentValues values = new ContentValues();
		values.put(VideoSharingData.KEY_STATUS, status);
		cr.update(vshDatabaseUri, values, VideoSharingData.KEY_SESSION_ID + " = '" + sessionId + "'", null);
	}

	/**
	 * Update the video sharing duration at the end of the call
	 * 
	 * @param sessionId Session ID of the entry
	 * @param duration Duration
	 */
	public void setVideoSharingDuration(String sessionId, long duration) {
		if (logger.isActivated()) {
			logger.debug("Update duration of session " + sessionId + " to " + duration);
		}
		ContentValues values = new ContentValues();
		values.put(VideoSharingData.KEY_DURATION, duration);
		cr.update(vshDatabaseUri, values, VideoSharingData.KEY_SESSION_ID + " = '" + sessionId + "'", null);
	}
	
	/*--------------------- Image sharing methods ----------------------*/

	/**
	 * Add a new image sharing in the call history 
	 * 
	 * @param contact Remote contact
	 * @param sessionId Session ID
	 * @param direction Call event direction
	 * @param content Shared content
	 * @param status Call status
	 */
	public Uri addImageSharing(String contact, String sessionId, int direction, MmContent content, int status) {
		if(logger.isActivated()){
			logger.debug("Add new image sharing for contact " + contact + ": session=" + sessionId + ", status=" + status);
		}

		contact = PhoneUtils.extractNumberFromUri(contact);
		ContentValues values = new ContentValues();
		values.put(ImageSharingData.KEY_SESSION_ID, sessionId);
		values.put(ImageSharingData.KEY_CONTACT, contact);
		values.put(ImageSharingData.KEY_DIRECTION, direction);
		values.put(ImageSharingData.KEY_NAME, content.getUrl());
		values.put(ImageSharingData.KEY_MIME_TYPE, content.getEncoding());
		values.put(ImageSharingData.KEY_NAME, content.getUrl());
		values.put(ImageSharingData.KEY_SIZE, 0);
		values.put(ImageSharingData.KEY_TOTAL_SIZE, content.getSize());
		values.put(ImageSharingData.KEY_STATUS, status);
		values.put(ImageSharingData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
		return cr.insert(ishDatabaseUri, values);
	}

	/**
	 * Update the image sharing status
	 * 
	 * @param sessionId Session ID of the entry
	 * @param status New status
	 */
	public void setImageSharingStatus(String sessionId, int status) {
		if (logger.isActivated()) {
			logger.debug("Update status of image session " + sessionId + " to " + status);
		}
		ContentValues values = new ContentValues();
		values.put(ImageSharingData.KEY_STATUS, status);
		cr.update(ishDatabaseUri, values, ImageSharingData.KEY_SESSION_ID + " = '" + sessionId + "'", null);
	}

	/**
	 * Update the image sharing progress
	 * 
	 * @param sessionId Session ID of the entry
	 * @param current Current size
	 * @param total Total size
	 */
	public void setImageSharingProgress(String sessionId, long current, long total) {
		if (logger.isActivated()) {
			logger.debug("Update image sharing progress");
		}
		ContentValues values = new ContentValues();
		values.put(ImageSharingData.KEY_SIZE, current);
		values.put(ImageSharingData.KEY_TOTAL_SIZE, total);
		values.put(ImageSharingData.KEY_STATUS, ImageSharing.State.STARTED);
		cr.update(ishDatabaseUri, values, ImageSharingData.KEY_SESSION_ID + " = '" + sessionId + "'", null);
	}	
}
