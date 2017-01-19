package com.discretix.drmassist;

import android.content.Context;

/**
 * abstract base class for all DRM actions involving UI (such as audio/video playback)
 */
public abstract class DrmActionUI extends DrmActionBase {
	
	
	/**
	 * Perform DRM action body 
	 * @param context application context
	 * @param filePath current file path
	 * @param extension current file extension
	 */
	protected abstract void perform(final Context context, final String filePath, final String extension);
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#run(android.content.Context, java.lang.String, java.lang.String)
	 */
	final void run(final Context context, final String fileName, final String extension) {
		perform(context, fileName, extension);
		
	}
	
	

}
