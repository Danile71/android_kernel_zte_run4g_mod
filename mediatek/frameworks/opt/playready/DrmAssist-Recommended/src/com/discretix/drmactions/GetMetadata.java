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
public class GetMetadata extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Get Metadata";
	}

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
	 */
	public boolean isSupported(String extension, String filePath) {
		return Utilities.isPlayreadyDrm(extension);
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionSync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected String perform(Context context, String filePath, String extension) {		
		DrmManagerClient drmClient = new DrmManagerClient(context);
		StringBuilder output = null;
		
		ContentValues metaData = drmClient.getMetadata(filePath);
		Log.d("DrmManagerClient::getMetadata %s", (null == metaData)? "null" : "ContentValues instance");

		if (null != metaData){
			output = new StringBuilder();
			
			for (String key  : metaData.keySet()) {
				output.append(key + " = " + metaData.getAsString(key)+ "\n");
			}
			
			if (output.length() == 0){
				return "N/A";
			}
			return output.toString();
		} else {
			return "N/A";
		}
	}
}
