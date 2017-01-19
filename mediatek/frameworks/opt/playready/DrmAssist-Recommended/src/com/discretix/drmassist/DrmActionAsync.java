package com.discretix.drmassist;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;

/**
 * Abstract base class for all asynchronous DRM actions<br>
 * Implementing actions assumed to be asynchronous thus they should not lock UI thread<br> 
 * <b>NOTE:</b> implementing class <b>MUST</b> call {@link #onTaskFinish(String)} method to notify UI that
 * DRM action is complete
 */
public abstract class DrmActionAsync extends DrmActionBase{
	
	private ProgressDialog progressDlg = null;
	private Context context = null;
	
	/**
	 * Perform DRM action body 
	 * @param context application context
	 * @param filePath current file path
	 * @param extension current file extension
	 */
	protected abstract void perform(final Context context, final String filePath, final String extension);
	
	/**
	 * Notify UI that current DRM action is complete
	 * @param message status message to be presented on UI 
	 */
	final protected void onTaskFinish(final String message) {
		progressDlg.dismiss();
		progressDlg = null;
		new AlertDialog.Builder(context).setTitle(getName() + " Returned:").setMessage(message).show();
		context = null;
		}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#run(android.content.Context, java.lang.String, java.lang.String)
	 */
	final void run(final Context context, final String fileName, final String extension) {
		progressDlg = new ProgressDialog(context);
		progressDlg.setTitle("Performing " + getName());
		progressDlg.setMessage("Please wait");
		progressDlg.setCanceledOnTouchOutside(false);
		progressDlg.setCancelable(false);
		progressDlg.show();
		this.context = context;
		perform(context, fileName, extension);
	}

}
