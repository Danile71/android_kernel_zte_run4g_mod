/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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

import android.app.Service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.nsd.NsdManager.RegistrationListener;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.rcse.activities.ConfigMessageActicity;
import com.mediatek.rcse.api.ICapabilityRemoteListener;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi.IRegistrationStatusListener;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager.RcseComponentController;
import com.mediatek.rcse.service.binder.IRemoteWindowBinder;
import com.mediatek.rcse.service.binder.IRemoteBlockingRequest;
import com.mediatek.rcse.service.binder.WindowBinder;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.service.StartService;
import com.mediatek.rcse.api.ContactsApiIntents;
import com.mediatek.rcse.api.terms.TermsApiIntents;

import java.util.List;

import org.gsma.joyn.Intents;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.JoynServiceRegistrationListener;
import org.gsma.joyn.capability.Capabilities;
import org.gsma.joyn.capability.CapabilityService;
import org.gsma.joyn.chat.ChatIntent;
import org.gsma.joyn.chat.GroupChatIntent;
import org.gsma.joyn.ft.FileTransferIntent;
import org.gsma.joyn.ish.ImageSharingIntent;
import org.gsma.joyn.vsh.VideoSharingIntent;

//This Service will provide the remote API to the applications.
public class ApiService extends Service {
    public static final String TAG = "ApiService";

    private ApiReceiver mReceiver = null;
    private RegistrationStatusStub mRegistrationStatusStub = null;
    //private CapabilitiesStub mCapabilitiesStub = null;
    private BlockingRequestStub mBlockingRequestStub = null;
    private WindowBinder mWindowBinder = null;
    // Low memory broadcast action
    private static final String MEMORY_LOW_ACTION = "ACTION_DEVICE_STORAGE_LOW";
    // Memory okay broadcast action
    private static final String MEMORY_OK_ACTION = "ACTION_DEVICE_STORAGE_OK";
    public static final String CORE_CONFIGURATION_STATUS = "status";
    public static RegistrationListener regListener = null;

    @Override
    public void onCreate() {
        Logger.v(TAG, "ApiService onCreate() entry");
        mReceiver = new ApiReceiver();
        IntentFilter intentFilter = new IntentFilter();
        
        intentFilter.addAction(ChatIntent.ACTION_NEW_CHAT);
        intentFilter.addAction(GroupChatIntent.ACTION_NEW_INVITATION);
        intentFilter.addAction(ApiManager.SERVICE_CONNECTED_EVENT);
        intentFilter.addAction(Intents.Client.SERVICE_UP);
        intentFilter.addAction(FileTransferIntent.ACTION_NEW_INVITATION);
        intentFilter.addAction(ContactsApiIntents.CONTACT_BLOCK_REQUEST);
        intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_OK);
        intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_LOW);        
        intentFilter.addAction(StartService.CONFIGURATION_STATUS);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_ACK);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_USER_NOTIFICATION);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_REQUEST);
        intentFilter.addAction(PluginUtils.ACTION_DB_CHANGE);
        
        this.registerReceiver(mReceiver, intentFilter);
        registerSdCardReceiver();
        mRegistrationStatusStub = new RegistrationStatusStub();
        //mCapabilitiesStub = new CapabilitiesStub(this);
        mBlockingRequestStub = new BlockingRequestStub(getApplicationContext());
        mWindowBinder = new WindowBinder();

        // Instantiate the contacts manager
        ContactsManager.createInstance(getApplicationContext());
        // Keep a initial IM blocked contacts list to local copy
        ContactsManager.getInstance().loadImBlockedContactsToLocal();

        Logger.v(TAG, "ApiService onCreate() exit");
    }
    
    private void registerSdCardReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addDataScheme("file");
        this.registerReceiver(new SdcardReceiver(), intentFilter);
    }

    @Override
    public void onDestroy() {
        Logger.v(TAG, "ApiService onDestroy() entry");
        super.onDestroy();
        if (null != mReceiver) {
            this.unregisterReceiver(mReceiver);
        } else {
            Logger.e(TAG, "onDestroy() mReceiver is null");
        }
        try {
			ApiManager.getInstance().getChatApi().removeServiceRegistrationListener(regListener);
		} catch (JoynServiceException e) {
			e.printStackTrace();
		}
        Logger.v(TAG, "ApiService onDestroy() exit");
    }

    @Override
    public IBinder onBind(Intent intent) {
        String action = intent.getAction();
        Logger.i(TAG, "onBind() entry, the action is " + action);
        if (IRegistrationStatus.class.getName().equals(action)) {
            return mRegistrationStatusStub;
        } else if (IRemoteWindowBinder.class.getName().equals(action)) {
            return mWindowBinder;
        } else if (IRemoteBlockingRequest.class.getName().equals(action)) {
            return mBlockingRequestStub;
        }
        
        Logger.v(TAG, "onBind() exit");
        return null;
    }

    private class RegistrationListener extends JoynServiceRegistrationListener
    {

		@Override
		public void onServiceRegistered() {	
			Logger.d(TAG, "onServiceRegistered entry");
			handleRegistrationStatus(true);
		}

		@Override
		public void onServiceUnregistered() {
			Logger.d(TAG, "onServiceUnregistered entry");
			handleRegistrationStatus(false);			
		}
    	
    }

    // This receiver will handle sdcard mount and unmount broadcast
    private static class SdcardReceiver extends BroadcastReceiver {
        private static final String TAG = "SdcardReceiver";
        @Override
        public void onReceive(final Context context, final Intent intent) {
            Logger.d(TAG, "onReceive() SdcardReceiver entry");
            new AsyncTask<Void, Void, Void>() {
                 @Override
                 protected Void doInBackground(Void... params) {
                     String action = intent.getAction();
                        Logger.d(TAG, "doInBackground() SdcardReceiver action is " + action);
                        if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                            Logger.d(TAG, "doInBackground() SdcardReceiver() sdcard mounted");
                            boolean ftCapability = FileTransferCapabilityManager
                                    .isFileTransferCapabilitySupported();
                            FileTransferCapabilityManager.setFileTransferCapability(context,
                                    ftCapability);
                        } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                            Logger.d(TAG, "doInBackground() SdcardReceiver() sdcard unmounted");
                            FileTransferCapabilityManager.setFileTransferCapability(context, false);
                        }
                     return null;
                 }
                }.execute();
            Logger.d(TAG, "onReceive() SdcardReceiver exit");
        }
    }

    private void handleRegistrationStatus(boolean status ) {
        Logger.d(TAG, "handleRegistrationStatus() entry");           
        if (!status) {
            if (RcsNotification.getInstance() != null) {
                RcsNotification.getInstance().sIsStoreAndForwardMessageNotified = false;
            } else {
                Logger.d(TAG,
                        "handleRegistrationStatus, RcsNotification.getInstance() is null!");
            }
            Logger.d(TAG, "handleRegistrationStatus, status is false, " +
                    "set sIsStoreAndForwardMessageNotified to false!");
        } else {
            Logger.d(TAG, "handleRegistrationStatus, status is true!");
            boolean ftCapability = FileTransferCapabilityManager
                    .isFileTransferCapabilitySupported();
            FileTransferCapabilityManager.setFileTransferCapability(getApplicationContext(), ftCapability);
        }
        Logger.i(TAG, "handleRegistrationStatus() the status is " + status);
        
        mRegistrationStatusStub.notifyRegistrationStatus(status);
        //mCapabilitiesStub.onStatusChanged(status);
    }

    // This receiver will handle some RCS-e related broadcasts.
    private class ApiReceiver extends BroadcastReceiver {
        public static final String TAG = "ApiReceiver";
        public static final String KEY_STATUS = "status";
        public static final String KEY_CONTACT = "contact";
        public static final String KEY_CAPABILITIES = "capabilities";

        @Override
        public void onReceive(final Context context, final Intent intent) {
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    asyncOnReceive(context, intent);
                    return null;
                }
            }.execute();
        }

        private void asyncOnReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Logger.v(TAG, "asyncOnReceive() entry, the action is " + action);
            if (ContactsApiIntents.CONTACT_BLOCK_REQUEST.equals(action)) {
                handleBlockRequest(context, intent);
            } 
            else if (ChatIntent.ACTION_NEW_CHAT.equalsIgnoreCase(action)
            		|| GroupChatIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)
                    || FileTransferIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)
                    || ImageSharingIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)
                    || VideoSharingIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)) {
                RcsNotification.handleInvitation(context, intent);
            } /*else if (ChatIntent.CHAT_SESSION_REPLACED.equalsIgnoreCase(action)) {
                RcsNotification.handleInvitation(context, intent);
            } */else if (action.equals(MEMORY_LOW_ACTION)) {
                final ExchangeMyCapability exchangeMyCapability =
                        ExchangeMyCapability.getInstance(ApiService.this);
                if (exchangeMyCapability == null) {
                    Logger.e(TAG, "Current ExchangeMyCapability instance is null");
                    return;
                }
                exchangeMyCapability.notifyCapabilityChanged(
                        ExchangeMyCapability.STORAGE_STATUS_CHANGE, false);
            } else if (action.equals(MEMORY_OK_ACTION)) {
                final ExchangeMyCapability exchangeMyCapability =
                        ExchangeMyCapability.getInstance(ApiService.this);
                if (exchangeMyCapability == null) {
                    Logger.e(TAG, "Current ExchangeMyCapability instance is null");
                    return;
                }
                exchangeMyCapability.notifyCapabilityChanged(
                        ExchangeMyCapability.STORAGE_STATUS_CHANGE, true);
            } else if (StartService.CONFIGURATION_STATUS.equals(action)) {
                boolean status = intent.getBooleanExtra(CORE_CONFIGURATION_STATUS, true);
                handleConfigurationStatus(status);
            } else if (PluginUtils.ACTION_DB_CHANGE.equals(action)) {
                //need to clear chat history
            	ControllerImpl controller = ControllerImpl.getInstance();
            	if(controller != null)
            	{
	            	Message controllerMessage = controller.obtainMessage(
	                        ChatController.EVENT_CLEAR_ALL_CHAT_HISTORY_MEMORY_ONLY, null, null);
	                controllerMessage.sendToTarget();
            	}
            	
            } else if (Intents.Client.SERVICE_UP.equals(action)) {
            	if (!JoynService.isServiceStarted(context)) {
                	Logger.d(TAG, "RcsCoreService is not started yet, so services won't connect ");
                }
                boolean status = intent.getBooleanExtra("status", false);
                handleRegistrationStatus(status);
            }
            else if (ApiManager.SERVICE_CONNECTED_EVENT.equals(action)) {
                boolean status = intent.getBooleanExtra("registrationStatus", true);
                Logger.d(TAG, "SERVICE_CONNECTED_EVENT entry , status " + status);
                regListener = new RegistrationListener();
                /*try {
        			//ApiManager.getInstance().getChatApi().addServiceRegistrationListener(regListener);
        		} catch (JoynServiceException e) {
        			e.printStackTrace();
        		}   */             
    			//handleRegistrationStatus(status);
            }
            else if (TermsApiIntents.TERMS_SIP_ACK.equalsIgnoreCase(action)
                    || TermsApiIntents.TERMS_SIP_REQUEST.equalsIgnoreCase(action)
                    || TermsApiIntents.TERMS_SIP_USER_NOTIFICATION.equalsIgnoreCase(action)) {
            	Logger.v(TAG, "asyncOnReceive() Handling Terms Request " + action);
            	intent.setClass(getApplicationContext(), ConfigMessageActicity.class);
            	intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);            	
                startActivity(intent);
            }
            else {
                Logger.e(TAG, "asyncOnReceive() unknown action! The action is " + action);
            }
            Logger.v(TAG, "asyncOnReceive() exit");
        }


        private void handleBlockRequest(Context context, Intent intent) {
            Logger.v(TAG, "handleBlockRequest() entry");
            Bundle data = intent.getExtras();
            if (null != data) {
                String number = data.getString("number");
                ContactsManager instance = ContactsManager.getInstance();
                if(instance != null)
                {
                	instance.setImBlockedForContact(number, true);
                	
                }
            } else {
                Logger.e(TAG, "handleBlockRequest() the data is null!");
            }
            Logger.v(TAG, "handleBlockRequest() exit");
        }      

        private void handleConfigurationStatus(boolean status) {
            Logger.d(TAG, "handleConfigurationStatus() entry the status is " + status);
            RcseComponentController rcseComponentController = ApiManager.getInstance().getRcseComponentController();
            Logger.d(TAG,"handleConfigurationStatus() : rcseComponentController " + rcseComponentController);
            if (rcseComponentController != null) {
                if (Logger.IS_DEBUG) {
                    Logger.d(TAG, "handleConfigurationStatus() it is debug version");
                } else {
                    rcseComponentController.onConfigurationStatusChanged(status);
                }
            } else {
                Logger.e(TAG, "handleConfigurationStatus()) " +
                        "ApiManager.getInstance().getRcseComponentController() is null");
            }
        }
    }
}

class RegistrationStatusStub extends IRegistrationStatus.Stub {
    public static final String TAG = "RegistrationStatusStub";
    private boolean mIsRcseRegistered = false;

    /**
     * List of listeners
     */
    private RemoteCallbackList<IRegistrationStatusRemoteListener> mListeners =
            new RemoteCallbackList<IRegistrationStatusRemoteListener>();
    /**
     * Lock used for synchronization
     */
    private Object mLock = new Object();

    @Override
    public void addRegistrationStatusListener(IRegistrationStatusRemoteListener listener)
            throws RemoteException {
        boolean result = mListeners.register(listener);
        Logger.i(TAG, "addRegistrationStatusListener() The result is " + result);
    }

    public void notifyRegistrationStatus(boolean status) {
        Logger.v(TAG, "notifyRegistrationStatus() entry: The status is " + status);
        // update the registration status
        mIsRcseRegistered = status;
        synchronized (mLock) {
            // Notify status listeners
            final int n = mListeners.beginBroadcast();
            for (int i = 0; i < n; i++) {
                try {
                    mListeners.getBroadcastItem(i).onStatusChanged(status);
                } catch (RemoteException e) {
                    Logger.w(TAG,
                            "notifyRegistrationStatus() Failed to notify target listener, the index is "
                                    + i);
                }
            }
            mListeners.finishBroadcast();
        }
        Logger.v(TAG, "notifyRegistrationStatus() exit");
    }

    @Override
    public boolean isRegistered() throws RemoteException {
        Logger.d(TAG, "isRegistered(), call ApiService:isRegistered()! mIsRcseRegistered = " + mIsRcseRegistered);
        return mIsRcseRegistered;
    }
}

class BlockingRequestStub extends IRemoteBlockingRequest.Stub
{
        public static final String TAG = "BlockingRequestStub";
        private Context mContext = null;
        BlockingRequestStub(Context context)
        {
            mContext = context;
        }
        
        public boolean blockContact(String contact, boolean status)
        {
            Logger.v(TAG, "blockContact() entry");
            ContactsManager instance = ContactsManager.getInstance();
            if(instance != null)
            {
                instance.setImBlockedForContact(contact, status);
                return true;
                
            }
            else {
                Logger.e(TAG, "blockContact() instance is null!");
            }
            
            Logger.v(TAG, "blockContact() exit");
            return false;
            
        }
        
        
        public boolean getBlockedStatus(String contact)
        {
            Logger.v(TAG, "getBlockedStatus() entry");
            ContactsManager instance = ContactsManager.getInstance();
            if(instance != null)
            {
                return instance.isImBlockedForContact(contact);
            }
            else
            {
                Logger.e(TAG, "getBlockedStatus() instance is null!");
            }
            return false;
            
        }
}


