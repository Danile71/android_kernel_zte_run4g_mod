package com.discretix.drmactions;

import android.content.ContentValues;
import android.content.Context;
import android.drm.DrmManagerClient;
import android.util.Log;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

/**
 * Get PlayReady DRM file metadata
 */
public class CanHandle extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "can Handle";
	}

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
	 */
	public boolean isSupported(String extension, String filePath) {
		return true; // this is all the meaning of this api - to check if it is supported by DRM FW
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionSync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected String perform(Context context, String filePath, String extension) {		
		DrmManagerClient drmClient = new DrmManagerClient(context);
		
		boolean canhandle = drmClient.canHandle(filePath, Utilities.extension2mimeType(extension));
		Log.d("DrmManagerClient::canHandle %s", (canhandle)? "True" : "False");

		return ((canhandle) ? "True" : "False");

		}
}
