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

package com.orangelabs.rcs.service;

import org.gsma.joyn.Intents;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.capability.ICapabilityService;
import org.gsma.joyn.chat.IChatService;
import org.gsma.joyn.contacts.IContactsService;
import org.gsma.joyn.ft.IFileTransferService;
import org.gsma.joyn.gsh.IGeolocSharingService;
import org.gsma.joyn.ipcall.IIPCallService;
import org.gsma.joyn.ish.IImageSharingService;
import org.gsma.joyn.session.IMultimediaSessionService;
import org.gsma.joyn.vsh.IVideoSharingService;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;

import com.mediatek.rcse.api.NetworkConnectivityApi;
import com.mediatek.rcse.api.INetworkConnectivityApi;
import com.mediatek.rcse.api.terms.ITermsApi;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.addressbook.AccountChangedReceiver;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.CoreListener;
import com.orangelabs.rcs.core.TerminalInfo;
import com.orangelabs.rcs.core.ims.ImsError;
import com.orangelabs.rcs.core.ims.network.NetworkApiService;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.core.ims.service.im.chat.OneOneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.TerminatingAdhocGroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.TerminatingOne2OneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.standfw.TerminatingStoreAndForwardMsgSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.ipcall.IPCallSession;
import com.orangelabs.rcs.core.ims.service.presence.pidf.PidfDocument;
import com.orangelabs.rcs.core.ims.service.richcall.geoloc.GeolocTransferSession;
import com.orangelabs.rcs.core.ims.service.richcall.image.ImageTransferSession;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoStreamingSession;
import com.orangelabs.rcs.core.ims.service.sip.GenericSipSession;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.eab.ContactsBackupHelper;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.ipcall.IPCallHistory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.sharing.RichCallHistory;
import com.orangelabs.rcs.service.api.CapabilityServiceImpl;
import com.orangelabs.rcs.service.api.ChatServiceImpl;
import com.orangelabs.rcs.service.api.ContactsServiceImpl;
import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.GeolocSharingServiceImpl;
import com.orangelabs.rcs.service.api.IPCallServiceImpl;
import com.orangelabs.rcs.service.api.ImageSharingServiceImpl;
import com.orangelabs.rcs.service.api.MultimediaSessionServiceImpl;
import com.orangelabs.rcs.service.api.TermsApiService;
import com.orangelabs.rcs.service.api.VideoSharingServiceImpl;
import com.orangelabs.rcs.utils.AppUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningSMS;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningUtils;
/**
 * RCS core service. This service offers a flat API to any other process (activities)
 * to access to RCS features. This service is started automatically at device boot.
 * 
 * @author Jean-Marc AUFFRET
 */
public class RcsCoreService extends Service implements CoreListener {
	/**
	 * Service name
	 */
	public static final String SERVICE_NAME = "com.orangelabs.rcs.SERVICE";

	/**
	 * Notification ID
	 */
	private final static int SERVICE_NOTIFICATION = 1000;
	
	/**
	 * CPU manager
	 */
	private CpuManager cpuManager = new CpuManager();

    /**
     * Account changed broadcast receiver
     */
    private AccountChangedReceiver accountChangedReceiver = null;

	// --------------------- RCSJTA API -------------------------
    /**
	 * Terms API
	 */
    private TermsApiService termsApi = new TermsApiService(); 
	/**
	 * Contacts API
	 */
    private ContactsServiceImpl contactsApi = null; 

    /**
	 * Capability API
	 */
    private CapabilityServiceImpl capabilityApi = null; 

	/**
	 * Chat API
	 */
    private ChatServiceImpl chatApi = null; 

	/**
	 * File transfer API
	 */
    private FileTransferServiceImpl ftApi = null; 

    /**
	 * Video sharing API
	 */
    private VideoSharingServiceImpl vshApi = null; 

    /**
	 * Image sharing API
	 */
    private ImageSharingServiceImpl ishApi = null; 
    
    /**
     * M: add for auto-rejoin group chat @{
     */
    private NetworkApiService mNetworkConnectivityApi = new NetworkApiService();
    /** @} */

	/**
     * Account changed broadcast receiver
     */
    private HttpsProvisioningSMS reconfSMSReceiver = null;

    /**
	 * Geoloc sharing API
	 */
    private GeolocSharingServiceImpl gshApi = null; 

    /**
	 * IP call API
	 */
    private IPCallServiceImpl ipcallApi = null; 

    /**
	 * Multimedia session API
	 */
	private MultimediaSessionServiceImpl sessionApi = null; 
		
	/**
	 * The logger
	 */
	private Logger logger = Logger.getLogger(this.getClass().getName());

	@Override
    public void onCreate() {
		// Set application context
		AndroidFactory.setApplicationContext(getApplicationContext());

		// Set the terminal version
		TerminalInfo.setProductVersion(AppUtils.getApplicationVersion(this));

    	// Start the core
    	startCore();
    }

    @Override
    public void onDestroy() {
        // Unregister account changed broadcast receiver
	    if (accountChangedReceiver != null) {
	        try {
	        	unregisterReceiver(accountChangedReceiver);
	        } catch (IllegalArgumentException e) {
	        	// Nothing to do
	        }
	    }
         // Unregister SMS receiver for network initiated configuration
	    if (reconfSMSReceiver != null) {
	        try {
	        	reconfSMSReceiver.unregisterSmsProvisioningReceiver();
	        } catch (IllegalArgumentException e) {
	        	// Nothing to do
	        }
	    }
        // Stop the core
        Thread t = new Thread() {
            /**
             * Processing
             */
            public void run() {
                stopCore();
            }
        };
        t.start();
    }

    /**
     * Start core
     */
    public synchronized void startCore() {
		if (Core.getInstance() != null) {
			// Already started
			return;
		}

        try {
    		if (logger.isActivated()) {
    			logger.debug("Start RCS core service");
    		}
    		
			// Instantiate the settings manager
            RcsSettings.createInstance(getApplicationContext());
            
        	// Instanciate API
            contactsApi = new ContactsServiceImpl(); 
            capabilityApi = new CapabilityServiceImpl(); 
            chatApi = new ChatServiceImpl(); 
            ftApi = new FileTransferServiceImpl(); 
            vshApi = new VideoSharingServiceImpl(); 
            ishApi = new ImageSharingServiceImpl(); 
            gshApi = new GeolocSharingServiceImpl(); 
            ipcallApi = new IPCallServiceImpl(); 
        	sessionApi = new MultimediaSessionServiceImpl();             
            
            // Set the logger properties
    		Logger.activationFlag = RcsSettings.getInstance().isTraceActivated();
    		Logger.traceLevel = RcsSettings.getInstance().getTraceLevel();

    		// Terminal version
            if (logger.isActivated()) {
                logger.info("RCS stack release is " + TerminalInfo.getProductVersion());
            }
    		
    		// Instantiate the contacts manager
            ContactsManager.createInstance(getApplicationContext());

            // Instantiate the rich messaging history 
            RichMessagingHistory.createInstance(getApplicationContext());
            
            // Instantiate the rich call history 
            RichCallHistory.createInstance(getApplicationContext());

            // Instantiate the IP call history 
            IPCallHistory.createInstance(getApplicationContext());

            ContactsBackupHelper.createInstance(getApplicationContext());

            // Create the core
			Core.createCore(this);

			// Start the core
			Core.getInstance().startCore();		

			// Create multimedia directory on sdcard
			FileFactory.createDirectory(RcsSettings.getInstance().getPhotoRootDirectory());
			FileFactory.createDirectory(RcsSettings.getInstance().getVideoRootDirectory());
			FileFactory.createDirectory(RcsSettings.getInstance().getFileRootDirectory());
			
			// Init CPU manager
			cpuManager.init();

            // Register account changed event receiver
            if (accountChangedReceiver == null) {
                accountChangedReceiver = new AccountChangedReceiver();

                // Register account changed broadcast receiver after a timeout of 2s (This is not done immediately, as we do not want to catch
                // the removal of the account (creating and removing accounts is done asynchronously). We can reasonably assume that no
                // RCS account deletion will be done by user during this amount of time, as he just started his service.
                Handler handler = new Handler();
                handler.postDelayed(
                        new Runnable() {
                            public void run() {
                                registerReceiver(accountChangedReceiver, new IntentFilter(
                                        "android.accounts.LOGIN_ACCOUNTS_CHANGED"));
                            }},
                        2000);
            }

            // Register SMS receiver for network initiated configuration
            


            if (reconfSMSReceiver == null) {
            	reconfSMSReceiver = new HttpsProvisioningSMS(this);
            	reconfSMSReceiver.registerSmsProvisioningReceiver(Integer.toString(HttpsProvisioningUtils.DEFAULT_SMS_PORT), null, null, null);
            }
         
            

	        // Show a first notification
	    	addRcsServiceNotification(false, getString(R.string.rcs_core_loaded));

			if (logger.isActivated()) {
				logger.info("RCS core service started with success");
			}
		} catch(Exception e) {
			// Unexpected error
			if (logger.isActivated()) {
				logger.error("Can't instanciate the RCS core service", e);
			}
			
			// Show error in notification bar
	    	addRcsServiceNotification(false, getString(R.string.rcs_core_failed));
	    	
			// Exit service
	    	stopSelf();
		}
    }
    
    /**
     * Stop core
     */
    public synchronized void stopCore() {
		if (Core.getInstance() == null) {
			// Already stopped
			return;
		}
		
		if (logger.isActivated()) {
			logger.debug("Stop RCS core service");
		}

    	// Close APIs
		if(contactsApi != null) {
	    contactsApi.close();
		}
		if(capabilityApi != null) {
		capabilityApi.close();
		}    
		if(ftApi != null) {
		ftApi.close();
		}    
		if(chatApi != null) {
		chatApi.close();
		}    
		if(ishApi != null) {
		ishApi.close();
		}
		if(gshApi != null) {
		gshApi.close();
		}
		if(ipcallApi != null) {
		ipcallApi.close();
		}
    	if(vshApi != null) {
    	vshApi.close();
    	}

    	// Terminate the core in background
		Core.terminateCore();

		// Close CPU manager
		if(cpuManager != null) {
		cpuManager.close();
		}

		if (logger.isActivated()) {
			logger.info("RCS core service stopped with success");
		}
    }

    @Override
    public IBinder onBind(Intent intent) {    	
        if (IContactsService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("Contacts service API binding");
    		}
            return contactsApi;
        } else
        if (ICapabilityService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("Capability service API binding");
    		}
            return capabilityApi;
        } else
        if (IFileTransferService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("File transfer service API binding");
    		}
            return ftApi;
        } else
        if (IChatService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("Chat service API binding");
    		}
            return chatApi;
        } else
        if (IVideoSharingService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("Video sharing service API binding");
    		}
            return vshApi;
        } else
        if (IImageSharingService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("Image sharing service API binding");
    		}
            return ishApi;
        } else
        if (IGeolocSharingService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("Geoloc sharing service API binding");
    		}
            return gshApi;
        } else
        if (IIPCallService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("IP call service API binding");
    		}
            return ipcallApi;
        } else
        	if (ITermsApi.class.getName().equals(intent.getAction())) {
        		if (logger.isActivated()) {
        			logger.debug("Terms API binding");
        		}
                return termsApi;
        } else
        if (IMultimediaSessionService.class.getName().equals(intent.getAction())) {
    		if (logger.isActivated()) {
    			logger.debug("SIP API binding");
    		}
            return sessionApi;
        } 
         /**
             * M : add for auto-rejoin group chat. @{
             */
           else if (INetworkConnectivityApi.class.getName().equals(intent.getAction())) {
            if (logger.isActivated()) {
                logger.debug("Network API binding");
            }
            return mNetworkConnectivityApi;
            /** @} */
        } 
         else {
        	return null;
        }
    }
    
    /**
     * Add RCS service notification
     * 
     * @param state Service state (ON|OFF)
     * @param label Label
     */
    public static void addRcsServiceNotification(boolean state, String label) {
    	// Create notification
    	Intent intent = new Intent(Intents.Client.ACTION_VIEW_SETTINGS);
    	intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
		PendingIntent contentIntent = PendingIntent.getActivity(AndroidFactory.getApplicationContext(), 0, intent, 0);
		int iconId; 
		if (state) {
			iconId  = R.drawable.rcs_core_notif_on_icon;
		} else {
			iconId  = R.drawable.rcs_core_notif_off_icon; 
		}
        Notification notif = new Notification(iconId, "", System.currentTimeMillis());
        notif.flags = Notification.FLAG_NO_CLEAR | Notification.FLAG_FOREGROUND_SERVICE;
        notif.setLatestEventInfo(AndroidFactory.getApplicationContext(),
        		AndroidFactory.getApplicationContext().getString(R.string.rcs_core_rcs_notification_title),
        		label, contentIntent);
        
        // Send notification
		NotificationManager notificationManager = (NotificationManager)AndroidFactory.getApplicationContext().getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(SERVICE_NOTIFICATION, notif);
    }
    
    /*---------------------------- CORE EVENTS ---------------------------*/

    /**
	 * Notify registration status to API
	 * 
	 * @param status Status
	 */
	private void notifyRegistrationStatusToApi(boolean status) {
		if (capabilityApi != null) {
			capabilityApi.notifyRegistrationEvent(status);
		}
		if (chatApi != null) {
			chatApi.notifyRegistrationEvent(status);
		}
		if (ftApi != null) {
			ftApi.notifyRegistrationEvent(status);
		}
		if (vshApi != null) {
			vshApi.notifyRegistrationEvent(status);
		}
		if (ishApi != null) {
			ishApi.notifyRegistrationEvent(status);
		}
		if (gshApi != null) {
			gshApi.notifyRegistrationEvent(status);
		}
		if (ipcallApi != null) {
			ipcallApi.notifyRegistrationEvent(status);
		}
		if (sessionApi != null) {
			sessionApi.notifyRegistrationEvent(status);
		}
	}    
    
    /**
     * Core layer has been started
     */
    public void handleCoreLayerStarted() {
		if (logger.isActivated()) {
			logger.debug("Handle event core started");
		}

		// Display a notification
		addRcsServiceNotification(false, getString(R.string.rcs_core_started));
		
		// Send service up intent
		Intent intent = new Intent(JoynService.ACTION_RCS_SERVICE_UP);
		getApplicationContext().sendBroadcast(intent);
    }

    /**
     * Core layer has been terminated
     */
    public void handleCoreLayerStopped() {
        // Display a notification
        if (logger.isActivated()) {
            logger.debug("Handle event core terminated");
        }
        addRcsServiceNotification(false, getString(R.string.rcs_core_stopped));
    }
    
    /**
	 * Handle "registration successful" event
	 * 
	 * @param registered Registration flag
	 */
	public void handleRegistrationSuccessful() {
		if (logger.isActivated()) {
			logger.debug("Handle event registration ok");
		}
		
		// Display a notification
		addRcsServiceNotification(true, getString(R.string.rcs_core_ims_connected));
		 // Send registration intent
		Intent intent = new Intent(Intents.Client.SERVICE_UP);
		intent.putExtra("status", true);
		getApplicationContext().sendStickyBroadcast(intent);
		// Notify APIs
		notifyRegistrationStatusToApi(true);
	}


/**M
	 * added to how notification of connecting and disconnecting states during registration
	 */	
	/**
	 * Handle "try registration" event
	 */
	public void handleTryRegister() {
		if (logger.isActivated()) {
			logger.debug("Handle event try registration");
		}

		// Display a notification
		addRcsServiceNotification(false, getString(R.string.rcs_core_ims_try_connection));
	}
	
	

	/**
	 * Handle "try registration" event
	 */
	public void handleTryDeregister() {
		if (logger.isActivated()) {
			logger.debug("Handle event try deregistration");
		}
	
		// Display a notification
		addRcsServiceNotification(false, getString(R.string.rcs_core_ims_try_disconnect));
	}
	/** @*/

	/**
	 * Handle "registration failed" event
	 * 
     * @param error IMS error
   	 */
	public void handleRegistrationFailed(ImsError error) {
		if (logger.isActivated()) {
			logger.debug("Handle event registration failed");
		}

		// Display a notification
		addRcsServiceNotification(false, getString(R.string.rcs_core_ims_connection_failed));
		Intent intent = new Intent(Intents.Client.SERVICE_UP);
		intent.putExtra("status", false);
		getApplicationContext().sendStickyBroadcast(intent);
		// Notify APIs
		notifyRegistrationStatusToApi(false);
	}

	/**
	 * Handle "registration terminated" event
	 */
	public void handleRegistrationTerminated() {
        if (logger.isActivated()) {
            logger.debug("Handle event registration terminated");
        }

        if (Core.getInstance().getImsModule().getImsConnectionManager().isDisconnectedByBattery()) {
            // Display a notification
            addRcsServiceNotification(false, getString(R.string.rcs_core_ims_battery_disconnected));
        } else {
            // Display a notification
        	addRcsServiceNotification(false, getString(R.string.rcs_core_ims_disconnected));
        }
        
		// Notify APIs
		notifyRegistrationStatusToApi(false);
	}

    /**
     * A new presence sharing notification has been received
     * 
     * @param contact Contact
     * @param status Status
     * @param reason Reason
     */
    public void handlePresenceSharingNotification(String contact, String status, String reason) {
		if (logger.isActivated()) {
			logger.debug("Handle event presence sharing notification for " + contact + " (" + status + ":" + reason + ")");
		}
		// Not used
    }

    /**
     * A new presence info notification has been received
     * 
     * @param contact Contact
     * @param presense Presence info document
     */
    public void handlePresenceInfoNotification(String contact, PidfDocument presence) {
    	if (logger.isActivated()) {
			logger.debug("Handle event presence info notification for " + contact);
		}
		// Not used
	}
    
    /**
     * Capabilities update notification has been received
     * 
     * @param contact Contact
     * @param capabilities Capabilities
     */
    public void handleCapabilitiesNotification(String contact, Capabilities capabilities) {
    	if (logger.isActivated()) {
			logger.debug("Handle capabilities update notification for " + contact + " (" + capabilities.toString() + ")");
		}

		// Extract number from contact 
		String number = PhoneUtils.extractNumberFromUri(contact);

		// Notify API
		capabilityApi.receiveCapabilities(number, capabilities);
    }
    
    /**
     * A new presence sharing invitation has been received
     * 
     * @param contact Contact
     */
    public void handlePresenceSharingInvitation(String contact) {
		if (logger.isActivated()) {
			logger.debug("Handle event presence sharing invitation");
		}
		// Not used
    }
    
    /**
     * New content sharing transfer invitation
     * 
     * @param session Content sharing transfer invitation
     */
    public void handleContentSharingTransferInvitation(ImageTransferSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event content sharing transfer invitation");
		}

		// Broadcast the invitation
		ishApi.receiveImageSharingInvitation(session);
    }
    
    /**
     * New content sharing transfer invitation
     * 
     * @param session Content sharing transfer invitation
     */
    public void handleContentSharingTransferInvitation(GeolocTransferSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event content sharing transfer invitation");
		}

		// Broadcast the invitation
		gshApi.receiveGeolocSharingInvitation(session);
    }
    
    /**
     * New content sharing streaming invitation
     * 
     * @param session CSh session
     */
    public void handleContentSharingStreamingInvitation(VideoStreamingSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event content sharing streaming invitation");
		}

		// Broadcast the invitation
		vshApi.receiveVideoSharingInvitation(session);
    }
	
	/**
	 * A new file transfer invitation has been received
	 * 
	 * @param fileSharingSession File transfer session
	 * @param isGroup Is group file transfer
	 */
	public void handleFileTransferInvitation(FileSharingSession fileSharingSession, boolean isGroup) {
		if (logger.isActivated()) {
			logger.debug("Handle event file transfer invitation");
		}

    	// Broadcast the invitation
		ftApi.receiveFileTransferInvitation(fileSharingSession, isGroup);
	}
    
	/**
	 * A new file transfer invitation has been received
	 * 
     * @param fileSharingSession File transfer session
     * @param isGroup Is group file transfer
     */
    public void handleHttpFileTransferInvitation(String remote, FileSharingSession fileSharingSession, boolean isGroup) {
        if (logger.isActivated()) {
            logger.debug("Handle event httpfile transfer invitation: " + remote);
        }

        // Broadcast the invitation
        ftApi.receiveHttpFileTransferInvitation(remote,fileSharingSession, isGroup);
    }
    
	/**
	 * A new file transfer invitation has been received
	 * 
	 * @param session File transfer session
	 */
	public void handle1to1FileTransferInvitation(FileSharingSession fileSharingSession, OneOneChatSession one2oneChatSession) {
		if (logger.isActivated()) {
			logger.debug("Handle event file transfer invitation");
		}
		
    	// Broadcast the invitation
    	ftApi.receiveFileTransferInvitation(fileSharingSession, one2oneChatSession);
	}
	
	/**
	 * A new file transfer invitation has been received
	 * 
	 * @param session File transfer session
	 * @param groupChatSession the created chat session (group)
	 */
	public void handleGroupFileTransferInvitation(FileSharingSession session, TerminatingAdhocGroupChatSession groupChatSession) {
		if (logger.isActivated()) {
			logger.debug("Handle event group file transfer invitation");
		}
		
    	// Broadcast the invitation
    	ftApi.receiveFileTransferInvitation(session, groupChatSession);
	}

	/**
     * New one-to-one chat session invitation
     * 
     * @param session Chat session
     */
	public void handleOneOneChatSessionInvitation(TerminatingOne2OneChatSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event receive 1-1 chat session invitation");
		}
		
    	// Broadcast the invitation
		chatApi.receiveOneOneChatInvitation(session);
    }

    /**
     * New ad-hoc group chat session invitation
     * 
     * @param session Chat session
     */
	public void handleAdhocGroupChatSessionInvitation(TerminatingAdhocGroupChatSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event receive ad-hoc group chat session invitation");
		}

    	// Broadcast the invitation
		chatApi.receiveGroupChatInvitation(session);
	}

	  /**
     * One-to-one chat session extended to a group chat session
     * 
     * @param groupSession Group chat session
     * @param oneoneSession 1-1 chat session
     */
    public void handleOneOneChatSessionExtended(GroupChatSession groupSession, OneOneChatSession oneoneSession) {
		if (logger.isActivated()) {
			logger.debug("Handle event 1-1 chat session extended");
		}

    	// Broadcast the event
		chatApi.extendOneOneChatSession(groupSession, oneoneSession);
    }
	
    /**
     * Store and Forward messages session invitation
     * 
     * @param session Chat session
     */
    public void handleStoreAndForwardMsgSessionInvitation(TerminatingStoreAndForwardMsgSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event S&F messages session invitation");
		}
		
    	// Broadcast the invitation
		chatApi.receiveOneOneChatInvitation(session);
    }
    
    /**
     * New message delivery status
     * 
     * @param contact Contact
	 * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String contact, String msgId, String status) {
		if (logger.isActivated()) {
			logger.debug("Handle message delivery status");
		}
    	
		// Notify listeners
		chatApi.receiveMessageDeliveryStatus(contact, msgId, status);
    }
    
    /**
     * New file delivery status
     *
     * @param ftSessionId File transfer session ID
     * @param status Delivery status
     */
    public void handleFileDeliveryStatus(String ftSessionId, String status,String contact) {
        if (logger.isActivated()) {
            logger.debug("Handle file delivery status: session " + ftSessionId + " status " + status + "Contact: " + contact);
        }

        // Notify listeners
        ftApi.handleFileDeliveryStatus(ftSessionId, status,contact);
    }

    /**
     * New SIP session invitation
     * 
     * @param intent Resolved intent
     * @param session SIP session
     */
    public void handleSipSessionInvitation(Intent intent, GenericSipSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event receive SIP session invitation");
		}
		
		// Broadcast the invitation
		sessionApi.receiveSipSessionInvitation(intent, session);
    }    
    
	/**
	 * New SIP instant message received
	 * 
     * @param intent Resolved intent
     * @param message Instant message request
	 */
    public void handleSipInstantMessageReceived(Intent intent, SipRequest message) {  
    	if (logger.isActivated()) {
			logger.debug("Handle event receive SIP instant message");
		}
		
		// Broadcast the message
		sessionApi.receiveSipInstantMessage(intent, message);
    }    

	/**
     * User terms confirmation request
     *
     * @param remote Remote server
     * @param id Request ID
     * @param type Type of request
     * @param pin PIN number requested
     * @param subject Subject
     * @param text Text
     * @param btnLabelAccept Label of Accept button
     * @param btnLabelReject Label of Reject button
     * @param timeout Timeout request
     */
    public void handleUserConfirmationRequest(String remote, String id,
    		String type, boolean pin, String subject, String text,
    		String acceptButtonLabel, String rejectButtonLabel, int timeout) {
        if (logger.isActivated()) {
			logger.debug("Handle event user terms confirmation request");
		}

		
        termsApi.receiveTermsRequest(remote, id, type, pin, subject, text, acceptButtonLabel, rejectButtonLabel, timeout);
    }

    /**
     * User terms confirmation acknowledge
     * 
     * @param remote Remote server
     * @param id Request ID
     * @param status Status
     * @param subject Subject
     * @param text Text
     */
    public void handleUserConfirmationAck(String remote, String id, String status, String subject, String text) {
		if (logger.isActivated()) {
			logger.debug("Handle event user terms confirmation ack");
		}


		termsApi.receiveTermsAck(remote, id, status, subject, text);
    }

    /**
     * User terms notification
     *
     * @param remote Remote server
     * @param id Request ID
     * @param subject Subject
     * @param text Text
     * @param btnLabel Label of OK button
     */
    public void handleUserNotification(String remote, String id, String subject, String text, String okButtonLabel) {
        if (logger.isActivated()) {
            logger.debug("Handle event user terms notification");
        }

		
        termsApi.receiveUserNotification(remote, id, subject, text, okButtonLabel);
    }

    /**
	 * SIM has changed
	 */
    public void handleSimHasChanged() {
        if (logger.isActivated()) {
            logger.debug("Handle SIM has changed");
        }

		// Restart the RCS service
        LauncherUtils.stopRcsService(getApplicationContext());
        LauncherUtils.launchRcsService(getApplicationContext(), true, false);
    }

    /**
     * New IP call invitation
     * 
     * @param session IP call session
     */
	public void handleIPCallInvitation(IPCallSession session) {
		if (logger.isActivated()) {
			logger.debug("Handle event IP call invitation");
		}
		
		// Broadcast the invitation
		ipcallApi.receiveIPCallInvitation(session);
	}
}
