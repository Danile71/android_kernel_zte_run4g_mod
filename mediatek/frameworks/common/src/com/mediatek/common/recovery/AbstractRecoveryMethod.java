package com.mediatek.common.recovery;

public abstract class AbstractRecoveryMethod {

	/**
	 * @hide
	 */
	public static final int RECOVER_METHOD_SUCCESS = 0;

	/**
	 * @hide
	 */
	public static final int RECOVER_METHOD_FAILED = -1;

	/**
	 * This method is called by RMS.
	 * Delete file by default.
	 * When modules need to realize their own recovery method, the recovery class MUST extends this class and realized this method.
	 * 
	 * @hide
	 */
	public abstract int doRecover(Object param);

}
