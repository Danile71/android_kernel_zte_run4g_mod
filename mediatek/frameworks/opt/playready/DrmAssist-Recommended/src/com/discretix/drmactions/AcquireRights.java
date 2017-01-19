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
 * Acquire rights for a encrypted file using file headers
 */
public class AcquireRights extends DrmActionSync {
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Acquire Rights by plugin";
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
	 * @see com.discretix.drmassist.DrmActionAsync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected String perform(Context context, String filePath, final String extension) {
		
		DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_RIGHTS_ACQUISITION_INFO, Utilities.extension2mimeType(extension));
		request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_GET_DRM_HEADER);
		request.put(Constants.DRM_DATA, filePath);		
			
		DrmManagerClient drmClient = new DrmManagerClient(context);
			
		// get header
		DrmInfo response = drmClient.acquireDrmInfo(request);
				
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") ) 
		{
			Log.e("AcquireRights", "DrmManagerClient::acquireRights failed");
			return("FAILURE");			
		}
		Log.d("AcquireRights", "DrmManagerClient::acquireRights returned header :" + response.get(Constants.DRM_DATA) );

		// prepare acquire license request
		request.put(Constants.DRM_ACTION, Constants.DRM_OP_CODE_RIGHTS_ACQUISITION_FULL);
		request.put(Constants.DRM_HEADER, response.get(Constants.DRM_DATA));		
		
		response = drmClient.acquireDrmInfo(request);
		
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
		{
			Log.e("AcquireRights", "DrmManagerClient::acquireRights failed");
			return("FAILURE");			
		}
		
		Log.i("AcquireRights", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
		
		return ("SUCCESS");

	}

}
