package org.gsma.joyn.session;

import org.gsma.joyn.IJoynServiceRegistrationListener;
import org.gsma.joyn.session.IMultimediaSession;
import org.gsma.joyn.session.IMultimediaSessionListener;

/**
 * Multimedia session service API
 */
interface IMultimediaSessionService {
	boolean isServiceRegistered();

	void addServiceRegistrationListener(IJoynServiceRegistrationListener listener);

	void removeServiceRegistrationListener(IJoynServiceRegistrationListener listener); 

	List<IBinder> getSessions(in String serviceId);
	
	IMultimediaSession getSession(in String sessionId);
	
	IMultimediaSession initiateSession(in String serviceId, in String contact, in IMultimediaSessionListener listener);
	
	boolean sendMessage(in String serviceId, in String contact, in byte[] content);
	
	int getServiceVersion();
}


