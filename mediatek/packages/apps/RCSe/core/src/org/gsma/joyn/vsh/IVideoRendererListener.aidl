package org.gsma.joyn.vsh;

/**
 * Video renderer event listener interface
 */
interface IVideoRendererListener {
	void onRendererOpened();

	void onRendererStarted();

	void onRendererStopped();

	void onRendererClosed();

	void onRendererFailed();		
	
}
