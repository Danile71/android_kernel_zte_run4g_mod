package com.discretix.drmactions;

import java.io.FileDescriptor;

import android.content.Context;
import android.drm.DrmManagerClient;
import android.util.Log;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

/**
 * Get MIME type of a clear content protected by PlayReady DRM scheme
 */
public class GetOriginalMimeType extends DrmActionSync {

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Get Mime Type";
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
		
		String mimeType =  drmClient.getOriginalMimeType(filePath) ;
		Log.d("GetOriginalMimeType", "DrmManagerClient::getOriginalMimeType returned " + mimeType);
		
		return mimeType;
	}

}
