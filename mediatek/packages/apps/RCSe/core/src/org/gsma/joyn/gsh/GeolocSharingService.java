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
package org.gsma.joyn.gsh;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.gsma.joyn.JoynContactFormatException;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.Permissions;
import org.gsma.joyn.chat.Geoloc;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.IInterface;

import com.mediatek.rcse.api.Logger;
/**
 * This class offers the main entry point to share geolocation info
 * during a CS call. Several applications may connect/disconnect to
 * the API.
 * 
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 * 
 * @author Jean-Marc AUFFRET
 */
public class GeolocSharingService extends JoynService {
	/**
	 * API
	 */
	private IGeolocSharingService api = null;
	
	public static final String TAG = "TAPI-GeolocSharingService";
	
    /**
     * Constructor
     * 
     * @param ctx Application context
     * @param listener Service listener
     */
    public GeolocSharingService(Context ctx, JoynServiceListener listener) {
    	super(ctx, listener);
    }

    /**
     * Connects to the API
     */
    public void connect() {
    	Logger.i(TAG, "GeolocSharing connect() entry");    	
    	ctx.bindService(new Intent(IGeolocSharingService.class.getName()), apiConnection, 0);
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
    	
        this.api = (IGeolocSharingService)api;
    }

    /**
	 * Service connection
	 */
	private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
        	Logger.i(TAG, "onServiceConnected entry " + className);
        	setApi(IGeolocSharingService.Stub.asInterface(service));
        	if (serviceListener != null) {
        		serviceListener.onServiceConnected();
        	}
        }

        public void onServiceDisconnected(ComponentName className) {
        	Logger.i(TAG, "onServiceDisconnected entry " + className);
        	setApi(null);
        	if (serviceListener != null) {
        		serviceListener.onServiceDisconnected(JoynService.Error.CONNECTION_LOST);
        	}
        }
    };

    /**
     * Shares a geolocation with a contact. An exception if thrown if there is no ongoing
     * CS call. The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI. If the format of the
     * contact is not supported an exception is thrown.
     * 
     * @param contact Contact
     * @param geoloc Geolocation info
     * @param listener Geoloc sharing event listener
     * @return Geoloc sharing
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
	 * @see Geoloc
     */
    public GeolocSharing shareGeoloc(String contact, Geoloc geoloc, GeolocSharingListener listener) throws JoynServiceException, JoynContactFormatException {
    	if (this.ctx.checkCallingOrSelfPermission(Permissions.RCS_LOCATION_SEND) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_LOCATION_SEND");
		}
    	if (this.ctx.checkCallingOrSelfPermission(Permissions.RCS_USE_CHAT) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_USE_CHAT");
		}

    	Logger.i(TAG, "shareGeoloc() entry contact="+contact +" geoloc ="+geoloc + " listener =" +listener);  
		if (api != null) {
			try {
				IGeolocSharing sharingIntf = api.shareGeoloc(contact, geoloc, listener);
				if (sharingIntf != null) {
					return new GeolocSharing(sharingIntf);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }    
    
    /**
     * Returns the list of geoloc sharings in progress
     * 
     * @return List of geoloc sharings
     * @throws JoynServiceException
     */
    public Set<GeolocSharing> getGeolocSharings() throws JoynServiceException {
    	Logger.i(TAG, "getGeolocSharings entry ");
		if (api != null) {
			try {
	    		Set<GeolocSharing> result = new HashSet<GeolocSharing>();
				List<IBinder> ishList = api.getGeolocSharings();
				for (IBinder binder : ishList) {
					GeolocSharing sharing = new GeolocSharing(IGeolocSharing.Stub.asInterface(binder));
					result.add(sharing);
				}
				Logger.i(TAG, "getGeolocSharings returning " + result);
				return result;
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }    

    /**
     * Returns a current geoloc sharing from its unique ID
     * 
     * @param sharingId Sharing ID
     * @return Geoloc sharing or null if not found
     * @throws JoynServiceException
     */
    public GeolocSharing getGeolocSharing(String sharingId) throws JoynServiceException {
    	Logger.i(TAG, "getGeolocSharing entry " + sharingId);
		if (api != null) {
			try {
				IGeolocSharing sharingIntf = api.getGeolocSharing(sharingId);
				if (sharingIntf != null) {
					return new GeolocSharing(sharingIntf);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }    
    
    /**
     * Returns a current geoloc sharing from its invitation Intent
     * 
     * @param intent Invitation intent
     * @return Geoloc sharing or null if not found
     * @throws JoynServiceException
     */
    public GeolocSharing getGeolocSharingFor(Intent intent) throws JoynServiceException {
    	Logger.i(TAG, "getGeolocSharingFor entry " + intent);
		if (api != null) {
			try {
				String sharingId = intent.getStringExtra(GeolocSharingIntent.EXTRA_SHARING_ID);
				if (sharingId != null) {
					return getGeolocSharing(sharingId);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }     
    
    /**
	 * Registers a new geoloc sharing invitation listener
	 * 
	 * @param listener New geoloc sharing listener
	 * @throws JoynServiceException
	 */
	public void addNewGeolocSharingListener(NewGeolocSharingListener listener) throws JoynServiceException {
		Logger.i(TAG, "addNewGeolocSharingListener entry " + listener);
		if (api != null) {
			try {
				api.addNewGeolocSharingListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Unregisters a new geoloc sharing invitation listener
	 * 
	 * @param listener New geoloc sharing listener
	 * @throws JoynServiceException
	 */
	public void removeNewGeolocSharingListener(NewGeolocSharingListener listener) throws JoynServiceException {
		Logger.i(TAG, "removeNewGeolocSharingListener entry " + listener);
		if (api != null) {
			try {
				api.removeNewGeolocSharingListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}
}
