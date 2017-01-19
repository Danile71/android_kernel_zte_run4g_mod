package com.discretix.drmassist;

import android.content.Context;


/**
 * Base class for all DRM actions
 */
abstract class DrmActionBase{
	
	/**
	 * @return get DRM action name
	 */
	public abstract String getName();
	
	/**
	 * Test whether current action is supported 
	 * @param extension file extension
	 * @param filePath absolute file path
	 * @return true - supported, false otherwise
	 */
	public abstract boolean isSupported(final String extension, final String filePath);

	@Override
	/*
	 * override default converter - required for using in this class in ArrayAdapter
	 */
	final public String toString() {
		return getName();
	}
	/**
	 * execute current DRM action
	 * @param context application context
	 * @param fileName file name that will be involved in current action
	 * @param extension file extension that will be involved in current action
	 */
	abstract void run (final Context context, final String fileName, final String extension);
	

}
