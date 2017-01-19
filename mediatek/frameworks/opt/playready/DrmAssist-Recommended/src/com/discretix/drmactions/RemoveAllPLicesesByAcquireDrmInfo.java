package com.discretix.drmactions;


import android.content.Context;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.util.Log;

import com.discretix.drmactions.auxillary.Constants;
import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

/**
 * Remove rights for a current file
 */
public class RemoveAllPLicesesByAcquireDrmInfo extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Remove All Licenses via AcquireDrmInfo";
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
	protected String perform(Context context, String filePath, String extension) 
	{
		
		DrmManagerClient drmClient = new DrmManagerClient(context);
		
		DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_UNREGISTRATION_INFO, "audio/isma");
		request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_IS_REMOVE_ALL_LICENSES_EXIST);
		
		DrmInfo response = drmClient.acquireDrmInfo(request);
		
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
		{
			Log.e("RemoveAllPLicesesByAcquireDrmInfo", "DrmManagerClient::RemoveAllPLicesesByAcquireDrmInfo failed");
			return("RemoveLicenses not supported");			
		}
		
		request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_REMOVE_ALL_LICENSES);
		response = drmClient.acquireDrmInfo(request);
		
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
		{
			Log.e("RemoveAllPLicesesByAcquireDrmInfo", "DrmManagerClient::RemoveAllPLicesesByAcquireDrmInfo failed");
			return("FAILURE");				
		}

		return("SUCCESS");
	}
}
