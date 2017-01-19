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

package com.orangelabs.rcs.provider.messaging;

import org.gsma.joyn.chat.ChatLog;

import android.net.Uri;

/**
 * Chat data constants
 * 
 * @author Jean-Marc AUFFRET
 */
public class ChatData {
	/**
	 * Database URIs
	 */
	static final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.chat/chat");
	
	/**
	 * Column name
	 */
	static final String KEY_ID = ChatLog.GroupChat.ID;
	
	/**
	 * Column name
	 */
	static final String KEY_CHAT_ID = ChatLog.GroupChat.CHAT_ID;

	/**
	 * Column name
	 */
	static final String KEY_REJOIN_ID = "rejoin_id";

	/**
	 * Column name
	 */
	static final String KEY_STATUS = ChatLog.GroupChat.STATE;

	/**
	 * Column name
	 */
	static final String KEY_SUBJECT = ChatLog.GroupChat.SUBJECT;

	/**
	 * Column name
	 */
	static final String KEY_PARTICIPANTS = "participants";

	/**
	 * Column name
	 */
	static final String KEY_DIRECTION = ChatLog.GroupChat.DIRECTION;	

	/**
	 * Column name
	 */
	static final String KEY_TIMESTAMP = ChatLog.GroupChat.TIMESTAMP;
	
	/**
	 * Column name
	 */
	static final String KEY_CONVERSATION_ID = ChatLog.GroupChat.CONVERSATION_ID;
}
