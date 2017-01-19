/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package org.gsma.joyn.capability;

import java.util.Iterator;
import java.util.Set;

import org.gsma.joyn.JoynContactFormatException;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.JoynServiceRegistrationListener;
import org.gsma.joyn.Permissions;
import org.gsma.joyn.capability.ICapabilityService;


import com.mediatek.rcse.api.Logger;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.IInterface;

/**
 * Capability service offers the main entry point to read capabilities
 * of remote contacts, to initiate capability discovery and to receive
 * capabilities updates. Several applications may connect/disconnect
 * to the API.
 * 
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 * 
 * @author Jean-Marc AUFFRET
 */
public class CapabilityService extends JoynService {
    /**
     * Intent broadcasted to discover extensions
     * 
     * @see CapabilityService.EXTENSION_MIME_TYPE
     */
    public final static String INTENT_EXTENSIONS = "org.gsma.joyn.capability.EXTENSION";
    
	/**
	 * Extension MIME type
	 */
	public final static String EXTENSION_MIME_TYPE = "org.gsma.joyn";

	/**
	 * API
	 */
	private ICapabilityService api = null;
	
	public static final String TAG = "CapabilityService";
	
    /**
     * Constructor
     * 
     * @param ctx Application context
     * @param listener Service listener
     */
    public CapabilityService(Context ctx, JoynServiceListener listener) {
    	super(ctx, listener);
    }

    /**
     * Connects to the API
     */
    public void connect() {
    	/*if (this.ctx.checkCallingOrSelfPermission(Permissions.RCS_READ_CAPABILITIES) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_READ_CAPABILITIES");
		}*/
    	/*if (this.ctx.checkCallingOrSelfPermission(Permissions.ANDROID_READ_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission ANDROID_READ_CONTACTS");
		}*/
    	Logger.i(TAG, "connect() entry");
    	ctx.bindService(new Intent(ICapabilityService.class.getName()), apiConnection, 0);
    }
    
    /**
     * Disconnects from the API
     */
    public void disconnect() {
    	try {
    		Logger.i(TAG, "disconnect() entry");
    		ctx.unbindService(apiConnection);
        } catch(IllegalArgumentException e) {
        	// Nothing to do
        }
    }

	/**
	 * Set API interface
	 * 
	 * @param api API interface
	 */
    protected void setApi(IInterface api) {
    	super.setApi(api);
    	Logger.i(TAG, "setApi entry" + api);
        this.api = (ICapabilityService)api;
    }
    
    /**
	 * Service connection
	 */
	private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
        	Logger.i(TAG, "onServiceConnected entry");
        	setApi(ICapabilityService.Stub.asInterface(service));
        	if (serviceListener != null) {
        		serviceListener.onServiceConnected();
        	}
        }

        public void onServiceDisconnected(ComponentName className) {
        	Logger.i(TAG, "onServiceDisconnected entry");
        	setApi(null);
        	if (serviceListener != null) {
        		serviceListener.onServiceDisconnected(Error.CONNECTION_LOST);
        	}
        }
    };
    
    /**
     * Returns the capabilities supported by the local end user. The supported
     * capabilities are fixed by the MNO and read during the provisioning.
     * 
     * @return Capabilities
     * @throws JoynServiceException
     */
    public Capabilities getMyCapabilities() throws JoynServiceException {
    	Logger.i(TAG, "getMyCapabilities entry");
		if (api != null) {
			try {
				return api.getMyCapabilities();
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }
    
    /**
     * Returns the capabilities of a given contact from the local database. This
     * method doesn’t request any network update to the remote contact. The parameter
     * contact supports the following formats: MSISDN in national or international
     * format, SIP address, SIP-URI or Tel-URI. If the format of the contact is not
     * supported an exception is thrown.
     * 
     * @param contact Contact
     * @return Capabilities
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
     */
    public Capabilities getContactCapabilities(String contact) throws JoynServiceException, JoynContactFormatException {
    	Logger.i(TAG, "getContactCapabilities entry" + contact);
    	if (api != null) {
			try {
				return api.getContactCapabilities(contact);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }    

    /**
	 * Requests capabilities to a remote contact. This method initiates in background
	 * a new capability request to the remote contact by sending a SIP OPTIONS. The
	 * result of the capability request is sent asynchronously via callback method of
	 * the capabilities listener. A capability refresh is only sent if the timestamp
	 * associated to the capability has expired (the expiration value is fixed via MNO
	 * provisioning). The parameter contact supports the following formats: MSISDN in
	 * national or international format, SIP address, SIP-URI or Tel-URI. If the format
	 * of the contact is not supported an exception is thrown. The result of the
	 * capability refresh request is provided to all the clients that have registered
	 * the listener for this event.
   	 * 
	 * @param contact Contact
	 * @throws JoynServiceException
	 * @throws JoynContactFormatException
	 */
	public void requestContactCapabilities(String contact) throws JoynServiceException, JoynContactFormatException {
		Logger.i(TAG, "requestContactCapabilities entry" + contact);
		if (api != null) {
			try {
				api.requestContactCapabilities(contact);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

    /**
     * Requests capabilities for a group of remote contacts. This method initiates
     * in background new capability requests to the remote contact by sending a
     * SIP OPTIONS. The result of the capability request is sent asynchronously via
     * callback method of the capabilities listener. A capability refresh is only
     * sent if the timestamp associated to the capability has expired (the expiration
     * value is fixed via MNO provisioning). The parameter contact supports the
     * following formats: MSISDN in national or international format, SIP address,
     * SIP-URI or Tel-URI. If the format of the contact is not supported an exception
     * is thrown. The result of the capability refresh request is provided to all the
     * clients that have registered the listener for this event.
	 * 
	 * @param contacts List of contacts
	 * @throws JoynServiceException
	 * @throws JoynContactFormatException
	 */
	public void requestContactCapabilities(Set<String> contacts) throws JoynServiceException, JoynContactFormatException {
		Logger.i(TAG, "requestContactCapabilities entry" + contacts);
		Iterator<String> values = contacts.iterator();
		while(values.hasNext()) {
			requestContactCapabilities(values.next());
		}
	}

    /**
	 * Requests capabilities for all contacts existing in the local address book. This
	 * method initiates in background new capability requests for each contact of the
	 * address book by sending SIP OPTIONS. The result of a capability request is sent
	 * asynchronously via callback method of the capabilities listener. A capability
	 * refresh is only sent if the timestamp associated to the capability has expired
	 * (the expiration value is fixed via MNO provisioning). The result of the capability
	 * refresh request is provided to all the clients that have registered the listener
	 * for this event.
	 * 
	 * @param contacts List of contacts
	 * @throws JoynServiceException
	 */
	public void requestAllContactsCapabilities() throws JoynServiceException {
		Logger.i(TAG, "requestAllContactsCapabilities entry");
		if (api != null) {
			try {
				api.requestAllContactsCapabilities();
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Returns service version
	 * 
	 * @return Version
	 * @see Build.VERSION_CODES
	 * @throws JoynServiceException
	 */
	public int getServiceVersion() throws JoynServiceException {
		Logger.i(TAG, "getServiceVersion entry");
		if (api != null) {
			if (version == null) {
				try {
					version = api.getServiceVersion();
				} catch (Exception e) {
					throw new JoynServiceException(e.getMessage());
				}
			}
			Logger.i(TAG, "getServiceVersion is" + version);
			return version;
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Returns true if the service is registered to the platform, else returns
	 * false
	 * 
	 * @return Returns true if registered else returns false
	 * @throws JoynServiceException
	 */
	public boolean isServiceRegistered() throws JoynServiceException {
		Logger.i(TAG, "isServiceRegistered entry");
		if (api != null) {
			boolean serviceStatus = false;
			try {
				serviceStatus = api.isServiceRegistered();
			} catch (Exception e) {				
				throw new JoynServiceException(e.getMessage());
			}
			Logger.i(TAG, "isServiceRegistered" + serviceStatus);
			return serviceStatus;
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}
	
	/**
	 * Registers a listener on service registration events
	 * 
	 * @param listener Service registration listener
     * @throws JoynServiceException
	 */
	public void addServiceRegistrationListener(JoynServiceRegistrationListener listener) throws JoynServiceException {
		Logger.i(TAG, "addServiceRegistrationListener entry" + listener);
		if (api != null) {
			try {
				api.addServiceRegistrationListener(listener);
			} catch (Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}
	
	/**
	 * Unregisters a listener on service registration events
	 * 
	 * @param listener Service registration listener
     * @throws JoynServiceException
	 */
	public void removeServiceRegistrationListener(JoynServiceRegistrationListener listener) throws JoynServiceException  {
		Logger.i(TAG, "removeServiceRegistrationListener entry" + listener);
		if (api != null) {
			try {
				api.removeServiceRegistrationListener(listener);
			} catch (Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Registers a capabilities listener on any contact
	 * 
	 * @param listener Capabilities listener
	 * @throws JoynServiceException
	 */
	public void addCapabilitiesListener(CapabilitiesListener listener) throws JoynServiceException {
		Logger.i(TAG, "addCapabilitiesListener entry" + listener);
		if (api != null) {
			try {
				api.addCapabilitiesListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Unregisters a capabilities listener
	 * 
	 * @param listener Capabilities listener
	 * @throws JoynServiceException
	 */
	public void removeCapabilitiesListener(CapabilitiesListener listener) throws JoynServiceException {
		Logger.i(TAG, "removeCapabilitiesListener entry" + listener);
		if (api != null) {
			try {
				api.removeCapabilitiesListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Registers a capabilities listener on a list of contacts
	 * 
	 * @param contacts Set of contacts
	 * @param listener Capabilities listener
	 * @throws JoynServiceException
	 * @throws JoynContactFormatException
	 */
	public void addCapabilitiesListener(Set<String> contacts, CapabilitiesListener listener) throws JoynServiceException, JoynContactFormatException {
		Logger.i(TAG, "addCapabilitiesListener entry" + contacts + listener);
		if (api != null) {
			try {
				Iterator<String> list = contacts.iterator();
				while(list.hasNext()) { 
					String contact = list.next();
					api.addContactCapabilitiesListener(contact, listener);
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Unregisters a capabilities listener on a list of contacts
	 * 
	 * @param contacts Set of contacts
	 * @param listener Capabilities listener
	 * @throws JoynServiceException
	 * @throws JoynContactFormatException
	 */
	public void removeCapabilitiesListener(Set<String> contacts, CapabilitiesListener listener) throws JoynServiceException, JoynContactFormatException {
		Logger.i(TAG, "removeCapabilitiesListener entry" + contacts + listener);
		if (api != null) {
			try {
				Iterator<String> list = contacts.iterator();
				while(list.hasNext()) { 
					String contact = list.next();
					api.removeContactCapabilitiesListener(contact, listener);
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}
}
