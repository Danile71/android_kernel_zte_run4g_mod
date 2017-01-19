package com.discretix.drmassist;

import android.app.AlertDialog;
import android.content.Context;

/**
 * Abstract base class for all synchronous DRM actions<br>
 * <b>NOTE:</b> implementing actions assumed to be short so they should not lock UI thread 
 */
public abstract class DrmActionSync extends DrmActionBase {
	
	/**
	 * Perform DRM action body 
	 * @param context application context
	 * @param filePath current file path
	 * @param extension current file extension
	 * @return status message to be presented on UI
	 */
	protected abstract String perform(final Context context, final String filePath, final String extension);
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#run(android.content.Context, java.lang.String, java.lang.String)
	 */
	void run(final Context context, final String fileName, final String extension) {
		String result = perform(context, fileName, extension);
		new AlertDialog.Builder(context).setTitle(getName() + " Returned:").setMessage(result).show();
	}
	
	

}
