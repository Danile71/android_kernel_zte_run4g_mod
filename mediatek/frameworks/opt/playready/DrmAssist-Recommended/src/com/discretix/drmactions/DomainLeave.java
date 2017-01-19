package com.discretix.drmactions;

import java.io.IOException;

import android.content.Context;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.util.Log;
import com.discretix.drmactions.auxillary.Constants;
import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

/**
 * Leave domain. Simulates use case when user has Domain service URL, account and service IDs and wants to leave domain.
 * This class will extract all required information from existing initiator, but in real world scenario user can have this information 
 * stored in other ways (for example internal DB) 
 */
public class DomainLeave extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Leave Domain";
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
	 */
	public boolean isSupported(String extension, String filePath) {
		if (!Utilities.isPlayreadyInitiator(extension))
			return false;
		String initiator;
		try {
			initiator = Utilities.loadTextFile(filePath);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return initiator.contains("LeaveDomain");
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionAsync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected String perform(Context context, String filePath, String extension) 
	{
		String initiator;
		DrmManagerClient drmClient = new DrmManagerClient(context);
		DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_UNREGISTRATION_INFO, Utilities.extension2mimeType(extension));
		
		request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_LEAVE_DOMAIN_INITIATOR);
			
		try 
		{
			initiator = Utilities.loadTextFile(filePath);
		} 
		catch (IOException e) 
		{
			e.printStackTrace();
			return ("FAILURE") ;
		}
		
		request.put(Constants.DRM_DATA, initiator);
		
		DrmInfo response = drmClient.acquireDrmInfo(request);
				
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
		{
			Log.e("LeaveDomain", "DrmManagerClient::acquireDrmInfo failed");
			return("FAILURE");			
		}
		
		Log.i("LeaveDomain", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
		
		return ("SUCCESS");
	}
}
