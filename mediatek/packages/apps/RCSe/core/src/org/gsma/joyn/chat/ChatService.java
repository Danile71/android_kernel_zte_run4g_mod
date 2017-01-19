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
package org.gsma.joyn.chat;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.gsma.joyn.IJoynServiceRegistrationListener;
import org.gsma.joyn.JoynContactFormatException;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.JoynServiceRegistrationListener;
import org.gsma.joyn.Permissions;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.IInterface;

import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.service.api.ChatServiceImpl;

/**
 * Chat service offers the main entry point to initiate chat 1-1 ang group
 * conversations with contacts. Several applications may connect/disconnect
 * to the API.
 * 
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 * 
 * @author Jean-Marc AUFFRET
 */
public class ChatService extends JoynService {
	/**
	 * API
	 */
	private IChatService api = null;

	public static final String TAG = "TAPI-ChatService";

	/**
     * Constructor
     * 
     * @param ctx Application context
     * @param listener Service listener
     */
    public ChatService(Context ctx, JoynServiceListener listener) {
    	super(ctx, listener);
    }

    /**
     * Connects to the API
     */
    public void connect() {
		/*if (this.ctx.checkCallingOrSelfPermission(Permissions.RCS_USE_CHAT) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_USE_CHAT");
		}*/
    	Logger.i(TAG, "connect() entry");
    	boolean connected = ctx.bindService(new Intent(IChatService.class.getName()), apiConnection, 0);
    	Logger.i(TAG, "connect() exit status" + connected);
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
        this.api = (IChatService)api;
    }

    /**
	 * Service connection
	 */
	private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
        	Logger.i(TAG, "onServiceConnected entry" + service);
        	setApi(IChatService.Stub.asInterface(service));
        	if (serviceListener != null) {
        		serviceListener.onServiceConnected();
        	}
        }

        public void onServiceDisconnected(ComponentName className) {
        	setApi(null);
        	Logger.i(TAG, "onServiceDisconnected entry");
        	if (serviceListener != null) {
        		serviceListener.onServiceDisconnected(Error.CONNECTION_LOST);
        	}
        }
    };
	
	/**
     * Returns the configuration of the chat service
     * 
     * @return Configuration
     * @throws JoynServiceException
     */
    public ChatServiceConfiguration getConfiguration() throws JoynServiceException {
		if (api != null) {
			try {
				Logger.i(TAG, "getConfiguration entry");
				return api.getConfiguration();
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
		if (api != null) {
			if (version == null) {
				try {
					version = api.getServiceVersion();
					Logger.i(TAG, "getServiceVersion entry " + version);
				} catch (Exception e) {
					throw new JoynServiceException(e.getMessage());
				}
			}
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
		Logger.i(TAG, "isServiceRegistered entry ");
		if (api != null) {
			boolean serviceStatus = false;
			try {
				serviceStatus = api.isServiceRegistered();
				Logger.i(TAG, "isServiceRegistered entry " + serviceStatus);
			} catch (Exception e) {				
				throw new JoynServiceException(e.getMessage());
			}
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
     * Open a single chat with a given contact and returns a Chat instance.
     * The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI.
     * 
     * @param contact Contact
     * @param listener Chat event listener
     * @return Chat or null 
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
     */
    public Chat openSingleChat(String contact, ChatListener listener) throws JoynServiceException, JoynContactFormatException {
    	Logger.i(TAG, "openSingleChat entry " + contact);
		if (api != null) {
			try {
				IChat chatIntf = api.openSingleChat(contact, listener);
				if (chatIntf != null) {
					return new Chat(chatIntf);
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
     * Initiates a group chat with a group of contact and returns a GroupChat
     * instance. The subject is optional and may be null.
     * 
     * @param contact List of contacts
     * @param subject Subject
     * @param listener Chat event listener
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
     */
    public GroupChat initiateGroupChat(Set<String> contacts, String subject, GroupChatListener listener) throws JoynServiceException, JoynContactFormatException {
    	Logger.i(TAG, "initiateGroupChat entry= " + contacts + " subject =" + subject );
    	if (api != null) {
			try {
				IGroupChat chatIntf = api.initiateGroupChat(new ArrayList<String>(contacts), subject, listener);
				if (chatIntf != null) {
					return new GroupChat(chatIntf);
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
     * Rejoins an existing group chat from its unique chat ID
     * 
     * @param chatId Chat ID
     * @return Group chat
     * @throws JoynServiceException
     */
    public GroupChat rejoinGroupChat(String chatId) throws JoynServiceException {
    	Logger.i(TAG, "rejoinGroupChat entry= " + chatId );
		if (api != null) {
			try {
				IGroupChat chatIntf = api.rejoinGroupChat(chatId);
				if (chatIntf != null) {
					return new GroupChat(chatIntf);
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
     * Restarts a previous group chat from its unique chat ID
     * 
     * @param chatId Chat ID
     * @return Group chat
     * @throws JoynServiceException
     */
    public GroupChat restartGroupChat(String chatId) throws JoynServiceException {
    	Logger.i(TAG, "restartGroupChat entry= " + chatId );
		if (api != null) {
			try {
				IGroupChat chatIntf = api.restartGroupChat(chatId);
				if (chatIntf != null) {
					return new GroupChat(chatIntf);
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
     * Returns the list of single chats in progress
     * 
     * @return List of chats
     * @throws JoynServiceException
     */
    public Set<Chat> getChats() throws JoynServiceException {
    	Logger.i(TAG, "getChats entry ");
		if (api != null) {
			try {
	    		Set<Chat> result = new HashSet<Chat>();
				List<IBinder> chatList = api.getChats();
				for (IBinder binder : chatList) {
					Chat chat = new Chat(IChat.Stub.asInterface(binder));
					result.add(chat);
				}
				Logger.i(TAG, "getChats returning with  " + result);
				return result;
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }
    
    /**
     * Returns a chat in progress with a given contact
     * 
     * @param contact Contact
     * @return Chat or null if not found
     * @throws JoynServiceException
     */
    public Chat getChat(String chatId) throws JoynServiceException {
    	Logger.i(TAG, "getChat entry " + chatId);
		if (api != null) {
			try {
				IChat chatIntf = api.getChat(chatId);
				if (chatIntf != null) {
					return new Chat(chatIntf);
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
     * Returns a single chat from its invitation Intent
     * 
     * @param intent Invitation Intent
     * @return Chat or null if not found
     * @throws JoynServiceException
     */
    public Chat getChatFor(Intent intent) throws JoynServiceException {
    	Logger.i(TAG, "getChatFor entry " + intent);
		if (api != null) {
			try {
				String contact = intent.getStringExtra(ChatIntent.EXTRA_CONTACT);
				if (contact != null) {
					return getChat(contact);
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
     * Returns the list of group chats in progress
     * 
     * @return List of group chat
     * @throws JoynServiceException
     */
    public Set<GroupChat> getGroupChats() throws JoynServiceException {
    	Logger.i(TAG, "getGroupChats entry ");
		if (api != null) {
			try {
	    		Set<GroupChat> result = new HashSet<GroupChat>();
				List<IBinder> chatList = api.getGroupChats();
				for (IBinder binder : chatList) {
					GroupChat chat = new GroupChat(IGroupChat.Stub.asInterface(binder));
					result.add(chat);
				}
				Logger.i(TAG, "getGroupChats returning with " + result);
				return result;
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }
    
    /**
     * Returns a group chat in progress from its unique ID
     * 
     * @param chatId Chat ID
     * @return Group chat or null if not found
     * @throws JoynServiceException
     */
    public GroupChat getGroupChat(String chatId) throws JoynServiceException {
    	Logger.i(TAG, "getGroupChat entry " + chatId);
		if (api != null) {
			try {
				IGroupChat chatIntf = api.getGroupChat(chatId);
				if (chatIntf != null) {
					return new GroupChat(chatIntf);
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
     * Returns a group chat from its invitation Intent
     * 
     * @param intent Intent invitation
     * @return Group chat or null if not found
     * @throws JoynServiceException
     */
    public GroupChat getGroupChatFor(Intent intent) throws JoynServiceException {
    	Logger.i(TAG, "getGroupChat entry " + intent);
		if (api != null) {
			try {
				String chatId = intent.getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
				if (chatId != null) {
					return getGroupChat(chatId);
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
	 * Registers a chat invitation listener
	 * 
	 * @param listener New chat listener
	 * @throws JoynServiceException
	 */
	public void addEventListener(NewChatListener listener) throws JoynServiceException {
		Logger.i(TAG, "addEventListener entry" + listener);
		if (api != null) {
			try {
				api.addEventListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Unregisters a chat invitation listener
	 * 
	 * @param listener New chat listener
	 * @throws JoynServiceException
	 */
	public void removeEventListener(NewChatListener listener) throws JoynServiceException {
		Logger.i(TAG, "removeEventListener entry" + listener);
		if (api != null) {
			try {
				api.removeEventListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}    
}
