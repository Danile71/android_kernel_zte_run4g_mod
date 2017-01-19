package org.gsma.joyn.ipcall;

/**
 * IP call renderer event listener interface
 */
interface IIPCallRendererListener {
	void onRendererOpened();

	void onRendererStarted();

	void onRendererStopped();

	void onRendererClosed();

	void onRendererFailed();
}
