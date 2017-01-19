package com.discretix.drmactions;

import android.content.Context;
import android.drm.DrmManagerClient;
import android.util.Log;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;
/**
 * Check rights status for current file 
 */
public class CheckRights extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Check rights status";
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
		
		int status = drmClient.checkRightsStatus(filePath);
		Log.d("CheckRights", "DrmManagerClient::checkRightsStatus(String) returned " + Utilities.rightsStatus2Str(status));
		
		return Utilities.rightsStatus2Str(status);
		
	}
}
