package com.discretix.drmactions;

import android.content.ContentValues;
import android.content.Context;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.util.Log;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

/**
 * Get constraints for a current file
 */
public class GetConstraints extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Get Constraints";
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
		
		ContentValues constraints = drmClient.getConstraints(filePath, DrmStore.Action.PLAY);
		Log.d("DrmManagerClient::getConstraints(String,Action.PLAY) returned %s", (null == constraints)? "null" : "ContentValues instance");

		if (null != constraints){
			output = new StringBuilder();
			String mapValue = null;
			//-----------------------------------------------------------------------------------------------
			mapValue = constraints.getAsString(DrmStore.ConstraintsColumns.LICENSE_AVAILABLE_TIME);
			if (null != mapValue){
				output.append("First play expiration - " + mapValue + "\n");
			}
			//-----------------------------------------------------------------------------------------------
			mapValue = constraints.getAsString(DrmStore.ConstraintsColumns.LICENSE_EXPIRY_TIME);
			if (null != mapValue){
				output.append("Expiration Time - " + mapValue + "\n");
			}
			//-----------------------------------------------------------------------------------------------
			mapValue = constraints.getAsString(DrmStore.ConstraintsColumns.LICENSE_START_TIME);
			if (null != mapValue){
				output.append("Start Time - " + mapValue + "\n");
			}
			//-----------------------------------------------------------------------------------------------
			mapValue = constraints.getAsString(DrmStore.ConstraintsColumns.MAX_REPEAT_COUNT);
			if (null != mapValue){
				output.append("Repeat Count - " + mapValue);
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
