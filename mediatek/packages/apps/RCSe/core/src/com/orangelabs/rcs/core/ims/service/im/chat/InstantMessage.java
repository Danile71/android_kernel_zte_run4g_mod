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

package com.orangelabs.rcs.core.ims.service.im.chat;

import java.util.Date;

/**
 * Instant message
 * 
 * @author Jean-Marc AUFFRET
 */
public class InstantMessage {
	/**
	 * MIME type
	 */
	public static final String MIME_TYPE = "text/plain";
	
	/**
	 * Remote user
	 */
	private String remote;
	
	/**
	 * Text message
	 */
	private String message;
	
	/**
	 * Receipt date of the message
	 */
	private Date receiptAt;
	
	/**
	 * Receipt date of the message on the server (i.e. CPIM date)
	 */
	private Date serverReceiptAt;

	/**
	 * Message Id
	 */
	private String msgId;
	
	/**
	 * Display Name 
	 */
	private String displayName;
	
	/**
	 * Flag indicating that an IMDN "displayed" is requested for this message
	 */
	private boolean imdnDisplayedRequested = false;

    /**
     * Constructor for outgoing message
     * 
     * @param messageId Message Id
     * @param remote Remote user
     * @param message Text message
     * @param imdnDisplayedRequested Flag indicating that an IMDN "displayed" is requested
	 */
	public InstantMessage(String messageId, String remote, String message, boolean imdnDisplayedRequested,String displayName) {
		this.msgId = messageId;
		this.remote = remote;
		this.message = message;
		this.imdnDisplayedRequested = imdnDisplayedRequested;
		Date date = new Date();
		this.receiptAt = date;
		this.serverReceiptAt = date;		
		this.displayName = displayName;
	}
	
	/**
     * Constructor for incoming message
     * 
     * @param messageId Message Id
     * @param remote Remote user
     * @param message Text message
     * @param imdnDisplayedRequested Flag indicating that an IMDN "displayed" is requested
	 * @param serverReceiptAt Receipt date of the message on the server
	 */
	public InstantMessage(String messageId, String remote, String message, boolean imdnDisplayedRequested, Date serverReceiptAt,String displayName) {
		this.msgId = messageId;
		this.remote = remote;
		this.message = message;
		this.imdnDisplayedRequested = imdnDisplayedRequested;
		this.receiptAt = new Date();
		this.serverReceiptAt = serverReceiptAt;
		this.displayName = displayName;
	}

	/**
	 * Returns the text message
	 * 
	 * @return String
	 */
	public String getTextMessage() {
		return message;
	}
	
	/**
	 * Returns display name
	 * 
	 * @return String
	 */
	public String getDisplayName() {
		return displayName;
	}
	
	/**
	 * Returns display name
	 * 
	 * @return String
	 */
	public void setDisplayName(String displayName) {
		this.displayName = displayName;
	}
	
	/**
	 * Returns the message Id
	 * 
	 * @return message Id
	 */
    public String getMessageId(){
    	return msgId;
    }
	
	/**
	 * Returns the remote user
	 * 
	 * @return Remote user
	 */
	public String getRemote() {
		return remote;
	}
	
	/**
	 * Returns true if the IMDN "displayed" has been requested 
	 * 
	 * @return Boolean
	 */
	public boolean isImdnDisplayedRequested() {
		return imdnDisplayedRequested;
	}
	
	/**
	 * Returns the receipt date of the message
	 * 
	 * @return Date
	 */
	public Date getDate() {
		return receiptAt;
	}

	/**
	 * Returns the receipt date of the message on the server
	 * 
	 * @return Date
	 */
	public Date getServerDate() {
		return serverReceiptAt;
	}
}
