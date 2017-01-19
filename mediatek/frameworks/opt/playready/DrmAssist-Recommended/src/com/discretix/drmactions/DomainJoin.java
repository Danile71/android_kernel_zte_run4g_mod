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
 * Join domain. Simulates use case when user has Domain service URL, account and service IDs and wants to join domain.
 * This class will extract all required information from existing initiator, but in real world scenario user can have this information 
 * stored in other ways (for example internal DB) 
 */
public class DomainJoin extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Join Domain";
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
		return initiator.contains("JoinDomain");
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionAsync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected String perform(Context context, String filePath, String extension) 
	{
		String initiator;
		DrmManagerClient drmClient = new DrmManagerClient(context);
		DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_REGISTRATION_INFO, Utilities.extension2mimeType(extension));
		
		request.put(Constants.DRM_ACTION, Constants.DRM_ACTION_JOIN_DOMAIN_INITIATOR);
			
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
			Log.e("JoinDomain", "DrmManagerClient::acquireDrmInfo failed");
			return("FAILURE");			
		}
		
		Log.i("JoinDomain", "acquireDrmInfo returned "+response.get(Constants.DRM_STATUS));
		
		return ("SUCCESS");
	
	}

}
