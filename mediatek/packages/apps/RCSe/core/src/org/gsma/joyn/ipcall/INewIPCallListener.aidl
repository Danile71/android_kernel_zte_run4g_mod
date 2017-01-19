package org.gsma.joyn.ipcall;

/**
 * Callback method for new IP call invitations
 */
interface INewIPCallListener {
	void onNewIPCall(in String callId);
}