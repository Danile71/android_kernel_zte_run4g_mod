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

package com.mediatek.rcse.service;

import org.gsma.joyn.JoynService;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.JoynServiceRegistrationListener;
import org.gsma.joyn.capability.Capabilities;
import org.gsma.joyn.capability.CapabilityService;
import org.gsma.joyn.chat.ChatMessage;
import org.gsma.joyn.chat.ChatService;
import org.gsma.joyn.chat.GeolocMessage;
import org.gsma.joyn.contacts.ContactsService;
import org.gsma.joyn.ft.FileTransferService;
import org.gsma.joyn.gsh.GeolocSharingService;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.RemoteException;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.provider.MediatekRichProviderHelper;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcse.api.NetworkConnectivityApi;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.mediatek.rcse.api.terms.TermsApi;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * This class manages the APIs, providing a convenient way for API invocations.
 */
public class ApiManager {
    public static final  String TAG = "ApiManager";

    private static ApiManager sInstance = null;

    private RegistrationApi mRegistrationApi = null;
    private CapabilityService mCapabilitiesApi = null;
    private TermsApi mTermsApi = null;
    private ChatService mChatApi = null;
    private ContactsService mContactsApi = null;
    private FileTransferService mFileTransferApi = null;
    private GeolocSharingService mGeolocSharingApi = null;
    private NetworkConnectivityApi mNetworkConnectivityApi = null;
    private Context mContext = null;
    private static long sMaxFileSize = 0;
    private static long sWarningFileSize = 0;
    private RcseComponentController mRcseComponentController = null;
    private static boolean registrationStatus = false;
    public static String SERVICE_CONNECTED_EVENT = "com.mediatek.rcse.SERVICE_CONNECTED_EVENT";


    /**
     * This method should only be called from ApiService, for APIs
     * initialization.
     * 
     * @param context The Context of this application.
     * @return true If initialize successfully, otherwise false.
     */
    public static synchronized boolean initialize(Context context) {
        Logger.v(TAG, "initialize() entry");
        if (null != sInstance) {
            Logger
                    .w(TAG,
                            "initialize() sInstance has existed, is it really the first time you call this method?");
            return true;
        } else {
            if (null != context) {
                RcsSettings rcsSetting = RcsSettings.getInstance();
                AppSettings appSetting = AppSettings.getInstance();
                if (rcsSetting == null) {
                    RcsSettings.createInstance(context);
                }
                if(appSetting == null)
                {
                	AppSettings.createInstance(context);
                }
                if(MediatekRichProviderHelper.getInstance()==null)
                	MediatekRichProviderHelper.createInstance(context);
                ApiManager apiManager = new ApiManager(context);
                sInstance = apiManager;
                AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... arg0) {
                        sMaxFileSize = RcsSettings.getInstance().getMaxFileTransferSize() * 1024;
                        sWarningFileSize =
                                RcsSettings.getInstance().getWarningMaxFileTransferSize() * 1024;
                        return null;
                    }
                };
                task.execute();
                return true;
            } else {
                Logger.e(TAG, "initialize() the context is null");
                return false;
            }
        }
    }

    /**
     * Get the context
     * 
     * @return Context
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Get the RcseComponentController
     * 
     * @return RcseComponentController
     */
    public RcseComponentController getRcseComponentController() {
        return mRcseComponentController;
    }

    /**
     * Get the max size for file transfer
     * 
     * @return max size
     */
    public long getMaxSizeforFileThransfer() {
        return sMaxFileSize;
    }

    /**
     * Get the warning size for file transfer
     * 
     * @return warning size
     */
    public long getWarningSizeforFileThransfer() {
        return sWarningFileSize;
    }

    /**
     * Get the instance of ApiManager
     * 
     * @return The instance of ApiManager, or null if the instance has not been
     *         initialized.
     */
    public static ApiManager getInstance() {
        Logger.v(TAG, "getInstance() : sInstance = " + sInstance);
        return sInstance;
    }

    /**
     * Get the connected RegistrationApi
     * 
     * @return The instance of RegistrationApi, or null if the instance has not
     *         connected.
     */
    public RegistrationApi getRegistrationApi() {
        Logger.v(TAG, "getRegistrationApi() : mRegistrationApi = " + mRegistrationApi);
        return mRegistrationApi;
    }

    /**
     * Get the connected CapabilityApi
     * 
     * @return The instance of CapabilityApi, or null if the instance has not
     *         connected.
     */
    public CapabilityService getCapabilityApi() {
        Logger.v(TAG, "getCapabilityApi() : mCapabilitiesApi = " + mCapabilitiesApi);
        return mCapabilitiesApi;
    }

    /**
     * Get the connected TermsApi
     * 
     * @return The instance of TermsApi, or null if the instance has not
     *         connected.
     */
    public TermsApi getTermsApi() {
        Logger.v(TAG, "getTermsApi() : mTermsApi = " + mTermsApi);
        return mTermsApi;
    }
    
    /**
     * Get the connected ChatService
     * 
     * @return The instance of ChatService, or null if the instance has not
     *         connected.
     */
    public ChatService getChatApi() {
        Logger.v(TAG, "getChatService() : mChatApi = " + mChatApi);
        return mChatApi;
    }

    /**
     * Get the connected ContactsService
     * 
     * @return The instance of ContactsService, or null if the instance has not
     *         connected.
     */
    public ContactsService getContactsApi() {
        Logger.v(TAG, "getContactsApi() : mContactsApi = " + mContactsApi);
        return mContactsApi;
    }

    /**
     * Get the connected ChatService
     * 
     * @return The instance of ChatService, or null if the instance has not
     *         connected.
     */
    public FileTransferService getFileTransferApi() {
        Logger.v(TAG, "getFileTransferApi() : mFileTransferApi = " + mFileTransferApi);
        return mFileTransferApi;
    }

    public GeolocSharingService getGeolocSharingApi() {
        Logger.v(TAG, "getGeolocSharingApi() : mGeolocSharingApi = " + mGeolocSharingApi);
        return mGeolocSharingApi;
    }

    /**
     * Get the connected NetworkConnectivityApi
     * 
     * @return The instance of getNetworkConnectivityApi, or null if the
     *         instance has not connected.
     */
    public NetworkConnectivityApi getNetworkConnectivityApi() {
        Logger.v(TAG,
                "getNetworkConnectivityApi() : mNetworkConnectivityApi = "
                        + mNetworkConnectivityApi);
        return mNetworkConnectivityApi;
    }


    private ApiManager(Context context) {
        Logger.d(TAG, "ApiManager() entry");
        mContext = context;
        if (!JoynService.isServiceStarted(context)) {
        	Logger.d(TAG, "RcsCoreService is not started yet, so services won't connect ");
        }
        new ManagedRegistrationApi(context).connect();       
        mTermsApi = new TermsApi(context, new MyTermsApiServiceListener());
        mTermsApi.connect();
        mCapabilitiesApi = new CapabilityService(context, new MyCapabilitiesServiceListener());
        mCapabilitiesApi.connect();
        mChatApi = new ChatService(context,new MyChatServiceListener());       
        mChatApi.connect();
        mContactsApi = new ContactsService(context,new MyContactsServiceListener());       
        mContactsApi.connect();
        mFileTransferApi = new FileTransferService(context, new MyFileTransferServiceListener());
        mFileTransferApi.connect();
        mGeolocSharingApi = new GeolocSharingService(context, new MyGeolocSharingServiceListener());
        mGeolocSharingApi.connect();
        mNetworkConnectivityApi = new NetworkConnectivityApi(context, new MyNetworkConnectivityServiceListener());
        mNetworkConnectivityApi.connect();
        mRcseComponentController = new RcseComponentController();
    }
    
    public class MyTermsApiServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {

		}

		@Override
		public void onServiceDisconnected(int error) {
				ApiManager.this.mTermsApi = null;
		}
    	
    }
    
    public class MyContactsServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {
			Logger.d(TAG, "ContactsService onServiceConnected entry ");
			
			}

		@Override
		public void onServiceDisconnected(int error) {
			Logger.i(TAG, "MyContactsServiceListener onServiceDisconnected entry");
			ApiManager.this.mContactsApi = null;
		}
    	
    }
    
    public class MyChatServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {
			Logger.d(TAG, "ChatService onServiceConnected entry ");
			if(ContactsListManager.getInstance() != null)
			{
			    ContactsListManager.getInstance().onContactsDbChange();// need to update contact list
            }
		}

		@Override
		public void onServiceDisconnected(int error) {
			Logger.i(TAG, "MyChatServiceListener onServiceDisconnected entry");
				ApiManager.this.mChatApi = null;
		}
    	
    }
    
    public class MyFileTransferServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {

		}

		@Override
		public void onServiceDisconnected(int error) {
				ApiManager.this.mFileTransferApi = null;
		}
    	
    }
    
    public class MyGeolocSharingServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {

		}

		@Override
		public void onServiceDisconnected(int error) {
				ApiManager.this.mGeolocSharingApi = null;
		}
    	
    }
    
    public class MyCapabilitiesServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {

		}

		@Override
		public void onServiceDisconnected(int error) {
				ApiManager.this.mCapabilitiesApi = null;
		}
    	
    }
    
    public class MyNetworkConnectivityServiceListener implements JoynServiceListener{

		@Override
		public void onServiceConnected() {

		}

		@Override
		public void onServiceDisconnected(int error) {
				ApiManager.this.mNetworkConnectivityApi = null;
		}
    	
    }
    
    

    private class ManagedRegistrationApi extends RegistrationApi {
        public static final String TAG = "ManagedRegistrationApi";

        public ManagedRegistrationApi(Context context) {
            super(context);
        }

        @Override
        public void handleConnected() {
            Logger.v(TAG, "handleConnected() entry, this.isRegistered() = "
                    + this.isRegistered());
            mRegistrationApi = this;

            mRegistrationApi
                    .addRegistrationStatusListener(new RegistrationStatusListener());
        }

        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            if (mRegistrationApi == this) {
                Logger.i(TAG,
                        "handleDisconnected() mRegistrationApi disconnected");
                mRegistrationApi = null;
            }
        }

        private class RegistrationStatusListener implements
                IRegistrationStatusListener {
            @Override
            public void onStatusChanged(boolean status) {
                Logger.d(TAG, "onStatusChanged() entry, status is " + status);
                if (status) {
                    Logger.d(TAG, "onStatusChanged() : mChatApi = "
                            + mChatApi);
                    //if (null == mChatApi) {
                        Logger.d(TAG,
                                "onStatusChanged() : try to initialize chatapi");
                        initializeMessagingApi();
                        initializeContactsApi();
                    //}
                    //if (null == mFileTransferApi) {
                        Logger.d(TAG,
                                "onStatusChanged() : try to initialize filetransferapi");
                        initializeFileTransferApi();
                    //}
                    //if (null == mNetworkConnectivityApi) {
                        Logger.d(TAG,
                                "onStatusChanged():try to initialize NetworkConnectivityApi");
                        initializeNetworkConnectivityApi();
                    //}
                    //if (null == mTermsApi) {
                        Logger.d(TAG,
                                "onStatusChanged():try to initialize TermsApi");
                        initializeTermsApi();
                    //}
                        
                        Logger.d(TAG,
                        "onStatusChanged():try to initialize CapabilityApi");
                        initializeCapabilityApi();
                        initializeGeolocSharingApi();
                    AsyncTask.execute(new Runnable() {
                        @Override
                        public void run() {
                            sMaxFileSize = RcsSettings.getInstance()
                                    .getMaxFileTransferSize() * 1024;
                            sWarningFileSize = RcsSettings.getInstance()
                                    .getWarningMaxFileTransferSize() * 1024;
                        }
                    });
                }
            }

            private boolean initializeContactsApi() {
            	Logger.d(TAG, "initializeContactsApi() entry");
                mContactsApi = new ContactsService(mContext, new MyContactsServiceListener());
                mContactsApi.connect();
                
                
                return true;
            }
            private boolean initializeNetworkConnectivityApi() {
                Logger.d(TAG, "initializeNetworkConnectivityApi() entry");
                
                mNetworkConnectivityApi = new NetworkConnectivityApi(mContext, new MyNetworkConnectivityServiceListener());
                mNetworkConnectivityApi.connect();
                return true;
            }

            private boolean initializeTermsApi() {
                Logger.d(TAG, "initializeTermsApi() entry");
                mTermsApi = new TermsApi(mContext, new MyTermsApiServiceListener());
                mTermsApi.connect();
                
                
                return true;
            }

            private boolean initializeMessagingApi() {
                Logger.d(TAG, "initializeMessagingApi() entry");
                mChatApi = new ChatService(mContext,new MyChatServiceListener());       
                mChatApi.connect();
                
                return true;
            }
            
            private boolean initializeCapabilityApi() {
                Logger.d(TAG, "initializeCapabilityApi() entry");
                mCapabilitiesApi = new CapabilityService(mContext, new MyCapabilitiesServiceListener());
                mCapabilitiesApi.connect();
                
                
                return true;
            }
            
            private boolean initializeFileTransferApi() {
                Logger.d(TAG, "initializeFileTransferApi() entry");
                mFileTransferApi = new FileTransferService(mContext, new MyFileTransferServiceListener());
                mFileTransferApi.connect();
                return true;
            }
            
            private boolean initializeGeolocSharingApi() {
                Logger.d(TAG, "initializeGeolocSharingApi() entry");
                mGeolocSharingApi = new GeolocSharingService(mContext, new MyGeolocSharingServiceListener());
                mGeolocSharingApi.connect();
                return true;
            }
        }
    }
    

    /**
     * control the RCS component according to the configuration and active
     * status.
     */
    public class RcseComponentController {
        private boolean mConfigurationStatus = true;
        private boolean mServiceActiveStatus = true;

        /**
         * Set configuration status and control the rcse component.
         * 
         * @param status the configuration status.
         */
        public void onConfigurationStatusChanged(boolean status) {
            Logger.d(TAG, "onConfigurationStatusChanged() entry status is " + status);
            mConfigurationStatus = status;
            controlRcseComponent();

        }

        /**
         * Set ServiceActive status and control the rcse component.
         * 
         * @param status the status of ServiceActive .
         */
        public void onServiceActiveStatusChanged(boolean status) {
            Logger.d(TAG, "onServiceActiveStatusChanged() entry status is " + status);
            mServiceActiveStatus = status;
            controlRcseComponent();
        }

        /**
         * control the RCS component according to the configuration and active
         * status.
         */
        private void controlRcseComponent() {
            Logger.d(TAG, "controlRcseComponent() entry  the mConfigurationstatus is "
                    + mConfigurationStatus + " ServiceactiveStatus is " + mServiceActiveStatus);
            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG, "controlRcseComponent() entry is integration mode ");
                CoreApplication.setIntegrationModeComponent(mContext);
            } else {
                if (mConfigurationStatus && mServiceActiveStatus) {
                    CoreApplication.setComponentStatus(mContext, true);
                } else {
                    CoreApplication.setComponentStatus(mContext, false);
                    RcsNotification.getInstance().cancelNotification();
                }
            }
        }
    }

}
