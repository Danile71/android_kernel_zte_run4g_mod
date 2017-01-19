package com.discretix.drmactions;


import java.io.IOException;

import android.content.Context;
import android.drm.DrmErrorEvent;
import android.drm.DrmEvent;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.drm.DrmManagerClient.OnErrorListener;
import android.drm.DrmManagerClient.OnEventListener;
import android.util.Log;

import com.discretix.drmactions.auxillary.Constants;
import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionAsync;

/**
 * Execute PlayReady initiator - by using this action you can acquire rights, join/leave domain and send metering update
 */
public class ExecuteInitiator extends DrmActionAsync {
		
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Execute Initiator";
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
	 */
	public boolean isSupported(final String extension, final String filePath) {
		
		return Utilities.isPlayreadyInitiator(extension);
		
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionAsync#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected void perform(final Context context, String filePath, final String extension) {
		DrmManagerClient drmClient = new DrmManagerClient(context);
		String initiator;
		try {
			initiator = Utilities.loadTextFile(filePath);
		} catch (IOException e) {
			e.printStackTrace();
			Log.e("LicenseAquisition", "Failed to load initiator");
			onTaskFinish("Failed to load initiator");
			return;
		}
		DrmInfo drmInfo = new DrmInfo(DrmInfoRequest.TYPE_RIGHTS_ACQUISITION_INFO,"placeholder".getBytes() , Constants.INITIATOR_MIME_TYPE);
		drmInfo.put(Constants.DRM_OP_CODE, Constants.DRM_OP_CODE_RIGHTS_ACQUISITION_INITIATOR);
		drmInfo.put(Constants.DRM_INITIATOR, initiator);
		
		
		Log.d("LicenseAquisition", "executing DrmManagerClient::OnEventListener");
		drmClient.setOnEventListener(new OnEventListener() {
			
			public void onEvent(DrmManagerClient client, DrmEvent event) {
				if (event.getType()==(DrmEvent.TYPE_DRM_INFO_PROCESSED) ){
					Log.i("LicenseAquisition", "OnEventListener called for TYPE_DRM_INFO_PROCESSED event");
					onTaskFinish("SUCCESS");
				}
			}
		});
		
		Log.d("LicenseAquisition", "executing DrmManagerClient::OnErrorListener");
		drmClient.setOnErrorListener(new OnErrorListener() {
			
			public void onError(DrmManagerClient client, DrmErrorEvent event) {
				Log.e("LicenseAquisition", "OnErrorListener called - error occured");
				onTaskFinish("OnErrorListener called - error occured");
			}				
		});
		
		
		Log.d("LicenseAquisition", "executing DrmManagerClient::processDrmInfo");
		int status = drmClient.processDrmInfo(drmInfo);
		Log.i("LicenseAquisition", "DrmManagerClient::processDrmInfo returned " + Utilities.drmClientStatus2Str(status) );
		if (DrmManagerClient.ERROR_NONE != status) {
			onTaskFinish("DrmManagerClient::processDrmInfo failed");
		}
	}	

}
