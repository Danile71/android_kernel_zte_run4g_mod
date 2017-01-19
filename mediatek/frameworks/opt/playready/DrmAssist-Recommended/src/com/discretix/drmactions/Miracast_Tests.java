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
public class Miracast_Tests extends DrmActionSync {

	static boolean firstTime = true;
	static boolean connected = false;
	static boolean testFinishedSuccessfuly = false; 
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		
		String str;
		
		if(firstTime)
		{
			firstTime = false;
			return "Miracast Connect/Disconnect test";
		}
		
		if(connected)
		{
			str = "Miracast Disconnect";
		}
		else
		{
			str = "Miracast Connect";
		}
		
		if(testFinishedSuccessfuly)
		{
			connected = !connected;
			testFinishedSuccessfuly = false;
		}

		
		return str;
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
		if(connected == false) 
			request.put(Constants.DRM_MIRACAST, Constants.DRM_MIRACAST_CONNECTED);
		else
			request.put(Constants.DRM_MIRACAST, Constants.DRM_MIRACAST_DISCONNECTED);
		
		
		DrmInfo response = drmClient.acquireDrmInfo(request);
		
		if ( null == response || !response.get(Constants.DRM_STATUS).equals("ok") )
		{
			Log.e("Miracast Tests", "DrmManagerClient::Miracast Tests failed");
			return("setting Miracast = test failed");			
		}
		
		testFinishedSuccessfuly = true;
		if(connected == false) //This state is before the test, now we need to reverse the string
		{
			return("SUCCESS!\nMiracast is Connected!");
		}
		
		return("SUCCESS!\nMiracast is Disconnected!");
	}
}
