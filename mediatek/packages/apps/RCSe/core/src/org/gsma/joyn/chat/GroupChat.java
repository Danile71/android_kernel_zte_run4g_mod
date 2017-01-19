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
import java.util.Set;

import org.gsma.joyn.JoynContactFormatException;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.ft.FileTransfer;
import org.gsma.joyn.ft.FileTransferListener;
import org.gsma.joyn.ft.IFileTransfer;

import com.mediatek.rcse.api.Logger;

/**
 * Group chat
 * 
 * @author Jean-Marc AUFFRET
 */
public class GroupChat {
	
	public static final String TAG = "TAPI-GroupChat";
    /**
     * Group chat state
     */
    public static class State {
    	/**
    	 * Unknown state
    	 */
    	public final static int UNKNOWN = 0;

    	/**
    	 * Chat invitation received
    	 */
    	public final static int INVITED = 1;
    	
    	/**
    	 * Chat invitation sent
    	 */
    	public final static int INITIATED = 2;
    	
    	/**
    	 * Chat is started
    	 */
    	public final static int STARTED = 3;
    	
    	/**
    	 * Chat has been terminated
    	 */
    	public final static int TERMINATED = 4;
    	   	
    	/**
    	 * Chat has been aborted 
    	 */
    	public final static int ABORTED = 5;
    	
    	/**
    	 * Chat has been closed by the user. A user which has closed a
    	 * conversation voluntary can't rejoin it afterward.
    	 */
    	public final static int CLOSED_BY_USER = 6;

    	/**
    	 * Chat has failed 
    	 */
    	public final static int FAILED = 7;
    	
        private State() {
        }    	
    }
    
    /**
     * Direction of the group chat
     */
    public static class Direction {
        /**
         * Incoming chat
         */
        public static final int INCOMING = 0;
        
        /**
         * Outgoing chat
         */
        public static final int OUTGOING = 1;
    }
    
    /**
     * Group chat error
     */
    public static class Error {
    	/**
    	 * Group chat has failed
    	 */
    	public final static int CHAT_FAILED = 0;
    	
    	/**
    	 * Group chat invitation has been declined by remote
    	 */
    	public final static int INVITATION_DECLINED = 1;

    	/**
    	 * Chat conversation not found
    	 */
    	public final static int CHAT_NOT_FOUND = 2;
    	    	
        private Error() {
        }    	
    }

    /**
     * Group chat interface
     */
    private IGroupChat chatInf;
    
    /**
     * Constructor
     * 
     * @param chatIntf Group chat interface
     */
    GroupChat(IGroupChat chatIntf) {
    	this.chatInf = chatIntf;
    }

    /**
     * Returns the chat ID
     * 
     * @return Chat ID
	 * @throws JoynServiceException
     */
	public String getChatId() throws JoynServiceException {
		Logger.i(TAG, "getChatId entry " + chatInf);
		try {
			return chatInf.getChatId();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}

	/**
     * Returns the chat ID
     * 
     * @return Chat ID
	 * @throws JoynServiceException
     */
	public String getChatSessionId() throws JoynServiceException {
		Logger.i(TAG, "getChatSessionId entry " + chatInf);
		try {
			return chatInf.getChatSessionId();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}

	/**
	 * Returns the direction of the group chat (incoming or outgoing)
	 * 
	 * @return Direction
	 * @see GroupChat.Direction
	 * @throws JoynServiceException
	 */
	public int getDirection() throws JoynServiceException {
		Logger.i(TAG, "getDirection entry " + chatInf);
		try {
			return chatInf.getDirection();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}	
	
	/**
	 * Returns the state of the group chat
	 * 
	 * @return State
	 * @see GroupChat.State
	 * @throws JoynServiceException
	 */
	public int getState() throws JoynServiceException {
		Logger.i(TAG, "getState() entry " + chatInf);
		try {
			return chatInf.getState();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}		
	
	/**	
	 * Returns the remote contact
	 * 
	 * @return Contact
	 * @throws JoynServiceException
	 */
	public String getRemoteContact() throws JoynServiceException {
		Logger.i(TAG, "getRemoteContact() entry " + chatInf);
		try {
			return chatInf.getRemoteContact();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}
	
	/**
	 * Returns the subject of the group chat
	 * 
	 * @return Subject
	 * @throws JoynServiceException
	 */
	public String getSubject() throws JoynServiceException {
		Logger.i(TAG, "getSubject() entry " + chatInf );
		try {
			return chatInf.getSubject();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}

	/**
	 * Returns the list of connected participants. A participant is identified
	 * by its MSISDN in national or international format, SIP address, SIP-URI or Tel-URI.
	 * 
	 * @return List of participants
	 * @throws JoynServiceException
	 */
	public Set<String> getParticipants() throws JoynServiceException {
		Logger.i(TAG, "getParticipants() entry " + chatInf );
		try {
			return new HashSet<String>(chatInf.getParticipants());
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}		
	}
	
	/**
	 * Returns the list of connected participants. A participant is identified
	 * by its MSISDN in national or international format, SIP address, SIP-URI or Tel-URI.
	 * 
	 * @return List of participants
	 * @throws JoynServiceException
	 */
	public Set<String> getAllParticipants() throws JoynServiceException {
		Logger.i(TAG, "getAllParticipants() entry " + chatInf );
		try {
			return new HashSet<String>(chatInf.getAllParticipants());
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}		
	}
	
	
	
	/**
	 * Accepts chat invitation
	 *  
	 * @throws JoynServiceException
	 */
	public void acceptInvitation() throws JoynServiceException {
		Logger.i(TAG, "acceptInvitation() entry " + chatInf );
		try {
			chatInf.acceptInvitation();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}
	
	/**
	 * Rejects chat invitation
	 * 
	 * @throws JoynServiceException
	 */
	public void rejectInvitation() throws JoynServiceException {
		Logger.i(TAG, "rejectInvitation() entry " + chatInf );
		try {
			chatInf.rejectInvitation();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}
	
	/**
	 * Sends a text message to the group
	 * 
	 * @param text Message
	 * @return Unique message ID or null in case of error
	 * @throws JoynServiceException
	 */
	public String sendMessage(String text) throws JoynServiceException {
		Logger.i(TAG, "sendMessage() entry " + text );
		try {
			return chatInf.sendMessage(text);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}		
	}
	
	/**
     * Sends a geoloc message
     * 
     * @param geoloc Geoloc info
	 * @return Unique message ID or null in case of error
   	 * @throws JoynServiceException
     */
    public String sendGeoloc(Geoloc geoloc) throws JoynServiceException {
    	Logger.i(TAG, "sendGeoloc() entry " + geoloc );
		try {
			return chatInf.sendGeoloc(geoloc);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	
    }	

    /**
     * Transfers a file to participants. The parameter filename contains the complete
     * path of the file to be transferred.
     * 
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listener File transfer event listener
     * @return File transfer
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
     */
    public FileTransfer sendFile(String filename, String fileicon, FileTransferListener listener) throws JoynServiceException {
    	Logger.i(TAG, "sendFile() entry filename=" + filename + " fileicon="+fileicon + " listener ="+listener);
    	try {
			IFileTransfer ftIntf = chatInf.sendFile(filename, fileicon, listener);
			if (ftIntf != null) {
				return new FileTransfer(ftIntf);
			} else {
				return null;
			}
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		} 
	}	
    
	/**
	 * Sends an Is-composing event. The status is set to true when typing
	 * a message, else it is set to false.
	 * 
	 * @param status Is-composing status
	 * @throws JoynServiceException
	 */
	public void sendIsComposingEvent(boolean status) throws JoynServiceException {
		Logger.i(TAG, "sendIsComposingEvent() entry " + status );
		try {
			chatInf.sendIsComposingEvent(status);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}		
	}
	
    /**
     * Sends a displayed delivery report for a given message ID
     * 
     * @param msgId Message ID
	 * @throws JoynServiceException
     */
    public void sendDisplayedDeliveryReport(String msgId) throws JoynServiceException {
    	Logger.i(TAG, "sendDisplayedDeliveryReport() entry " + msgId );
		try {
			chatInf.sendDisplayedDeliveryReport(msgId);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	
    }	
	
	/**
	 * Adds participants to a group chat
	 * 
	 * @param participants List of participants
	 * @throws JoynServiceException
	 */
	public void addParticipants(Set<String> participants) throws JoynServiceException {
		Logger.i(TAG, "addParticipants() entry " + participants );
		try {
			chatInf.addParticipants(new ArrayList<String>(participants));
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}		
	}
	
	/**
	 * Returns the max number of participants in the group chat. This limit is
	 * read during the conference event subscription and overrides the provisioning
	 * parameter.
	 * 
	 * @return Number
	 * @throws JoynServiceException
	 */
	public int getMaxParticipants() throws JoynServiceException {
		Logger.i(TAG, "getMaxParticipants() entry ");
		try {
			return chatInf.getMaxParticipants();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}		
	}
	
	/**
	 * Quits a group chat conversation. The conversation will continue between
	 * other participants if there are enough participants.
	 * 
	 * @throws JoynServiceException
	 */
	public void quitConversation() throws JoynServiceException {
		Logger.i(TAG, "quitConversation() entry ");
		try {
			chatInf.quitConversation();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
	}
	
	/**
	 * Adds a listener on chat events
	 * 
	 * @param listener Group chat event listener 
	 * @throws JoynServiceException
	 */
	public void addEventListener(GroupChatListener listener) throws JoynServiceException {
		Logger.i(TAG, "addEventListener() entry " + listener);
		
		try {
			chatInf.addEventListener(listener);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	    			
	}
	
	/**
	 * Removes a listener on chat events
	 * 
	 * @param listener Group chat event listener 
	 * @throws JoynServiceException
	 */
	public void removeEventListener(GroupChatListener listener) throws JoynServiceException {
		Logger.i(TAG, "removeEventListener() entry " + listener);
		try {
			chatInf.removeEventListener(listener);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	    			
	}
}
