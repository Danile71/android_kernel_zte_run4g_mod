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

package com.mediatek.rcse.mvc;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.TreeSet;
import java.util.UUID;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicReference;
import java.sql.Blob;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.capability.Capabilities;
import org.gsma.joyn.capability.CapabilitiesListener;
import org.gsma.joyn.capability.CapabilityService;
import org.gsma.joyn.chat.Chat;
import org.gsma.joyn.chat.ChatIntent;
import org.gsma.joyn.chat.ChatListener;
import org.gsma.joyn.chat.ChatLog;
import org.gsma.joyn.chat.ChatMessage;
import org.gsma.joyn.chat.ChatService;
import org.gsma.joyn.chat.ChatServiceConfiguration;
import org.gsma.joyn.chat.GroupChat;
import org.gsma.joyn.chat.GroupChatIntent;
import org.gsma.joyn.chat.GroupChatListener;
import org.gsma.joyn.chat.IChat;
import org.gsma.joyn.chat.IChatListener;
import org.gsma.joyn.chat.IChatService;
import org.gsma.joyn.chat.IGroupChat;
import org.gsma.joyn.chat.IGroupChatListener;
import org.gsma.joyn.chat.INewChatListener;
import org.gsma.joyn.chat.NewChatListener;
import org.gsma.joyn.ft.FileTransfer;
import org.gsma.joyn.ft.FileTransferIntent;
import org.gsma.joyn.ft.FileTransferListener;
import org.gsma.joyn.ft.FileTransferLog;
import org.gsma.joyn.ft.FileTransferService;
import org.gsma.joyn.ft.FileTransferServiceConfiguration;
import org.gsma.joyn.ft.IFileTransferListener;
import org.gsma.joyn.ft.INewFileTransferListener;
import org.gsma.joyn.ft.NewFileTransferListener;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;



import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;
import android.os.ParcelUuid;

import android.os.RemoteException;


import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.PluginProxyActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.ReceivedFileTransferItemBinder;
import com.mediatek.rcse.activities.widgets.UnreadMessagesContainer;
//import com.mediatek.rcse.api.CapabilityApi;
//import com.mediatek.rcse.api.CapabilityApi.ICapabilityListener;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.api.RegistrationApi.IRegistrationStatusListener;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChat1;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.NetworkChangedReceiver;
import com.mediatek.rcse.service.NetworkChangedReceiver.OnNetworkStatusChangedListerner;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.Utils;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;





/**
 * This is the virtual Module part in the MVC pattern
 */
public class ModelImpl implements IChatManager {
	

    public static final String TAG = "M0CF ModelImpl";
    private static final String CONTACT_NAME = "contactDisplayname";
    private static final String INTENT_MESSAGE = "messages";
    private static final String EMPTY_STRING = "";
    private static final String COMMA = ", ";
    private static final String SEMICOLON = ";";
    public static final Timer TIMER = new Timer();
    public static final int INDEXT_ONE = 1;
    private static int sIdleTimeout = 0;
    private ApiReceiver mReceiver = null;
    private IntentFilter intentFilter = null;
    private static final IChatManager INSTANCE = new ModelImpl();
    private static final HandlerThread CHAT_WORKER_THREAD = new HandlerThread("Chat Worker");
    static {
        CHAT_WORKER_THREAD.start();
        AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... arg0) {
                sIdleTimeout = RcsSettings.getInstance().getIsComposingTimeout() * 1000;
                return null;
            }
        };
        task.execute();
    }

    protected List<String> mReceivedAfterReloadFt =
            new ArrayList<String>(); 

    // The map retains Object&IChat&List<Participant>
    private final Map<Object, IChat1> mChatMap = new HashMap<Object, IChat1>();

    public static IChatManager getInstance() {
        
        return INSTANCE;
    }

    public ModelImpl() {
        super();
        if (mReceiver == null && intentFilter == null) {
            mReceiver = new ApiReceiver();
            intentFilter = new IntentFilter();
              intentFilter.addAction(GroupChatIntent.ACTION_NEW_INVITATION); //TODo check this
              intentFilter.addAction(ChatIntent.ACTION_DELIVERY_STATUS); 
            AndroidFactory.getApplicationContext().registerReceiver(mReceiver,
                    intentFilter);
        }        
    }

    @Override
    public IChat1 addChat(List<Participant> participants, Object chatTag,String chatId) {
        Logger.d(TAG, "addChat() entry, participants: " + participants + " chatTag: " + chatTag);
        int size = 0;
        IChat1 chat = null;
        ParcelUuid parcelUuid = null;
        if (chatTag == null) {
            UUID uuid = UUID.randomUUID();
            parcelUuid = new ParcelUuid(uuid);
        } else {
            parcelUuid = (ParcelUuid) chatTag;
        }
        Logger.d(TAG, "addChat() parcelUuid: " + parcelUuid + ",participants = " + participants);
        if (null != participants && participants.size() > 0) {
            size = participants.size();
            if (size > 1) {
                chat = new GroupChat1(this, null, participants, parcelUuid);
                mChatMap.put(parcelUuid, chat);

              
                
                for( int i =0 ; i< ((GroupChat1) chat).getParticipantInfos().size();i++)
                ((GroupChat1) chat).getParticipantInfos().get(i).setmChatID(chatId);

                IGroupChatWindow chatWindow = ViewImpl.getInstance().addGroupChatWindow(parcelUuid,
                        ((GroupChat1) chat).getParticipantInfos());
                ((GroupChat1) chat).setChatWindow(chatWindow);
            } else {
                chat = getOne2OneChat(participants.get(0));
                if (null == chat) {
                    Logger.i(TAG, "addChat() The one-2-one chat with " + participants
                            + "doesn't exist.");
                    Participant participant = participants.get(0);
                    chat = new One2OneChat(this, null, participant, parcelUuid);
                    mChatMap.put(parcelUuid, chat);
                    String number = participant.getContact();
                    if (ContactsListManager.getInstance().isLocalContact(number)
                            || ContactsListManager.getInstance().isStranger(number)) {
                        Logger.d(TAG, "addChat() the number is local or stranger" + number);
                    } else {
                       try{
                        CapabilityService capabilityApi = ApiManager.getInstance()
                                .getCapabilityApi();
                        Logger.d(TAG,
                                "addChat() the number is not local or stranger"
                                        + number + ", capabilityApi= "
                                        + capabilityApi);
                        if (capabilityApi != null) {
							Capabilities capability = null;
							try{
                            	 capability = capabilityApi.getContactCapabilities(number);
							}
							catch(JoynServiceException e){
								Logger.d(TAG, "addChat() getContactCapabilities JoynServiceException");
							}
                            Logger.d(TAG, "capability = " + capability);
                            if (capability != null) {
                                if (capability.isImSessionSupported()) {
                                    ContactsListManager.getInstance()
                                            .setStrangerList(number, true);
                                } else {
                                    ContactsListManager.getInstance()
                                            .setStrangerList(number, false);
                                }
                            }
                        }
                    }
					  catch(Exception e){
					  	 e.printStackTrace();
					  }
                    }
                    IOne2OneChatWindow chatWindow = ViewImpl.getInstance().addOne2OneChatWindow(
                            parcelUuid, participants.get(0));
                    ((One2OneChat) chat).setChatWindow(chatWindow);
                } else {
                    Logger.i(TAG, "addChat() The one-2-one chat with " + participants
                            + "has existed.");
                    ViewImpl.getInstance().switchChatWindowByTag(
                            (ParcelUuid) (((One2OneChat) chat).mTag));
                }
            }
        }
        Logger.d(TAG, "addChat(),the chat is " + chat);
        return chat;
    }

    private IChat1 addChat(List<Participant> participants) {
        return addChat(participants, null,null);
    }

    public IChat1 getOne2oneChatByContact(String contact) {
        Logger.i(TAG, "ABC getOne2oneChatByContact() contact: " + contact);
        IChat1 chat = null;
        if (TextUtils.isEmpty(contact)) {
			Logger.i(TAG, "ABC getOne2oneChatByContact() return null");
            return chat;
        }
        ArrayList<Participant> participant = new ArrayList<Participant>();
        participant.add(new Participant(contact, contact));
        chat = addChat(participant, null,null);
		Logger.i(TAG, "ABC getOne2oneChatByContact() return NEW CHAT");
        return chat;
    }

    private IChat1 getOne2OneChat(Participant participant) {
        Logger.i(TAG, "getOne2OneChat() entry the participant is " + participant);
        Collection<IChat1> chats = mChatMap.values();
        for (IChat1 chat : chats) {
            if (chat instanceof One2OneChat && ((One2OneChat) chat).isDuplicated(participant)) {
                Logger.d(TAG, "getOne2OneChat() find the 1-2-1 chat with " + participant
                        + " has existed");
                return chat;
            }
        }
        Logger.d(TAG, "getOne2OneChat() could not find the 1-2-1 chat with " + participant);
        return null;
    }

    @Override
    public IChat1 getChat(Object mTag) {
        Logger.v(TAG, "getChat(),tag = " + mTag);
		if (mTag == null) {
            Logger.v(TAG, "tag is null so return null");
            return null;
        }
        if (mChatMap.isEmpty()) {
            Logger.v(TAG, "mChatMap is empty so no chat exist");
            return null;
        }
        if (mTag instanceof ParcelUuid) {
			ParcelUuid tag = new ParcelUuid(UUID.fromString(mTag.toString()));
            Logger.v(TAG, "tgetChat(),tag instanceof ParcelUuid");
        IChat1 chat = mChatMap.get(tag);
        Logger.v(TAG, "return chat = " + chat);
        return chat;
        } else {
            Logger.v(TAG, "tgetChat(),tag not instanceof ParcelUuid");
			IChat1 chat = mChatMap.get(mTag);
	        Logger.v(TAG, "return chat = " + chat);
	        return chat;
        }      
    }

    @Override
    public List<IChat1> listAllChat() {
        List<IChat1> list = new ArrayList<IChat1>();
        if (mChatMap.isEmpty()) {
            Logger.w(TAG, "mChatMap is empty");
            return list;
        }
        Collection<IChat1> collection = mChatMap.values();
        Iterator<IChat1> iter = collection.iterator();
        while (iter.hasNext()) {
            IChat1 chat = iter.next();
            if (chat != null) {
                Logger.w(TAG, "listAllChat()-IChat1 is empty");
                list.add(chat);
            }
        }
        return list;
    }

    @Override
    public boolean removeChat(Object tag) {
        if (tag == null) {
            Logger.w(TAG, "removeChat()-The tag is null");
            return false;
        }

        ParcelUuid uuid = new ParcelUuid(UUID.fromString(tag.toString()));
        IChat1 chat = mChatMap.remove(uuid);
        if (chat != null) {
            ((ChatImpl) chat).onDestroy();
            
			//IChat1 chatToRemove = getChat(tag);
			if (chat instanceof One2OneChat) {
				//EventsLogApi eventsLogApi = null;
				if (ApiManager.getInstance() != null) {
					//eventsLogApi = ApiManager.getInstance().getEventsLogApi(); //TODo check this
					 RichMessagingDataProvider.getInstance().deleteMessagingLogForContact(((One2OneChat) chat).getParticipant().getContact());
				}
			} else {
				//for group currently not supported
			}
			
            mOutGoingFileTransferManager.onChatDestroy(tag);
            ViewImpl.getInstance().removeChatWindow(((ChatImpl) chat).getChatWindow());
            return true;
        } else {
            Logger.d(TAG, "removeChat()-The chat is null");
            return false;
        }
    }

    /**
     * Remove group chat, but does not close the window associated with the
     * chat.
     * 
     * @param tag The tag of the group chat to be removed.
     * @return True if success, else false.
     */
    public boolean quitGroupChat(Object tag) {
        Logger.d(TAG, "quitGroupChat() entry, with tag is " + tag);
        if (tag == null) {
            Logger.w(TAG, "quitGroupChat() tag is null");
            return false;
        }
        IChat1 chat = getChat(tag);
        if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).onQuit();
            mOutGoingFileTransferManager.onChatDestroy(tag);
            return true;
        } else {
            Logger.d(TAG, "quitGroupChat() chat is null");
            return false;
        }
    }
    /**
     * export group chat messages
     * 
     * @param tag The tag of the group chat to be exported.
     * @return True if success, else false.
     */
    public boolean exportGroupChat(Object tag) {
        Logger.d(TAG, "exportGroupChat() entry, with tag is " + tag);
        if (tag == null) {
            Logger.w(TAG, "exportGroupChat() tag is null");
            return false;
        }
        IChat1 chat = getChat(tag);
        if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).initiateExportChat();
            return true;
        } else {
            Logger.d(TAG, "exportGroupChat() chat is null");
            return false;
        }
    }

    /**
     * export group chat messages
     * 
     * @param tag The tag of the group chat to be exported.
     * @return True if success, else false.
     */
    public boolean clearExtraMessageGroup(Object tag) {
        Logger.d(TAG, "clearExtraMessageGroup() entry, with tag is " + tag);
        if (tag == null) {
            Logger.w(TAG, "clearExtraMessageGroup() tag is null");
            return false;
        }
        IChat1 chat = getChat(tag);
        if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).clearExtraMessageGroup();
            return true;
        } else {
            Logger.d(TAG, "clearExtraMessageGroup() chat is null");
            return false;
        }
    }    
    

    /**
     * when there is not not file transfer capability, cancel all the file
     * transfer
     * 
     * @param tag The tag of the chat.
     * @param reason the reason for file transfer not available.
     */
    public void handleFileTransferNotAvailable(Object tag, int reason) {
        Logger.d(TAG, "handleFileTransferNotAvailable() entry, with tag is " + tag + " reason is "
                + reason);
        if (tag == null) {
            Logger.w(TAG, "handleFileTransferNotAvailable() tag is null");
        } else {
            mOutGoingFileTransferManager.clearTransferWithTag(tag, reason);
        }
    }

    /**
     * remove chat according the relevant participant
     * 
     * @param the participant of the chat
     */
    public void removeChatByContact(Participant participant) {
        IChat1 chat = getOne2OneChat(participant);
        Logger.v(TAG, "removeChatByContact(),participant = " + participant
                + ", chat = " + chat);
        if (chat != null) {
            removeChat((ChatImpl) chat);
        }
        
       // EventsLogApi eventsLogApi = null; //TODo check this
        if (ApiManager.getInstance() != null) {
        	//eventsLogApi = ApiManager.getInstance().getEventsLogApi();
         RichMessagingDataProvider.getInstance().deleteMessagingLogForContact(participant.getContact());
        }
    }

    private boolean removeChat(final ChatImpl chat) {
        if (mChatMap.isEmpty()) {
            Logger.w(TAG, "removeChat()-mChatMap is empty");
            return false;
        }
        if (!mChatMap.containsValue(chat)) {
            Logger.w(TAG, "removeChat()-mChatMap didn't contains this IChat1");
            return false;
        } else {
            Logger.v(TAG, "mchatMap size is " + mChatMap.size());
            mChatMap.values().remove(chat);
            Logger.v(TAG, "After removded mchatMap size is " + mChatMap.size() + ", chat = "
                    + chat);
            if (chat != null) {
                chat.onDestroy();
                mOutGoingFileTransferManager.onChatDestroy(chat.mTag);
                ViewImpl.getInstance().removeChatWindow(chat.getChatWindow());
            }
            return true;
        }
    }

    /**
     * Clear all the chat messages. Include clear all the messages in date base
     * ,clear messages in all the chat window and clear the last message in each
     * chat list item.
     */
    public void clearAllChatHistory() {
	    if( ApiManager.getInstance() != null && ApiManager.getInstance().getContext() != null ){
        ContentResolver contentResolver =
                ApiManager.getInstance().getContext().getContentResolver();
			if(contentResolver != null){
        contentResolver.delete(ChatLog.Message.CONTENT_URI, null, null); 
			}
        List<IChat1> list = INSTANCE.listAllChat();
        int size = list.size();
        Logger.d(TAG, "clearAllChatHistory(), the size of the chat list is " + size);
        for (int i = 0; i < size; i++) {
            IChat1 chat = list.get(i);
            if (chat != null) {
                ((ChatImpl) chat).clearChatWindowAndList();
            }
        }
    }
    }
        
    public void handleDeleteMessage(Object tag, String msgId) {
        Logger.i(TAG, "handleDeleteMessage msgId: " + msgId);         
       // delete from window Add window function to delete
        IChat1 chat = ModelImpl.getInstance().getChat(tag);
        if (chat instanceof ChatImpl) {
			try{
            ((ChatImpl) chat).getChatWindow().removeChatMessage(msgId);
        } 	
			catch(Exception e){
				e.printStackTrace();
			}
        } 	
        if(RichMessagingDataProvider.getInstance() == null){
        	RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
        }	
        //delete message from database    	
        RichMessagingDataProvider.getInstance().deleteMessage(msgId);
		//delete file from database    	
        RichMessagingDataProvider.getInstance().deleteFileTranfser(msgId);
    }

	 public void handleDeleteMessageList(Object tag, List<String> msgIdList){              
       // delete from window Add window function to delete
       Logger.i(TAG, "handleDeleteMessageList size: " + msgIdList.size());      
       try{
	        IChat1 chat = ModelImpl.getInstance().getChat(tag);
		    for(int i = 0;i < msgIdList.size(); i++){
				try{
					handleDeleteMessage(tag, msgIdList.get(i) );
				}
				catch(Exception e)
			   	{
			   		e.printStackTrace();
			   	}
		   	}
       	}
	   catch(Exception e)
	   	{
	   		e.printStackTrace();
	   	}
    }
    
    /**
     * This class is the implementation of a chat message used in model part
     */
    private static class ChatMessage1 implements IChatMessage {
        private ChatMessage mChatMessage;

        public ChatMessage1(ChatMessage chatMessage) {
            mChatMessage = chatMessage;
        }

        @Override
        public ChatMessage getChatMessage() {
            return mChatMessage;
        }
    }

    public static class ChatListProvider {

        private Integer messageId;
        private String  chat_id;
        public Integer getMessageId() {
            return messageId;
        }

        public void setMessageId(Integer messageId) {
            this.messageId = messageId;
        }
        public void setChatId(String ChatID) {
            this.chat_id = ChatID;
        }
        
        public String getChatID(){
            return chat_id;
        }

        private ArrayList<Participant> participantlist;

        public ArrayList<Participant> getParticipantlist() {
            return participantlist;
        }

        public void setParticipantlist(ArrayList<Participant> participantlist) {
            this.participantlist = participantlist;
        }

    }
    
    public ArrayList<ChatListProvider> getChatListHistory(Context context)
    { 
        ArrayList<ChatListProvider> recentChats = null;        
        if(RichMessagingDataProvider.getInstance()==null)
        RichMessagingDataProvider.createInstance(context);

        RichMessagingDataProvider msgDataProvider = RichMessagingDataProvider.getInstance();
        
        //get the recent chats history
        recentChats = msgDataProvider.getRecentChats();
        
        return recentChats;
    }
    
    /**
     *  This class used to receive broadcasts from RCS Core Service
     */
    
    private class ApiReceiver extends BroadcastReceiver {       
        
        @Override
        public void onReceive(final Context context, final Intent intent) {
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                Logger.i(TAG, "ABC ModelImpl onReceive ");
                String action = intent.getAction();
		        if (action == null) {
					Logger.i(TAG, "ABC ModelImpl onReceive return");
		            return null;
		        }

			     if(ChatIntent.ACTION_DELIVERY_STATUS.equalsIgnoreCase(action)){
				 	    String remoteContact = intent.getStringExtra(ChatIntent.EXTRA_CONTACT);
						String msgId = intent.getStringExtra("msgId");
						String status = intent.getStringExtra("status");
						//String remoteContact = null;
				 	    Logger.i(TAG, "ABC handleMessageDeliveryStatus() entry, remoteContact:" + remoteContact +"Msgid:" + msgId);
						
				 	    Logger.i(TAG, "ABC handleMessageDeliveryStatus() entry, remoteContact:" + remoteContact + "status" + status);
				        //remoteContact = PhoneUtils.extractNumberFromUri(remoteContact);
				        IChat1 chat = null;
						chat = getOne2oneChatByContact(remoteContact);
				        if (null != chat) {
						 	Logger.i(TAG, "ABC handleMessageDeliveryStatus() chat found");
				        	if (chat instanceof One2OneChat) {
				            	((One2OneChat) chat).onMessageDelivered(msgId, status, 0);
				            }
				        }				        					    
				    	chat = getGroupChat(null);
						if (null != chat) {
				           if (chat instanceof GroupChat1) {
				              ((GroupChat1) chat).onMessageDelivered(msgId, status, 0);
				           }      	   
				        }
						return null;
			     }
				 else if(GroupChatIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)){
                    try {
						Logger.v(TAG, "DEF New chat inviation");
                        String groupChatId = intent.getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
						if(ApiManager.getInstance() == null || ApiManager.getInstance().getChatApi() == null ){
							return null;
						}
                        GroupChat groupChatImpl = ApiManager.getInstance().getChatApi().getGroupChat(groupChatId);
						if(groupChatImpl == null){
							return null;
						}
                        ChatMessage msg = intent
                                .getParcelableExtra("firstMessage");
                        
						if(msg != null){
						 	Logger.w(TAG, "first message group" + msg.getMessage());
						}
						 else{
							Logger.w(TAG, "first message group firstmessage:NULL");
						}
                        
                        /**
                         *  managing extra local chat participants that are 
                         * not present in the invitation for sending them invite request.@{
                         */
                        //getting the participant list for this invitaion
                        String participantList = intent.getStringExtra("participantList");
                        
                        ArrayList<IChatMessage> chatMessages = new ArrayList<IChatMessage>();
                        chatMessages.add(new ChatMessageReceived(msg));
                        String chatId = intent
                                .getStringExtra(RcsNotification.CHAT_ID);

                        IChat1 chat = getGroupChat(chatId);
                        if(intent.getBooleanExtra("isGroupChatExist", false));
                        {
						  Logger.v(TAG, "DEF handleReInvitation");
                          if(chat!=null)
                          ((GroupChat1)chat).handleReInvitation(groupChatImpl, participantList);
                          else
                          Logger.v(TAG, "Chat null in onReceive");
                        }
                    } catch (JoynServiceNotAvailableException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
					 catch (Exception e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                    return null;
                }
				 return null;
            }
		}.execute();
       }
    }
    

    /**
     * This class is the implementation of a sent chat message used in model
     * part
     */
    public static class ChatMessageSent extends ChatMessage1 {
        public ChatMessageSent(ChatMessage sentMessage) {
            super(sentMessage);
        }
    }

    /**
     * This class is the implementation of a received chat message used in model
     * part
     */
    public static class ChatMessageReceived extends ChatMessage1 {
        public ChatMessageReceived(ChatMessage receivedMessage) {
            super(receivedMessage);
        }
    }

    /**
     * The call-back method of the interface called when the unread message
     * number changed
     */
    public interface UnreadMessageListener {
        /**
         * The call-back method to update the unread message when the number of
         * unread message changed
         * 
         * @param chatWindowTag The chat window tag indicates which to update
         * @param clear Whether cleared all unread message
         */
        void onUnreadMessageNumberChanged(Object chatWindowTag, boolean clear);
    }

    /**
     * It's a implementation of IChat, it indicates a specify chat model.
     */
    public abstract class ChatImpl extends CapabilitiesListener implements IChat1, IRegistrationStatusListener
             {
        protected Participant mParticipant = null;

        private static final String TAG = "M0CF ChatImpl";
        // Chat message list
        private final List<IChatMessage> mMessageList = new LinkedList<IChatMessage>();

        protected final AtomicReference<GroupChat> mCurrentGroupChatImpl =
                new AtomicReference<GroupChat>();
        protected ComposingManager mComposingManager = new ComposingManager();
        protected Object mTag = null;
        protected boolean mIsInBackground = true;

        protected IChatWindow mChatWindow = null;

        protected List<ChatMessage> mReceivedInBackgroundToBeDisplayed =
                new ArrayList<ChatMessage>();
        protected List<ChatMessage> mReceivedInBackgroundToBeRead =
                new ArrayList<ChatMessage>();

		 protected List<FileTransfer> mReceivedInBackgroundFtToBeDisplayed =
                new ArrayList<FileTransfer>();
        protected List<FileTransfer> mReceivedInBackgroundFtToBeRead =
                new ArrayList<FileTransfer>();
        
        protected List<String> mReceivedAfterReloadMessage =
                new ArrayList<String>();

        protected RegistrationApi mRegistrationApi = null;
        protected Thread mWorkerThread = CHAT_WORKER_THREAD;
        protected Handler mWorkerHandler = new Handler(CHAT_WORKER_THREAD.getLooper());

        public static final int FILETRANSFER_ENABLE_OK = 0;
        public static final int FILETRANSFER_DISABLE_REASON_NOT_REGISTER = 1;
        public static final int FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED = 2;
        public static final int FILETRANSFER_DISABLE_REASON_REMOTE = 3;
		public static final int GROUPFILETRANSFER_DISABLE = 4;
        com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;

        protected SentMessageManager mSentMessageManager = new SentMessageManager();
        
        /**
         * This class is used manage the sent messages, especially for the time-out
         * ones
         */
        protected class SentMessageManager {
            public final String tag = "M0CFC SentMessageManager@" + mTag;


            protected static final long TIME_OUT_MILLI = 30000;
            //private static final long TIME_OUT_MILLI = RcsSettings.getInstance().getDeliveryTimeout();
            protected final Map<String, SendMessageWatcher> mSendingMessage =
                    new LinkedHashMap<String, SendMessageWatcher>();

            public void onMessageSent(ISentChatMessage sentMessage) {
                if (null != sentMessage) {
                    synchronized (mSendingMessage) {
                        SendMessageWatcher newWatcher = new SendMessageWatcher(sentMessage);
                        mSendingMessage.put(sentMessage.getId(), newWatcher);
                        ModelImpl.TIMER.schedule(newWatcher, TIME_OUT_MILLI);
                        Logger.d(tag, "onMessageSent() add watcher: " + newWatcher + ", current size: "
                                + mSendingMessage.size());
                    }
                } else {
                    Logger.w(tag, "onMessageSent() sentMessage is null");
                }
            }

            public void onMessageDelivered(final String messageId, final com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status status, final long timeStamp) {
                Logger.d(tag, "onMessageDelivered() messageId is " + messageId + ", status is " + status + ", time stamp: "
                        + timeStamp);
                ISentChatMessage message = (ISentChatMessage) mChatWindow.getSentChatMessage(messageId);
				
                if (null != message) {
                    message.updateStatus(status);
                    if (timeStamp > 0) {
                       // message.updateDate(new Date(timeStamp));
                    }
                } else {
                    Logger.e(tag, "onMessageDelivered() message is null");
                }
    			
                if (mSendingMessage.containsKey(messageId)) {
                    synchronized (mSendingMessage) {
                        SendMessageWatcher watcher = mSendingMessage.get(messageId);
                        Logger.d(tag, "onMessageDelivered() watcher: " + watcher
                                + ", messageId = " + messageId);
                        if (null != watcher) {
                            watcher.cancel();
                            mSendingMessage.remove(messageId);
                        }
                    }
                }

    			
            }

            public void markSendingMessagesDisplayed() {
                Logger.v(tag, "markSendingMessagesDisplayed() entry");
                synchronized (mSendingMessage) {
                    Collection<SendMessageWatcher> values = mSendingMessage.values();
                    if (values.size() > 0) {
                        for (SendMessageWatcher watcher : values) {
                            watcher.onMessageDelivered();
                            watcher.cancel();
                        }
                        mSendingMessage.clear();
                    }
                }
            }

            public void onChatDestroy() {
                Logger.v(tag, "onChatDestroy() entry");
                synchronized (mSendingMessage) {
                    Collection<SendMessageWatcher> values = mSendingMessage.values();
                    if (values.size() > 0) {
                        for (SendMessageWatcher watcher : values) {
                            watcher.cancel();
                        }
                        mSendingMessage.clear();
                    }
                }
            }

            protected void onTimeout(SendMessageWatcher timerTask) {
                synchronized (mSendingMessage) {
                    Logger.d(TAG, "onTimeout() timerTask: " + timerTask);
					 try{
    				 	CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
					 	if(capabilityApi != null){
    				 		capabilityApi.requestContactCapabilities(mParticipant.getContact());
					 	}
					 }
					 catch(JoynServiceException e){
						Logger.d(TAG, "onTimeout() getContactCapabilities JoynServiceException");
					 }
					  catch(Exception e){
						Logger.d(TAG, "onTimeout() getContactCapabilities JoynServiceException1");
						e.printStackTrace();
					 }
					
    				//Core.getInstance().getCapabilityService().requestContactCapabilities(mParticipant.getContact());
                    mSendingMessage.remove(timerTask.getId());
                    Logger.d(TAG, "onTimeout() current size: " + mSendingMessage.size());
                }
            }

            /**
             * A time-out watcher for one single sent message
             */
            protected class SendMessageWatcher extends TimerTask {
                public String tag = "M0CFC SendMessageWatcher:";
                private ISentChatMessage mMessage = null;
                private ISentChatMessage.Status mStatus =  com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status.SENDING;
                public SendMessageWatcher(ISentChatMessage sentChatMessage) {
                    mMessage = sentChatMessage;
                    tag += getId();
                    Logger.v(tag, "Constructor");
                }

                @Override
                public String toString() {
                    return tag;
                }

                public String getId() {
                    return mMessage.getId();
                }

                public void onMessageDelivered() {
                    Logger.v(tag, "onMessageDelivered() entry");
                    switch (mStatus) {
                        case SENDING:
                        case DELIVERED:
                            mStatus =  com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status.DISPLAYED;
                            mMessage.updateStatus(mStatus);
                            Logger.d(tag, "onMessageDelivered() update status: " + mStatus);
                            break;
                        case FAILED:
                            Logger.w(tag, "onMessageDelivered() invalid status: " + mStatus);
                            break;
                        case DISPLAYED:
                            Logger.w(tag, "onMessageDelivered() do nothing, status: " + mStatus);
                            break;
                        default:
                            Logger.w(tag, "onMessageDelivered() unknown status: " + mStatus);
                            break;
                    }
                }

                @Override
                public void run() {
                    onTimeout(SendMessageWatcher.this);
                    switch (mStatus) {
                        case SENDING:
                            mStatus =  com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status.FAILED;
                            mMessage.updateStatus(mStatus);
                            mMessage.updateDate(new Date());
                            Logger.d(tag, "run() update status: " + mStatus);
                            break;
                        case FAILED:
                            Logger.w(tag, "run() invalid status: " + mStatus);
                            break;
                        case DISPLAYED:
                        case DELIVERED:
                            Logger.w(tag, "run() do nothing, status: " + mStatus);
                            break;
                        default:
                            Logger.w(tag, "run() unknown status: " + mStatus);
                            break;
                    }
                }
            }
        }
		
        protected FileTransferController mFileTransferController = null;

        protected class FileTransferController {
            private boolean mRegistrationStatus = true;
            private boolean mRemoteFtCapability = true;
            private boolean mLocalFtCapability = true;
		 private boolean mRemoteGroupFtCapability = true;
        private boolean mLocalFtGroupCapability = true;

		private static final String TAG = "M0CFF ChatImpl";

            public void setRegistrationStatus(boolean status) {
                Logger.d(TAG, "setRegistrationStatus entry status is " + status);
                mRegistrationStatus = status;
            }

            public void setRemoteFtCapability(boolean status) {
                Logger.d(TAG, "setRemoteFtCapability entry status is " + status);
                mRemoteFtCapability = status;
            }

            public void setLocalFtCapability(boolean status) {
                Logger.d(TAG, "setLocalFtCapability entry status is " + status);
                mLocalFtCapability = status;
            }

		public void setLocalGroupFtCapability(boolean status) {
            Logger.i(TAG, "setGroupLocalFtCapability entry status is " + status);
            mLocalFtGroupCapability = status;
        }

		public void setRemoteGroupFtCapability(boolean status) {
            Logger.i(TAG, "setGroupRemoteFtCapability entry status is " + status);
            mRemoteGroupFtCapability = status;
        }

        public void controlGroupFileTransferIconStatus() {
            Logger.i(TAG, "controlGroupFileTransferIconStatus entry: mChatWindow = "
                    + mChatWindow + ", mRegistrationStatus "
                    + mRegistrationStatus);
			Logger.d(TAG,"controlGroupFileTransferIconStatus mLocalFtGroupCapability:" + mLocalFtGroupCapability +"mRemoteGroupFtCapability"
				                  + mRemoteGroupFtCapability);
            if (mRegistrationStatus && mRemoteGroupFtCapability
                    && mLocalFtGroupCapability) {
                if (mChatWindow != null) {
                    ((IGroupChatWindow) mChatWindow) 
                            .setFileTransferEnable(FILETRANSFER_ENABLE_OK);
                    Logger.d(TAG,"controlGroupFileTransferIconStatus reason is FILETRANSFER_ENABLE_OK");
                }
            }
			else{
				 if (mChatWindow != null) {
                    ((IGroupChatWindow) mChatWindow) 
                            .setFileTransferEnable(GROUPFILETRANSFER_DISABLE);
                    Logger.i(TAG,"controlGrouFileTransferIconStatus Disable");
                }
				}
        }

            /**
             * control the file transfer icon status caused by registration, self
             * capability, remote capability. All conditions are satisfied, the icon
             * works well
             */
            public void controlFileTransferIconStatus() {
                Logger.i(TAG, "controlFileTransferIconStatus entry: mChatWindow = "
                        + mChatWindow + ", mRegistrationStatus "
                        + mRegistrationStatus);
                if (mRegistrationStatus && mRemoteFtCapability
                        && mLocalFtCapability) {
                    if (mChatWindow != null) {
                    ((IOne2OneChatWindow) mChatWindow).setFileTransferEnable(FILETRANSFER_ENABLE_OK);
                        Logger.i(TAG,"controlFileTransferIconStatus reason is FILETRANSFER_ENABLE_OK");
                    }
                } else {
                    if (!mRegistrationStatus) {
                        if (mChatWindow != null) {
                        		((IOne2OneChatWindow) mChatWindow)
                                .setFileTransferEnable(FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
                               Logger.i(TAG,"controlFileTransferIconStatus reason is FILETRANSFER_DISABLE_REASON_NOT_REGISTER");
                        }

                    } else if (!mLocalFtCapability) {
                        if (mChatWindow != null) {
                        ((IOne2OneChatWindow) mChatWindow)
                                .setFileTransferEnable(FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
                            Logger.i(TAG, "controlFileTransferIconStatus()" +
                                    " reason is FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED");
                        }

                    } else if (!mRemoteFtCapability) {
                        if (mChatWindow != null) {
                        ((IOne2OneChatWindow) mChatWindow)
                                .setFileTransferEnable(FILETRANSFER_DISABLE_REASON_REMOTE);
                            Logger.i(TAG, "controlFileTransferIconStatus()" +
                                    " reason is FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED");
                        }
                    }
                }
            }
        }
        
        /**
         * This class is used to manage the whole receive file transfers
         */
        protected class ReceiveFileTransferManager {
            private static final String TAG = "M0CFF ReceiveFileTransferManager";

            private CopyOnWriteArrayList<ReceiveFileTransfer> mActiveList =
                    new CopyOnWriteArrayList<ReceiveFileTransfer>();

            /**
             * Handle a new file transfer invitation
             */
            public synchronized void addReceiveFileTransfer(FileTransfer fileTransferObject, boolean isAutoAccept,boolean isGroup) {
                Logger.d(TAG, "addReceiveFileTransfer() entry, fileTransferObject = "
                        + fileTransferObject);
                if (null != fileTransferObject) {
                    ReceiveFileTransfer receiveFileTransfer =
     new ReceiveFileTransfer(fileTransferObject, isAutoAccept,isGroup);
                    mActiveList.add(receiveFileTransfer);
                }
            }

			public ReceiveFileTransfer getReceiveTransfer(){
				ReceiveFileTransfer resume_file = null;
				Logger.d(TAG, "getReceiveTransfer 1");
				try{
               		 resume_file = mActiveList.get(0);
				}
				catch (Exception e) {
					Logger.d(TAG, "getReceiveTransfer exception");
				}
				return resume_file;
			}

            /**
             * Handle a new file transfer invitation
             */
            public synchronized void removeReceiveFileTransfer(ReceiveFileTransfer receiveFileTransfer) {
                Logger.d(TAG, "removeReceiveFileTransfer() entry, receiveFileTransfer = "
                        + receiveFileTransfer);
                if (null != receiveFileTransfer) {
                    mActiveList.remove(receiveFileTransfer);
                    Logger.d(TAG,
                            "removeReceiveFileTransfer() the file transfer with receiveFileTransfer: "
                                    + receiveFileTransfer);
                }

            }

            /**
             * Cancel all the receive file transfers
             */
            public synchronized void cancelReceiveFileTransfer() {
                Logger.d(TAG, "cancelReceiveFileTransfer entry");
                ArrayList<ReceiveFileTransfer> tempList =
                        new ArrayList<ReceiveFileTransfer>(mActiveList);
                int size = tempList.size();
                for (int i = 0; i < size; i++) {
                    ReceiveFileTransfer receiveFileTransfer = tempList.get(i);
                    if (null != receiveFileTransfer) {
                        receiveFileTransfer.cancelFileTransfer();
                    }
                }
            }

            /**
             * Search an existing file transfer in the receive list
             */
            public synchronized ReceiveFileTransfer findFileTransferByTag(Object targetTag) {
                if (null != mActiveList && null != targetTag) {
                    for (ReceiveFileTransfer receiveFileTransfer : mActiveList) {
                        Object fileTransferTag = receiveFileTransfer.mFileTransferTag;
                        if (targetTag.equals(fileTransferTag)) {
                            Logger.d(TAG, "findFileTransferByTag() the file transfer with targetTag "
                                    + targetTag + " found");
                            return receiveFileTransfer;
                        }
                    }
                    Logger.d(TAG, "findFileTransferByTag() not found targetTag " + targetTag);
                    return null;
                } else {
                    Logger.e(TAG, "findFileTransferByTag(), targetTag is " + targetTag);
                    return null;
                }
            }
        }
        protected ReceiveFileTransferManager mReceiveFileTransferManager =
                new ReceiveFileTransferManager();
        /**
         * This class describe one single in-coming file transfer, and control the
         * status itself
         */
        protected class ReceiveFileTransfer {
            private static final String TAG = "M0CFF ReceiveFileTransfer";
            protected Object mFileTransferTag = UUID.randomUUID();

            protected FileStruct mFileStruct = null;

            protected IFileTransfer mFileTransfer = null;

            protected FileTransfer mFileTransferObject = null;

            protected FileTransferListener mFileTransferListener = null;

            public ReceiveFileTransfer(FileTransfer fileTransferObject, boolean isAutoAccept,boolean isGroup) {
                if (null != fileTransferObject) {
                    Logger.d(TAG, "ReceiveFileTransfer() constructor FileTransfer is "
                            + fileTransferObject);
                    handleFileTransferInvitation(fileTransferObject, isAutoAccept,isGroup);
                }
            }

            protected void handleFileTransferInvitation(FileTransfer fileTransferObject, boolean isAutoAccept,boolean isGroupChat) {
                Logger.d(TAG, "handleFileTransferInvitation() entry " + isAutoAccept);
                if (fileTransferObject == null) {
                    Logger.e(TAG, "handleFileTransferInvitation, fileTransferSession is null!");
                    return;
                }
                try {
                    mFileTransferObject = fileTransferObject;
                    mFileTransferTag = mFileTransferObject.getTransferId();
                    mFileTransferListener = new FileTransferReceiverListener();
                    mFileTransferObject.addEventListener(mFileTransferListener);
                    mFileStruct = FileStruct.from(mFileTransferObject, null);
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
				catch (RemoteException e) {
                    e.printStackTrace();
                }

                if (mFileStruct != null) {
					if(isGroupChat){
						 mFileTransfer = ((IGroupChatWindow) mChatWindow)
                            .addReceivedFileTransfer(mFileStruct,isAutoAccept, !mIsInBackground);
                    	if(isAutoAccept){
                    	fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING;
						}
						else{
							fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;
						}
					}
					else
					{
	                    mFileTransfer = ((IOne2OneChatWindow) mChatWindow)
	                            .addReceivedFileTransfer(mFileStruct,isAutoAccept);
						if(isAutoAccept){
	                    	fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING;
						}
						else{
							fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;
						}
					}
                    
                }
            }


            protected void acceptFileTransferInvitation() {
                if (mFileTransferObject != null) {
                    try {
                        // Received file size in byte
                        long receivedFileSize = mFileTransferObject.getFileSize();
                        long currentStorageSize = Utils.getFreeStorageSize();
                        Logger.d(TAG, "receivedFileSize = " + receivedFileSize
                                + "/currentStorageSize = " + currentStorageSize);
                        if (currentStorageSize > 0) {
                            if (receivedFileSize <= currentStorageSize) {
                                mFileTransferObject.acceptInvitation();
                                mFileTransfer
                                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING);
                                fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING;
                            } else {
                                mFileTransferObject.rejectInvitation();
                                mFileTransfer
                                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED);
                                fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED;
                                new Handler(Looper.getMainLooper()).post(new Runnable() {
                                    @Override
                                    public void run() {
                                        Context context = ApiManager.getInstance().getContext();
                                        String strToast = context
                                                .getString(R.string.rcse_no_enough_storage_for_file_transfer);
                                        Toast.makeText(context, strToast, Toast.LENGTH_LONG).show();
                                    }
                                });
                            }
                        } else {
                            mFileTransferObject.rejectInvitation();
                            mFileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED);
                            new Handler(Looper.getMainLooper()).post(new Runnable() {
                                @Override
                                public void run() {
                                    Context context = ApiManager.getInstance().getContext();
                                    String strToast = context
                                            .getString(R.string.rcse_no_external_storage_for_file_transfer);
                                    Toast.makeText(context, strToast, Toast.LENGTH_LONG).show();
                                }
                            });
                        }
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    } catch(Exception e) {
                    	 e.printStackTrace();
                    }
					
                } else {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                    Logger.d(TAG, "acceptFileTransferInvitation(), mFileTransferObject is null");
                }
            }

            protected void rejectFileTransferInvitation() {
                try {
                    if (mFileTransferObject != null) {
                        if (mFileTransferListener != null) {
                            mFileTransferObject.removeEventListener(mFileTransferListener);
                            mFileTransferListener = null;
                        } else {
                            Logger
                                    .w(TAG,
                                            "rejectFileTransferInvitation(), mFileTransferReceiverListener is null!");
                        }
                        mFileTransferObject.rejectInvitation();
                        if (mFileTransfer != null) {
                            mFileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED);
                            fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED;
                        }
                    } else {
                        mFileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                        fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED;
                        Logger.e(TAG, "rejectFileTransferInvitation(), mFileTransferObject is null!");
                    }
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
                mReceiveFileTransferManager.removeReceiveFileTransfer(this);
            }

            protected void cancelFileTransfer() {
                if (null != mFileTransferObject) {
                    try {
                        mFileTransferObject.abortTransfer();
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }
                onFileTransferCancel();
            }

			 protected void onPauseReceiveTransfer() {
			 	Logger.v(TAG,"onPauseReceiveTransfer 1");
                if (null != mFileTransferObject) {
                    try {
						Logger.v(TAG,"onPauseReceiveTransfer 1");
                        mFileTransferObject.pauseTransfer();
                    } catch (JoynServiceException e) {
                    	Logger.v(TAG,"onPauseReceiveTransfer exception" + e);
                        e.printStackTrace();
                    }
                }
                //onFileTransferCancel();
            }

			  protected void onResumeReceiveTransfer() {
			  	Logger.v(TAG,"onResumeReceiveTransfer");
                if (null != mFileTransferObject) {
                    try {
						Logger.v(TAG,"onResumeReceiveTransfer 1");
						//if(mFileTransferObject.isSessionPaused()){ //TODo check this
						//	Logger.v(TAG,"onResumeReceiveTransfer 2");
                        	mFileTransferObject.resumeTransfer();
						//}
                    } catch (JoynServiceException e) {
                    	Logger.v(TAG,"onResumeReceiveTransfer exception" + e);
                        e.printStackTrace();
                    }
                }
                //onFileTransferCancel();
            }

            protected void onFileTransferFailed(boolean dataSentCompleted) {
                Logger.v(TAG,
                        "onFileTransferFailed() entry : mFileTransferListener = "
                                + mFileTransferListener + "mFileTransfer = "
                                + mFileTransfer + ", mFileTransferSession = "
                                + mFileTransferObject);
                // notify file transfer canceled
                if (mFileTransfer != null) {
                    if(dataSentCompleted){
                        mFileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                    }else{
                        mFileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCELED);
                    }
                } 
                if (mFileTransferListener != null) {
                    if (mFileTransferObject != null) {
                        try {
                            mFileTransferObject.removeEventListener(mFileTransferListener);
                        } catch (JoynServiceException e) {
                            e.printStackTrace();
                        }
                    } 
                }

                mReceiveFileTransferManager.removeReceiveFileTransfer(this);
            }

            /**
             * A file transfer in progress was canceled by remote.
             */
            protected void onFileTransferCanceled() {
                Logger.v(TAG,
                        "canceledFileTransfer() entry : mFileTransferListener = "
                                + mFileTransferListener + "mFileTransfer = "
                                + mFileTransfer + ", mFileTransferObject = "
                                + mFileTransferObject);
                // notify file transfer canceled
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCELED);
                    fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCELED;
                } 
                if (mFileTransferListener != null) {
                    if (mFileTransferObject != null) {
                        try {
                            mFileTransferObject.removeEventListener(mFileTransferListener);
                        } catch (JoynServiceException e) {
                            e.printStackTrace();
                        }
                    } 
                }

                mReceiveFileTransferManager.removeReceiveFileTransfer(this);
            }

            protected void onFileTransferFailed() {
                Logger.v(TAG,
                        "TIMEOUT failedFileTransfer() entry : mFileTransferListener = "
                                + mFileTransferListener + "mFileTransfer = "
                                + mFileTransfer + ", mFileTransferObject = "
                                + mFileTransferObject);
                // notify file transfer failed on timeout
                if (mFileTransfer != null) {
                    if(!(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCELED))
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                } 
                if (mFileTransferListener != null) {
                    if (mFileTransferObject != null) {
                        try {
                            mFileTransferObject.removeEventListener(mFileTransferListener);
                        } catch (JoynServiceException e) {
                            e.printStackTrace();
                        }
                    } 
                }

                mReceiveFileTransferManager.removeReceiveFileTransfer(this);
            }
            /**
             * You cancel a file transfer.
             */
            protected void onFileTransferCancel() {
                Logger.v(TAG,
                        "canceledFileTransfer() entry : mFileTransferListener = "
                                + mFileTransferListener + ", mFileTransfer = "
                                + mFileTransfer + ", mFileTransferObject= "
                                + mFileTransferObject);
                // notify file transfer cancel
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCEL);
                }
                if (mFileTransferListener != null) {
                    if (mFileTransferObject != null) {
                        try {
                            mFileTransferObject.removeEventListener(mFileTransferListener);
                        } catch (JoynServiceException e) {
                            e.printStackTrace();
                        }
                    }
                }

                mReceiveFileTransferManager.removeReceiveFileTransfer(this);
            }

            /**
             * File transfer session event listener
             */
            private class FileTransferReceiverListener extends FileTransferListener {
                private static final String TAG = "M0CFF FileTransferReceiverListener";
                private long mTotalSize = 0;
                private long mCurrentSize = 0;

					/**
		 * Callback called when the file transfer is started
		 */
		public  void onTransferStarted(){
		    // notify that this chat have a file transfer
            Logger.v(TAG, "onTransferStarted() entry");
		}
		
		/**
		 * Callback called when the file transfer has been aborted
		 */
		public  void onTransferAborted(){
		 	Logger.v(TAG, "onTransferAborted, File transfer onTransferAborted");
            if (mFileTransfer != null) {
                checkCapabilities();
            }
            mFileTransferObject = null;
            onFileTransferFailed();
		}

		/**
		 * Callback called when the transfer has failed
		 * 
		 * @param error Error
		 * @see FileTransfer.Error
		 */
		public  void onTransferError(int error){
		    Logger.d(TAG, "onTransferError, error is :" + error);
            if (error == FileSharingError.SESSION_INITIATION_DECLINED) {
                onFileTransferCanceled();
                mFileTransferObject = null;
            } else if (error == FileSharingError.MEDIA_SAVING_FAILED) {
                mFileStruct.mFilePath = ReceivedFileTransferItemBinder.FILEPATH_NO_SPACE;
                onFileTransferCancel();
                mFileTransferObject = null;
            } else if (error == FileSharingError.UNEXPECTED_EXCEPTION) {
                onFileTransferCanceled();
                mFileTransferObject = null;
        	} else if(error == FileSharingError.SESSION_INITIATION_FAILED){
                boolean dataSentCompleted = true;
                if(mTotalSize == 0){
                    dataSentCompleted = false;
                } else{
                    dataSentCompleted = (mTotalSize == mCurrentSize ? true : false);
                }
                onFileTransferFailed(dataSentCompleted);
                mFileTransferObject = null;
            }
			else{
			 onFileTransferCanceled();
             mFileTransferObject = null;
			}
		}
		
		/**
		 * Callback called during the transfer progress
		 * 
		 * @param currentSize Current transferred size in bytes
		 * @param totalSize Total size to transfer in bytes
		 */
		public  void onTransferProgress(long currentSize, long totalSize){
		 	Logger.d(TAG, "handleTransferProgress() entry: currentSize = "
                            + currentSize + ", totalSize = " + totalSize
                            + ", mFileTransfer = " + mFileTransfer);
            if (mFileTransfer != null) {
                mFileTransfer.setProgress(currentSize);
            }
            mTotalSize = totalSize;
            mCurrentSize = currentSize;
			}

		/**
		 * Callback called when the file has been transferred
		 * 
		 * @param filename Filename including the path of the transferred file
		 */
		public  void onFileTransferred(String filename){
		  		Logger.d(TAG, "onFileTransferred, filename is :" + filename
                            + ", mFileTransfer = " + mFileTransfer + "tag: " + mFileStruct.mFileTransferTag);
				if(RichMessagingDataProvider.getInstance() == null){
					RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
				}
				RichMessagingDataProvider.getInstance().updateFileTransferUrl(mFileStruct.mFileTransferTag.toString() ,filename);
				RichMessagingDataProvider.getInstance().updateFileTransferStatus(mFileStruct.mFileTransferTag.toString() ,4);
                if (mFileTransfer != null) {
					try{
                    mFileTransfer.setFilePath(filename);
					}
					catch(Exception e){
						Logger.d(TAG, "onFileTransferred exception");
					}
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                    mFileTransfer = null;
                }
                ReceiveFileTransfer receiveFileTransfer =
                        mReceiveFileTransferManager.findFileTransferByTag(mFileTransferTag);
                if (null != receiveFileTransfer) {
                    mReceiveFileTransferManager.removeReceiveFileTransfer(receiveFileTransfer);
                }
			}
/*
                // Session is started
                public void handleSessionStarted() {
                    // notify that this chat have a file transfer
                    Logger.v(TAG, "handleSessionStarted() entry");
                }

    			// File transfer has been paused	
    			@Override
    			public void handleFileTransferPaused()
    			{
    			}

                // Session has been aborted
                public void handleSessionAborted(int reason) {
                    Logger.v(TAG, "handleSessionAborted, File transfer handleSessionAborted");
                    if (mFileTransfer != null) {
                        checkCapabilities();
                    }
                    mFileTransferObject = null;
                    onFileTransferFailed();
                }

                // Session has been terminated by remote
                public void handleSessionTerminatedByRemote() {
                    Logger
                            .v(TAG,
                                    "handleSessionTerminatedByRemote, File transfer handleSessionTerminatedByRemote");
                    if (mFileTransfer != null) {
                        checkCapabilities();
                    }
                    mFileTransferObject = null;
                    onFileTransferCanceled();
                }

                // File transfer progress
                public void handleTransferProgress(final long currentSize, final long totalSize) {
                    Logger.d(TAG, "handleTransferProgress() entry: currentSize = "
                            + currentSize + ", totalSize = " + totalSize
                            + ", mFileTransfer = " + mFileTransfer);
                    if (mFileTransfer != null) {
                        mFileTransfer.setProgress(currentSize);
                    }
                    mTotalSize = totalSize;
                    mCurrentSize = currentSize;
                }

    			public void handleFileUploaded(){
    				}

                // File transfer error
                public void handleTransferError(final int error) {
                    Logger.d(TAG, "handleTransferError, error is :" + error);
                    if (error == FileSharingError.SESSION_INITIATION_DECLINED) {
                        onFileTransferCanceled();
                        mFileTransferObject = null;
                    } else if (error == FileSharingError.MEDIA_SAVING_FAILED) {
                        mFileStruct.mFilePath = ReceivedFileTransferItemBinder.FILEPATH_NO_SPACE;
                        onFileTransferCancel();
                        mFileTransferObject = null;
                    } else if (error == FileSharingError.UNEXPECTED_EXCEPTION) {
                        onFileTransferCanceled();
                        mFileTransferObject = null;
                } else if(error == FileSharingError.SESSION_INITIATION_FAILED){
                        boolean dataSentCompleted = true;
                        if(mTotalSize == 0){
                            dataSentCompleted = false;
                        } else{
                            dataSentCompleted = (mTotalSize == mCurrentSize ? true : false);
                        }
                        onFileTransferFailed(dataSentCompleted);
                        mFileTransferObject = null;
                    }
				else{
					 onFileTransferCanceled();
                     mFileTransferObject = null;
					}
                }

                // File has been transfered
                public void handleFileTransfered(final String filename) {
                    Logger.d(TAG, "handleFileTransfered, filename is :" + filename
                            + ", mFileTransfer = " + mFileTransfer);
                    if (mFileTransfer != null) {
                        mFileTransfer.setFilePath(filename);
                        mFileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                        mFileTransfer = null;
                    }
                    ReceiveFileTransfer receiveFileTransfer =
                            mReceiveFileTransferManager.findFileTransferByTag(mFileTransferTag);
                    if (null != receiveFileTransfer) {
                        mReceiveFileTransferManager.removeReceiveFileTransfer(receiveFileTransfer);
                    }
                }
                
                public void handleTransferTerminated(){
                    
                }
    			public void handleFileTransferResumed(){
    				}
    				 */
            }

        }
        
        /**
         * Clear all the messages in chat Window and the latest message in chat
         * list.
         */
        public void clearChatWindowAndList() {
            Logger.d(TAG, "clearChatWindowAndList() entry");
            mChatWindow.removeAllMessages();
            Logger.d(TAG, "clearChatWindowAndList() exit");
        }

        /**
         * Set chat window for this chat.
         * 
         * @param chatWindow The chat window to be set.
         */
        public void setChatWindow(IChatWindow chatWindow) {
            Logger.d(TAG, "setChatWindow entry");
            mChatWindow = chatWindow;
        }

        /**
         * Add the unread message of this chat
         * 
         * @param message The unread message to add
         */
        protected void addUnreadMessage(ChatMessage message) {
            if (message.isDisplayedReportRequested()) {
                Logger.d(TAG, "mReceivedInBackgroundToBeDisplayed = "
                        + mReceivedInBackgroundToBeDisplayed);
                if (mReceivedInBackgroundToBeDisplayed != null) {
                    mReceivedInBackgroundToBeDisplayed.add(message);
                    if (mTag instanceof UUID) {
                        ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                        UnreadMessagesContainer.getInstance().add(parcelUuid);
                    } else {
                        UnreadMessagesContainer.getInstance().add((ParcelUuid) mTag);
                    }
                    UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
                }
            } else {
                Logger.d(TAG, "mReceivedInBackgroundToBeRead = "
                        + mReceivedInBackgroundToBeRead);
                if (mReceivedInBackgroundToBeRead != null) {
                    mReceivedInBackgroundToBeRead.add(message);
                    if (mTag instanceof UUID) {
                        ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                        UnreadMessagesContainer.getInstance().add(parcelUuid);
                    } else {
                        UnreadMessagesContainer.getInstance().add(
                                (ParcelUuid) mTag);
                    }
                    UnreadMessagesContainer.getInstance()
                            .loadLatestUnreadMessage();
                }
            }
        }


		 /**
         * Add the unread message of this chat
         * 
         * @param message The unread message to add
         */
        protected void addUnreadFt(FileTransfer fileObject) {
            /*if (fileObject.isDisplayedReportRequested()) {
                Logger.d(TAG, "mReceivedInBackgroundFtToBeDisplayed = "
                        + mReceivedInBackgroundFtToBeDisplayed);
                if (mReceivedInBackgroundFtToBeDisplayed != null) {
                    mReceivedInBackgroundFtToBeDisplayed.add(message);
                    if (mTag instanceof UUID) {
                        ParcelUuid parcelUuid = new ParcelUuid((UUID) mTag);
                        UnreadMessagesContainer.getInstance().add(parcelUuid);
                    } else {
                        UnreadMessagesContainer.getInstance().add((ParcelUuid) mTag);
                    }
                    UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
                }
            } else {*/
                Logger.d(TAG, "mReceivedInBackgroundFtToBeRead = "
                        + mReceivedInBackgroundFtToBeRead);
                if (mReceivedInBackgroundFtToBeRead != null) {
                    mReceivedInBackgroundFtToBeRead.add(fileObject);   
                }
            //}
        }

		 /**
         * Get unread messages of this chat.
         * 
         * @return The unread messages.
         */
        public List<FileTransfer> getUnreadFt() {
            if (mReceivedInBackgroundFtToBeDisplayed != null
                    && mReceivedInBackgroundFtToBeDisplayed.size() > 0) {
                return mReceivedInBackgroundFtToBeDisplayed;
            } else {
                return mReceivedInBackgroundFtToBeRead;
            }
        }

        /**
         * Clear all the unread message of this chat
         */
        protected void clearUnreadFt() {
            Logger.v(TAG,
                    "clearUnreadFt(): mReceivedInBackgroundFtToBeDisplayed = "
                            + mReceivedInBackgroundFtToBeDisplayed
                            + ", mReceivedInBackgroundFtToBeRead = "
                            + mReceivedInBackgroundFtToBeRead);
            if (null != mReceivedInBackgroundFtToBeDisplayed) {
                mReceivedInBackgroundFtToBeDisplayed.clear();
            } 
            if (null != mReceivedInBackgroundFtToBeRead) {
                mReceivedInBackgroundFtToBeRead.clear();
            }
        }

        /**
         * Get unread messages of this chat.
         * 
         * @return The unread messages.
         */
        public List<ChatMessage> getUnreadMessages() {
            if (mReceivedInBackgroundToBeDisplayed != null
                    && mReceivedInBackgroundToBeDisplayed.size() > 0) {
                return mReceivedInBackgroundToBeDisplayed;
            } else {
                return mReceivedInBackgroundToBeRead;
            }
        }

        /**
         * Clear all the unread message of this chat
         */
        protected void clearUnreadMessage() {
            Logger.v(TAG,
                    "clearUnreadMessage(): mReceivedInBackgroundToBeDisplayed = "
                            + mReceivedInBackgroundToBeDisplayed
                            + ", mReceivedInBackgroundToBeRead = "
                            + mReceivedInBackgroundToBeRead);
            if (null != mReceivedInBackgroundToBeDisplayed) {
                mReceivedInBackgroundToBeDisplayed.clear();
                UnreadMessagesContainer.getInstance().remove((ParcelUuid) mTag);
                UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
            } 
            if (null != mReceivedInBackgroundToBeRead) {
                mReceivedInBackgroundToBeRead.clear();
                UnreadMessagesContainer.getInstance().remove((ParcelUuid) mTag);
                UnreadMessagesContainer.getInstance().loadLatestUnreadMessage();
            }
        }

        protected ChatImpl(Object tag) {
            mTag = tag;
            // register IRegistrationStatusListener
            ApiManager apiManager = ApiManager.getInstance();
            Logger.v(TAG, "ChatImpl() entry: apiManager = " + apiManager);
            if (null != apiManager) {
                mRegistrationApi = ApiManager.getInstance().getRegistrationApi();
                Logger.v(TAG, "mRegistrationApi = " + mRegistrationApi);
                if (mRegistrationApi != null) {
                    mRegistrationApi.addRegistrationStatusListener(this);
                }
                CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                Logger.v(TAG, "capabilityApi = " + capabilityApi);
                if (capabilityApi != null) {
					try{
                    	capabilityApi.addCapabilitiesListener(this);
					}
					 catch(JoynServiceException e){
						Logger.d(TAG, "ChatImpl() getContactCapabilities JoynServiceException");
					 }					 
                }
            }
        }

        /**
         * Get the chat tag of current chat
         * 
         * @return The chat tag of current chat
         */
        public Object getChatTag() {
            return mTag;
        }

        protected void onPause() {
            Logger.v(TAG, "onPause() entry, tag: " + mTag);
            mIsInBackground = true;
        }

        protected void onResume() {
            Logger.v(TAG, "onResume() entry, tag: " + mTag);
            mIsInBackground = false;
            if (mChatWindow != null) {
                mChatWindow.updateAllMsgAsRead();
                if(mParticipant != null)
                {
                    mChatWindow.updateAllMsgAsReadForContact(mParticipant);
                }
            }
            markUnreadMessageDisplayed();
            markUnreadFtDisplayed();
            markReloadDisplayed();
            //loadChatMessages(One2OneChat.LOAD_ZERO_SHOW_HEADER);
        }

        protected synchronized void onDestroy() {
            this.queryCapabilities();
            ApiManager apiManager = ApiManager.getInstance();
            Logger.v(TAG, "onDestroy() apiManager = " + apiManager);
            if (null != apiManager) {
                CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                Logger.v(TAG, "onDestroy() capabilityApi = " + capabilityApi);
                if (capabilityApi != null) {
					try{
                    	capabilityApi.removeCapabilitiesListener(this);
					}
					catch(JoynServiceException e){
						Logger.d(TAG, "addChat() onDestroy JoynServiceException");
					 }
                }
            } 
            Logger.v(TAG, "onDestroy() mRegistrationApi = " + mRegistrationApi
                    + ",mCurrentGroupChatImpl = " + mCurrentGroupChatImpl.get());
            if (mRegistrationApi != null) {
                mRegistrationApi.removeRegistrationStatusListener(this);
            }
            if (mCurrentGroupChatImpl.get() != null) {
                try {
                    terminateGroupChatImpl();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        /**
         * Return the IChatWindow
         * 
         * @return IChatWindow
         */
        IChatWindow getChatWindow() {
            return mChatWindow;
        }

        protected void markMessageAsDisplayed(ChatMessage msg) {
            Logger.d(TAG, "markMessageAsDisplayed() entry");
            if (msg == null) {
                Logger.d(TAG, "markMessageAsDisplayed(),msg is null");
                return;
            }
            try {
                GroupChat tmpChatImpl = mCurrentGroupChatImpl.get();
				// ChatService messagingApi = ApiManager.getInstance().getChatApi();
                SharedPreferences settings =
                        PreferenceManager.getDefaultSharedPreferences(ApiManager.getInstance()
                                .getContext());
                boolean isSendReadReceiptChecked =
                        settings.getBoolean(SettingsFragment.RCS_SEND_READ_RECEIPT, true);
                Logger.d(TAG, "markMessageAsDisplayed() ,the value of isSendReadReceiptChecked is "
                        + isSendReadReceiptChecked);
				
				 if (ApiManager.getInstance() != null) {
                //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
	                if(RichMessagingDataProvider.getInstance() == null){
	        			RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
	        		}	
	                RichMessagingDataProvider.getInstance().markChatMessageAsRead(msg.getId(), true);
	            }
				 
                if (tmpChatImpl == null) {
                    Logger.d(TAG, "markMessageAsDisplayed() ,tmpChatSession is null");
                    return;
                }
                Logger.d(TAG,
                        "markMessageAsDisplayed() ,the value of isSendReadReceiptChecked is "
                                + isSendReadReceiptChecked);
                if (isSendReadReceiptChecked) {
                        Logger.v(TAG,
                                "markMessageAsDisplayed(),send displayed message by msrp message");
                    tmpChatImpl.sendDisplayedDeliveryReport(msg.getId());
                }
            } catch (JoynServiceException e) {
                e.printStackTrace();
            } catch(Exception e){
           		 e.printStackTrace();
            }
            Logger.d(TAG, "markMessageAsDisplayed() exit");
        }

        protected void markMessageAsRead(ChatMessage msg) {
            Logger.i(TAG, "markMessageAsRead() entry");
           // EventsLogApi events = null;
            if (ApiManager.getInstance() != null) {
                //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                if(RichMessagingDataProvider.getInstance() == null){
        			RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
        		}	
				try {
                 RichMessagingDataProvider.getInstance().markChatMessageAsRead(msg.getId(), true);
				} catch(UnsupportedOperationException e) {
				    e.printStackTrace();
            }
            }
            Logger.i(TAG, "markMessageAsRead() exit");
        }

		 protected void markFtAsRead(FileTransfer msg) {
            Logger.d(TAG, "markMessageAsRead() entry");
           // EventsLogApi events = null;
            if (ApiManager.getInstance() != null) {
                //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                if(RichMessagingDataProvider.getInstance() == null){
        			RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
        		}	
				try {
                    RichMessagingDataProvider.getInstance().markFTMessageAsRead(msg.getTransferId(), true);
				} catch(Exception e) {
					e.printStackTrace();
				}
            }
            Logger.d(TAG, "markMessageAsRead() exit");
        }

        protected void markUnreadMessageDisplayed() {
            Logger.v(TAG, "markUnreadMessageDisplayed() entry");
            int size = mReceivedInBackgroundToBeDisplayed.size();
            for (int i = 0; i < size; i++) {
                ChatMessage msg = mReceivedInBackgroundToBeDisplayed.get(i);
                markMessageAsDisplayed(msg);
                Logger.v(TAG, "The message " + msg.getMessage() + " is displayed");
            }
            size = mReceivedInBackgroundToBeRead.size();
            for (int i = 0; i < size; i++) {
                ChatMessage msg = mReceivedInBackgroundToBeRead.get(i);
                markMessageAsRead(msg);
                Logger.v(TAG, "The message " + msg.getMessage() + " is read");
            }
            clearUnreadMessage();
            Logger.v(TAG, "markUnreadMessageDisplayed() exit");
        }

		protected void markUnreadFtDisplayed() {
            Logger.v(TAG, "markUnreadFtDisplayed() entry");
            int size = mReceivedInBackgroundFtToBeDisplayed.size();
            for (int i = 0; i < size; i++) {
                //ChatMessage msg = mReceivedInBackgroundFtToBeDisplayed.get(i);
                //markMessageAsDisplayed(msg);
                //Logger.v(TAG, "The message " + msg.getMessage() + " is displayed");
            }
            size = mReceivedInBackgroundToBeRead.size();
            for (int i = 0; i < size; i++) {
                FileTransfer msg = mReceivedInBackgroundFtToBeRead.get(i);
                markFtAsRead(msg);
				try{
                    Logger.v(TAG, "The message " + msg.getTransferId() + " is read");
				} catch(Exception e) {
					e.printStackTrace();
				}
            }
            clearUnreadFt();
            Logger.v(TAG, "markUnreadFtDisplayed() exit");
        }
		
		protected void markReloadDisplayed() {
            Logger.v(TAG, "markReloadDisplayed() entry");
            Logger.v(TAG, "markReloadDisplayed() mReceivedAfterReloadMessage: " + mReceivedAfterReloadMessage);
            Logger.v(TAG, "markReloadDisplayed() mReceivedAfterReloadFt: " + mReceivedAfterReloadFt);
            int size = mReceivedAfterReloadMessage.size();
            size = mReceivedAfterReloadMessage.size();
            for (int i = 0; i < size; i++) {
                    if(RichMessagingDataProvider.getInstance() == null){
                        RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
                    }   
                    try {
                        RichMessagingDataProvider.getInstance().markChatMessageAsRead(mReceivedAfterReloadMessage.get(i), true);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                }
            
            size = mReceivedAfterReloadFt.size();
            for (int i = 0; i < size; i++) {
                if (ApiManager.getInstance() != null) {
                    //events = ApiManager.getInstance().getEventsLogApi(); //TODo check this
                    if(RichMessagingDataProvider.getInstance() == null){
                        RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
                    }   
                    try {
                        RichMessagingDataProvider.getInstance().markFTMessageAsRead(mReceivedAfterReloadFt.get(i), true);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                }
            }
            
            mReceivedAfterReloadFt.clear();
            mReceivedAfterReloadMessage.clear();
            Logger.v(TAG, "markUnreadFtDisplayed() exit");
        }

        private void terminateGroupChatImpl() throws RemoteException {
            if (mCurrentGroupChatImpl.get() != null) {
				try{
                	mCurrentGroupChatImpl.get().quitConversation();
				}
				catch(JoynServiceException e){
            	}
                mCurrentGroupChatImpl.set(null);
                Logger.i(TAG, "terminateGroupChatImpl()---mCurrentGroupChatImpl cancel and is null");
            }
        }

        protected void reloadMessage(ChatMessage message, int messageType, int status, int messageTag, String chatId) {
            Logger.w(TAG, "reloadMessage() sub-class needs to override this method");
        }

        protected void reloadFileTransfer(FileStruct fileStruct, int transferType, int status) {
            Logger.w(TAG, "M0CFF reloadFileTransfer() sub-class needs to override this method");
        }

        protected class ComposingManager {
            private static final int ACT_STATE_TIME_OUT = 60 * 1000;
            private boolean mIsComposing = false;
            private static final int STARTING_COMPOSING = 1;
            private static final int STILL_COMPOSING = 2;
            private static final int MESSAGE_WAS_SENT = 3;
            private static final int ACTIVE_MESSAGE_REFRESH = 4;
            private static final int IS_IDLE = 5;
            private ComposingHandler mWorkerHandler =
                    new ComposingHandler(CHAT_WORKER_THREAD.getLooper());

            public ComposingManager() {
            }

            protected class ComposingHandler extends Handler {
                public static final String TAG = "M0CF ComposingHandler";

                public ComposingHandler(Looper looper) {
                    super(looper);
                }

                @Override
                public void handleMessage(Message msg) {
                    Logger.w(TAG, "handleMessage() the msg is:" + msg.what);
                    switch (msg.what) {
                        case STARTING_COMPOSING: 
                            if (setIsComposing(true)) {
                                mIsComposing = true;
                                mWorkerHandler.sendEmptyMessageDelayed(IS_IDLE, sIdleTimeout);
                                mWorkerHandler.sendEmptyMessageDelayed(ACTIVE_MESSAGE_REFRESH,
                                        ACT_STATE_TIME_OUT);
                            } else {
                                Logger.d(TAG,
                                        "STARTING_COMPOSING-> failed to set isComposing to true");
                            }
                            break;
                        
                        case STILL_COMPOSING: 
                            mWorkerHandler.removeMessages(IS_IDLE);
                            mWorkerHandler.sendEmptyMessageDelayed(IS_IDLE, sIdleTimeout);
                            break;
                        
                        case MESSAGE_WAS_SENT: 
                           // if (setIsComposing(false)) {
                           if(true){
						   	 	Logger.w(TAG, "handleMessage()12 ");
                                mComposingManager.hasNoText();
                                mWorkerHandler.removeMessages(IS_IDLE);
                                mWorkerHandler.removeMessages(ACTIVE_MESSAGE_REFRESH);
                            } else {
                                Logger.d(TAG,
                                        "MESSAGE_WAS_SENT-> failed to set isComposing to false");
                            }
                            break;
                        
                        case ACTIVE_MESSAGE_REFRESH: 
                            if (setIsComposing(true)) {
                                mWorkerHandler.sendEmptyMessageDelayed(ACTIVE_MESSAGE_REFRESH,
                                        ACT_STATE_TIME_OUT);
                            } else {
                                Logger
                                        .d(TAG,
                                                "ACTIVE_MESSAGE_REFRESH-> failed to set isComposing to true");
                            }
                            break;
                        
                        case IS_IDLE: 
                            if (setIsComposing(false)) {
                                mComposingManager.hasNoText();
                                mWorkerHandler.removeMessages(ACTIVE_MESSAGE_REFRESH);
                            } else {
                                Logger.d(TAG, "IS_IDLE-> failed to set isComposing to false");
                            }
                            break;
                        
                        default: 
                            Logger.w(TAG, "handlemessage()--message" + msg.what);
                            break;
                        
                    }
                }
            }

            public void hasText(Boolean isEmpty) {
                Logger.w(TAG, "hasText() entry the edit is " + isEmpty);
                if (isEmpty) {
                    mWorkerHandler.sendEmptyMessage(MESSAGE_WAS_SENT);
                } else {
                    if (!mIsComposing) {
                        mWorkerHandler.sendEmptyMessage(STARTING_COMPOSING);
                    } else {
                        mWorkerHandler.sendEmptyMessage(STILL_COMPOSING);
                    }
                }
            }

            public void hasNoText() {
                mIsComposing = false;
            }

            public void messageWasSent() {
                mWorkerHandler.sendEmptyMessage(MESSAGE_WAS_SENT);
            }
        }

        protected boolean setIsComposing(boolean isComposing) {
            if (mCurrentGroupChatImpl == null) {
                Logger.e(TAG, "setIsComposing() -- The chat with the tag " + " doesn't exist!");
                return false;
            } else {
                try {
					 if (mCurrentGroupChatImpl.get() != null) {
                     	mCurrentGroupChatImpl.get().sendIsComposingEvent(isComposing);
					 }
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
                return true;
            }
        }

        @Override
        public IChatMessage getSentChatMessage(int index) {
            if (index < 0 || index > mMessageList.size()) {
                return null;
            }
            return mMessageList.get(index);
        }

        @Override
        public int getChatMessageCount() {
            return mMessageList.size();
        }

        @Override
        public List<IChatMessage> listAllChatMessages() {
            return mMessageList;
        }

        @Override
        public abstract void loadChatMessages(int count);

        @Override
        public boolean removeMessage(int index) {
            if (index < 0 || index > mMessageList.size()) {
                return false;
            }
            mMessageList.remove(index);
            return true;
        }

        @Override
        public boolean removeMessages(int start, int end) {
            if (start < 0 || start > mMessageList.size()) {
                return false;
            }
            return true;
        }

        /**
         * Check capabilities before inviting a chat
         */
        protected abstract void checkCapabilities();

        /**
         * Query capabilities after terminating a chat
         */
        protected abstract void queryCapabilities();

        @Override
        public void hasTextChanged(boolean isEmpty) {
            mComposingManager.hasText(isEmpty);
        }

        //public abstract void onCapabilityChanged(String contact, Capabilities capabilities);

		public abstract void onCapabilitiesReceived(String contact, Capabilities capabilities);
		
        @Override
        public abstract void onStatusChanged(boolean status);
    }

    private boolean isChatExisted(Participant participant) {
        Logger.v(TAG, "isHaveChat() The participant is " + participant);
        boolean bIsHaveChat = false;
        // Find the chat in the chat list
        Collection<IChat1> chatList = mChatMap.values();
        if (chatList != null) {
            for (IChat1 chat : chatList) {
                if (chat instanceof One2OneChat) {
                    if (null != participant && (((One2OneChat) chat).isDuplicated(participant))) {
                        bIsHaveChat = true;
                    }
                }
            }
        }
        Logger.i(TAG, "isHaveChat() end, bIsHaveChat = " + bIsHaveChat);
        return bIsHaveChat;
    }

    @Override
    public boolean handleInvitation(Intent intent, boolean isCheckSessionExist) {
        Logger.v(TAG, "handleInvitation entry");
        String action = null;
        if (intent == null) {
            Logger.d(TAG, "handleInvitation intent is null");
            return false;
        } else {
            action = intent.getAction();
        }
		/*
        if (MessagingApiIntents.CHAT_SESSION_REPLACED.equalsIgnoreCase(action)) {
            Logger.d(TAG, " handleInvitation() the action is CHAT_SESSION_REPLACED ");
        } else if (MessagingApiIntents.CHAT_INVITATION.equalsIgnoreCase(action)) {
            return handleChatInvitation(intent, isCheckSessionExist);
        }*/
		if (ChatIntent.ACTION_NEW_CHAT.equalsIgnoreCase(action)) {
            //return handleChatInvitation(intent, isCheckSessionExist); //TODo check this
        }
        Logger.v(TAG, "handleInvitation exit: action = " + action);
        return false;
    }

    public boolean handleO2OInvitation(Intent intent, boolean isCheckSessionExist) {
        Logger.v(TAG, "handleO2OInvitation entry");
        String action = null;
        if (intent == null) {
            Logger.d(TAG, "handleO2OInvitation intent is null");
            return false;
        } else {
            action = intent.getAction();
        }
		/*
        if (MessagingApiIntents.CHAT_SESSION_REPLACED.equalsIgnoreCase(action)) { //TODo check this
            Logger.d(TAG, " handleO2OInvitation() the action is CHAT_SESSION_REPLACED ");
        } else if (ChatIntent.ACTION_NEW_CHAT.equalsIgnoreCase(action)) {
            return handleO2OChatInvitation(intent, isCheckSessionExist);
        }*/

		if (ChatIntent.ACTION_NEW_CHAT.equalsIgnoreCase(action)) {
            return handleO2OChatInvitation(intent, isCheckSessionExist);
        }
        Logger.v(TAG, "handleO2OInvitation exit: action = " + action);
        return false;
    }

    public boolean handleNewGroupInvitation(Intent intent, boolean isCheckSessionExist) {
        Logger.v(TAG, "handleNewGroupInvitation entry");
        String action = null;
        if (intent == null) {
            Logger.d(TAG, "handleNewGroupInvitation intent is null");
            return false;
        } else {
            action = intent.getAction();
        }
		/*
        if (MessagingApiIntents.CHAT_SESSION_REPLACED.equalsIgnoreCase(action)) {  //TODo check this
            Logger.d(TAG, " handleNewGroupInvitation() the action is CHAT_SESSION_REPLACED ");
        } else if (GroupChatIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)) {
            return handleNewGroupChatInvitation(intent, isCheckSessionExist);
        }*/
		if (GroupChatIntent.ACTION_NEW_INVITATION.equalsIgnoreCase(action)) {
            return handleNewGroupChatInvitation(intent, isCheckSessionExist);
        }
        Logger.v(TAG, "handleNewGroupInvitation exit: action = " + action);
        return false;
    }

	private List<String> convertSetToList(Set<String> participantInfos) {
		// create an iterator
       Iterator iterator = participantInfos.iterator(); 
	   List<String> pList = new ArrayList<String>();
      
      // check values
        while (iterator.hasNext()){
            pList.add(iterator.next().toString());
        }
        return pList;
    }

	 public boolean handleNewGroupChatInvitation(Intent intent, boolean isCheckSessionExist) {
        ApiManager instance = ApiManager.getInstance();
        String sessionId = intent.getStringExtra("sessionId");
        boolean isAutoAccept = intent.getBooleanExtra(RcsNotification.AUTO_ACCEPT, false);
		String  contactNumber = intent.getStringExtra(GroupChatIntent.EXTRA_CONTACT);
        if (instance != null) {
            ChatService chatService = instance.getChatApi();
            if (chatService != null) {
                try {
					String groupChatId = intent.getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                    GroupChat groupChatImpl = chatService.getGroupChat(groupChatId);
                    if (groupChatImpl == null) {
                        Logger.e(TAG, "The groupChatImpl is null");
                        return false;
                    }
                    List<String> participants = convertSetToList(groupChatImpl.getAllParticipants());
                    if (participants == null || participants.size() == 0) {
                        Logger.e(TAG, "The getParticipants is null, or size is 0");
                        return false;
                    }
                    Logger.i(TAG, "handleNewGroupChatInvitation message contact: " + contactNumber);
                    int participantCount = participants.size();
					
                	Logger.w(TAG, "handleChatInvitation group");
					String groupSubject = intent.getStringExtra(GroupChatIntent.EXTRA_SUBJECT);
                    List<Participant> participantsList = new ArrayList<Participant>();
                    for (int i = 0; i < participantCount; i++) {
                        String remoteParticipant = participants.get(i).toString();
                        String number = PhoneUtils.extractNumberFromUri(remoteParticipant);
                        String name = number;
                        if (PhoneUtils.isANumber(number)) {
                            name = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(number);
                        } else {
                            Logger.e(TAG, "the participant " + number
                                            + " is not a real number");
                        }
                        Participant fromSessionParticipant = new Participant(number, name);
                        participantsList.add(fromSessionParticipant);
                    }
                    String chatId = intent.getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                    ParcelUuid tag = (ParcelUuid) intent
                            .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);//TODo ask RCS notification to add this tag
                    IChat1 chat = getGroupChat(chatId);
					Logger.w(TAG, "handleChatInvitation group ChatId : " + chatId);
                    if (chat == null) {
						Logger.w(TAG, "handleChatInvitation chat is null Tag: " + tag);
                        chat = addChat(participantsList, tag,chatId);
                       ((GroupChat1)chat).setmChatId(chatId);
                         /* add group name , get from Intent */
                       ((GroupChat1)chat).addGroupSubjectFromInvite(groupSubject);
                       ((GroupChat1)chat).setmInvite(true);
                    } else {
                        // restart chat.
                        Logger.w(TAG, "handleChatInvitation restatrt chat ");
                        ParcelUuid chatTag = (ParcelUuid) (((GroupChat1) chat).mTag);
                        Logger.d(TAG, "handleChatInvitation() restart chat tag: " + chatTag);
                        ChatScreenWindowContainer.getInstance().focus(chatTag);
                        ViewImpl.getInstance().switchChatWindowByTag(chatTag);
                    }
					Logger.w(TAG, "ADD TO chat "+isAutoAccept);
					chat.setInviteContact(contactNumber);
                    chat.handleInvitation(groupChatImpl, null, isAutoAccept);
                    return true; 
                } catch (JoynServiceNotAvailableException e) {
                    Logger.e(TAG, "getChatSession fail");
                    e.printStackTrace();
                } 
				catch (Exception e) {
	                Logger.e(TAG, "getChatSession fail");
	                e.printStackTrace();
           		} 
            }
        }
        return false;
    }

	public boolean handleO2OChatInvitation(Intent intent, boolean isCheckSessionExist) {
        ApiManager instance = ApiManager.getInstance();
        String sessionId = intent.getStringExtra("sessionId");
        boolean isAutoAccept = intent.getBooleanExtra(RcsNotification.AUTO_ACCEPT, false);
		String  contactNumber = intent.getStringExtra(ChatIntent.EXTRA_CONTACT);
        if (instance != null) {
            ChatService chatService = instance.getChatApi();
            if (chatService != null) {
                try {
                    Chat o2oChatImpl = chatService.getChat(contactNumber);
					 if (o2oChatImpl == null) {
						try {
							Set<Chat> totalChats = null;
							if(o2oChatImpl == null){
								totalChats = chatService.getChats();
								Logger.e(TAG, "aaa2 getChatSession size: " + totalChats.size());
								 Logger.d(TAG, "The2 chat session is null3 : " + totalChats.size());
							}
							for(Chat setElement: totalChats) {
						        if(setElement.getRemoteContact().equals(contactNumber)) {
									Logger.e(TAG, "Ge2t chat session finally");
						            //might work or might throw exception, Java calls it indefined behaviour:
						            o2oChatImpl = setElement;
									break;
						        } 
					    	}
						}catch (JoynServiceException e) {
								Logger.e(TAG, "Ge2t chat session xyz");
					            e.printStackTrace();
						}	
					 }
                    if (o2oChatImpl == null) {
                        Logger.e(TAG, "The getChatSession is null");
                        return false;
                    }                 
                    Logger.v(TAG, "handleChatInvitation message");
                    ArrayList<ChatMessage> messages =
                            intent.getParcelableArrayListExtra(ChatIntent.EXTRA_MESSAGE);
                    ArrayList<IChatMessage> chatMessages = new ArrayList<IChatMessage>();
                    if (null != messages) {
                        int size = messages.size();
                        for (int i = 0; i < size; i++) {
                            ChatMessage msg = messages.get(i);
							if(msg != null){
                            	Logger.w(TAG, "ChatMessage:" + "msg" + msg + "string" + msg.getMessage());	
							}
							else{
								Logger.w(TAG, "ChatMessage: is NULL");	
							}
							
                            if (msg != null) {
                                chatMessages.add(new ChatMessageReceived(msg));
                            }
                        }
                    }
						Logger.v(TAG, "handleChatInvitation O2O");
                        List<Participant> participantsList = new ArrayList<Participant>();
                        String name = intent.getStringExtra(ChatIntent.EXTRA_DISPLAY_NAME);
                        Participant fromSessionParticipant = new Participant(contactNumber, name);
                        participantsList.add(fromSessionParticipant);
                        if (!isCheckSessionExist) {
                            IChat1 currentChat = addChat(participantsList);
                            currentChat.handleInvitation(o2oChatImpl, chatMessages, isAutoAccept);
                            return true;
                        } else {
                            if (isChatExisted(fromSessionParticipant)) {
                                IChat1 currentChat = addChat(participantsList);
                                currentChat.handleInvitation(o2oChatImpl, chatMessages, isAutoAccept);
                                Logger.v(TAG, "handleInvitation exit with true");
                                return true;
                            } else {
                                Logger.v(TAG, "handleInvitation exit with false");
                                return false;
                            }
                        }
                } catch (JoynServiceNotAvailableException e) {
                    Logger.e(TAG, "handleO2OChatInvitation fail1");
                    e.printStackTrace();
                }
				catch (Exception e) {
                    Logger.e(TAG, "handleO2OChatInvitation fail1");
                    e.printStackTrace();
                }
            }
        }
        return false;
    }
/*
    private boolean handleChatInvitation(Intent intent, boolean isCheckSessionExist) {
        ApiManager instance = ApiManager.getInstance();
        String sessionId = intent.getStringExtra("sessionId");
        boolean isAutoAccept = intent.getBooleanExtra(RcsNotification.AUTO_ACCEPT, false);
        if (instance != null) {
            MessagingApi messageApi = instance.getChatApi();
            if (messageApi != null) {
                try {
                    IChatSession chatSession = messageApi.getChatSession(sessionId);
                    if (chatSession == null) {
                        Logger.e(TAG, "The getChatSession is null");
                        return false;
                    }
                    List<String> participants = chatSession.getInivtedParticipants();
                    if (participants == null || participants.size() == 0) {
                        Logger.e(TAG, "The getParticipants is null, or size is 0");
                        return false;
                    }
                    Logger.v(TAG, "handleChatInvitation message");
                    int participantCount = participants.size();
                    ArrayList<ChatMessage> messages =
                            intent.getParcelableArrayListExtra(INTENT_MESSAGE);
                    ArrayList<IChatMessage> chatMessages = new ArrayList<IChatMessage>();
                    if (null != messages) {
                        int size = messages.size();
                        for (int i = 0; i < size; i++) {
                            ChatMessage msg = messages.get(i);
							if(msg != null){
                            	Logger.w(TAG, "ChatMessage:" + "msg" + msg + "string" + msg.getMessage());	
							}
							else{
								Logger.w(TAG, "ChatMessage: is NULL");	
							}
							
                            if (msg != null) {
                                chatMessages.add(new ChatMessageReceived(msg));
                            }
                        }
                    }
                    if (participantCount == 1) {
						Logger.v(TAG, "handleChatInvitation O2O");
                        List<Participant> participantsList = new ArrayList<Participant>();
                        String remoteParticipant = participants.get(0);
                        String number = PhoneUtils.extractNumberFromUri(remoteParticipant);
                        String name = intent.getStringExtra(CONTACT_NAME);
                        Participant fromSessionParticipant = new Participant(number, name);
                        participantsList.add(fromSessionParticipant);
                        if (!isCheckSessionExist) {
                            IChat1 currentChat = addChat(participantsList);
                            currentChat.handleInvitation(chatSession, chatMessages, isAutoAccept);
                            return true;
                        } else {
                            if (isChatExisted(fromSessionParticipant)) {
                                IChat1 currentChat = addChat(participantsList);
                                currentChat.handleInvitation(chatSession, chatMessages, isAutoAccept);
                                Logger.v(TAG, "handleInvitation exit with true");
                                return true;
                            } else {
                                Logger.v(TAG, "handleInvitation exit with false");
                                return false;
                            }
                        }
                    } else if (participantCount > 1) {
                    	Logger.w(TAG, "handleChatInvitation group");
						String groupSubject = intent.getStringExtra("subject");
                        List<Participant> participantsList = new ArrayList<Participant>();
                        for (int i = 0; i < participantCount; i++) {
                            String remoteParticipant = participants.get(i);
                            String number = PhoneUtils.extractNumberFromUri(remoteParticipant);
                            String name = number;
                            if (PhoneUtils.isANumber(number)) {
                                name = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(number);
                            } else {
                                Logger
                                        .e(TAG, "the participant " + number
                                                + " is not a real number");
                            }
                            Participant fromSessionParticipant = new Participant(number, name);
                            participantsList.add(fromSessionParticipant);
                        }
                        String chatId = intent.getStringExtra(RcsNotification.CHAT_ID);
                        ParcelUuid tag = (ParcelUuid) intent
                                .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);
                        IChat1 chat = getGroupChat(chatId);
                        if (chat == null) {
							Logger.w(TAG, "handleChatInvitation chat is null");
                            chat = addChat(participantsList, tag,null);
                           ((GroupChat1)chat).setmChatId(chatId);
                             /* add group name , get from Intent */
                          /* ((GroupChat1)chat).addGroupSubjectFromInvite(groupSubject);
                           ((GroupChat1)chat).setmInvite(true);
                        } else {
                            // restart chat.
                            Logger.w(TAG, "handleChatInvitation restatrt chat ");
                            ParcelUuid chatTag = (ParcelUuid) (((GroupChat1) chat).mTag);
                            Logger.d(TAG, "handleChatInvitation() restart chat tag: " + chatTag);
                            ChatScreenWindowContainer.getInstance().focus(chatTag);
                            ViewImpl.getInstance().switchChatWindowByTag(chatTag);
                            chatMessages.clear();
                        }
						Logger.w(TAG, "ADD TO chat "+isAutoAccept);
                        chat.handleInvitation(chatSession, chatMessages, isAutoAccept);
                        return true;
                    } else {
                        Logger.e(TAG, "Illegal paticipants");
                        return false;
                    }
                } catch (ClientApiException e) {
                    Logger.e(TAG, "getChatSession fail");
                    e.printStackTrace();
                } catch (RemoteException e) {
                    Logger.e(TAG, "getParticipants fail");
                    e.printStackTrace();
                }
            }
        }
        return false;
    }
*/
    public boolean handleFileTransferInvitation(String fileTransferId, boolean isAutoAccept) {
		return true;
	}

    public boolean handleFileTransferInvitation(String fileTransferId, boolean isAutoAccept,boolean isGroup,String chatSessionId,String chatId) {
		Logger.w(TAG, "M0CFF handleFileTransferInvitation isGroup" + isGroup + "isAutoAccept" + isAutoAccept);
		Logger.e(TAG, "M0CFF handleFileTransferInvitation chatSessionId" + chatSessionId + "sessionId" + fileTransferId +"chatid" + chatId);
        ApiManager instance = ApiManager.getInstance();
        if (instance != null) {
            FileTransferService fileTransferService = instance.getFileTransferApi();
            if (fileTransferService != null) {
                try {
                    FileTransfer fileTransferObject = fileTransferService.getFileTransfer(fileTransferId);
                    if (fileTransferObject == null) {
                        Logger.w(TAG,"M0CFF handleFileTransferInvitation-The getFileTransferSession is null");
                        return false;
                    }

                    List<Participant> participantsList = new ArrayList<Participant>();
                    String number = PhoneUtils.extractNumberFromUri(fileTransferObject.getRemoteContact());

                    // Get the contact name from contact list
                    String name = EMPTY_STRING;
                    Logger.e(TAG, "M0CFF handleFileTransferInvitation, number = " + name);
                    if (null != number) {
                        String tmpContact = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(number);
                        if (tmpContact != null) {
                            name = tmpContact;
                        } else {
                            name = number;
                        }
                    }

                    Participant fromSessionParticipant = new Participant(number, name);
                    participantsList.add(fromSessionParticipant);
					if(isGroup){
						IChat1 chat = getGroupChat(chatId);
						if(null != chat){
						((GroupChat1) chat).addReceiveFileTransfer(fileTransferObject, isAutoAccept,true);
						if (!((GroupChat1) chat).mIsInBackground) {
							Logger.v(TAG, "M0CFF handleFileTransferInvitation() group" +
                                     "-handleInvitation exit with true, is the current chat!");
							return true;
						}
						else{
							Logger.v(TAG, "M0CFF handleFileTransferInvitation() group" +
                                     "-handleInvitation exit with false, is not the current chat!");
							return false;
						}
					  }
					}
                    else {
                    if (isChatExisted(fromSessionParticipant)) {
                        IChat1 chat = addChat(participantsList);
						if(chat != null){
                        ((One2OneChat) chat).addReceiveFileTransfer(fileTransferObject, isAutoAccept,false);
                        if (!((One2OneChat) chat).mIsInBackground) {
                            Logger.v(TAG, "M0CFF handleFileTransferInvitation()" +
                                     "-handleInvitation exit with true, is the current chat!");
                            return true;
                        } else {
                            Logger.v(TAG, "M0CFF handleFileTransferInvitation()" +
                                     "-handleInvitation exit with true, is not the current chat!");
                            return false;
                        }
					  }
                    } else {
                        Logger.v(TAG,
                                "M0CFF handleFileTransferInvitation-handleInvitation exit with false");
                        IChat1 chat = addChat(participantsList);
                        ((One2OneChat) chat).addReceiveFileTransfer(fileTransferObject, isAutoAccept,false);
                        return false;
                    }
                   }
                } catch (JoynServiceException e) {
                    Logger.e(TAG, "M0CFF handleFileTransferInvitation-getChatSession fail");
                    e.printStackTrace();
                } catch (Exception e) {
                    Logger.e(TAG, "M0CFF handleFileTransferInvitation-getParticipants fail");
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    @Override
    public void handleMessageDeliveryStatus(String contact, String msgId, String status,
            long timeStamp) {
        ContentResolver contentResolver = AndroidFactory.getApplicationContext()
                .getContentResolver();
        String[] selectionArg = {
            msgId.toString()
        };
        Cursor cursor = contentResolver.query(ChatLog.Message.CONTENT_URI, null,
                ChatLog.Message.MESSAGE_ID + "=?", selectionArg, null);
        String remoteContact = null;
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                remoteContact = cursor.getString(cursor
                        .getColumnIndex(ChatLog.Message.CONTACT_NUMBER));
            }
            cursor.close();
        }
		
        if (remoteContact == null) {
            Logger.i(TAG, "handleMessageDeliveryStatus() exit by null remote contact");
            return;
        }
		
        Logger.i(TAG, "handleMessageDeliveryStatus() entry, remoteContact:" + remoteContact +"Msgid:" + msgId);
        remoteContact = PhoneUtils.extractNumberFromUri(remoteContact);
        IChat1 chat = null;
		
        	 chat = getOne2OneChat(new Participant(remoteContact, remoteContact));
        if (null != chat) {
		 	Logger.i(TAG, "handleMessageDeliveryStatus() chat found");
        	 if (chat instanceof One2OneChat) {
            ((One2OneChat) chat).onMessageDelivered(msgId, status, timeStamp);
        }
        	 }
        	
    	//TODO fix me
    	 chat = getGroupChat(null);
		 if (null != chat) {
        	 if (chat instanceof GroupChat1) {
            ((GroupChat1) chat).onMessageDelivered(msgId, status, timeStamp);
        	}      	   
        }
    }

    /**
     * Called by the controller when the user needs to cancel an on-going file
     * transfer
     * 
     * @param tag The chat window tag where the file transfer is in
     * @param fileTransferTag The tag indicating which file transfer will be
     *            canceled
     */
    public void handleCancelFileTransfer(Object tag, Object fileTransferTag) {
        Logger.v(TAG, "M0CFF handleCancelFileTransfer(),tag = " + tag + ",fileTransferTag = "
                + fileTransferTag);
        IChat1 chat = getChat(tag);
        if (chat == null) {
            chat = getOne2oneChatByContact((String) tag);
        }
        if (chat instanceof One2OneChat) {
            One2OneChat oneOneChat = ((One2OneChat) chat);
            oneOneChat.handleCancelFileTransfer(fileTransferTag);
        }
        Logger.d(TAG, "M0CFF handleCancelFileTransfer() it's a sent file transfer");
        mOutGoingFileTransferManager.cancelFileTransfer(fileTransferTag);
    }

    /**
     * Called by the controller when the user needs to resend a file
     * 
     * @param fileTransferTag The tag of file which the user needs to resend
     */
    public void handleResendFileTransfer(Object fileTransferTag) {
    	Logger.v(TAG, "M0CFF handleResendFileTransfer() TAG: " + fileTransferTag);
        mOutGoingFileTransferManager.resendFileTransfer(fileTransferTag);
    }

    /**
     * Called by the controller when the user needs to resend a file
     * 
     * @param fileTransferTag The tag of file which the user needs to resend
     */
    public void handlePauseFileTransfer(Object chatWindowTag ,Object fileTransferTag,int option) {
         Logger.v(TAG, "M0CFF handlePauseFileTransfer(),tag = " + chatWindowTag + ",fileTransferTag = "+ fileTransferTag + "option:" + option);
        if(option == 0)
        	mOutGoingFileTransferManager.pauseFileTransfer(fileTransferTag);
		else{
            IChat1 chat = getChat(chatWindowTag);
			if (chat instanceof One2OneChat) {
				Logger.v(TAG, "M0CFF handlePauseFileTransfer() One2OneChat");
	            One2OneChat oneOneChat = ((One2OneChat) chat);
	            oneOneChat.handlePauseReceiveFileTransfer(fileTransferTag);
        	}
			else if(chat instanceof GroupChat1) {
				Logger.v(TAG, "M0CFF handlePauseFileTransfer() GroupChat1");
	            GroupChat1 goupChat = ((GroupChat1) chat);
	            goupChat.handlePauseReceiveFileTransfer(fileTransferTag);
        	}
		}
    }

	/**
     * Called by the controller when the user needs to resend a file
     * 
     * @param fileTransferTag The tag of file which the user needs to resend
     */
    public void handleResumeFileTransfer(Object chatWindowTag ,Object fileTransferTag,int option) {
    	Logger.v(TAG, "M0CFF handleResumeFileTransfer() TAG: " + fileTransferTag + "OPTION: " + option);
    	if(option == 0)
        	mOutGoingFileTransferManager.resumeFileTransfer(fileTransferTag);
		else{
            IChat1 chat = getChat(chatWindowTag);
			if (chat instanceof One2OneChat) {
	            One2OneChat oneOneChat = ((One2OneChat) chat);
	            oneOneChat.handleResumeReceiveFileTransfer(fileTransferTag);
        	}
			else if(chat instanceof GroupChat1) {
	            GroupChat1 goupChat = ((GroupChat1) chat);
	            goupChat.handleResumeReceiveFileTransfer(fileTransferTag);
        	}
		}
    }

    /**
     * This class represents a file structure used to be shared
     */
    public static class FileStruct {
        public static final String TAG = "M0CFF FileStruct";

        /**
         * Generate a file struct instance using a session and a path, this
         * method should only be called for Received File Transfer
         * 
         * @param fileTransferSession The session of the file transfer
         * @param filePath The path of the file
         * @return The file struct instance
         * @throws RemoteException
         */
        public static FileStruct from(FileTransfer fileTransferObject, String filePath)
                throws RemoteException {
            FileStruct fileStruct = null;
			String fileName = null;
            try{
	            fileName = fileTransferObject.getFileName();			
	            long fileSize = fileTransferObject.getFileSize();
	            String fileTransferId = fileTransferObject.getTransferId();
	            Date date = new Date();
				if(!RcsSettings.getInstance().isFileTransferThumbnailSupported()){
					Logger.v(TAG, "Filestruct thumbnail not supported fileContact"+fileTransferObject.getRemoteContact());			
		            fileStruct = new FileStruct(filePath, fileName, fileSize, fileTransferId, date, fileTransferObject.getRemoteContact());
				}
				else{
					Logger.v(TAG, "FileStruct thumbnail supported" + fileTransferObject.getFileIconName());
	                fileStruct = new FileStruct(filePath, fileName, fileSize, fileTransferId, date,fileTransferObject.getRemoteContact(),fileTransferObject.getFileIconName());
				}
			}
			catch(JoynServiceException e){
			}
            return fileStruct;
        }

        /**
         * Generate a file struct instance using a path, this method should only
         * be called for Sent File Transfer
         * 
         * @param filePath The path of the file
         * @return The file struct instance
         */
        public static FileStruct from(String filePath) {
            FileStruct fileStruct = null;
            File file = new File(filePath);
            if (file.exists()) {
                Date date = new Date();
                fileStruct =
                        new FileStruct(filePath, file.getName(), file.length(), new ParcelUuid(UUID
                                .randomUUID()), date);
            }
            Logger.d(TAG, "from() fileStruct: " + fileStruct);
            return fileStruct;
        }

        public FileStruct(String filePath, String name, long size, Object fileTransferTag, Date date) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
        }

        public FileStruct(String filePath, String name, long size, Object fileTransferTag, Date date, String remote) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
            mRemote = remote;
        }

        public FileStruct(String filePath, String name, long size, Object fileTransferTag, Date date, String remote, String thumbNail) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
            mRemote = remote;
            mThumbnail = thumbNail;
        }

        public FileStruct(String filePath, String name, long size, Object fileTransferTag, Date date, String remote, String thumbNail,int reload) {
            mFilePath = filePath;
            mName = name;
            mSize = size;
            mFileTransferTag = fileTransferTag;
            mDate = (Date) date.clone();
            mRemote = remote;
            mThumbnail = thumbNail;
            mReload = reload;
        }

        public String mFilePath = null;

        public String mThumbnail = null;

        public int mReload = 0;

        public String mName = null;

        public long mSize = -1;

        public Object mFileTransferTag = null;

        public Date mDate = null;

        public String mRemote = null;

        public String toString() {
            return TAG + "file path is " + mFilePath + " file name is " + mName + " size is "
                    + mSize + " FileTransferTag is " + mFileTransferTag + " date is " + mDate;
        }
    }

    /**
     * This class represents a chat event structure used to be shared
     */
    public static class ChatEventStruct {
        public static final String TAG = "M0CF ChatEventStruct";

        public ChatEventStruct(Information info, Object relatedInfo, Date dt) {
            information = info;
            relatedInformation = relatedInfo;
            date = (Date) dt.clone();
        }

        public Information information = null;

        public Object relatedInformation = null;

        public Date date = null;

        public String toString() {
            return TAG + "information is " + information + " relatedInformation is "
                    + relatedInformation + " date is " + date;
        }
    }

    /**
     * Clear all history of the chats, include both one2one chat and group chat.
     * 
     * @return True if successfully clear, false otherwise.
     */
    public boolean clearAllHistory() {
        Logger.d(TAG, "clearAllHistory() entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        boolean result = false;
        if (null != controller) {
            Message controllerMessage = controller.obtainMessage(
                    ChatController.EVENT_CLEAR_CHAT_HISTORY, null, null);
            controllerMessage.sendToTarget();
            result = true;
        }
        Logger.w(TAG, "clearAllHistory() exit with result = " + result);
        return result;
    }

    /**
     * Called by the controller when the user needs to start a file transfer
     * 
     * @param target The tag of the chat window in which the file transfer
     *            starts or the contact to receive this file
     * @param filePath The path of the file to be transfered
     */
    public void handleSendFileTransferInvitation(Object target, String filePath,
            Object fileTransferTag) {
        Logger.w(TAG, "M0CFF handleSendFileTransferInvitation() user starts to send file " + filePath
                + " to target " + target + ", fileTransferTag: " + fileTransferTag);
        IChat1 chat = null;
        if (target instanceof String) {
            String contact = (String) target;
            List<Participant> participants = new ArrayList<Participant>();
            participants.add(new Participant(contact,contact));
            chat = addChat(participants);
        } else {
            chat = getChat(target);
        }

        Logger.w(TAG, "M0CFF handleSendFileTransferInvitation() chat: " + chat);
        if (chat instanceof One2OneChat) {
            Participant participant = ((One2OneChat) chat).getParticipant();
            Logger.w(TAG, "M0CFF handleSendFileTransferInvitation() user starts to send O2O file " + filePath
                    + " to " + participant + ", fileTransferTag: " + fileTransferTag);
            mOutGoingFileTransferManager.onAddSentFileTransfer(((One2OneChat) chat).generateSentFileTransfer(
                    filePath, fileTransferTag));
        }        
        else if(chat instanceof GroupChat1)
        {
        	List<ParticipantInfo> participantsInfo = ((GroupChat1) chat).getParticipantInfos();
        	List<String> participants = new ArrayList<String>();
        	for (ParticipantInfo info : participantsInfo)
        	{
        		participants.add(info.getContact());
        	}
        	 mOutGoingFileTransferManager.onAddSentFileTransfer(((GroupChat1) chat).generateSentFileTransfer(chat,filePath, fileTransferTag));
        	
        }
		/*
		try{
			if(fileService != null){
					if(ApiManager.getInstance() != null){
					fileService = ApiManager.getInstance().getFileTransferApi();
					if(fileService != null){
						deliveryListener =  new FileTransferDeliveryListener();
						fileService.addNewFileTransferListener(deliveryListener);
					}
				}
			}
		}
		catch(JoynServiceException e){
			Logger.d(TAG, "handleSendFileTransferInvitation() exception");
		}
		*/
    }

	List<Object> fileGroupTransferTag = new ArrayList<Object>();
	List<String> fileGroupTransferPath = new ArrayList<String>();
	Object groupFileTransferTarget = null;

	public void startGroupFileStransfer(){
		Logger.d(TAG, "M0CFF startGroupFileStransfer()");
		if(groupFileTransferTarget != null){
			Logger.d(TAG, "M0CFF startGroupFileStransfer() not empty");
			Object target = groupFileTransferTarget;
			int index = 0;
			IChat1 chat = null;
			Logger.d(TAG, "M0CFF startGroupFileStransfer() size is: " + fileGroupTransferTag.size());
			if (target instanceof String) {
				Logger.d(TAG, "M0CFF startGroupFileStransfer()1");
	            String contact = (String) target;
	            List<Participant> participants = new ArrayList<Participant>();
	            participants.add(new Participant(contact,contact));
	            chat = addChat(participants);
	        } else {
	        	Logger.d(TAG, "M0CFF startGroupFileStransfer()2");
	            chat = getChat(target);
	        }
			for (String filePath : fileGroupTransferPath) {
				Object fileTransferTag = fileGroupTransferTag.get(index);
				Logger.d(TAG, "M0CFF startGroupFileStransfer() Tag is: " + fileTransferTag + "Paths is: " + filePath);
				if(chat instanceof GroupChat1)
	        	{
		        	List<ParticipantInfo> participantsInfo = ((GroupChat1) chat).getParticipantInfos();
		        	List<String> participants = new ArrayList<String>();
		        	for (ParticipantInfo info : participantsInfo)
		        	{
		        		participants.add(info.getContact());
		        	}
		        	 mOutGoingFileTransferManager.onAddSentFileTransfer(((GroupChat1) chat).generateSentFileTransfer(chat, filePath, fileTransferTag));
		        	
	        	}
		   }
			groupFileTransferTarget = null;
			fileGroupTransferTag.clear();
			fileGroupTransferPath.clear();
		}
		
		Logger.d(TAG, "M0CFF startGroupFileStransfer() exit");
	}

	 /**
     * Called by the controller when the user needs to start a file transfer
     * 
     * @param target The tag of the chat window in which the file transfer
     *            starts or the contact to receive this file
     * @param filePath The path of the file to be transfered
     */
    public void handleGroupSendFileTransferInvitation(Object target, String filePath,
            Object fileTransferTag) {
        Logger.d(TAG, "M0CFF handleGroupSendFileTransferInvitation() user starts to send file " + filePath
                + " to target " + target + ", fileTransferTag: " + fileTransferTag);
		
		fileGroupTransferTag.add(fileTransferTag);
		fileGroupTransferPath.add(filePath);
		groupFileTransferTarget = target;
    }

	public void handleFileResumeAfterStatusChange() {
		Logger.d(TAG, "M0CFF resumeFileSend 02 entry");
		mOutGoingFileTransferManager.resumesFileSend();
		}

    /**
     * This class describe one single out-going file transfer, and control the
     * status itself
     */
    public static class SentFileTransfer implements OnNetworkStatusChangedListerner {
        private static final String TAG = "M0CFF SentFileTransfer";
        public static final String KEY_FILE_TRANSFER_TAG = "file transfer tag";

        protected Object mChatTag = null;

        protected Object mFileTransferTag = null;

        protected FileStruct mFileStruct = null;

        protected IFileTransfer mFileTransfer = null;

        protected FileTransfer mFileTransferObject = null;

        protected FileTransferListener mFileTransferListener = null;

        protected Participant mParticipant = null;
        protected List<String> mParticipants = null;
		protected IChat1 mChat = null;
        
        public SentFileTransfer(Object chatTag, IChat1 chat,IGroupChatWindow groupChat,
                String filePath, List<String> participants, Object fileTransferTag) {
        

            Logger.d(TAG, "SentFileTransfer() constructor chatTag is "
                    + chatTag + " one2OneChat is " + groupChat
                    + " filePath is " + filePath + "fileTransferTag: "
                    + fileTransferTag);
            if (null != chatTag && null != groupChat && null != filePath
                    && null != participants) {
                mChatTag = chatTag;
				mChat = chat;
                mFileTransferTag = (fileTransferTag != null ? fileTransferTag
                        : UUID.randomUUID());
                mFileStruct = new FileStruct(filePath,
                        extractFileNameFromPath(filePath), 0, mFileTransferTag,
                        new Date());
                mFileTransfer = groupChat.addSentFileTransfer(mFileStruct);
                mFileTransfer.setStatus(Status.PENDING);
                mParticipants = participants;
                NetworkChangedReceiver.addListener(this);
            }
        
        }

        public SentFileTransfer(Object chatTag, IOne2OneChatWindow one2OneChat,
                String filePath, Participant participant, Object fileTransferTag) {
            Logger.d(TAG, "SentFileTransfer() constructor chatTag is "
                    + chatTag + " one2OneChat is " + one2OneChat
                    + " filePath is " + filePath + "fileTransferTag: "
                    + fileTransferTag);
            if (null != chatTag && null != one2OneChat && null != filePath
                    && null != participant) {
                mChatTag = chatTag;
                mFileTransferTag = (fileTransferTag != null ? fileTransferTag
                        : UUID.randomUUID());
                mFileStruct = new FileStruct(filePath,
                        extractFileNameFromPath(filePath), 0, mFileTransferTag,
                        new Date());
                mFileTransfer = one2OneChat.addSentFileTransfer(mFileStruct);
                mFileTransfer.setStatus(Status.PENDING);
                mParticipant = participant;
                NetworkChangedReceiver.addListener(this);
            }
        }

		/**
	 * Create a thumbnail from a filename
	 * 
	 * @param filename Filename
	 * @return Thumbnail
	 */
	public static byte[] createFileThumbnail(String filename) {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		InputStream in = null;
		try {
			File file = new File(filename);
			in = new FileInputStream(file);
			Bitmap bitmap = BitmapFactory.decodeStream(in);
			int width = bitmap.getWidth();
			int height = bitmap.getHeight();
			long size = file.length();

			// Resize the bitmap
			float scale = 0.4f;
			Matrix matrix = new Matrix();
			matrix.postScale(scale, scale);

			// Recreate the new bitmap
			Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
					height, matrix, true);

			// Compress the file to be under the limit (10KBytes)
			int quality = 90;
			int maxSize = 1024 * 10;
			if(size > maxSize) {
			while(size > maxSize) {
				out = new ByteArrayOutputStream();
				resizedBitmap.compress(CompressFormat.JPEG, quality, out);
				out.flush();
				out.close();
				size = out.size();
				quality -= 10;
			}}
			else{
				out = new ByteArrayOutputStream();
				resizedBitmap.compress(CompressFormat.JPEG, 90, out);
				out.flush();
				out.close();
			}
		if(in != null){
			in.close();
			}
		} catch (Exception e) {
			return null;
		}
		return out.toByteArray();
	}

/**
	 * Create a thumbnail from a filename
	 * 
	 * @param filename Filename
	 * @return Thumbnail
	 */
	public static byte[] createImageThumbnail(String filename) {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		InputStream in = null;
		try {
			File file = new File(filename);
			in = new FileInputStream(file);
			Bitmap bitmap = BitmapFactory.decodeStream(in);
			int width = bitmap.getWidth();
			int height = bitmap.getHeight();
			long size = file.length();

			// Resize the bitmap
			
			float scale = 0.1f;
			if(size > 1000 * 1024){
				scale = 0.02f;
			}
			else if(size > 500 * 1024){
				scale = 0.04f;
			}
			else if(size > 300 * 1024){
				scale = 0.1f;
			}
			else if(size < 300*1024 && size > 50 *1024){
				scale = 0.15f;
			}
			else{
				scale = 0.2f;
			}
	
			Matrix matrix = new Matrix();
			matrix.postScale(scale, scale);

			// Recreate the new bitmap
			Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
					height, matrix, true);

			// Compress the file to be under the limit (10KBytes)
			int quality = 90;
			int maxSize = 1024 * 2;
			if(size > maxSize) {
			while(size > maxSize) {
				out = new ByteArrayOutputStream();
				resizedBitmap.compress(CompressFormat.JPEG, quality, out);
				out.flush();
				out.close();
				size = out.size();
				quality -= 5;
			}}
			else{
				out = new ByteArrayOutputStream();
				resizedBitmap.compress(CompressFormat.JPEG, 90, out);
				out.flush();
				out.close();
			}
		if(in != null){
			in.close();
			}
		} catch (Exception e) {
			try{
			in.close();
				}
			 catch (Exception g) {
			 	}
			return null;
		}
		return out.toByteArray();
	}

		public String getThumnailFile(String fileName){
			return null;
		}

		private Set<String> convertParticipantsToContactsSet(List<String> participants) {
	        Set<String> pList = new HashSet<String>();
	        int size = participants.size();
	        for (int i = 0; i < size; i++) {
	            pList.add(participants.get(i));
	        }
	        return pList;
		}

        protected void send() {
			Logger.w(TAG, "checkNext() send new file");
            ApiManager instance = ApiManager.getInstance();
            if (instance != null) {
                FileTransferService fileTransferService = instance.getFileTransferApi();
                if (fileTransferService != null) {
                    try {
                    	//TODO find out how to get this
                    	//if(mParticipant != null)
                    	mFileTransferListener = new FileTransferSenderListener();
                    	if(mParticipant != null)
                    	{
	                       if(RcsSettings.getInstance().isFileTransferThumbnailSupported()){
						   	Logger.w(TAG, "checkNext() send new file thumbnail supported");
	                           mFileTransferObject =
	                                fileTransferService.transferFile(mParticipant.getContact(),
	                                    mFileStruct.mFilePath,getThumnailFile(mFileStruct.mFilePath),mFileTransferListener);
                    	   }
						   else{
						   	Logger.w(TAG, "checkNext() send new file thumbnail not supported");
						   	   mFileTransferObject =
	                                fileTransferService.transferFile(mParticipant.getContact(),
	                                    mFileStruct.mFilePath,mFileTransferListener);
						   	}
                    	}
                    	else
                    	{         
                    	    GroupChat mGroupChat = ((GroupChat1) mChat).getGroupChatImpl();
							
							if(RcsSettings.getInstance().isFileTransferThumbnailSupported()){
								Logger.w(TAG, "checkNext() send new group file thumbnail supported");
	                        	mFileTransferObject = fileTransferService.transferFileToGroup(mGroupChat.getChatSessionId(),convertParticipantsToContactsSet(mParticipants),mFileStruct.mFilePath,getThumnailFile(mFileStruct.mFilePath),mFileTransferListener);
							}
							else{
								Logger.w(TAG, "checkNext() send new group file thumbnail not supported");
								mFileTransferObject = fileTransferService.transferFileToGroup(mGroupChat.getChatSessionId(),convertParticipantsToContactsSet(mParticipants),mFileStruct.mFilePath,mFileTransferListener);
							}
	                        
                    	}
                        if (null != mFileTransferObject) {
                            mFileStruct.mSize = mFileTransferObject.getFileSize();
                            String fileTransferId = mFileTransferObject.getTransferId();
                            mFileTransfer.updateTag(fileTransferId,
                                    mFileStruct.mSize);
                            mFileTransferTag = fileTransferId;
                            mFileStruct.mFileTransferTag = fileTransferId;                          
                            mFileTransfer.setStatus(Status.WAITING);
                            setNotification();
                        } else {
                        	 //mFileTransfer.setStatus(Status.WAITING);
                            Logger.e(TAG,
                                    "send() failed, mFileTransferObject is null, filePath is "
                                            + mFileStruct.mFilePath);
                            onFailed();
                            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        }
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                        onFailed();
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    }
                }
            }
        }

        private void onPrepareResend() {
            Logger.d(TAG, "onPrepareResend() file " + mFileStruct.mFilePath
                    + " to " + mParticipant);
            if (null != mFileTransfer) {
                mFileTransfer.setStatus(Status.PENDING);
            }
        }

		private void onPause() {
           // Logger.d(TAG, "onPause() file " + mFileStruct.mFilePath   
          //          + " to " + mParticipant);
           Logger.d(TAG, "onPause() file 123");
            if (null != mFileTransfer) {
				try{
					Logger.d(TAG, "onPause 1");
                	mFileTransferObject.pauseTransfer(); 
				}
				catch (JoynServiceException e) {
                e.printStackTrace();
            	}
            }
        }

		private void onResume() {
           // Logger.d(TAG, "onResume() file " + mFileStruct.mFilePath   
            //        + " to " + mParticipant);
            Logger.d(TAG, "onResume 123");
            if (null != mFileTransfer) {
				try{
					Logger.d(TAG, "onResume 1");
					if(null != mFileTransferObject){
					//if(mFileTransferObject.isSessionPaused()){ ////TODo check this
						Logger.d(TAG, "onResume 2");
	                	mFileTransferObject.resumeTransfer();
					//}
				}
				}
				catch (JoynServiceException e) {
                	e.printStackTrace();
            	}
            }
        }

        private void onCancel() {
            Logger.d(TAG, "onCancel() entry: mFileTransfer = " + mFileTransfer);
            if (null != mFileTransfer) {
                mFileTransfer.setStatus(Status.CANCEL);
            }
        }

        private void onFailed() {
            Logger.d(TAG, "onFailed() entry: mFileTransfer = " + mFileTransfer);
            if (null != mFileTransfer) {
                mFileTransfer.setStatus(Status.FAILED);
            }
        }

        private void onNotAvailable(int reason) {
            Logger.d(TAG, "onNotAvailable() reason is " + reason);
            if (One2OneChat.FILETRANSFER_ENABLE_OK == reason) {
                return;
            } else {
                switch (reason) {
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE:
                        onFailed();
                        break;
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED:
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER:
                        onCancel();
                        break;
                    default:
                        Logger.w(TAG, "onNotAvailable() unknown reason " + reason);
                        break;
                }
            }
        }

        private void onDestroy() {
            Logger.d(TAG, "onDestroy() sent file transfer mFilePath "
                    + ((null == mFileStruct) ? null : mFileStruct.mFilePath)
                    + " mFileTransferObject = " + mFileTransferObject
                    + ", mFileTransferListener = " + mFileTransferListener);
            if (null != mFileTransferObject) {
                try {
                    if (null != mFileTransferListener) {
                        mFileTransferObject
                                .removeEventListener(mFileTransferListener);
                    }
                    cancelNotification();
                    mFileTransferObject.abortTransfer();
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
            }
        }

        protected void onFileTransferFinished(
                IOnSendFinishListener.Result result) {
            Logger.d(TAG, "onFileTransferFinished() mFileStruct = "
                    + mFileStruct + ", file = "
                    + ((null == mFileStruct) ? null : mFileStruct.mFilePath)
                    + ", mOnSendFinishListener = " + mOnSendFinishListener
                    + ", mFileTransferListener = " + mFileTransferListener
                    + ", result = " + result);
            if (null != mOnSendFinishListener) {
                mOnSendFinishListener.onSendFinish(SentFileTransfer.this,
                        result);
                if (null != mFileTransferObject) {
                    try {
                        if (null != mFileTransferListener) {
                            mFileTransferObject
                                    .removeEventListener(mFileTransferListener);
                        }
                    } catch (JoynServiceException e) {
                        e.printStackTrace();
                    }
                }
            }
            // Remove network listener when transfer is finished.
            NetworkChangedReceiver.removeListener(this);
        }

        public static String extractFileNameFromPath(String filePath) {
            if (null != filePath) {
                int lastDashIndex = filePath.lastIndexOf("/");
                if (-1 != lastDashIndex && lastDashIndex < filePath.length() - 1) {
                    String fileName = filePath.substring(lastDashIndex + 1);
                    return fileName;
                } else {
                    Logger.e(TAG, "extractFileNameFromPath() invalid file path:" + filePath);
                    return null;
                }
            } else {
                Logger.e(TAG, "extractFileNameFromPath() filePath is null");
                return null;
            }
        }

        /**
         * File transfer session event listener
         */
        private class FileTransferSenderListener extends FileTransferListener {
            private static final String TAG = "M0CFF FileTransferSenderListener";

		/**
		 * Callback called when the file transfer is started
		 */
		public  void onTransferStarted(){
			Logger.d(TAG, "onTransferStarted() this file is " + mFileStruct.mFilePath);
		}
		
		/**
		 * Callback called when the file transfer has been aborted
		 */
		public  void onTransferAborted(){
		  	Logger.v(TAG,
                        "File transfer onTransferAborted(): mFileTransfer = "
                                + mFileTransfer);
            if (mParticipant == null) {
                Logger.d(TAG,
                        "FileTransferSenderListener onTransferAborted mParticipant is null");
                return;
            }
            if (mFileTransfer != null) {
                mFileTransfer.setStatus(Status.FAILED);
                IChat1 chat = ModelImpl.getInstance().getChat(mChatTag); //TODo Check this
                Logger.v(TAG, "onTransferAborted(): chat = " + chat);
                if (chat instanceof ChatImpl) {
                    ((ChatImpl) chat).checkCapabilities();
                }
            }
            try {
                mFileTransferObject.removeEventListener(this);
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }
            mFileTransferObject = null;
            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
		}

		/**
		 * Callback called when the transfer has failed
		 * 
		 * @param error Error
		 * @see FileTransfer.Error
		 */
		public  void onTransferError(int error){
		 	Logger.v(TAG, "onTransferError(),error = " + error + ", mFileTransfer ="
                        + mFileTransfer);
            switch (error) {   
                case FileTransfer.Error.TRANSFER_FAILED:
                    Logger.d(TAG,
                            "onTransferError(), the file transfer invitation is failed.");
                    if (mFileTransfer != null) {
                        mFileTransfer.setStatus(ChatView.IFileTransfer.Status.TIMEOUT);
                    }
                    onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    break;
                    
                case FileTransfer.Error.INVITATION_DECLINED:
                    Logger.d(TAG,
                            "onTransferError(), your file transfer invitation has been rejected");
                    if (mFileTransfer != null) {
                        mFileTransfer.setStatus(ChatView.IFileTransfer.Status.REJECTED);
                    }
                    onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    break;
                case FileTransfer.Error.SAVING_FAILED:
                    Logger.d(TAG,
                            "onTransferError(), saving of file failed");
                    if (mFileTransfer != null) {
                        mFileTransfer.setStatus(ChatView.IFileTransfer.Status.FAILED);
                    }
                    onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    break;
                default:
                    Logger.e(TAG, "onTransferError() unknown error " + error);
                    onFailed();
                    onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                    break;
            }
		}
		
		/**
		 * Callback called during the transfer progress
		 * 
		 * @param currentSize Current transferred size in bytes
		 * @param totalSize Total size to transfer in bytes
		 */
		public  void onTransferProgress(long currentSize, long totalSize){
		  	// Because of can't received file transfered callback, so add
            // current size equals total size change status to finished
            Logger.d(TAG, "onTransferProgress() the file "
                    + mFileStruct.mFilePath + " with " + mParticipant
                    + " is transferring, currentSize is " + currentSize
                    + " total size is " + totalSize + ", mFileTransfer = "
                    + mFileTransfer);
            if (mFileTransfer != null) {
                if (currentSize < totalSize) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING);
                    mFileTransfer.setProgress(currentSize);
                    updateNotification(currentSize);
                } else {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
//                        onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
                }
            } 
		}

		/**
		 * Callback called when the file has been transferred
		 * 
		 * @param filename Filename including the path of the transferred file
		 */
		public  void onFileTransferred(String filename){
		 	Logger.d(TAG, "onFileTransferred() entry, fileName is " + filename + ", mFileTransfer = " + mFileTransfer);
            if (mFileTransfer != null) {
                mFileTransfer
                        .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
            }
           onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
		}
/*
            // Session is started
            @Override
            public void handleSessionStarted() {
                Logger.d(TAG, "handleSessionStarted() this file is " + mFileStruct.mFilePath);
            }

			// File transfer has been paused	
			@Override
			public void handleFileTransferPaused()
			{
				Logger.d(TAG, "handleFileTransferPaused() ");
			}

            // Session has been aborted
            @Override
            public void handleSessionAborted(int reason) {
                Logger.v(TAG,
                        "File transfer handleSessionAborted(): mFileTransfer = "
                                + mFileTransfer);
                if (mParticipant == null) {
                    Logger.d(TAG,
                            "FileTransferSenderListener handleSessionAborted mParticipant is null");
                    return;
                }
                if (mFileTransfer != null) {
                    mFileTransfer.setStatus(Status.FAILED);
                    IChat1 chat = ModelImpl.getInstance().getChat(mChatTag);
                    Logger.v(TAG, "handleSessionAborted(): chat = " + chat);
                    if (chat instanceof ChatImpl) {
                        ((ChatImpl) chat).checkCapabilities();
                    }
                }
                try {
                    mFileTransferObject.removeEventListener(this);
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
                mFileTransferObject = null;
                onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
				
            }

			public void handleFileUploaded(){
				Logger.d(TAG, "handleFileUploaded() ");
				}

            // Session has been terminated by remote
            @Override
            public void handleSessionTerminatedByRemote() {
                Logger.v(TAG,
                        "File transfer handleSessionTerminatedByRemote(): mFileTransfer = "
                                + mFileTransfer);
                if (mFileTransfer != null) {
                    mFileTransfer.setStatus(Status.CANCELED);
                    IChat1 chat = ModelImpl.getInstance().getChat(mChatTag);
                    Log.v(TAG, "chat = " + chat);
                    if (chat instanceof ChatImpl) {
                        ((ChatImpl) chat).checkCapabilities();
                    }
                }
                mFileTransferObject = null;
                onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
            }

            // File transfer progress
            @Override
            public void handleTransferProgress(final long currentSize, final long totalSize) {
                // Because of can't received file transfered callback, so add
                // current size equals total size change status to finished
                Logger.d(TAG, "handleTransferProgress() the file "
                        + mFileStruct.mFilePath + " with " + mParticipant
                        + " is transferring, currentSize is " + currentSize
                        + " total size is " + totalSize + ", mFileTransfer = "
                        + mFileTransfer);
                if (mFileTransfer != null) {
                    if (currentSize < totalSize) {
                        mFileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING);
                        mFileTransfer.setProgress(currentSize);
                        updateNotification(currentSize);
                    } else {
                        mFileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
//                        onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
                    }
                } 
            }

            // File transfer error
            @Override
            public void handleTransferError(final int error) {
                Logger.v(TAG, "handleTransferError(),error = " + error + ", mFileTransfer ="
                        + mFileTransfer);
                switch (error) {
                    case FileSharingError.UNEXPECTED_EXCEPTION:
                        Logger.v(TAG, "handleTransferError(), UNEXPECTED_EXCEPTION");
                        break;
                    case FileSharingError.SESSION_INITIATION_FAILED:
                        Logger.d(TAG,
                                "handleTransferError(), the file transfer invitation is failed.");
                        if (mFileTransfer != null) {
                            mFileTransfer.setStatus(ChatView.IFileTransfer.Status.FAILED);
                        }
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        break;
                        
                    case FileSharingError.SESSION_INITIATION_TIMEOUT:
                        Logger.d(TAG,
                                "handleTransferError(), the file transfer invitation is failed.");
                        if (mFileTransfer != null) {
                            mFileTransfer.setStatus(ChatView.IFileTransfer.Status.TIMEOUT);
                        }
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        break;
                        
                    case FileSharingError.SESSION_INITIATION_DECLINED:
                        Logger.d(TAG,
                                "handleTransferError(), your file transfer invitation has been rejected");
                        if (mFileTransfer != null) {
                            mFileTransfer.setStatus(ChatView.IFileTransfer.Status.REJECTED);
                        }
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        break;
                    case FileSharingError.SESSION_INITIATION_CANCELLED:
                        Logger.d(TAG,
                                "handleTransferError(), your file transfer invitation has been rejected");
                        if (mFileTransfer != null) {
                            mFileTransfer.setStatus(ChatView.IFileTransfer.Status.TIMEOUT);
                        }
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        break;
                    case FileSharingError.NO_CHAT_SESSION:
                        Logger.d(TAG,
                                "handleTransferError(), NO CHAT SESSION EXISTS");
                        if (mFileTransfer != null) {
                            mFileTransfer.setStatus(ChatView.IFileTransfer.Status.TIMEOUT);
                        }
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        break;
                    default:
                        Logger.e(TAG, "handleTransferError() unknown error " + error);
                        onFailed();
                        onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
                        break;
                }
            }

            // File has been transfered
            @Override
            public void handleFileTransfered(final String fileName) {
                Logger.d(TAG, "handleFileTransfered() entry, fileName is "
                        + fileName + ", mFileTransfer = " + mFileTransfer);
                if (mFileTransfer != null) {
                    mFileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                }
               onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
            }
            
            public void handleTransferTerminated() {
                Logger.d(TAG, "handleTransferTerminated() entry");
                onFileTransferFinished(IOnSendFinishListener.Result.REMOVABLE);
            }

			public void handleFileTransferResumed(){
				}
		*/

        }

        private NotificationManager getNotificationManager() {
            ApiManager apiManager = ApiManager.getInstance();
            if (null != apiManager) {
                Context context = apiManager.getContext();
                if (null != context) {
                    Object systemService = context.getSystemService(Context.NOTIFICATION_SERVICE);
                    return (NotificationManager) systemService;
                } else {
                    Logger.e(TAG, "getNotificationManager() context is null");
                    return null;
                }
            } else {
                Logger.e(TAG, "getNotificationManager() apiManager is null");
                return null;
            }
        }

        private int getNotificationId() {
            if (null != mFileTransferTag) {
                return mFileTransferTag.hashCode();
            } else {
                Logger.w(TAG, "getNotificationId() mFileTransferTag: " + mFileTransferTag);
                return 0;
            }
        }

        private void setNotification() {
            updateNotification(0);
        }

        private void updateNotification(long currentSize) {
            Logger.d(TAG, "updateNotification() entry, currentSize is "
                    + currentSize + ", mFileTransferTag is " + mFileTransferTag
                    + ", mFileStruct = " + mFileStruct);
            NotificationManager notificationManager = getNotificationManager();
            if (null != notificationManager) {
                if (null != mFileStruct) {
                    Context context = ApiManager.getInstance().getContext();
                    Notification.Builder builder = new Notification.Builder(context);
                    builder
                            .setProgress((int) mFileStruct.mSize, (int) currentSize,
                                    currentSize < 0);
                    String title = null;
                    if(mParticipant != null)
                	{
                    
                         title =   context.getResources().getString(
                                    R.string.ft_progress_bar_title,
                                            ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                                                    mParticipant.getContact()));
                	}
                    else
                    {
                    	 IChat1 chat = ModelImpl.getInstance().getChat(mChatTag);

                         if (chat instanceof GroupChat1) {
                    	  title = context.getResources().getString(
                                         R.string.ft_progress_bar_title,((GroupChat1)chat).getGroupSubject());
                         }
                    }
                    builder.setContentTitle(title);
                    builder.setAutoCancel(false);
                    builder.setContentText(extractFileNameFromPath(mFileStruct.mFilePath));
                    builder.setContentInfo(buildPercentageLabel(context, mFileStruct.mSize,
                            currentSize));
                    builder.setSmallIcon(R.drawable.rcs_notify_file_transfer);
                    PendingIntent pendingIntent;
                    if (Logger.getIsIntegrationMode()) {
                        Intent intent = new Intent();
Uri uri;
                        intent.setAction(Intent.ACTION_SENDTO);
                        if(RcsSettings.getInstance().getMessagingUx() == 0) 
                        {
	                        if(mParticipant != null)
	                    	{
                         uri = Uri.parse(PluginProxyActivity.MMSTO + IpMessageConsts.JOYN_START
                                + mParticipant.getContact());
	                    	}
	                    	else
	                    	{
	                    		IChat1 chat = ModelImpl.getInstance().getChat(mChatTag);
	
	                    		uri = Uri.parse(PluginProxyActivity.MMSTO + ((GroupChat1)chat).getGroupSubject());
	
	                    	}
                        intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);

                        }   
                        else
                        {
                        	if(mParticipant != null)
                        	{
                        uri = Uri.parse(PluginProxyActivity.MMSTO + mParticipant.getContact());
                        	}
                        	else
                        	{
                        		IChat1 chat = ModelImpl.getInstance().getChat(mChatTag);

                        		uri = Uri.parse(PluginProxyActivity.MMSTO + ((GroupChat1)chat).getGroupSubject());

                        	}
                        }

                        
                        intent.setData(uri);
                        pendingIntent = PendingIntent.getActivity(context, 0, intent,PendingIntent.FLAG_UPDATE_CURRENT);
                    } else {
                        Intent intent = new Intent(context, ChatScreenActivity.class);
                        intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, (ParcelUuid) mChatTag);
                        pendingIntent = PendingIntent.getActivity(context, 0, intent,
                                PendingIntent.FLAG_UPDATE_CURRENT);
                    }
                    builder.setContentIntent(pendingIntent);
                    notificationManager.notify(getNotificationId(), builder.getNotification());
                }
            }
        }

        private void cancelNotification() {
            NotificationManager notificationManager = getNotificationManager();
            Logger.d(TAG, "cancelNotification() entry, mFileTransferTag is "
                    + mFileTransferTag + ",notificationManager = "
                    + notificationManager);
            if (null != notificationManager) {
                notificationManager.cancel(getNotificationId());
            } 
        }

        private static String buildPercentageLabel(Context context, long totalBytes,
                long currentBytes) {
            if (totalBytes <= 0) {
                return null;
            } else {
                final int percent = (int) (100 * currentBytes / totalBytes);
                return context.getString(R.string.ft_percent, percent);
            }
        }

        protected IOnSendFinishListener mOnSendFinishListener = null;

        protected interface IOnSendFinishListener {
            static enum Result {
                REMOVABLE, // This kind of result indicates that this File
                // transfer should be removed from the manager
                RESENDABLE
                // This kind of result indicates that this File transfer will
                // have a chance to be resent in the future
            };

            void onSendFinish(SentFileTransfer sentFileTransfer, Result result);
        }

        @Override
        public void onNetworkStatusChanged(boolean isConnected) {
        boolean  hhtpFT = false;
		try{
			if(mFileTransferObject != null){
           hhtpFT = mFileTransferObject.isHttpFileTransfer();
		}
		}
		catch (JoynServiceException e) {
            e.printStackTrace();
        }
		catch (Exception e) {
            e.printStackTrace();
        }
		Logger.d(TAG, "onNetworkStatusChanged Model" + isConnected + "Http: " + hhtpFT);
		
		if(!isConnected && !hhtpFT){
            onFailed();
            onFileTransferFinished(IOnSendFinishListener.Result.RESENDABLE);
        }
    }
    }

	public void clearFileTransferQueue(){
		Logger.w(TAG, "clearFileTransferQueue");
		mOutGoingFileTransferManager.clearFtList();
	}

    private SentFileTransferManager mOutGoingFileTransferManager = new SentFileTransferManager();

    /**
     * This class is used to manage the whole sent file transfers and make it
     * work in queue
     */
    private static class SentFileTransferManager implements SentFileTransfer.IOnSendFinishListener {
        private static final String TAG = "M0CFF SentFileTransferManager";

        private static final int MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM = 1;

        private ConcurrentLinkedQueue<SentFileTransfer> mPendingList =
                new ConcurrentLinkedQueue<SentFileTransfer>();

        private CopyOnWriteArrayList<SentFileTransfer> mActiveList =
                new CopyOnWriteArrayList<SentFileTransfer>();

        private CopyOnWriteArrayList<SentFileTransfer> mResendableList =
                new CopyOnWriteArrayList<SentFileTransfer>();

        private synchronized void checkNext() {
            int activatedNum = mActiveList.size();
            if (activatedNum < MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM) {
                Logger.w(TAG, "checkNext() current activatedNum is " + activatedNum
                        + " will find next file transfer to send");
                final SentFileTransfer nextFileTransfer = mPendingList.poll();
                if (null != nextFileTransfer) {
                    Logger.w(TAG, "checkNext() next file transfer found, just send it!");
					
					AsyncTask.execute(new Runnable() {
                	@Override
	               	public void run() {
                    nextFileTransfer.send();
	                	}
            		});
                    mActiveList.add(nextFileTransfer);
                } else {
                    Logger.w(TAG, "checkNext() next file transfer not found, pending list is null");
                }
            } else {
                Logger.w(TAG, "checkNext() current activatedNum is " + activatedNum
                        + " MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM is "
                        + MAX_ACTIVATED_SENT_FILE_TRANSFER_NUM
                        + " so no need to find next pending file transfer");
            }
        }

		public void clearFtList(){
			Logger.w(TAG, "clearFtList mPendingList size: " + mPendingList.size() + "resend size: " + mResendableList.size());
			if(RichMessagingDataProvider.getInstance() == null){
				RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
			}	
			/*
			Iterator pendingIterator = mPendingList.iterator();
			while(pendingIterator.hasNext()){
				SentFileTransfer pendingFile = pendingIterator.next();
				//delete from database		
				RichMessagingDataProvider.getInstance().deleteFileTranfser(pendingFile.mFileTransferTag);
			}
			Iterator resendableIterator = mResendableList.iterator();
			while(resendableIterator.hasNext()){
				SentFileTransfer resendableFile = resendableIterator.next();
				//delete from database		
				RichMessagingDataProvider.getInstance().deleteFileTranfser(resendableFile.mFileTransferTag);
			}
			*/
			RichMessagingDataProvider.getInstance().deleteFileTranfser(null);
			mPendingList.clear();
			mResendableList.clear();
		}

		public void clearContactFtList(String tag){
			Logger.w(TAG, "clearContactFtList mPendingList size: " + mPendingList.size() + "resend size: " + mResendableList.size());
			if(RichMessagingDataProvider.getInstance() == null){
				RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
			}	
			Logger.w(TAG, "clearContactFtList tag: " + tag);
			Iterator pendingIterator = mPendingList.iterator();
			while(pendingIterator.hasNext()){
				SentFileTransfer pendingFile = (SentFileTransfer)pendingIterator.next();
				if(pendingFile.mChatTag.toString().equals(tag)){
					mPendingList.remove(pendingFile);
					RichMessagingDataProvider.getInstance().deleteFileTranfser(pendingFile.mFileTransferTag.toString());
				}		
			}
			Iterator resendableIterator = mResendableList.iterator();
			while(resendableIterator.hasNext()){
				SentFileTransfer resendableFile = (SentFileTransfer)resendableIterator.next();
				if(resendableFile.mChatTag.toString().equals(tag)){
					mResendableList.remove(resendableFile);
					RichMessagingDataProvider.getInstance().deleteFileTranfser(resendableFile.mFileTransferTag.toString());
				}		
			}
		}

		public void resumesFileSend() {
			Logger.w(TAG, "resumeFileSend 01 entry");
			SentFileTransfer resume_file = null;
			try{
                resume_file = mActiveList.get(0);
			}
			catch (Exception e) {
				Logger.d(TAG, "resumeFileSend exception");
			}
			if(resume_file != null){
			Logger.d(TAG, "resumeFileSend not null");
			resume_file.onResume();
			}
			Logger.d(TAG, "resumeFileSend 01 exit");
		}

        public void onAddSentFileTransfer(SentFileTransfer sentFileTransfer) {
            Logger.w(TAG, "onAddSentFileTransfer() entry, sentFileTransfer =  "
                    + sentFileTransfer);
            if (null != sentFileTransfer) {
                Logger.w(TAG, "onAddSentFileTransfer() entry, file "
                        + sentFileTransfer.mFileStruct + " is going to be sent");
                sentFileTransfer.mOnSendFinishListener = this;
                mPendingList.add(sentFileTransfer);
                checkNext();
            }
        }

        public void onChatDestroy(Object tag) {
            Logger.d(TAG, "onChatDestroy entry, tag is " + tag);
            clearTransferWithTag(tag, One2OneChat.FILETRANSFER_ENABLE_OK);
        }

        public void clearTransferWithTag(Object tag, int reason) {
            Logger.d(TAG, "onFileTransferNotAvalible() entry tag is " + tag + " reason is "
                    + reason);
            if (null != tag) {
                Logger.d(TAG, "onFileTransferNotAvalible() tag is " + tag);
                ArrayList<SentFileTransfer> toBeDeleted = new ArrayList<SentFileTransfer>();
                for (SentFileTransfer fileTransfer : mActiveList) {
                    if (tag.equals(fileTransfer.mChatTag)) {
                        Logger.d(TAG,
                                "onFileTransferNotAvalible() sent file transfer with chatTag "
                                        + tag + " found in activated list");
                        fileTransfer.onNotAvailable(reason);
                        fileTransfer.onDestroy();
                        toBeDeleted.add(fileTransfer);
                    }
                }
                if (toBeDeleted.size() > 0) {
                    Logger
                            .d(TAG,
                                    "onFileTransferNotAvalible() need to remove some file transfer from activated list");
                    mActiveList.removeAll(toBeDeleted);
                    toBeDeleted.clear();
                }
                for (SentFileTransfer fileTransfer : mPendingList) {
                    if (tag.equals(fileTransfer.mChatTag)) {
                        Logger.d(TAG,
                                "onFileTransferNotAvalible() sent file transfer with chatTag "
                                        + tag + " found in pending list");
                        fileTransfer.onNotAvailable(reason);
                        toBeDeleted.add(fileTransfer);
                    }
                }
                if (toBeDeleted.size() > 0) {
                    Logger
                            .d(TAG,
                                    "onFileTransferNotAvalible() need to remove some file transfer from pending list");
                    mPendingList.removeAll(toBeDeleted);
                    toBeDeleted.clear();
                }
                for (SentFileTransfer fileTransfer : mResendableList) {
                    if (tag.equals(fileTransfer.mChatTag)) {
                        Logger.d(TAG,
                                "onFileTransferNotAvalible() sent file transfer with chatTag "
                                        + tag + " found in mResendableList list");
                        fileTransfer.onNotAvailable(reason);
                        toBeDeleted.add(fileTransfer);
                    }
                }
                if (toBeDeleted.size() > 0) {
                    Logger.d(TAG, "onFileTransferNotAvalible() " +
                            "need to remove some file transfer from mResendableList list");
                    mResendableList.removeAll(toBeDeleted);
                    toBeDeleted.clear();
                }
            }
        }

        public void resendFileTransfer(Object targetFileTransferTag) {
            SentFileTransfer fileTransfer = findResendableFileTransfer(targetFileTransferTag);
            Logger.w(TAG, "resendFileTransfer() the file transfer with tag "
                    + targetFileTransferTag + " is " + fileTransfer);
            if (null != fileTransfer) {
                fileTransfer.onPrepareResend();
                Logger.d(TAG, "resendFileTransfer() the file transfer with tag "
                        + targetFileTransferTag
                        + " found, remove it from resendable list and add it into pending list");
                mResendableList.remove(fileTransfer);
                mPendingList.add(fileTransfer);
                checkNext();
            }
        }

		public void pauseFileTransfer(Object targetFileTransferTag) {
            SentFileTransfer fileTransfer = findActiveFileTransfer(targetFileTransferTag);
            Logger.w(TAG, "pauseFileTransfer() the file transfer with tag "
                    + targetFileTransferTag + " is " + fileTransfer);
            if (null != fileTransfer) {
                Logger.w(TAG, "pauseFileTransfer() the file transfer with tag "
                        + targetFileTransferTag + " found, ");
				fileTransfer.onPause();
            }
        }


		public void resumeFileTransfer(Object targetFileTransferTag) {
            SentFileTransfer fileTransfer = findActiveFileTransfer(targetFileTransferTag);
            Logger.w(TAG, "resumeFileTransfer() the file transfer with tag "
                    + targetFileTransferTag + " is " + fileTransfer);
            if (null != fileTransfer) {
                Logger.w(TAG, "resumeFileTransfer() the file transfer with tag "
                        + targetFileTransferTag+ " found");
				fileTransfer.onResume();
            }
        }

        public void cancelFileTransfer(Object targetFileTransferTag) {
            Logger.d(TAG, "cancelFileTransfer() begin to cancel file transfer with tag "
                    + targetFileTransferTag);
            SentFileTransfer fileTransfer = findPendingFileTransfer(targetFileTransferTag);
            if (null != fileTransfer) {
                Logger.d(TAG, "cancelFileTransfer() the target file transfer with tag "
                        + targetFileTransferTag + " found in pending list");
                fileTransfer.onCancel();
                mPendingList.remove(fileTransfer);
            } else {
                fileTransfer = findActiveFileTransfer(targetFileTransferTag);
                Logger.w(TAG,
                        "cancelFileTransfer() the target file transfer with tag "
                                + targetFileTransferTag
                                + " found in active list is " + fileTransfer);
                if (null != fileTransfer) {
                    Logger.w(TAG,
                            "cancelFileTransfer() the target file transfer with tag "
                                    + targetFileTransferTag
                                    + " found in active list");
                    fileTransfer.onCancel();
                    fileTransfer.onDestroy();
                    onSendFinish(fileTransfer, Result.REMOVABLE);
                }
            }
        }

        @Override
        public void onSendFinish(final SentFileTransfer sentFileTransfer, final Result result) {
            Logger.w(TAG, "onSendFinish(): sentFileTransfer = "
                    + sentFileTransfer + ", result = " + result);
            if (mActiveList.contains(sentFileTransfer)) {
                sentFileTransfer.cancelNotification();
                Logger.w(TAG, "onSendFinish() file transfer " + sentFileTransfer.mFileStruct
                        + " with " + sentFileTransfer.mParticipant + " finished with " + result
                        + " remove it from activated list");
                switch (result) {
                    case RESENDABLE:
                        mResendableList.add(sentFileTransfer);
                        mActiveList.remove(sentFileTransfer);
                        break;
                    case REMOVABLE:
                        mActiveList.remove(sentFileTransfer);
                        break;
                    default:
                        break;
                }
				 AsyncTask.execute(new Runnable() {
                @Override
	                public void run() {
                checkNext();
            }
            	});
            }
        }

        private SentFileTransfer findActiveFileTransfer(Object targetTag) {
            Logger.w(TAG, "findActiveFileTransfer entry, targetTag is " + targetTag);
            return findFileTransferByTag(mActiveList, targetTag);
        }

        private SentFileTransfer findPendingFileTransfer(Object targetTag) {
            Logger.w(TAG, "findPendingFileTransfer entry, targetTag is " + targetTag);
            return findFileTransferByTag(mPendingList, targetTag);
        }

        private SentFileTransfer findResendableFileTransfer(Object targetTag) {
            Logger.d(TAG, "findResendableFileTransfer entry, targetTag is " + targetTag);
            return findFileTransferByTag(mResendableList, targetTag);
        }

        private SentFileTransfer findFileTransferByTag(Collection<SentFileTransfer> whereToFind,
                Object targetTag) {
            if (null != whereToFind && null != targetTag) {
                for (SentFileTransfer sentFileTransfer : whereToFind) {
                    Object fileTransferTag = sentFileTransfer.mFileTransferTag;
                    if (targetTag.equals(fileTransferTag)) {
                        Logger.w(TAG, "findFileTransferByTag() the file transfer with targetTag "
                                + targetTag + " found");
                        return sentFileTransfer;
                    }
                }
                Logger.w(TAG, "findFileTransferByTag() not found targetTag " + targetTag);
                return null;
            } else {
                Logger.e(TAG, "findFileTransferByTag() whereToFind is " + whereToFind
                        + " targetTag is " + targetTag);
                return null;
            }
        }
    }

    @Override
    public void reloadMessages(String tag, List<Integer> messageIds) {
        Logger.w(TAG, "reloadMessages() messageIds: " + messageIds + " tag is " + tag);
        ContentResolver contentResolver = AndroidFactory.getApplicationContext()
                .getContentResolver();	
			if (tag != null) {
				reloadGroupMessage(tag, messageIds, contentResolver);
			} else {
				reloadOne2OneMessages(messageIds, contentResolver);
			}
		
    }

    public void reloadNewMessages() {
        Logger.w(TAG, "reloadNewMessages() ");
		reloadNewO2OMessages();
		reloadNewGroupMessages();
    }

	private void reloadNewO2OMessages(){
		 Logger.w(TAG, "reloadNewO2OMessages() ");
		if(RichMessagingDataProvider.getInstance() == null){
        	RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
        }
		List<Integer> messageIds = RichMessagingDataProvider.getInstance().getAllO2OMessageID();
		Logger.w(TAG, "reloadNewO2OMessages() before msgid: " + messageIds);
		Cursor cursor = RichMessagingDataProvider.getInstance().getAllO2OFtMessageID();
		try{
		if(cursor!=null){
			while(cursor.moveToNext()){
			Integer messageId = -1;
			 messageId = cursor.getInt(0);
					  if(!messageIds.contains(messageId)){
       		 messageIds.add(messageId);
					  }
       		 Logger.w(TAG, "reloadNewO2OMessages() cursor: " + messageId);
            }
		}
		Logger.w(TAG, "reloadNewO2OMessages() msgid: " + messageIds);
		ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();	
		reloadOne2OneMessages(messageIds, contentResolver);
	}
		catch(Exception e){
			e.printStackTrace();
		}
		finally {
			if(cursor!=null){
				cursor.close();
			}
		}
	}

	private void reloadNewGroupMessages(){
		Logger.w(TAG, "reloadNewGroupMessages() ");
		if(RichMessagingDataProvider.getInstance() == null){
        	RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
        }
		Cursor cursor = RichMessagingDataProvider.getInstance().getAllGroupChatMessageID();
		Cursor ftCursor = RichMessagingDataProvider.getInstance().getAllGroupFtMessageID();
		// The map retains Object&IChat&List<Participant>
    	Map<String, ArrayList<Integer>> mChatMa = new HashMap<String, ArrayList<Integer>>();
    	Logger.w(TAG, "reloadNewGroupMessages() before outside cursor: ");
		Set<String> chatSet = new HashSet<String>();
		if(cursor!=null || ftCursor != null){
		        Logger.w(TAG, "reloadNewGroupMessages() close is not null ");
		try{
        	while(cursor.moveToNext()){
        		 String chatId = "";
        		 chatId += cursor.getString(1);
        		 Logger.w(TAG, "reloadNewGroupMessages() cursor: " + chatId);
        		 chatSet.add(chatId);
             }
        	while(ftCursor.moveToNext()){
	       		 String chatId = "";
	       		 chatId += ftCursor.getString(1);
	       		 Logger.w(TAG, "reloadNewGroupMessages() cursor: " + chatId + "MessageId: " + ftCursor.getInt(0));
	       		 if(!chatSet.contains(chatId)){
	       			 chatSet.add(chatId);
	       		 }
            }
       	 	 Logger.w(TAG, "reloadNewGroupMessages() cursor0: chatSet: " + chatSet );
			 cursor.moveToFirst();
			 ftCursor.moveToFirst();
			 for (String s : chatSet) {
			 	Logger.w(TAG, "reloadNewGroupMessages() cursor1: " + s);
			 	ArrayList<SortedIds> msgIDList = new ArrayList<SortedIds>();
				try{
					if(cursor.getCount() > 0) {
			    do{
					String chatId = cursor.getString(1);
					if(chatId.equals(s)){
						Integer msgId  = cursor.getInt(0);
						Long timestamp = cursor.getLong(2);
						SortedIds newId = new SortedIds(msgId,timestamp);
						 Logger.w(TAG, "reloadNewGroupMessages() cursor2: matches" + s + ": messageId: " + msgId);
						msgIDList.add(newId);
					}
			    }while(cursor.moveToNext());
					}
				} catch(Exception e){
				    e.printStackTrace();
				}
				
				try{
				if(ftCursor.getCount() > 0) {
			    do{
					String chatId = ftCursor.getString(1);
					if(chatId.equals(s)){
						Integer msgId  = ftCursor.getInt(0);
						Long timestamp = ftCursor.getLong(2);
						SortedIds newId = new SortedIds(msgId,timestamp);
						newId.file = 1;
						 Logger.w(TAG, "reloadNewGroupMessages() cursor3: matches" + s + ": messageId: " + msgId);
						msgIDList.add(newId);
					}
			    }while(ftCursor.moveToNext());
					}
				} catch(Exception e){
				    e.printStackTrace();
				}
			    Logger.w(TAG, "reloadNewGroupMessages() msgIDList: " + msgIDList);
			    ArrayList<SortedIds> newMsgIdList = sortTimestamp(msgIDList);
			    Logger.w(TAG, "reloadNewGroupMessages() newMsgIdList: " + newMsgIdList);
			    List<Integer> msgIDList1 = new ArrayList<Integer>();
				List<Integer> booleanList1 = new ArrayList<Integer>();
			    for(int i = 0; i < newMsgIdList.size(); i ++){
			    	msgIDList1.add(newMsgIdList.get(i).msgId);
					booleanList1.add(newMsgIdList.get(i).file);
			    }
				Logger.w(TAG, "reloadNewGroupMessages() msgIDList1: " + msgIDList1 + " s1:" + s);
				ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();	
				reloadNewSingleGroupMessage(s,msgIDList1,booleanList1,contentResolver,0);
				cursor.moveToFirst();
				ftCursor.moveToFirst();
			}
			     Logger.w(TAG, "reloadNewGroupMessages() close outside cursor: ");
			}
			catch(Exception e){
				e.printStackTrace();
			}
			finally{
            cursor.close();
	            ftCursor.close();
			}
	     } 		
	     Logger.w(TAG, "reloadNewGroupMessages() exit outside cursor: ");
	     //reloadNewGroupfILEMessages();
	}
	
	public ArrayList<SortedIds> sortTimestamp(ArrayList<SortedIds> msgIDList){
		ArrayList<SortedIds> newSortedList = new ArrayList<SortedIds>();
		int loopCount = msgIDList.size();
		SortedIds temp = null;
		for (int i = 0;i < msgIDList.size();){		
			temp = msgIDList.get(i);	
			for(int j = 0; j< msgIDList.size() ;j ++){
				if(temp.timestamp > msgIDList.get(j).timestamp){
					temp = msgIDList.get(j);
				}
			}
			Logger.w(TAG, "sortTimestamp: " + temp.timestamp);
			newSortedList.add(temp);
			msgIDList.remove(temp);
		}
		return newSortedList;
	}
	
	 public  class SortedIds {
		 
	        public SortedIds(Integer msgId2, Long timestamp2) {
			// TODO Auto-generated constructor stub
	        	this.msgId = msgId2;
	        	this.timestamp = timestamp2;
				this.file = 0;
		}

			/**
	         * Content message
	         */
	        public Integer msgId = 0;
	        
	        /**
	         * System message
	         */
	        public Long timestamp = 1L;

			public Integer file = 0;
	        
	        

	}
	
	private void reloadNewGroupfILEMessages(){
		Logger.w(TAG, "reloadNewGroupfILEMessages() ");
		if(RichMessagingDataProvider.getInstance() == null){
        	RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
        }
		Cursor cursor = RichMessagingDataProvider.getInstance().getAllGroupFtMessageID();
		// The map retains Object&IChat&List<Participant>
    	Map<String, ArrayList<Integer>> mChatMa = new HashMap<String, ArrayList<Integer>>();
    	Logger.w(TAG, "reloadNewGroupfILEMessages() before outside cursor: ");
		Set<String> chatSet = new HashSet<String>();
		if(cursor!=null){
		        Logger.w(TAG, "reloadNewGroupfILEMessages() close is not null ");
        	while(cursor.moveToNext()){
        		 String chatId = "";
        		 chatId += cursor.getString(1);
        		 Logger.w(TAG, "reloadNewGroupfILEMessages() cursor: " + chatId);
        		 chatSet.add(chatId);
             }
        Logger.w(TAG, "reloadNewGroupfILEMessages() cursor0: ");
			 cursor.moveToFirst();
			 for (String s : chatSet) {
			 	Logger.w(TAG, "reloadNewGroupfILEMessages() cursor1: " + s);
			 	ArrayList<Integer> msgIDList = new ArrayList<Integer>();
			    do{
					String chatId = cursor.getString(1);
					if(chatId.equals(s)){
						Integer msgId  = cursor.getInt(0);
						 Logger.w(TAG, "reloadNewGroupfILEMessages() cursor2: matches" + s + ": messageId: " + msgId);
						msgIDList.add(msgId);
					}
			    }while(cursor.moveToNext());
			    
				ContentResolver contentResolver = AndroidFactory.getApplicationContext().getContentResolver();	
				//reloadNewSingleGroupMessage(s,msgIDList,contentResolver,1);
				cursor.moveToFirst();
			}
			     Logger.w(TAG, "reloadNewGroupfILEMessages() close outside cursor: ");
            cursor.close();
	     } 		
	     Logger.w(TAG, "reloadNewGroupfILEMessages() exit outside cursor: ");
	}

	private void reloadNewSingleGroupMessage(String tag, List<Integer> messageIds,List<Integer> booleanIds,
            ContentResolver contentResolver,int flag1) {
        Logger.w(TAG, "reloadNewSingleGroupMessage() entry: " + tag);
        //Collections.reverse(messageIds);
        int length = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER.length();
		ParcelUuid parcelUuid = null;
	    UUID uuid = UUID.randomUUID();
	    parcelUuid = new ParcelUuid(uuid);
		int i = 0;
		int j = 0;

        HashSet<Participant> participantList = new HashSet<Participant>();
        ArrayList<ReloadMessageInfo> messageList = new ArrayList<ReloadMessageInfo>();
        TreeSet<String> sessionIdList = new TreeSet<String>();
        for (Integer messageId : messageIds) {
		//	for(int i = 0;i<=1;i++){
		//		if(flag1 == 1){
		//			i = 1;
		//		}

		 Logger.w(TAG, "reloadNewSingleGroupMessage() entry: " + messageId + "file: " + booleanIds.get(i));

            ReloadMessageInfo messageInfo =
                    loadMessageFromId(sessionIdList, messageId, contentResolver,booleanIds.get(i));
			i++;
			//ReloadMessageInfo messageInfo = null;
            if (null != messageInfo) {
                Object obj = messageInfo.getMessage();
                messageList.add(messageInfo);
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    String contact = message.getContact();
                    contact = PhoneUtils.extractNumberFromUri(contact);
                    if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
                        Logger.w(TAG, "reloadNewSingleGroupMessage() the contact is not valid user "
                                + contact);
                        continue;
                    }
                    Logger.w(TAG, "reloadNewSingleGroupMessage() the contact is " + contact);
                    if (!TextUtils.isEmpty(contact)) {
                        Participant participant = new Participant(contact, contact);
                        participantList.add(participant);
                    }
                }
            }
		  //}
        }
        if(RichMessagingHistory.getInstance() == null)
        {
        	RichMessagingHistory.createInstance(AndroidFactory.getApplicationContext());
        }
		List<String> connectedParticapnts = RichMessagingHistory.getInstance().getGroupChatConnectedParticipants(tag);
		 Logger.w(TAG, "reloadNewSingleGroupMessage() connectedParticapnts: " + connectedParticapnts);
		for (String participant : connectedParticapnts) {
			if (!ContactsManager.getInstance().isRcsValidNumber(participant)) {
                    Logger.w(TAG, "reloadNewSingleGroupMessage() the participant contact is not valid user "
                            + participant);
                    continue;
             }
             if (!TextUtils.isEmpty(participant)) {
                    Participant newParticipant = new Participant(participant, participant);
                    participantList.add(newParticipant);
             }
		}
        Logger.w(TAG, "reloadNewSingleGroupMessage() the sessionIdList is " + sessionIdList);
        fillParticipantList(sessionIdList, participantList, contentResolver);
        Logger.w(TAG, "reloadNewSingleGroupMessage() participantList is " + participantList);
        if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
            Logger.w(TAG, "reloadNewSingleGroupMessage() not group");
            return;
        }
        IChat1 chat = mChatMap.get(parcelUuid);
        if (chat != null) {
            Logger.w(TAG, "reloadNewSingleGroupMessage() the chat already exist chat is " + chat);
        } else {
             Logger.w(TAG, "reloadNewSingleGroupMessage() the chat does not exist ");
            chat = new GroupChat1(this, null, new ArrayList<Participant>(participantList), parcelUuid);
            mChatMap.put(parcelUuid, chat);
            IGroupChatWindow chatWindow = ViewImpl.getInstance().addGroupChatWindow(parcelUuid,
                    ((GroupChat1) chat).getParticipantInfos());
            ((GroupChat1) chat).setChatWindow(chatWindow);
        }
        for (ReloadMessageInfo messageInfo : messageList) {
            if (null != messageInfo) {
				Object obj = messageInfo.getMessage();
				if (obj instanceof ChatMessage) {
                ChatMessage message = (ChatMessage) obj;
                int messageType = messageInfo.getMessageType();
                ((ChatImpl) chat).reloadMessage(message, messageType, messageInfo.getMessageStatus(),-1,messageInfo.getChatId());
            }else if(obj instanceof FileStruct) {
                FileStruct fileStruct = (FileStruct) obj;
                int messageType = messageInfo.getMessageType();
							 String contact = fileStruct.mRemote;
							 Logger.w(TAG, "reloadGroupMessage() file : contact = " + contact + "filesize" + fileStruct.mSize + "Thumb:" + fileStruct.mThumbnail);
							 Logger.w(TAG, "reloadGroupMessage() file : name = " + fileStruct.mName);
							((ChatImpl) chat).reloadFileTransfer(fileStruct, messageType, messageInfo.getMessageStatus());
                //((ChatImpl) chat).reloadMessage(message, messageType, -1,-1,messageInfo.getChatId());
            }
        }
     }
    }

	private void reloadGroupMessage(String tag, List<Integer> messageIds,
            ContentResolver contentResolver) {
        Logger.w(TAG, "reloadGroupMessage() entry");
        Collections.reverse(messageIds);
		int flag = 0;
        int length = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER.length();
		ParcelUuid parcelUuid = null;
		if(tag != null){
	        String realTag = tag;
	        if(tag.contains(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)){
	        	realTag = tag.substring(length);
	        }
	        parcelUuid = ParcelUuid.fromString(realTag);
		}
		else{
	        UUID uuid = UUID.randomUUID();
	        parcelUuid = new ParcelUuid(uuid);
		}

        HashSet<Participant> participantList = new HashSet<Participant>();
        ArrayList<ReloadMessageInfo> messageList = new ArrayList<ReloadMessageInfo>();
        TreeSet<String> sessionIdList = new TreeSet<String>();
        for (Integer messageId : messageIds) {
			for(int i = 0;i<=1;i++){
            ReloadMessageInfo messageInfo =
                    loadMessageFromId(sessionIdList, messageId, contentResolver,i);
			//ReloadMessageInfo messageInfo = null;
			if (null != messageInfo) {
			if(flag == 0){
				flag =1;
				 Logger.w(TAG, "reloadGroupMessage() the participant chat id is: " + messageInfo.getChatId());
                 if(RichMessagingDataProvider.getInstance() == null){
					RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
				}
				List<String> connectedParticapnts = RichMessagingDataProvider.getInstance().getGroupChatConnectedParticipants(messageInfo.getChatId());
		        Logger.w(TAG, "reloadGroupMessage() the participant contact size is: " + connectedParticapnts.size());
				
				for (String participant : connectedParticapnts) {
					Logger.w(TAG, "reloadGroupMessage() the participant contact " + participant);
					if (!ContactsManager.getInstance().isRcsValidNumber(participant)) {  
						Logger.d(TAG, "reloadGroupMessage() the participant contact is not valid user "
		                            + participant);
		                    continue;
		             }
		             if (!TextUtils.isEmpty(participant)) {
					 		 Logger.d(TAG, "reloadGroupMessage() add to list");
		                    Participant newParticipant = new Participant(participant, participant);
		                    participantList.add(newParticipant);
		             }
				}
			}
			}
			
            if (null != messageInfo) {
                Object obj = messageInfo.getMessage();
                messageList.add(messageInfo);
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    String contact = message.getContact();
                    contact = PhoneUtils.extractNumberFromUri(contact);
                    if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
                        Logger.w(TAG, "reloadGroupMessage() the contact is not valid user "
                                + contact);
                        continue;
                    }
                    Logger.w(TAG, "reloadGroupMessage() the contact is " + contact);
                    if (!TextUtils.isEmpty(contact)) {
                        Participant participant = new Participant(contact, contact);
                        participantList.add(participant);
                    }
                }
            }
		 }
        }
				
        Logger.w(TAG, "reloadGroupMessage() the sessionIdList is " + sessionIdList);
        fillParticipantList(sessionIdList, participantList, contentResolver);
        Logger.w(TAG, "reloadGroupMessage() participantList is " + participantList);
        if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
            Logger.w(TAG, "reloadGroupMessage() not group");
            return;
        }
        IChat1 chat = mChatMap.get(parcelUuid);
        if (chat != null) {
            Logger.w(TAG, "reloadGroupMessage() the chat already exist chat is " + chat);
        } else {
             Logger.w(TAG, "reloadGroupMessage() the chat does not exist ");
            chat = new GroupChat1(this, null, new ArrayList<Participant>(participantList), parcelUuid);
            mChatMap.put(parcelUuid, chat);
            IGroupChatWindow chatWindow = ViewImpl.getInstance().addGroupChatWindow(parcelUuid,
                    ((GroupChat1) chat).getParticipantInfos());
            ((GroupChat1) chat).setChatWindow(chatWindow);
        }
        for (ReloadMessageInfo messageInfo : messageList) {
            if (null != messageInfo) {
				Object obj = messageInfo.getMessage();
				if (obj instanceof ChatMessage) {
                ChatMessage message = (ChatMessage) obj;
                int messageType = messageInfo.getMessageType();
                ((ChatImpl) chat).reloadMessage(message, messageType, messageInfo.getMessageStatus(),-1,messageInfo.getChatId());
            } else if(obj instanceof FileStruct) {
                FileStruct fileStruct = (FileStruct) obj;
                int messageType = messageInfo.getMessageType();
				 String contact = fileStruct.mRemote;
				 Logger.w(TAG, "reloadGroupMessage() file : contact = " + contact + "filesize" + fileStruct.mSize + "Thumb:" + fileStruct.mThumbnail);
				 Logger.w(TAG, "reloadGroupMessage() file : name = " + fileStruct.mName);
				((ChatImpl) chat).reloadFileTransfer(fileStruct, messageType, messageInfo.getMessageStatus());
                //((ChatImpl) chat).reloadMessage(message, messageType, -1,-1,messageInfo.getChatId());
            }
        }
    }
    }


    private void fillParticipantList(TreeSet<String> sessionIdList,
            HashSet<Participant> participantList, ContentResolver contentResolver) {
        Logger.w(TAG, "fillParticipantList() entry the  sessionIdList is " + sessionIdList
                + " participantList is " + participantList);
        for (String sessionId : sessionIdList) {
            Cursor cursor = null;
            String[] selectionArg = {
                    sessionId,
                    //Integer.toString(EventsLogApi.TYPE_GROUP_CHAT_SYSTEM_MESSAGE) //TODo check this
            };
            try {
                cursor = contentResolver.query(ChatLog.Message.CONTENT_URI, null,
                        ChatLog.Message.CHAT_ID + "=? AND "
                                + ChatLog.Message.MESSAGE_TYPE + "=?", selectionArg, null);
                if (cursor != null && cursor.moveToFirst()) {
                    do {
                        String remote = cursor.getString(cursor
                                .getColumnIndex(ChatLog.Message.CONTACT_NUMBER));
                        Logger.d(TAG, "fillParticipantList() the remote is " + remote);
                        if (remote.length() - INDEXT_ONE <= INDEXT_ONE) {
                            Logger.e(TAG, "fillParticipantList() the remote is no content");
                            continue;
                        }
                        if (remote.contains(COMMA)) {
                            Logger.d(TAG, "fillParticipantList() the remote has COMMA ");
                            String subString = remote.substring(INDEXT_ONE, remote.length()
                                    - INDEXT_ONE);
                            Logger.d(TAG, "fillParticipantList() the remote is " + subString);
                            String[] contacts = subString.split(COMMA);
                            for (String contact : contacts) {
                                Logger.d(TAG, "fillParticipantList() arraylist the contact is "
                                        + contact);
                                Participant participant = new Participant(contact, contact);
                                participantList.add(participant);
                            }
                        } else if (remote.contains(SEMICOLON)) {
                            Logger.d(TAG, "fillParticipantList() the remote has SEMICOLON ");
                            String[] contacts = remote.split(SEMICOLON);
                            for (String contact : contacts) {
                                Logger.d(TAG, "fillParticipantList() arraylist the contact is "
                                        + contact);
                                Participant participant = new Participant(contact, contact);
                                participantList.add(participant);
                            }
                        } else {
                            Logger.d(TAG, "fillParticipantList() remote is single ");
                            Participant participant = new Participant(remote, remote);
                            participantList.add(participant);
                        }
                    } while (cursor.moveToNext());
                } else {
                    Logger.e(TAG, "fillParticipantList() the cursor is null");
                }
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        }
    }
    
    private void reloadOne2OneMessages(List<Integer> messageIds, ContentResolver contentResolver) {
        Logger.w(TAG, "reloadOne2OneMessages() entry");
        TreeSet<String> sessionIdList = new TreeSet<String>();
        for (Integer messageId : messageIds) {
			for(int i = 0;i<=1;i++){
			Logger.w(TAG, "reloadOne2OneMessages() kkkk messageId: " + messageId);
            ReloadMessageInfo info = loadMessageFromId(sessionIdList, messageId, contentResolver,i);
			//ReloadMessageInfo info = null;
            if (null != info) {
                Object obj = info.getMessage();
                int messageType = info.getMessageType();
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    int messageStatus = info.getMessageStatus();
                    String contact = message.getContact();
                    Logger.w(TAG, "reloadOne2OneMessages() : contact = " + contact);
					if (null != contact) {
                    if(contact.contains("sip:conference"))
                    {
                    	 Logger.v(TAG, "contact is conference , ie NULL");
                    	continue;
                    }
                    contact = PhoneUtils.extractNumberFromUri(contact);
					}
                    Logger.w(TAG, "reloadOne2OneMessages() : contact = " + contact);
                    if (!TextUtils.isEmpty(contact)) {
                        ArrayList<Participant> participantList = new ArrayList<Participant>();
                        participantList.add(new Participant(contact, contact));
                        IChat1 chat = addChat(participantList);
                        ((ChatImpl) chat).reloadMessage(message, messageType, messageStatus, info.getMessageTag(),info.getChatId());
                    }
                } else if (obj instanceof FileStruct) {
                    FileStruct fileStruct = (FileStruct) obj;
                    String contact = fileStruct.mRemote;
                    Logger.w(TAG, "reloadOne2OneMessages() file : contact = " + contact + "filesize" + fileStruct.mSize + "Thumb:" + fileStruct.mThumbnail);
					 Logger.w(TAG, "reloadOne2OneMessages() file : name = " + fileStruct.mName);
                    if (!TextUtils.isEmpty(contact)) {
                        ArrayList<Participant> participantList = new ArrayList<Participant>();
                        participantList.add(new Participant(contact, contact));
                        IChat1 chat = addChat(participantList);
                        ((ChatImpl) chat).reloadFileTransfer(fileStruct, messageType, info.getMessageStatus());
                    }
                }
            }
		 }
        }
    }

    
   private void reloadGroupMessages(List<Integer> messageIds, ContentResolver contentResolver,Object tag , IChat1 chat) {
        Logger.w(TAG, "reloadGroupMessages() entry");
        TreeSet<String> sessionIdList = new TreeSet<String>();
        String chatId;       
        for (Integer messageId : messageIds) {
			for(int i = 0;i<=1;i++){
            ReloadMessageInfo info = loadMessageFromId(sessionIdList, messageId, contentResolver,i);
           // ReloadMessageInfo info = null;
            if (null != info) {
                Object obj = info.getMessage();
                int messageType = info.getMessageType();
                if (obj instanceof ChatMessage) {
                    ChatMessage message = (ChatMessage) obj;
                    int messageStatus = info.getMessageStatus();
                         //chatId = RichMessaging.getInstance().getGroupChatId(sessionIdList.first().toString()); //TODo check this
                         chatId = sessionIdList.first().toString();
                        ((GroupChat1)chat).setmChatId(chatId);
                        ((ChatImpl) chat).reloadMessage(message, messageType, messageStatus, -1,info.getChatId());                                                                         
                    //}
                } else if (obj instanceof FileStruct) {
                    FileStruct fileStruct = (FileStruct) obj;
                    String contact = fileStruct.mRemote;
                    Logger.w(TAG, "reloadOne2OneMessages() : contact = " + contact);
                    if (!TextUtils.isEmpty(contact)) {
                        ArrayList<Participant> participantList = new ArrayList<Participant>();
                        participantList.add(new Participant(contact, contact));                       
                        ((ChatImpl) chat).reloadFileTransfer(fileStruct, messageType, info.getMessageStatus());
                    }
                }
            }
		  }
        }
        
 		if(sessionIdList.size() == 1 )
        {
           //((GroupChat1)chat).reloadSessionStack((RichMessaging.getInstance().getGroupChatStatus(sessionIdList.first().toString())) ); //TODo check this
           Logger.w(TAG, "reloadGroupMessages() : stack size is 1");
		   ((GroupChat1)chat).reloadStackApi(sessionIdList.first().toString());
    
        }
    }

    private ReloadMessageInfo loadMessageFromId(TreeSet<String> sessionIdList, Integer msgId,
            ContentResolver contentResolver,int flag) {
        Logger.w(TAG, "loadMessageFronId() msgId: " + msgId + " flag: " + flag);
        Cursor cursor = null;
		int emptyCursor = 0;
        String[] selectionArg = {msgId.toString(),"2"};
		if(flag == 0){
			Logger.w(TAG, "loadMessageFronId() wwww");
	        try {
	            cursor = contentResolver.query(ChatLog.Message.CONTENT_URI, 
	                    null, ChatLog.Message.ID + "=?"
	                    + " AND  " + ChatLog.Message.MESSAGE_TYPE + "<> ?"
	                    , selectionArg, null);
	            if (cursor.moveToFirst()) {
					emptyCursor = 1;
	                int messageDirection = cursor.getInt(cursor.getColumnIndex(ChatLog.Message.DIRECTION));
					int messageType = cursor.getInt(cursor.getColumnIndex(ChatLog.Message.MESSAGE_TYPE));
	                String messageId = cursor.getString(cursor
	                        .getColumnIndex(ChatLog.Message.MESSAGE_ID));
	                String remote = cursor.getString(cursor
	                        .getColumnIndex(ChatLog.Message.CONTACT_NUMBER));
	                byte[] bodyData = cursor.getBlob(cursor.getColumnIndex(ChatLog.Message.BODY));
					//byte[] bodyData = blobtext.getBytes(1, (int) blobtext.length());
					String text = null;
					if(bodyData != null){
						 text = new String(bodyData);
					}
	                int messagetag = cursor.getInt(cursor.getColumnIndex(ChatLog.Message.ID));
	                String chatId = cursor.getString(cursor.getColumnIndex(ChatLog.Message.CHAT_ID));
	                int messageStatus = cursor.getInt(cursor.getColumnIndex(ChatLog.Message.MESSAGE_STATUS));
	                sessionIdList.add(chatId);
	                Date date = new Date();
	                long timeStamp = cursor.getLong(cursor
	                        .getColumnIndex(ChatLog.Message.TIMESTAMP));
	                date.setTime(timeStamp);
					
	                /*if (ChatLog.Message.Direction.INCOMING == messageType
	                        || EventsLogApi.TYPE_OUTGOING_CHAT_MESSAGE == messageType
	                        || EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE == messageType
	                        || EventsLogApi.TYPE_OUTGOING_GROUP_CHAT_MESSAGE == messageType
	                        || EventsLogApi.TYPE_GROUP_CHAT_SYSTEM_MESSAGE == messageType) {*/
	                 if(!(ChatLog.Message.Type.SYSTEM == messageType && remote.equals(chatId))){  //TODo check this
	                    ChatMessage message = new ChatMessage(messageId, remote, text, date ,false,null);
	                    Logger.w(TAG, "loadMessageFronId() messageId: " + messageId + " , remote: "
	                            + remote + " , text: " + text + " , timeStamp: " + timeStamp
	                            + " , messageType: " + messageDirection + " , messageStatus: "
	                            + messageStatus);
	                    ReloadMessageInfo messageInfo =
	                            new ReloadMessageInfo(message, messageDirection, messageStatus,chatId);
	                    messageInfo.setMessageTag(messagetag);
	                    return messageInfo;
	                }
					else{
					    return null;
					}
				  }
				else {           	
	                Logger.w(TAG, "loadMessageFronId() empty message cursor");
					return null;
	             }
				}finally {
		            if (null != cursor) {
		                cursor.close();
		            }
		        }
			}
		else{
			Logger.w(TAG, "loadMessageFronId() wwww1");
			try {
				 String[] ftSelectionArg = {msgId.toString()};
				 cursor = contentResolver.query(FileTransferLog.CONTENT_URI, 
                    null, FileTransferLog.ID + "=?"
                    , ftSelectionArg, null);
				  if (cursor.moveToFirst()) {
				  	emptyCursor = 1;
				  int messageDirection = cursor.getInt(cursor.getColumnIndex(FileTransferLog.DIRECTION)); //TODo check this
				  	/*
				 if (EventsLogApi.TYPE_INCOMING_FILE_TRANSFER == messageType
                        || EventsLogApi.TYPE_OUTGOING_FILE_TRANSFER == messageType) {*/
                  if(messageDirection == 0 || messageDirection == 1){ //TODo check this
                    String fileName = cursor.getString(cursor
                            .getColumnIndex(FileTransferLog.FILENAME));
					//String fileThumb = cursor.getString(cursor.getColumnIndex(FileTransferLog.FILEICON));
					String fileThumb = null;
                    long fileSize = cursor.getLong(cursor
                            .getColumnIndex(FileTransferLog.FILESIZE));
					 String remote = cursor.getString(cursor
                        .getColumnIndex(FileTransferLog.CONTACT_NUMBER));
					int messageStatus =
                        cursor.getInt(cursor.getColumnIndex(FileTransferLog.STATE));
					 String messagetag = cursor.getString(cursor.getColumnIndex(FileTransferLog.FT_ID));
					
					 Date date = new Date();
               		 long timeStamp = cursor.getLong(cursor
                        .getColumnIndex(ChatLog.Message.TIMESTAMP));
                	date.setTime(timeStamp);
                	File filePath = new File(fileName);
                	String fName = filePath.getName();
					int reload = 0;
					if(messageStatus == FileTransfer.State.DISPLAYED){
						reload = 1;
					} else {
					    Logger.d(TAG, "add mReceivedAfterReloadFt Id: " + messagetag);
					    mReceivedAfterReloadFt.add(messagetag);
					}
					Logger.w(TAG, "loadMessageFronId() file messageId: " + messagetag + " , remote: " + remote +"filsize" + fileSize + "Thumb:" + fileThumb + "fName =" + fName + "reload: "+reload );
                    FileStruct fileStruct = new FileStruct(fileName, fName, fileSize, messagetag,
                            date, remote , fileThumb,reload);
                    ReloadMessageInfo fileInfo = new ReloadMessageInfo(fileStruct, messageDirection, messageStatus);
                    fileInfo.setMessageTag(0);
                    return fileInfo;
                  }
				  else{
				  	return null;
				  }
	            } else {
	            	if(emptyCursor == 0){
		            	Intent it = new Intent();
		                it.setAction(PluginUtils.ACTION_REALOD_FAILED);
		                it.putExtra("ipMsgId",msgId);
		                //AndroidFactory.getApplicationContext().sendStickyBroadcast(it);
						Logger.w(TAG, "loadMessageFronId() empty FT cursor, removing from mms DB");
	            	}
	                Logger.w(TAG, "loadMessageFronId() empty FT cursor");
	                return null;
	            }
	        } finally {
	            if (null != cursor) {
	                cursor.close();
	            }
	        }
		}
    }
    
    @Override
    public void closeAllChat() {
        Logger.w(TAG, "closeAllChat()");
        Collection<IChat1> chatSet = mChatMap.values();
        List<Object> tagList = new ArrayList<Object>();
        for (IChat1 iChat : chatSet) {
            tagList.add(((ChatImpl) iChat).getChatTag());
        }
        for (Object object : tagList) {
            removeChat(object);
        }
        //clear all chat history
        ContentResolver contentResolver =
                ApiManager.getInstance().getContext().getContentResolver();
        contentResolver.delete(ChatLog.Message.CONTENT_URI, null, null);
		contentResolver.delete(FileTransferLog.CONTENT_URI, null, null);
    }

    @Override
    public void closeAllChatFromMemory() {
        Logger.w(TAG, "closeAllChat()");
        Collection<IChat1> chatSet = mChatMap.values();
        List<Object> tagList = new ArrayList<Object>();
        for (IChat1 iChat : chatSet) {
            tagList.add(((ChatImpl) iChat).getChatTag());
        }
        for (Object object : tagList) {
            removeChat(object);
        }
        
    }


    /**
     * Get group chat with chat id.
     * 
     * @param chatId The chat id.
     * @return The group chat.
     */
    public IChat1 getGroupChat(String chatId) {
        Logger.w(TAG, "getGroupChat() entry, chatId: " + chatId);
        Collection<IChat1> chats = mChatMap.values();
        IChat1 result = null;
        for (IChat1 chat : chats) {
            if (chat instanceof GroupChat1) {
                String id = ((GroupChat1) chat).getChatId();
                if (id != null && id.equals(chatId)) {
                    result = chat;
                    break;
                }
            }
        }
        Logger.w(TAG, "getGroupChat() exit, result: " + result);
        return result;
    }
    
    /**
     * Used to reload message information
     */
    private static class ReloadMessageInfo {
        private Object mMessage;
        private int mMessageType;
        private int mMessageStatus;
        private int messageTag;
		private String mChatId = null;

        public int getMessageTag() {
			return messageTag;
		}

		public void setMessageTag(int messageTag) {
			this.messageTag = messageTag;
		}

        /**
         * Constructor
         * 
         * @param message The message
         * @param type The message type
         * @param status The message status
         */
        public ReloadMessageInfo(Object message, int type, int status) {
            this.mMessage = message;
            this.mMessageType = type;
            this.mMessageStatus = status;
        }

		 /**
         * Constructor
         * 
         * @param message The message
         * @param type The message type
         * @param status The message status
         */
        public ReloadMessageInfo(Object message, int type, int status,String chatId) {
            this.mMessage = message;
            this.mMessageType = type;
            this.mMessageStatus = status;
			this.mChatId = chatId;
        }

        /**
         * Constructor
         * 
         * @param message The message
         * @param type The message type
         */
        public ReloadMessageInfo(Object message, int type) {
            this.mMessage = message;
            this.mMessageType = type;
        }

        /**
         * Get the message
         * 
         * @return The message
         */
        public Object getMessage() {
            return mMessage;
        }

        /**
         * Get the message type
         * 
         * @return The message type
         */
        public int getMessageType() {
            return mMessageType;
        }

        /**
         * Get the message status
         * 
         * @return The message status
         */
        public int getMessageStatus() {
            return mMessageStatus;
        }

		 /**
         * Get the message type
         * 
         * @return The message type
         */
        public String getChatId() {
            return mChatId;
        }
    }
}
