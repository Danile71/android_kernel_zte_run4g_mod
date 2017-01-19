/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.activities;

import java.util.List;

import org.gsma.joyn.JoynContactFormatException;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.chat.ChatService;
import org.gsma.joyn.chat.Geoloc;
import org.gsma.joyn.gsh.GeolocSharingListener;
import org.gsma.joyn.gsh.GeolocSharingService;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.orangelabs.rcs.R;

/**
 * The activity is a container which contains blacklist fragments.
 */
public class GeolocActivity extends Activity {
	
	private static final long MINIMUM_DISTANCE_CHANGE_FOR_UPDATES = 1; // in Meters
	private static final long MINIMUM_TIME_BETWEEN_UPDATES = 1000; // in Milliseconds

	public static final String TAG = "GeolocActivity";
	protected LocationManager mLocationManager = null;
	protected Button mShareLocationButton = null;
	double mLongitude=0, mLatitude=0;
	GeolocSharingService mGeolocApi = null;
	String mContact= "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Logger.d(TAG, "onCreate() called ");
        setContentView(R.layout.geoloc_activity);
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.i(TAG, "ApiManager instance is null");
        }
        if(instance != null) {
        	mGeolocApi = instance.getGeolocSharingApi();
        	if (mGeolocApi == null) {
	            Logger.d(TAG, "mGeolocApi instance is null");
	        } 
        }
        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);

        Button mShareLocationButton = (Button) findViewById(R.id.share_location_button);
        mShareLocationButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				shareGeoloc();
				
			}
		});
        mLocationManager.requestLocationUpdates(
        		                LocationManager.GPS_PROVIDER,
        		                MINIMUM_TIME_BETWEEN_UPDATES,
        		                MINIMUM_DISTANCE_CHANGE_FOR_UPDATES,
        		                new MyLocationListener()
        		                );
        List<RcsContact> list = ContactsListManager.getInstance().CONTACTS_LIST;
        RcsContact rcsContact = list.get(0);
        mContact = rcsContact.mNumber;

    }
    
    protected void shareGeoloc() {
    	Logger.d(TAG, "shareGeoloc() called ");
    	Location location = mLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
    	Geoloc geoloc = null;
    	if(location != null) {
    	    geoloc = new Geoloc("geoloc", location.getLatitude(), location.getLongitude(), location.getTime());
    	}
    	else {
    		geoloc = new Geoloc("geoloc", 0, 0, 0);
    	}
    	if(mGeolocApi != null)
    	{
    		try {
				mGeolocApi.shareGeoloc(mContact, geoloc, new MyGeolocSharingListener());
			} catch (JoynContactFormatException e) {
				
				e.printStackTrace();
			} catch (JoynServiceException e) {
				
				e.printStackTrace();
			}
    	}
    }
    
    public class MyGeolocSharingListener extends GeolocSharingListener{

		@Override
		public void onSharingStarted() {
			Logger.d(TAG, "onSharingStarted() called ");
			
		}

		@Override
		public void onSharingAborted() {
			Logger.d(TAG, "onSharingAborted() called ");
			
		}

		@Override
		public void onSharingError(int error) {
			Logger.d(TAG, "onSharingError() called " + error);
			
		}

		@Override
		public void onSharingProgress(long currentSize, long totalSize) {
			Logger.d(TAG, "onSharingProgress() called ");
			
		}

		@Override
		public void onGeolocShared(Geoloc geoloc) {
			Logger.d(TAG, "onGeolocShared() called " + geoloc);
			
		}
    	
    }
    
    
    private class MyLocationListener implements LocationListener {

		@Override
		public void onLocationChanged(Location location) {
			
			mLongitude = location.getLongitude();
			mLatitude = location.getLatitude();
			Logger.d(TAG, "onLocationChanged called latitude ="+mLatitude + ", longitude="+mLongitude);   
			
		}

		@Override
		public void onProviderDisabled(String provider) {
			
			Logger.d(TAG, "onProviderDisabled " +provider);   
		}

		@Override
		public void onProviderEnabled(String provider) {
			
			Logger.d(TAG, "onProviderEnabled " +provider);
		}

		@Override
		public void onStatusChanged(String provider, int status, Bundle extras) {
			
			Logger.d(TAG, "onStatusChanged " +provider + ",status:"+status);
		}
    	
    }

}
