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

import org.gsma.joyn.JoynServiceException;

import com.mediatek.rcse.api.Logger;

/**
 * Chat
 * 
 * @author Jean-Marc AUFFRET
 */
public class Chat {
    /**
     * Chat interface
     */
    private IChat chatInf;
    
    public static final String TAG = "TAPI-Chat";
    
    /**
     * Constructor
     * 
     * @param chatIntf Chat interface
     */
    Chat(IChat chatIntf) {
    	this.chatInf = chatIntf;
    }

    /**
     * Returns the remote contact
     * 
     * @return Contact
	 * @throws JoynServiceException
     */
    public String getRemoteContact() throws JoynServiceException {
    	Logger.i(TAG, "getRemoteContact entry");
		try {
			return chatInf.getRemoteContact();
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}
    }
    
	/**
     * Sends a chat message
     * 
     * @param message Message
	 * @return Unique message ID or null in case of error
   	 * @throws JoynServiceException
     */
    public String sendMessage(String message) throws JoynServiceException {
    	Logger.i(TAG, "ABC sendMessage entry " + message);
		try {
			return chatInf.sendMessage(message);
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
    	
    	Logger.i(TAG, "sendGeoloc entry " + geoloc);
		try {
			return chatInf.sendGeoloc(geoloc);
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
    	Logger.i(TAG, "sendDisplayedDeliveryReport entry " + msgId);
		try {
			chatInf.sendDisplayedDeliveryReport(msgId);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	
    }
	
    /**
     * Sends an Is-composing event. The status is set to true when
     * typing a message, else it is set to false.
     * 
     * @param status Is-composing status
	 * @throws JoynServiceException
     */
    public void sendIsComposingEvent(boolean status) throws JoynServiceException {
    	Logger.i(TAG, "sendIsComposingEvent entry " + status);
		try {
			chatInf.sendIsComposingEvent(status);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	
    }
	
    /**
     * Adds a listener on chat events
     *  
     * @param listener Chat event listener
	 * @throws JoynServiceException
     */
    public void addEventListener(ChatListener listener) throws JoynServiceException {
    	Logger.i(TAG, "addEventListener entry " + listener);
		try {
			chatInf.addEventListener(listener);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	
    }
	
    /**
     * Removes a listener on chat events
     * 
     * @param listener Chat event listener
	 * @throws JoynServiceException
     */
    public void removeEventListener(ChatListener listener) throws JoynServiceException {
    	Logger.i(TAG, "removeEventListener entry " + listener);
		try {
			chatInf.removeEventListener(listener);
		} catch(Exception e) {
			throw new JoynServiceException(e.getMessage());
		}    	    	
    }
}
