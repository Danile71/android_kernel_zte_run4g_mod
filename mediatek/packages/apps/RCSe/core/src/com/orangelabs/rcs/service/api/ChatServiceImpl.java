package com.orangelabs.rcs.service.api;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;

import org.gsma.joyn.Build;
import org.gsma.joyn.IJoynServiceRegistrationListener;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.chat.ChatIntent;
import org.gsma.joyn.chat.ChatLog;
import org.gsma.joyn.chat.ChatMessage;
import org.gsma.joyn.chat.ChatServiceConfiguration;
import org.gsma.joyn.chat.Geoloc;
import org.gsma.joyn.chat.GroupChat;
import org.gsma.joyn.chat.GroupChatIntent;
import org.gsma.joyn.chat.IChat;
import org.gsma.joyn.chat.IChatListener;
import org.gsma.joyn.chat.IChatService;
import org.gsma.joyn.chat.IGroupChat;
import org.gsma.joyn.chat.IGroupChatListener;
import org.gsma.joyn.chat.INewChatListener;

import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteCallbackList;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.OneOneChatSession;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Chat service implementation
 * 
 * @author Jean-Marc AUFFRET
 */
public class ChatServiceImpl extends IChatService.Stub {
	/**
	 * List of service event listeners
	 */
	private RemoteCallbackList<IJoynServiceRegistrationListener> serviceListeners = new RemoteCallbackList<IJoynServiceRegistrationListener>();

	/**
	 * List of chat sessions
	 */
	private static Hashtable<String, IChat> chatSessions = new Hashtable<String, IChat>();  

	/**
	 * List of chat sessions
	 */
	private static Hashtable<String, IChat> storeForwardChatSessions = new Hashtable<String, IChat>();  

	/**
	 * List of group chat sessions
	 */
	private static Hashtable<String, IGroupChat> groupChatSessions = new Hashtable<String, IGroupChat>();  

	/**
	 * List of file chat invitation listeners
	 */
	private RemoteCallbackList<INewChatListener> listeners = new RemoteCallbackList<INewChatListener>();

	/**
	 * The logger
	 */
	private static Logger logger = Logger.getLogger(ChatServiceImpl.class.getName());

	/**
	 * Lock used for synchronization
	 */
	private Object lock = new Object();

	/**
	 * Constructor
	 */
	public ChatServiceImpl() {
		if (logger.isActivated()) {
			logger.info("Chat service API is loaded");
		}
	}

	/**
	 * Close API
	 */
	public void close() {
		// Clear list of sessions
		chatSessions.clear();
		storeForwardChatSessions.clear();
		groupChatSessions.clear();
		
		if (logger.isActivated()) {
			logger.info("Chat service API is closed");
		}
	}
	
    /**
     * Returns true if the service is registered to the platform, else returns false
     * 
	 * @return Returns true if registered else returns false
     */
    public boolean isServiceRegistered() {
    	return ServerApiUtils.isImsConnected();
    }

	/**
	 * Registers a listener on service registration events
	 * 
	 * @param listener Service registration listener
	 */
	public void addServiceRegistrationListener(IJoynServiceRegistrationListener listener) {
    	synchronized(lock) {
			if (logger.isActivated()) {
				logger.info("Add a service listener");
			}

			serviceListeners.register(listener);
		}
	}
	
	/**
	 * Unregisters a listener on service registration events
	 * 
	 * @param listener Service registration listener
	 */
	public void removeServiceRegistrationListener(IJoynServiceRegistrationListener listener) {
    	synchronized(lock) {
			if (logger.isActivated()) {
				logger.info("Remove a service listener");
			}
			
			serviceListeners.unregister(listener);
    	}	
	}    
    
    /**
     * Receive registration event
     * 
     * @param state Registration state
     */
    public void notifyRegistrationEvent(boolean state) {
    	// Notify listeners
    	synchronized(lock) {
			final int N = serviceListeners.beginBroadcast();
	        for (int i=0; i < N; i++) {
	            try {
	            	if (state) {
	            		serviceListeners.getBroadcastItem(i).onServiceRegistered();
	            	} else {
	            		serviceListeners.getBroadcastItem(i).onServiceUnregistered();
	            	}
	            } catch(Exception e) {
	            	if (logger.isActivated()) {
	            		logger.error("Can't notify listener", e);
	            	}
	            }
	        }
	        serviceListeners.finishBroadcast();
	    }    	    	
    }
    
    /**
	 * Receive a new chat invitation
	 * 
	 * @param session Chat session
	 */
    public void receiveOneOneChatInvitation(OneOneChatSession session) {
		if (logger.isActivated()) {
			logger.info("Receive chat invitation from " + session.getRemoteContact() + "  Display name: " + session.getRemoteDisplayName());
		}

		// Extract number from contact 
		String number = PhoneUtils.extractNumberFromUri(session.getRemoteContact());

		// Update rich messaging history
		// Nothing done in database
		// Update rich messaging history with o2o chat message
		
		InstantMessage msg = session.getFirstMessage();
		msg.setDisplayName(session.getRemoteDisplayName());
		if (logger.isActivated()) {
			logger.info("Receive chat invitation from first message display: " + msg.getDisplayName());
		}
		RichMessagingHistory.getInstance().addChatSystemMessage(msg, ChatLog.Message.Direction.INCOMING);
		

		// Add session in the list
		ChatImpl sessionApi = new ChatImpl(number, session);
		ChatServiceImpl.addChatSession(number, sessionApi);

		// Broadcast intent related to the received invitation
    	Intent intent = new Intent(ChatIntent.ACTION_NEW_CHAT);
    	intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
    	intent.putExtra(ChatIntent.EXTRA_CONTACT, number);
    	intent.putExtra(ChatIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
    	ChatMessage msgApi;
    	if (msg instanceof GeolocMessage) {
    		GeolocMessage geoloc = (GeolocMessage)msg;
        	Geoloc geolocApi = new Geoloc(geoloc.getGeoloc().getLabel(),
        			geoloc.getGeoloc().getLatitude(), geoloc.getGeoloc().getLongitude(),
        			geoloc.getGeoloc().getExpiration());
        	msgApi = new org.gsma.joyn.chat.GeolocMessage(geoloc.getMessageId(),
        			PhoneUtils.extractNumberFromUri(geoloc.getRemote()),
        			geolocApi, geoloc.getDate(), geoloc.isImdnDisplayedRequested());
	    	intent.putExtra(ChatIntent.EXTRA_MESSAGE, msgApi);
    	} else {
        	msgApi = new ChatMessage(msg.getMessageId(),
        			PhoneUtils.extractNumberFromUri(msg.getRemote()),
        			msg.getTextMessage(), msg.getServerDate(),
        			msg.isImdnDisplayedRequested(),msg.getDisplayName());
        	intent.putExtra(ChatIntent.EXTRA_MESSAGE, msgApi);    		
    	}
    	AndroidFactory.getApplicationContext().sendBroadcast(intent);
    	
    	// Notify chat invitation listeners
    	synchronized(lock) {
			final int N = listeners.beginBroadcast();
	        for (int i=0; i < N; i++) {
	            try {
	            	listeners.getBroadcastItem(i).onNewSingleChat(number, msgApi);
	            } catch(Exception e) {
	            	if (logger.isActivated()) {
	            		logger.error("Can't notify listener", e);
	            	}
	            }
	        }
	        listeners.finishBroadcast();
	    }    	    	
    }
    
    /**
     * Open a single chat with a given contact and returns a Chat instance.
     * The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI.
     * 
     * @param contact Contact
     * @param listener Chat event listener
     * @return Chat
	 * @throws ServerApiException
     */
    public IChat openSingleChat(String contact, IChatListener listener) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Open a 1-1 chat session with " + contact);
		}

		// Test IMS connection
		ServerApiUtils.testIms();
		
		try {
			// Extract number from contact 
			String number = PhoneUtils.extractNumberFromUri(contact);

			// Check if there is an existing chat or not
			ChatImpl sessionApi = (ChatImpl)ChatServiceImpl.getChatSession(number);
			//if(sessionApi != null){
			//	OneOneChatSession tempSession = sessionApi.getCoreSession();
			//}
			
			if (sessionApi != null && sessionApi.getCoreSession() != null && !sessionApi.getCoreSession().isStoreAndForward()) {
			//if (sessionApi != null ){
				if (logger.isActivated()) {
					logger.debug("Chat session already exist for " + number);
				}
				
				// Add session listener
				sessionApi.addEventListener(listener);

				// Check core session state
				final OneOneChatSession coreSession = sessionApi.getCoreSession();
				if (coreSession != null) {
					if (logger.isActivated()) {
						logger.debug("Core chat session already exist: " + coreSession.getSessionID());
					}

					if (coreSession.getDialogPath().isSessionTerminated() ||
							coreSession.getDialogPath().isSessionCancelled()) {
						if (logger.isActivated()) {
							logger.debug("Core chat session is terminated: reset it");
						}
						
						// Session has expired, remove it
						sessionApi.resetCoreSession();
					} else
					if (!coreSession.getDialogPath().isSessionEstablished()) {
						if (logger.isActivated()) {
							logger.debug("Core chat session is pending: auto accept it");
						}
						
						// Auto accept the pending session
				        Thread t = new Thread() {
				    		public void run() {
								coreSession.acceptSession();
				    		}
				    	};
				    	t.start();
					} else {
						if (logger.isActivated()) {
							logger.debug("Core chat session is already established");
						}
					}
				}
			} else {
				if (logger.isActivated()) {
					logger.debug("Create a new chat session with " + number);
				}

				// Add session listener
				sessionApi = new ChatImpl(number);
				sessionApi.addEventListener(listener);
	
				// Add session in the list
				ChatServiceImpl.addChatSession(number, sessionApi);
			}
		
			return sessionApi;
		
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
    }    

    /**
     * Receive message delivery status
     * 
	 * @param contact Contact
	 * @param msgId Message ID
     * @param status Delivery status
     */
    public void receiveMessageDeliveryStatus(String contact, String msgId, String status) {
    	synchronized(lock) {
			if (logger.isActivated()) {
				logger.info("ABC Receive message delivery status for message " + msgId + ", status " + status);
			}

			String number = PhoneUtils.extractNumberFromUri(contact);

			if (logger.isActivated()) {
				logger.info("ABC receiveMessageDeliveryStatus " + contact + ", number " + number);
			}
	
	  		// Notify message delivery listeners
			ChatImpl chat = (ChatImpl)ChatServiceImpl.getChatSession(contact);
			if (chat != null) {
				if (logger.isActivated()) {
					logger.info("ABC chat is not null");
				}
            	chat.handleMessageDeliveryStatus(msgId, status);
	    	}
			else{
				if (logger.isActivated()) {
					logger.info("ABC chat is null");
				}
				Intent intent = new Intent(ChatIntent.ACTION_DELIVERY_STATUS);
		    	intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
		    	intent.putExtra(ChatIntent.EXTRA_CONTACT, number);
				intent.putExtra("msgId", msgId);
				intent.putExtra("status", status);
				AndroidFactory.getApplicationContext().sendBroadcast(intent);
			}
    	}
    }
    
	/**
	 * Add a chat session in the list
	 * 
	 * @param contact Contact
	 * @param session Chat session
	 */
	public static void addChatSession(String contact, ChatImpl session) {
		if (logger.isActivated()) {
			logger.debug("Add a chat session in the list (size=" + chatSessions.size() + ") for " + contact);
		}

		if((session.getCoreSession() != null) && session.getCoreSession().isStoreAndForward()){
			if (logger.isActivated()) {
				logger.debug("ChatSession s&f");
			}
			storeForwardChatSessions.put(contact, session);
		}
		else{
			chatSessions.put(contact, session);
		}
	}

	/**
	 * Get a chat session from the list for a given contact
	 * 
	 * @param contact Contact
	 * @param GroupChat session
	 */
	protected static IChat getStoreChatSession(String contact) {
		if (logger.isActivated()) {
			logger.debug("Get ChatSession s&f " + contact);
		}
		
		return storeForwardChatSessions.get(contact);
	}

	/**
	 * Remove a chat session from the list
	 * 
	 * @param contact Contact
	 */
	protected static void removeStoreChatSession(String contact) {
		if (logger.isActivated()) {
			logger.debug("Remove removeStoreChatSession (size=" + storeForwardChatSessions.size() + ") for " + contact);
		}
		
		if ((storeForwardChatSessions != null) && (contact != null)) {
			storeForwardChatSessions.remove(contact);
		}
	}

	/**
	 * Get a chat session from the list for a given contact
	 * 
	 * @param contact Contact
	 * @param GroupChat session
	 */
	protected static IChat getChatSession(String contact) {
		if (logger.isActivated()) {
			logger.debug("Get a chat session for " + contact);
		}
		
		return chatSessions.get(contact);
	}

	/**
	 * Remove a chat session from the list
	 * 
	 * @param contact Contact
	 */
	protected static void removeChatSession(String contact) {
		if (logger.isActivated()) {
			logger.debug("Remove a chat session from the list (size=" + chatSessions.size() + ") for " + contact);
		}
		
		if ((chatSessions != null) && (contact != null)) {
			chatSessions.remove(contact);
		}
	}
	
    /**
     * Returns the list of single chats in progress
     * 
     * @return List of chats
     * @throws ServerApiException
     */
    public List<IBinder> getChats() throws ServerApiException {
    	int size = chatSessions.size() + storeForwardChatSessions.size();
		if (logger.isActivated()) {
			logger.info("Get chat sessions sze: " + size);
		}

		try {
			ArrayList<IBinder> result = new ArrayList<IBinder>(size);
			for (Enumeration<IChat> e = chatSessions.elements() ; e.hasMoreElements() ;) {
				IChat sessionApi = (IChat)e.nextElement() ;
				result.add(sessionApi.asBinder());
			}
			//ArrayList<IBinder> result = new ArrayList<IBinder>(storeForwardChatSessions.size());
			for (Enumeration<IChat> e = storeForwardChatSessions.elements() ; e.hasMoreElements() ;) {
				IChat sessionApi = (IChat)e.nextElement() ;
				result.add(sessionApi.asBinder());
			}
			return result;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
    }
    
    /**
     * Returns a chat in progress from its unique ID
     * 
     * @param contact Contact
     * @return Chat or null if not found
     * @throws ServerApiException
     */
    public IChat getChat(String contact) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Get chat session with " + contact);
		}

		// Return a session instance
		return chatSessions.get(contact);
    }
    
    /**
	 * Receive a new group chat invitation
	 * 
	 * @param session Chat session
	 */
    public void receiveGroupChatInvitation(GroupChatSession session) {
		if (logger.isActivated()) {
			logger.info("Receive group chat invitation from " + session.getRemoteContact() + "  Display name: " + session.getGroupRemoteDisplayName() + "new name: " + SipUtils.getDisplayNameFromUri(session.getRemoteContact()));
		}

		// Extract number from contact 
		String number = PhoneUtils.extractNumberFromUri(session.getRemoteContact());

		Boolean isChatExist = false;
		isChatExist = RichMessagingHistory.getInstance().isGroupChatExists(session.getContributionID());

		// Update rich messaging history
		RichMessagingHistory.getInstance().addGroupChat(session.getContributionID(),
				session.getSubject(), session.getParticipants().getList(), GroupChat.State.INVITED, GroupChat.Direction.INCOMING);
		
		// Add session in the list
		GroupChatImpl sessionApi = new GroupChatImpl(session);
		ChatServiceImpl.addGroupChatSession(sessionApi);		

		if (logger.isActivated()) {
			logger.info("DEF receiveGroupChatInvitation ischatexists: " + isChatExist);
		}

		// Broadcast intent related to the received invitation
    	Intent intent = new Intent(GroupChatIntent.ACTION_NEW_INVITATION);
    	intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
    	intent.putExtra(GroupChatIntent.EXTRA_CONTACT, number);
    	intent.putExtra(GroupChatIntent.EXTRA_DISPLAY_NAME, session.getGroupRemoteDisplayName());
    	intent.putExtra(GroupChatIntent.EXTRA_CHAT_ID, sessionApi.getChatId());
    	intent.putExtra(GroupChatIntent.EXTRA_SUBJECT, sessionApi.getSubject());
    	intent.putExtra("autoAccept", RcsSettings.getInstance().isGroupChatAutoAccepted());
		intent.putExtra("isGroupChatExist", isChatExist);
		 /**
         * M: managing extra local chat participants that are 
         * not present in the invitation for sending them invite request.@{
         */
        String participants = "";
        List<String> ListParticipant = session.getParticipants().getList();
        for(String currentPartc : ListParticipant){
         participants += currentPartc + ";";	
        }
        /**
    	 * @}
    	 */      
        intent.putExtra("participantList", participants);
		
    	AndroidFactory.getApplicationContext().sendBroadcast(intent);
    	
    	// Notify chat invitation listeners
    	synchronized(lock) {
			final int N = listeners.beginBroadcast();
	        for (int i=0; i < N; i++) {
	            try {
	            	listeners.getBroadcastItem(i).onNewGroupChat(sessionApi.getChatId());
	            } catch(Exception e) {
	            	if (logger.isActivated()) {
	            		logger.error("Can't notify listener", e);
	            	}
	            }
	        }
	        listeners.finishBroadcast();
	    }    	
    }

	/**
	 * Extend a 1-1 chat session
	 * 
     * @param groupSession Group chat session
     * @param oneoneSession 1-1 chat session
	 */
    public void extendOneOneChatSession(GroupChatSession groupSession, OneOneChatSession oneoneSession) {
		if (logger.isActivated()) {
			logger.info("Extend a 1-1 chat session");
		}

		// Add session in the list
		GroupChatImpl sessionApi = new GroupChatImpl(groupSession);
		ChatServiceImpl.addGroupChatSession(sessionApi);
		
		// Broadcast intent related to the received invitation
		Intent intent = new Intent(GroupChatIntent.ACTION_SESSION_REPLACED);
		intent.putExtra("sessionId", groupSession.getSessionID());
		intent.putExtra(GroupChatIntent.EXTRA_CHAT_ID, sessionApi.getChatId());
		AndroidFactory.getApplicationContext().sendBroadcast(intent);
    }
	
	/**
	 * Add a group chat session in the list
	 * 
	 * @param session Chat session
	 */
	protected static void addGroupChatSession(GroupChatImpl session) {
		if (logger.isActivated()) {
			logger.debug("Add a group chat session in the list (size=" + groupChatSessions.size() + ")");
		}
		
		groupChatSessions.put(session.getChatId(), session);
	}

	/**
	 * Remove a group chat session from the list
	 * 
	 * @param chatId Chat ID
	 */
	protected static void removeGroupChatSession(String chatId) {
		if (logger.isActivated()) {
			logger.debug("Remove a group chat session from the list (size=" + groupChatSessions.size() + ")");
		}
		
		groupChatSessions.remove(chatId);
	}

    /**
     * Initiates a group chat with a group of contact and returns a GroupChat
     * instance. The subject is optional and may be null.
     * 
     * @param contact List of contacts
     * @param subject Subject
     * @param listener Chat event listener
	 * @throws ServerApiException
     */
    public IGroupChat initiateGroupChat(List<String> contacts, String subject, IGroupChatListener listener) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Initiate an ad-hoc group chat session");
		}
		
		// Test IMS connection
		ServerApiUtils.testIms();

		try {
			// Initiate the session
			final ChatSession session = Core.getInstance().getImService().initiateAdhocGroupChatSession(contacts, subject);

			// Add session listener
			GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session);
			sessionApi.addEventListener(listener);

			// Update rich messaging history
			RichMessagingHistory.getInstance().addGroupChat(session.getContributionID(),
					session.getSubject(), session.getParticipants().getList(),
					GroupChat.State.INITIATED, GroupChat.Direction.OUTGOING);

			// Start the session
	        Thread t = new Thread() {
	    		public void run() {
					session.startSession();
	    		}
	    	};
	    	t.start();
						
			// Add session in the list
			ChatServiceImpl.addGroupChatSession(sessionApi);
			return sessionApi;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
    }
    
    /**
     * Rejoins an existing group chat from its unique chat ID
     * 
     * @param chatId Chat ID
     * @return Group chat
     * @throws ServerApiException
     */
    public IGroupChat rejoinGroupChat(String chatId) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Rejoin group chat session related to the conversation " + chatId);
		}

		// Test IMS connection
		ServerApiUtils.testIms();

		try {
			// Initiate the session
			final ChatSession session = Core.getInstance().getImService().rejoinGroupChatSession(chatId);
			
			// Start the session
	        Thread t = new Thread() {
	    		public void run() {
	    			session.startSession();
	    		}
	    	};
	    	t.start();

			// Add session in the list
			GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session);
			ChatServiceImpl.addGroupChatSession(sessionApi);
			return sessionApi;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
    }
    
    /**
     * Restarts a previous group chat from its unique chat ID
     * 
     * @param chatId Chat ID
     * @return Group chat
     * @throws ServerApiException
     */
    public IGroupChat restartGroupChat(String chatId) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Restart group chat session related to the conversation " + chatId);
		}

		// Test IMS connection
		ServerApiUtils.testIms();

		try {
			// Initiate the session
			final ChatSession session = Core.getInstance().getImService().restartGroupChatSession(chatId);

			// Start the session
	        Thread t = new Thread() {
	    		public void run() {
	    			session.startSession();
	    		}
	    	};
	    	t.start();
			
			// Add session in the list
			GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session);
			ChatServiceImpl.addGroupChatSession(sessionApi);
			return sessionApi;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
    }
    
    /**
     * Returns the list of group chats in progress
     * 
     * @return List of group chat
     * @throws ServerApiException
     */
    public List<IBinder> getGroupChats() throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Get group chat sessions");
		}

		try {
			ArrayList<IBinder> result = new ArrayList<IBinder>(groupChatSessions.size());
			for (Enumeration<IGroupChat> e = groupChatSessions.elements() ; e.hasMoreElements() ;) {
				IGroupChat sessionApi = (IGroupChat)e.nextElement() ;
				result.add(sessionApi.asBinder());
			}
			return result;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
    }
    
    /**
     * Returns a group chat in progress from its unique ID
     * 
     * @param chatId Chat ID
     * @return Group chat or null if not found
     * @throws ServerApiException
     */
    public IGroupChat getGroupChat(String chatId) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Get group chat session " + chatId);
		}

		// Return a session instance
		return groupChatSessions.get(chatId);
	}
    
    /**
     * Adds a listener on new chat invitation events
     * 
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void addEventListener(INewChatListener listener) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Add a chat invitation listener");
		}
		
		listeners.register(listener);
    }
    
    /**
     * Removes a listener on new chat invitation events
     * 
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void removeEventListener(INewChatListener listener) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Remove a chat invitation listener");
		}
		
		listeners.unregister(listener);
    }
    
    /**
     * Returns the configuration of the chat service
     * 
     * @return Configuration
     */
    public ChatServiceConfiguration getConfiguration() {
    	return new ChatServiceConfiguration(
    			RcsSettings.getInstance().isStoreForwardWarningActivated(),
    			RcsSettings.getInstance().getChatIdleDuration(),
    			RcsSettings.getInstance().getIsComposingTimeout(),
    			RcsSettings.getInstance().getMaxChatParticipants(),
    			RcsSettings.getInstance().getMaxChatMessageLength(),
    			RcsSettings.getInstance().getMaxGroupChatMessageLength(),
    			RcsSettings.getInstance().getMaxChatSessions(),
    			RcsSettings.getInstance().isSmsFallbackServiceActivated(),
    			RcsSettings.getInstance().isChatAutoAccepted(),
    			RcsSettings.getInstance().isGroupChatAutoAccepted(),
    			RcsSettings.getInstance().isImReportsActivated(),
    			RcsSettings.getInstance().getMaxGeolocLabelLength(),
    			RcsSettings.getInstance().getGeolocExpirationTime(),
    			RcsSettings.getInstance().isImAlwaysOn());
	}    

    /**
	 * Registers a new chat invitation listener
	 * 
	 * @param listener New file transfer listener
	 * @throws ServerApiException
	 */
	public void addNewChatListener(INewChatListener listener) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Add a new chat invitation listener");
		}
		
		listeners.register(listener);
	}

	/**
	 * Unregisters a chat invitation listener
	 * 
	 * @param listener New file transfer listener
	 * @throws ServerApiException
	 */
	public void removeNewChatListener(INewChatListener listener) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Remove a chat invitation listener");
		}
		
		listeners.unregister(listener);
	}

	/**
	 * Returns IM always on or not
	 * 
	 * @return boolean value
	 * @throws ServerApiException
	 */
	public boolean isImCapAlwaysOn(){
		return RcsSettings.getInstance().isImAlwaysOn();
	}

	/**
	 * Returns service version
	 * 
	 * @return Version
	 * @see Build.VERSION_CODES
	 * @throws ServerApiException
	 */
	public int getServiceVersion() throws ServerApiException {
		return Build.API_VERSION;
	}
}
