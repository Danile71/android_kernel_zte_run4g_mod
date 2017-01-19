package org.gsma.joyn.session;

import org.gsma.joyn.session.IMultimediaSessionListener;

/**
 * Multimedia session interface
 */
interface IMultimediaSession {
	String getSessionId();
	
	String getRemoteContact();
	
	String getServiceId();
	
	int getState();
	
	int getDirection();
	
	void acceptInvitation();
	
	void rejectInvitation();
	
	void abortSession();
	
	void addEventListener(in IMultimediaSessionListener listener);
	
	void removeEventListener(in IMultimediaSessionListener listener);

	boolean sendMessage(in byte[] content);
}

