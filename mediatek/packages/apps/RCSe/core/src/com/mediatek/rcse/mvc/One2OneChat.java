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

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.HashSet;
import java.util.Set;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.HashMap;

import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.capability.Capabilities;
import org.gsma.joyn.capability.CapabilitiesListener;
import org.gsma.joyn.capability.CapabilityService;
import org.gsma.joyn.chat.Chat;
import org.gsma.joyn.chat.ChatListener;
import org.gsma.joyn.chat.ChatLog;
import org.gsma.joyn.chat.ChatMessage;
import org.gsma.joyn.chat.ChatService;
import org.gsma.joyn.chat.ChatServiceConfiguration;
import org.gsma.joyn.chat.GeolocMessage;
import org.gsma.joyn.chat.GroupChat;
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
import org.gsma.joyn.ft.FileTransferService;
import org.gsma.joyn.ft.FileTransferServiceConfiguration;
import org.gsma.joyn.ft.IFileTransferListener;
import org.gsma.joyn.ft.INewFileTransferListener;
import org.gsma.joyn.ft.NewFileTransferListener;


import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.widget.Toast;

import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.interfaces.ChatModel.IChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageReceived;
import com.mediatek.rcse.mvc.ModelImpl.ChatMessageSent;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ModelImpl.SentFileTransfer;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.provider.UnregMessageProvider;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettings.SelfCapabilitiesChangedListener;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.mediatek.rcse.service.Utils;



/**
 * This class is the implementation of a 1-2-1 chat model
 */
public class One2OneChat extends ChatImpl  implements SelfCapabilitiesChangedListener{
    public static final String TAG = "M0CF One2OneChat";
  /*  public static final int FILETRANSFER_ENABLE_OK = 0;
    public static final int FILETRANSFER_DISABLE_REASON_NOT_REGISTER = 1;
    public static final int FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED = 2;
    public static final int FILETRANSFER_DISABLE_REASON_REMOTE = 3;*/

    public static final int LOAD_DEFAULT = 20;
    public static final int LOAD_ZERO_SHOW_HEADER = 0;
    public static final int TYPE_CHAT_SYSTEM_MESSAGE = ChatLog.Message.Type.SYSTEM;
    public static final int TYPE_GROUP_CHAT_SYSTEM_MESSAGE = ChatLog.Message.Type.SYSTEM;
    public static final int STATUS_TERMINATED = ChatLog.Message.Status.Content.FAILED;
   // com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status fileTransferStatus = com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING;

    private CopyOnWriteArrayList<IChatMessage> mAllMessages = new CopyOnWriteArrayList<IChatMessage>();
   
    public One2OneChat(ModelImpl modelImpl, IOne2OneChatWindow chatWindow, Participant participant,
            Object tag) {
        modelImpl.super(tag);
        Logger.d(TAG, "One2OneChat() entry with modelImpl is " + modelImpl + " chatWindow is "
                + chatWindow + " participant is " + participant + " tag is " + tag);
        mChatWindow = chatWindow;
        mParticipant = participant;
        mFileTransferController = new FileTransferController();
        RcsSettings rcsSetting = RcsSettings.getInstance();
        if (rcsSetting == null) {
            Logger.d(TAG, "One2OneChat() the rcsSetting is null ");
            Context context = ApiManager.getInstance().getContext();
            RcsSettings.createInstance(context);
            rcsSetting = RcsSettings.getInstance();
        }
        rcsSetting.registerSelfCapabilitiesListener(this);
    }

	int capabilityTimeoutValue = RcsSettings.getInstance().getMessagingCapbailitiesValidiy();

    /**
     * Set chat window for this chat.
     * 
     * @param chatWindow The chat window to be set.
     */
    public void setChatWindow(IChatWindow chatWindow) {
        Logger.d(TAG, "setChatWindow entry: mChatWindow = " + mChatWindow
                + ", chatWindow = " + chatWindow);
        super.setChatWindow(chatWindow);
    }

    protected void checkAllCapability() {
        Logger.d(TAG, "M0CFF checkAllCapability() entry: mFileTransferController = "
                + mFileTransferController + ", mParticipant = " + mParticipant);
        final RegistrationApi registrationApi = ApiManager.getInstance().getRegistrationApi();
		try{
        if (mFileTransferController != null) {
            if (registrationApi != null && registrationApi.isRegistered()) {
                Logger.d(TAG, "M0CFF checkAllCapability() already registered");
                mFileTransferController.setRegistrationStatus(true);
                CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
                Logger.v(TAG, "M0CFF checkAllCapability() capabilityApi = " + capabilityApi);
                if (capabilityApi != null) {
                    Capabilities myCapablities = capabilityApi.getMyCapabilities();
                    if (null != mParticipant) {
                        String contact = mParticipant.getContact();
						Capabilities remoteCapablities = null;
						try{
                        	 remoteCapablities = capabilityApi.getContactCapabilities(contact);
							// capabilityApi.requestContactCapabilities(contact);
						}
						catch(JoynServiceException e){
							Logger.d(TAG, "M0CFF checkAllCapability() getContactCapabilities JoynServiceException");
						}						
                        Logger.v(TAG, "M0CFF checkAllCapability() myCapablities = "
                                + myCapablities + ",remoteCapablities = "
                                + remoteCapablities);
                        if (myCapablities != null) {
                            mFileTransferController.setLocalFtCapability(false);
                            if (myCapablities.isFileTransferSupported()) {
                                Logger.d(TAG,
                                        "M0CFF checkAllCapability() my capability support filetransfer");
                                mFileTransferController
                                        .setLocalFtCapability(true);
                                if (remoteCapablities != null) {
                                    mFileTransferController
                                            .setRemoteFtCapability(false);
                                    if (remoteCapablities
                                            .isFileTransferSupported()) {
                                        Logger.d(TAG,
                                                "M0CFF checkAllCapability() participant support filetransfer ");
                                        mFileTransferController
                                                .setRemoteFtCapability(true);
                                    }
                                }
                            }
                        }
                    }
				 }
                }
            } else {
                mFileTransferController.setRegistrationStatus(false);
            }
        }
		catch(JoynServiceException e){
			e.printStackTrace();
		}
		catch(NullPointerException e){
			e.printStackTrace();
		}
        if (mFileTransferController != null) {
            mFileTransferController.controlFileTransferIconStatus();
        }
    }

    /**
     * Return the IChatWindow
     * 
     * @return IChatWindow
     */
    public IChatWindow getChatWindow() {
        return mChatWindow;
    }

    /**
     * Return the whole messages
     * 
     * @return The message list
     */
    public CopyOnWriteArrayList<IChatMessage> getAllMessages() {
        return mAllMessages;
    }

    /**
     * Clear the 1-2-1 chat history of a particular participant.
     * 
     * @return True if success, else False.
     */
    public boolean clearHistoryForContact() {
        Logger.d(TAG, "clearHistoryForContact() entry: mParticipant = " + mParticipant);
        ApiManager instance = ApiManager.getInstance();
        if (instance == null) {
            Logger.d(TAG, "clearHistoryForContact(), the ApiManager instance is null");
            return false;
        }
        Context context = instance.getContext();
        if (context == null) {
            Logger.d(TAG, "clearHistoryForContact(), the Context instance is null");
            return false;
        }
            if (mParticipant != null) {
                mWorkerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        String contact = mParticipant.getContact();
                        Logger.d(TAG, "mParticipant.getContact() is " + contact);
						if(RichMessagingDataProvider.getInstance() != null){
							RichMessagingDataProvider.createInstance(AndroidFactory.getApplicationContext());
						}
                        RichMessagingDataProvider.getInstance().deleteMessagingLogForContact(contact);
						RichMessagingDataProvider.getInstance().deleteFtLogForContact(contact);
                    }
                });
                clearChatWindowAndList();
                mChatWindow.addLoadHistoryHeader(false);
                Logger.d(TAG, "clearHistoryForContact() exit with true");
                return true;
            }
        Logger.d(TAG, "clearHistoryForContact() exit with false");
        return false;
    }

    @Override
    protected synchronized void onDestroy() {
        mSentMessageManager.onChatDestroy();
        mMessageDepot.removeUnregisteredMessage();
        mAllMessages.clear();
		try{
        ContactsListManager.getInstance().setStrangerList(mParticipant.getContact(), false);
        RcsSettings rcsSetting = RcsSettings.getInstance();
        Logger.v(TAG, "onDestroy(): rcsSetting = " + rcsSetting);
        if (rcsSetting != null) {
            rcsSetting.unregisterSelfCapabilitiesListener(this);
        }
        ChatService chatService = ApiManager.getInstance().getChatApi();
        if (null != chatService) {
                Chat chatImpl = chatService.getChat(mParticipant.getContact());
                if (null != chatImpl) {
                        //String sessionId = chatSession.getSessionID();
                        //Chat chatSession = ChatMessage.Stub.asInterface(binder);
                       // int state = chatSession.getSessionState();
                       /* switch (state) {
                            case SessionState.ESTABLISHED:
                            case SessionState.PENDING:
                                Logger.d(TAG, "onDestroy() sessionId:" + sessionId
                                        + " is in state " + state + " will be cancelled.");
                                chatSession.cancelSession();
                                break;
                            case SessionState.CANCELLED:
                            case SessionState.TERMINATED:
                            case SessionState.UNKNOWN:
                                Logger.d(TAG, "onDestroy() sessionId:" + sessionId
                                        + " is in state " + state + " has been cancelled.");
                                break;
                            default:
                                break;
                        }*/
                }
	        } else {
	            Logger.e(TAG, "getSessionByState() messagingApi is null");
	        }
		}
		catch (JoynServiceException e) {
                e.printStackTrace();
            }
		catch (NullPointerException e) {
                e.printStackTrace();
        }
        super.onDestroy();
    }

    protected boolean getRegistrationState() {
        Logger.d(TAG, "One2OneChat-getRegistrationState()");
        ApiManager apiManager = ApiManager.getInstance();
        if (apiManager == null) {
            Logger.d(TAG, "getRegistrationState()-The apiManager is null");
            return false;
        } else {
            RegistrationApi registrationApi = apiManager.getRegistrationApi();
            if (registrationApi == null) {
                Logger.d(TAG, "getRegistrationState()-The registrationApi is null");
                return false;
            } else {
                return registrationApi.isRegistered();
            }
        }
    }
/*
    private IChatSession getSessionByState(int sessionState) {
        Logger.d(TAG, "getSessionByState() entry, participant is " + mParticipant
                + " sessionState is " + sessionState);
        MessagingApi messagingApi = ApiManager.getInstance().getChatApi();
        Logger.v(TAG, "onDestroy(): messagingApi = " + messagingApi);
        if (null != messagingApi && null != mParticipant) {
            List<IBinder> binders = null;
            try {
                binders = messagingApi.getChatSessionsWith(mParticipant.getContact());
                Logger.v(TAG, "onDestroy(): binders = " + binders);
                if (null != binders) {
                    for (IBinder binder : binders) {
                        IChatSession chatSession = IChatSession.Stub.asInterface(binder);
                        if (chatSession.isStoreAndForward() || chatSession.isGroupChat()) {
                            Logger.d(TAG, "getSessionByState() S&F session found, skip it");
                            continue;
                        }
                        if (SessionState.PENDING != sessionState) {
                            if (sessionState == chatSession.getSessionState()) {
                                Logger.d(TAG, "getSessionByState() session with state "
                                        + sessionState + " found, session id:"
                                        + chatSession.getSessionID() + " chat id:"
                                        + chatSession.getChatID() + " is group:"
                                        + chatSession.isGroupChat());
                                return chatSession;
                            }
                        } else {
                            boolean isInComingSession = chatSession.isInComing();
                            if (isInComingSession) {
                                Logger.d(TAG, "getSessionByState() incoming pending session found,"
                                        + " session id:" + chatSession.getSessionID() + " chat id:"
                                        + chatSession.getChatID());
                                return chatSession;
                            }
                        }
                    }
                }
            } catch (JoynServiceException e) {
                e.printStackTrace();
                return null;
            } catch (JoynServiceNotAvailableException e) {
                e.printStackTrace();
                return null;
            }
        }
        return null;
    }
*/
    private ChatMessage sendMessageViaSession(Chat o2oChatImpl, String content) {
        try {
            String messageId = o2oChatImpl.sendMessage(content);
            if (!TextUtils.isEmpty(messageId) && mParticipant != null) {
                return new ChatMessage(messageId, mParticipant.getContact(), content, new Date(),true,RcsSettings.getInstance().getJoynUserAlias());
            } else {
                Logger.e(TAG, "mParticipant is null or messageId is empty, " + "mParticipant is " + mParticipant
                        + " messageId is " + messageId);
                return null;
            }
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private ChatMessage sendMessageViaInvite(String content) {
		try {
        ChatService chatService = ApiManager.getInstance().getChatApi();
		CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
        if (null != chatService && null != mParticipant) {
			if(isCapbilityExpired(mParticipant.getContact())){
				Logger.v(TAG, "ABC sendMessageViaInvite() checkAllCapability");
				checkAllCapability();
			}
				One2OneChatListener newO2OChatListener = new One2OneChatListener();
                Chat o2oChatImpl = chatService.openSingleChat(mParticipant.getContact(),newO2OChatListener);
				newO2OChatListener.setChatImpl(o2oChatImpl);
                //o2oChatImpl.addSessionListener(new One2OneChatListener(o2oChatImpl));
                String messageId = null;
                messageId =  o2oChatImpl.sendMessage(content); //TODo To return first message
                return new ChatMessage(messageId ,mParticipant.getContact(),content,new Date() , true,RcsSettings.getInstance().getJoynUserAlias());
	        } else {
	            Logger.e(TAG, "sendMessageViaInvite() messagingApi is " + chatService
	                    + " ,mParticipant is " + mParticipant);
	            return null;
	        }
		}
		 catch (JoynServiceException e) {
                e.printStackTrace();
                return null;
            } catch (NullPointerException e) {
                e.printStackTrace();
                return null;
            }
    }

    private ChatMessage sendRegisteredMessage(String content) {
		Chat o2oChatImpl = null;
		try{
			ChatService chatService = ApiManager.getInstance().getChatApi();
        	 o2oChatImpl = chatService.getChat(mParticipant.getContact());
		}
		catch(JoynServiceException e){
			e.printStackTrace();
		}
		catch(NullPointerException e){
			e.printStackTrace();
		}
        if (null != o2oChatImpl) {
			Logger.d(TAG, "ABC sendRegisteredMessage() ChatIMpl is not null");
            return sendMessageViaSession(o2oChatImpl, content);
        } else {
            /**
             * Changed to notify the user that the sip invite is failed.@{
             */
            ChatMessage message = null;
			Logger.d(TAG, "ABC sendRegisteredMessage() ChatIMpl is  null");
            message = sendMessageViaInvite(content);
            
            /*change to check null pointer */
            if(message!=null){
            //message.setAsInviteMessage(true); //TOD0 check this
            } 
            return message;
            /**
             * @}
             */
        }
    }

    // This is a helper method for auto testing
    protected void sendMessage(final String content, Integer messageTag) {
        sendMessage(content, messageTag.intValue());
    }

    @Override
    public void sendMessage(final String content, final int messageTag) {
        Logger.w(TAG, "ABC sendMessage() The content is " + content);
        if (messageTag == 0) {
            // messageTag is 0 means will send this meesage via sms
            ChatMessage messageViaSms = new ChatMessage(null, mParticipant.getContact(), content, new Date(),false,RcsSettings.getInstance().getJoynUserAlias());
            if (mChatWindow != null) {
                mChatWindow.addSentMessage(messageViaSms, messageTag);
            }
            return;
        }
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                ChatMessage message = null;
                boolean isSuccess = false;
				String messageId = null;
                if (getRegistrationState()) {
                    Logger.w(TAG, "sendMessage() send registered message");
                    message = sendRegisteredMessage(content);
                    if (null != message) {
						//mMessageDepot.storeMessage(content, messageTag);
						Logger.w(TAG, "sendMessage() send registered pass");
                        isSuccess = true;
						if(messageDeleteTag == messageTag){
							Logger.w(TAG, "sendMessage() messageSent TRUE" + messageTag);
							messageSent = 1;
						}
                    } else {
                        Logger.w(TAG, "sendMessage() message is null");
                        message = mMessageDepot.storeMessage(content, messageTag);
                    }
                } else {
                    // Store the unregistered sending messages
                    Logger.w(TAG, "sendMessage() store unregistered message");
                    message = mMessageDepot.storeMessage(content, messageTag);
                }

				
				if(message != null){
					messageId = message.getId();
					Logger.d(TAG, "sendMessage() message id is"+messageId);
				}

                mComposingManager.messageWasSent();
                if (mChatWindow != null) {
                    ISentChatMessage sentMessage = mMessageDepot.checkIsResend(messageTag);
                    if (null == sentMessage) {
						Logger.w(TAG, "sendMessage() new message");
                        // This is a new message, not a resent one
                        sentMessage = mChatWindow.addSentMessage(message, messageTag);
                        mAllMessages.add(new ChatMessageSent(message));
                    }
                    /**
                     * Added to notify the user that the sip invite is failed.@{
                     */
                  //  if (message.isInviteMessage()) {
					//	Logger.w(TAG, "sendMessage() isInviteMessage");  //TODo check this
                   //     mLastISentChatMessageList.add(sentMessage);
                   // }
                    /**
                     * @}
                     */

                    Logger.w(TAG, "sendMessage() sentMessage: " + sentMessage + " isSuccess: "
                            + isSuccess);

                    if (null != sentMessage) {
                        if (isSuccess) {
                            if (sentMessage instanceof SentChatMessage) {
                                Logger.w(TAG, "sendMessage(), is SentMessage, update message");
                                ((SentChatMessage) sentMessage).updateMessage(message);
                            }
							mMessageDepot.updateStoredMessage(messageTag, sentMessage);
                            mSentMessageManager.onMessageSent(sentMessage);
                            sentMessage.updateStatus(Status.SENDING);
                        } else {
                        	Logger.w(TAG, "sendMessage() status failed");
				//mMessageDepot.addMessageTag(messageTag,messageId);
                            mMessageDepot.updateStoredMessage(messageTag, sentMessage);
                            sentMessage.updateStatus(Status.FAILED);
                        }
                    }
                }
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.w(TAG, "sendMessage() run on worker thread");
            worker.run();
        } else {
            Logger.w(TAG, "sendMessage() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }

    @Override
    protected boolean setIsComposing(boolean isComposing) {
    	try {
        ChatService chatService = ApiManager.getInstance().getChatApi();
		 Logger.w(TAG, "setIsComposing() isComposing= " + isComposing);
        if(chatService != null){
	        Chat o2oChatImpl = chatService.getChat(mParticipant.getContact());
		    if (null != o2oChatImpl) {
		        o2oChatImpl.sendIsComposingEvent(isComposing);
		        return true;
		    } 
		}
    	}
		catch (JoynServiceException e) {
            e.printStackTrace();
         }
		catch (NullPointerException e) {
	        e.printStackTrace();
        }
        return false;
    }

    private void onChatListenerDestroy(ChatListener chatListener) {
		ChatService chatService = ApiManager.getInstance().getChatApi();
		try{
        	Chat o2oChatImpl = chatService.getChat(mParticipant.getContact());	
	        Logger.d(TAG, "onChatListenerDestroy() entry: o2oChatImpl = " + o2oChatImpl);
	        if (null == o2oChatImpl) {
	            Logger.d(TAG, "onChatListenerDestroy() o2oChatImpl is null");
	            ((IOne2OneChatWindow) mChatWindow).setIsComposing(false);
	            ((IOne2OneChatWindow) mChatWindow).setRemoteOfflineReminder(false);
	        }
		}
		catch(JoynServiceException e){
		}
		catch (NullPointerException e) {
	        e.printStackTrace();
	    }
    }

    /**
     * This method is normally to handle the received delivery notifications via
     * SIP
     * 
     * @param messageId The message id of the delivery notification
     * @param status The type of the delivery notification
     */
    public void onMessageDelivered(final String messageId, final String status, final long timeStamp) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                mSentMessageManager.onMessageDelivered(messageId, formatStatus(status), timeStamp);
            }
        };
        Thread currentThread = Thread.currentThread();
        if (currentThread.equals(mWorkerThread)) {
            Logger.v(TAG, "onMessageDelivered() run on worker thread");
            worker.run();
        } else {
            Logger.v(TAG, "onMessageDelivered() post to worker thread");
            mWorkerHandler.post(worker);
        }
    }

    private void onReceiveMessage(ChatMessage message) {
        String messageInfo = "messageId:" + message.getId() + " message Text:" + message.getMessage();
        Logger.d(TAG, "onReceiveMessage() entry, " + messageInfo);
        Context context = AndroidFactory.getApplicationContext();
        if (message.isDisplayedReportRequested()) {
            Logger.d(TAG, "onReceiveMessage() DisplayedRequested is true, " + messageInfo);
            if (!mIsInBackground) {
                /*
                 * Mark the received message as displayed when the chat window
                 * is not in background
                 */
                markMessageAsDisplayed(message);
            } else {
                /*
                 * Save the message and will mark it as displayed when the chat
                 * screen resumes
                 */
                One2OneChat.this.addUnreadMessage(message);
                /*
                 * Showing notification of a new incoming message when the chat
                 * window is in background
                 */
                RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, message,
                        null, 0);
            }
        } else {
            if (!mIsInBackground) {
                /*
                 * Mark the received message as read if the chat window is not
                 * in background
                 */
                markMessageAsRead(message);
            } else {
                /*
                 * Save the message and will mark it as read when the activity
                 * resumes
                 */
                One2OneChat.this.addUnreadMessage(message);
                /*
                 * Showing notification of a new incoming message when the chat
                 * window is in background
                 */
                RcsNotification.getInstance().onReceiveMessageInBackground(context, mTag, message,
                        null, 0);
            }
        }
        mChatWindow.addReceivedMessage(message, !mIsInBackground);
        mAllMessages.add(new ChatMessageReceived(message));
        mSentMessageManager.markSendingMessagesDisplayed();
    }

    @Override
    protected void markMessageAsDisplayed(ChatMessage msg) {
    	try{
        String messageInfo =
                "message id:" + msg.getId() + " message text:" + msg.getMessage();
        Logger.d(TAG, "markMessageAsDisplayed() entry, " + messageInfo);
        if (AndroidFactory.getApplicationContext() == null) {
            Logger.e(TAG, "getApplicationContext() return null");
            return;
        }
        SharedPreferences settings =
                PreferenceManager.getDefaultSharedPreferences(AndroidFactory
                        .getApplicationContext());
        boolean isSendReadReceiptChecked =
                settings.getBoolean(SettingsFragment.RCS_SEND_READ_RECEIPT, true);
        if (!isSendReadReceiptChecked) {
            Logger.d(TAG, "markMessageAsDisplayed() isSendReadReceiptChecked is false, "
                    + messageInfo);
            return;
        }
		ChatService chatService = ApiManager.getInstance().getChatApi();
        Chat o2oChatImpl = chatService.getChat(mParticipant.getContact());
		if (o2oChatImpl == null) {
			try {
				Set<Chat> totalChats = null;
				if(o2oChatImpl == null){
					totalChats = chatService.getChats();
					Logger.e(TAG, "aaa1 getChatSession size: " + totalChats.size());
					 Logger.d(TAG, "The1 chat session is null3 : " + totalChats.size());
				}
				for(Chat setElement: totalChats) {
			        if(setElement.getRemoteContact().equals(mParticipant.getContact())) {
						Logger.e(TAG, "Get1 chat session finally");
			            //might work or might throw exception, Java calls it indefined behaviour:
			            o2oChatImpl = setElement;
						break;
			        } 
		    	}
			}catch (JoynServiceException e) {
					Logger.e(TAG, "Get1 chat session xyz");
		            e.printStackTrace();
			}	
		 }

		if (o2oChatImpl == null) {
			try{
				Logger.w(TAG, "openSingleChat xyz0 " + mParticipant.getContact());
				One2OneChatListener newO2OChatListener = new One2OneChatListener();				
				o2oChatImpl = chatService.openSingleChat(mParticipant.getContact(),newO2OChatListener);
				newO2OChatListener.setChatImpl(o2oChatImpl);
			}catch (JoynServiceException e) {
					Logger.e(TAG, "openSingleChat xyz1");
		            e.printStackTrace();
			}	
		}
		
		 if (null != o2oChatImpl) {
	        Logger.d(TAG, "markMessageAsDisplayed() chat found, " + messageInfo);
	            Logger.d(TAG, "markMessageAsDisplayed()  " + messageInfo);
	                o2oChatImpl.sendDisplayedDeliveryReport(msg.getId());      
	        } 
		}
		catch (JoynServiceException e) {
            e.printStackTrace();
        }
		catch (NullPointerException e) {
	        e.printStackTrace();
	    }
		 //TODO if chatImpl is null
    }

    private class One2OneChatListener extends ChatListener {

        private static final String TAG = "M0CF One2OneChatListener";

        private Chat currentChatImpl = null;

        private String mSessionId = null;
        private String mCallId = null;

        public One2OneChatListener() {           
            if (null != currentChatImpl) {
				Logger.d(TAG, "Constructor() session:" + mSessionId + " Call Id: " + mCallId);
               /* try {
                    //mSessionId = currentChatImpl.getSessionID();
                   // mCallId = currentChatImpl.getChatID();
                    Logger.d(TAG, "Constructor() session:" + mSessionId + " Call Id: " + mCallId);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }*/
            }
        }

		public void setChatImpl(Chat o2oChatImpl){
			currentChatImpl = o2oChatImpl;
		}

        private void destroySelf() {
            Logger.d(TAG, "destroySelf entry : mCurrentSession = " + currentChatImpl);
            if (null != currentChatImpl) {
                try {
                    Logger.d(TAG, "destroySelf() session id is " + mSessionId + " Call Id: "
                            + mCallId);
                    currentChatImpl.removeEventListener(this);
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }
                currentChatImpl = null;
            }
            onChatListenerDestroy(this);
        }

		/**
	 * Callback called when a new message has been received
	 * 
	 * @param message Chat message
	 * @see ChatMessage
	 */
	public  void onNewMessage(final ChatMessage message){
	 	Logger.d(TAG, "onNewMessage()   message id:" + message.getId() + " message text:"
                            + message.getMessage());
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    onReceiveMessage(message);
                }
            });
		}
	
	/**
	 * Callback called when a new geoloc has been received
	 * 
	 * @param message Geoloc message
	 * @see GeolocMessage
	 */
	public  void onNewGeoloc(GeolocMessage message){
		}

	/**
	 * Callback called when a message has been delivered to the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageDelivered(final String msgId){
		Logger.d(TAG, "onReportMessageDelivered()  session:" + msgId );
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.DELIVERED , 0);
                }
            });
		}

	/**
	 * Callback called when a message has been displayed by the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageDisplayed(final String msgId){
	Logger.d(TAG, "onReportMessageDisplayed()  session:" + msgId );
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.DISPLAYED , 0);
                }
            });
		}

	/**
	 * Callback called when a message has failed to be delivered to the remote
	 * 
	 * @param msgId Message ID
	 */
	public  void onReportMessageFailed(final String msgId){
	Logger.d(TAG, "onReportMessageFailed()  session:" + msgId );
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSentMessageManager.onMessageDelivered(msgId,ISentChatMessage.Status.FAILED , 0);
                }
            });
		}

	/**
	 * Callback called when an Is-composing event has been received. If the
	 * remote is typing a message the status is set to true, else it is false.
	 * 
	 * @param status Is-composing status
	 */
	public  void onComposingEvent(final boolean status){
	 	Logger.d(TAG, "onComposingEvent()  session: the status is " + status);
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    ((IOne2OneChatWindow) mChatWindow).setIsComposing(status);
                }
            });
		}

       
   }

    private static ISentChatMessage.Status formatStatus(String s) {
        Logger.d(TAG, "formatStatus entry with status: " + s);
        ISentChatMessage.Status status = ISentChatMessage.Status.SENDING;
        if (s == null) {
            return status;
        }
        if (s.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            status = ISentChatMessage.Status.DELIVERED;
        } else if (s.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
            status = ISentChatMessage.Status.DISPLAYED;
        } else {
            status = ISentChatMessage.Status.FAILED;
        }
        Logger.d(TAG, "formatStatus entry exit");
        return status;
    }

    /**
     * Judge whether participants is duplicated.
     * 
     * @param participants The participants to be compared.
     * @return True, if participants is duplicated, else false.
     */
    public boolean isDuplicated(Participant participant) {
        return mParticipant.equals(participant);
    }

    @Override
    protected void checkCapabilities() {
        checkAllCapability();
    }

    @Override
    protected void queryCapabilities() {
        checkAllCapability();
    }

	public void setInviteContact(String contact){
	}

	public void handleInvitation(GroupChat o2oChatImpl, ArrayList<IChatMessage> messages, boolean isAutoAccept) {
		}

    public void handleInvitation(Chat o2oChatImpl, ArrayList<IChatMessage> messages, boolean isAutoAccept) {
        Logger.v(TAG, "handleInvitation entry, tag: " + mTag + " Participant: " + mParticipant);
        if (o2oChatImpl == null) {
            return;
        }

        try {
			One2OneChatListener chatListener = new One2OneChatListener();
			chatListener.setChatImpl(o2oChatImpl);
            o2oChatImpl.addEventListener(chatListener);
        } catch (JoynServiceException e) {
            Logger.d(TAG, "chatSession acceptSession or addSessionListener fail");
            e.printStackTrace();
        }

        if (messages == null) {
            Logger.v(TAG, "handleInvitation messages is null");
            return;
        }
        int size = messages.size();
        Logger.d(TAG, "handleInvitation() message size: " + size);
        for (int i = 0; i < size; i++) {
            ChatMessage msg = messages.get(i).getChatMessage();
            Logger.d(TAG, "handleInvitation()" + i + " message: "
                    + ((msg == null) ? null : msg.getMessage()));
            mChatWindow.addReceivedMessage(msg, !mIsInBackground);
            mAllMessages.add(new ChatMessageReceived(msg));
            mSentMessageManager.markSendingMessagesDisplayed();
            Logger.d(TAG, "handleInvitation() mIsInBackground:" + mIsInBackground);
            if (!mIsInBackground) {
                SharedPreferences settings =
                        PreferenceManager.getDefaultSharedPreferences(AndroidFactory
                                .getApplicationContext());
                boolean isSendReadReceiptChecked =
                        settings.getBoolean(SettingsFragment.RCS_SEND_READ_RECEIPT, true);
                Logger.d(TAG,
                        "handleInvitation() mIsInBackground is false and isSendReadReceiptChecked is "
                                + isSendReadReceiptChecked);
                try {
                    if (isSendReadReceiptChecked) {
                        o2oChatImpl.sendDisplayedDeliveryReport( msg.getId());
                    }
                } catch (JoynServiceException e) {
                    e.printStackTrace();
                }				
            } else {
                One2OneChat.this.addUnreadMessage(msg);

                RcsNotification.getInstance().onReceiveMessageInBackground(
                        AndroidFactory.getApplicationContext(), mTag, msg, null, 0);
            }
        }
        Logger.v(TAG, "handleInvitation exit");
    }

    @Override
    protected synchronized void onResume() {
        super.onResume();
        Logger.v(TAG, "onResume() entry");
        RcsNotification.getInstance().cancelFileTransferNotificationWithContact(
                mParticipant.getContact());
        RcsNotification.getInstance().cancelReceiveMessageNotification(mTag);
		if(isCapbilityExpired(mParticipant.getContact())){
			Logger.v(TAG, "onResume() checkAllCapabilityA");
        checkAllCapability();
		}
        acceptPendingSession();
        Logger.v(TAG, "onResume() exit");
    }

	boolean isCapbilityExpired(String contact){
		CapabilityService capabilityApi = ApiManager.getInstance().getCapabilityApi();
		Logger.v(TAG, "M0CFF isCapbilityExpired() entry" + contact);
		if(capabilityApi != null){
			Capabilities currentRemoteCapablities = null;
			try{
				 currentRemoteCapablities = capabilityApi.getContactCapabilities(contact);
			}
			catch(JoynServiceException e){
				Logger.d(TAG, "M0CFF isCapbilityExpired() getContactCapabilities JoynServiceException");
			}
			long delta = 0;
			if(currentRemoteCapablities != null){
				//delta	= (System.currentTimeMillis()-currentRemoteCapablities.getTimestamp())/1000; //TODo check this
				//Logger.v(TAG, "timestamp12() current " + System.currentTimeMillis() + "remote " +currentRemoteCapablities.getTimestamp());
				//Logger.v(TAG, "isCapbilityExpired() delta" + delta + "deiff:"+ (System.currentTimeMillis()-currentRemoteCapablities.getTimestamp()));
			}
			else{
				Logger.v(TAG, "M0CFF isCapbilityExpired() true1");
				return true;
			}
			if(delta >= capabilityTimeoutValue || capabilityTimeoutValue == 0 || delta == 0){
				Logger.v(TAG, "isCapbilityExpired() true2");
				return true;
			}
		}
		else{
			Logger.v(TAG, "M0CFF isCapbilityExpired() true3");
			return true;
		}
		Logger.v(TAG, "M0CFF isCapbilityExpired() false");
		return false;
	}

    private void acceptPendingSession() {
		try{
		ChatService chatService = ApiManager.getInstance().getChatApi();
        Chat pendingO2OChatImpl = chatService.getChat(mParticipant.getContact());
        Logger.v(TAG, "acceptPendingSession() entry : pendingSession = " + pendingO2OChatImpl);
        if (null != pendingO2OChatImpl) {
            Logger.d(TAG, "acceptPendingSession() pending session found");
           /* try {
                //pendingO2OChatImpl.acceptSession(); //TODo check this
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }*/
        }
		}
		catch(JoynServiceException e){
			 e.printStackTrace();
		}
		catch(NullPointerException e){
			 e.printStackTrace();
		}
    }

    @Override
    public void onStatusChanged(boolean status) {
        Logger.w(TAG, "onStatusChanged the status is " + status
                + ", mFileTransferController = " + mFileTransferController);
		Logger.d(TAG, "resumeFileSend onStatusChanged");
        if (status) {
            // Re-send the stored unregistered messages only if the status is
            // true
            mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mMessageDepot.resendStoredMessages();
                }
            });

            if (mFileTransferController != null) {
                mFileTransferController.setRegistrationStatus(true);
                mFileTransferController.controlFileTransferIconStatus();
            } 
			ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
			Logger.d(TAG, "resumeFileSend 03");
			if (instance != null) {
				instance.handleFileResumeAfterStatusChange();
				handleResumePauseReceiveFileTransfer();
				Logger.d(TAG, "resumeFileSend 04");
			}
        } else {
            if (mFileTransferController != null) {
                mFileTransferController.setRegistrationStatus(false);
                mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
			if(!RcsSettings.getInstance().isFtAlwaysOn()){
            if (instance != null) {
                instance.handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
            }
            mReceiveFileTransferManager.cancelReceiveFileTransfer();
        }
    }
    }

    /**
     * Get participant of this chat.
     * 
     * @return participant of this chat.
     */
    public Participant getParticipant() {
        return mParticipant;
    }

    @Override
    public void loadChatMessages(int count) {
        Logger.v(TAG, "loadChatMessage entry!");
        Context context;
        if (ApiManager.getInstance() != null) {
            context = ApiManager.getInstance().getContext();
        } else {
            Logger.e(TAG, "ApiManager getInstance is null!");
            return;
        }
        Logger.e(TAG, "Then load history , context = " + context + "mChatWindow = "
                + mChatWindow + "mAllMessages = " + mAllMessages);
        if (context != null && mChatWindow != null && mAllMessages != null) {
            QueryHandler queryHandler =
                    new QueryHandler(ApiManager.getInstance().getContext(), mChatWindow,
                            mAllMessages);

            // Do not take the chat terminated messages
            String chatTerminatedExcludedSelection =
                    " AND NOT((" + ChatLog.Message.MESSAGE_TYPE + "==" + TYPE_CHAT_SYSTEM_MESSAGE
                            + ") AND (" + ChatLog.Message.MESSAGE_STATUS + "== " + STATUS_TERMINATED
                            + "))";

            // Do not take the group chat entries concerning this contact
            chatTerminatedExcludedSelection +=
                    " AND NOT(" + ChatLog.Message.MESSAGE_TYPE + "=="
                            + TYPE_GROUP_CHAT_SYSTEM_MESSAGE + ")";

            // take all concerning this contact
            String firstMessageId = null;
            for (int i = 0; i < mAllMessages.size(); i++) {
                IChatMessage firstChatMessage = mAllMessages.get(i);
                if (firstChatMessage != null) {
                    ChatMessage firstMessage = firstChatMessage.getChatMessage();
                    if (firstMessage != null) {
                        firstMessageId = firstMessage.getId();
                        break;
                    }
                }
            }
            queryHandler.startQuery(count, firstMessageId,  ChatLog.Message.CONTENT_URI, null,
                     ChatLog.Message.CONTACT_NUMBER + "='"
                            + PhoneUtils.formatNumberToInternational(mParticipant.getContact())
                            + "'" + chatTerminatedExcludedSelection, null,
                     ChatLog.Message.TIMESTAMP + " DESC");
        }
    }

    private static class QueryHandler extends AsyncQueryHandler {
        IChatWindow mWindow;
        CopyOnWriteArrayList<IChatMessage> mAllMessages;

        public QueryHandler(Context context, IChatWindow chatWindow,
                CopyOnWriteArrayList<IChatMessage> allMessages) {
            super(context.getContentResolver());
            mWindow = chatWindow;
            mAllMessages = allMessages;
        }

        private boolean judgeUnLoadedHistory(Cursor cursor, Object firstMessageId) {
            String firstMessage;
            if (firstMessageId == null) {
                firstMessage = "";
                Logger.d(TAG, "judgeUnLoadedHistory, firstMessageId is null");
            } else {
                firstMessage = firstMessageId.toString();
                Logger.d(TAG, "judgeUnLoadedHistory, firstMessageId = " + firstMessageId);
            }
            if (cursor != null && !cursor.isAfterLast()) {
                do {
                    int messageType =
                            cursor.getInt(cursor.getColumnIndex(ChatLog.Message.DIRECTION));
                    String messageId =
                            cursor.getString(cursor
                                    .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                    if (messageType == ChatLog.Message.Direction.INCOMING
                            || messageType == ChatLog.Message.Direction.OUTGOING) {
                        if (null != messageId && !messageId.equals(firstMessage)) {
                            return true;
                        } else {
                            Logger.d(TAG, "judgeUnLoadedHistory, the two message is the same!");
                        }
                    }
                } while (cursor.moveToNext());
            } else {
                Logger.w(TAG, "judgeUnLoadedHistory, The cursor is null or cursor is last data!");
            }
            return false;
        }

        @Override
        protected void onQueryComplete(int count, Object loadedId, Cursor cursor) {
            Logger.v(TAG, "onQueryComplete enter!");
            String messageId = null;
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    if (loadedId != null) {
                        findLoadedMessage(cursor,loadedId);
                    }
                    // judge if have history, show the header
                    if (count == 0) {
                        if (judgeUnLoadedHistory(cursor, loadedId)) {
                            mWindow.addLoadHistoryHeader(true);
                        } else {
                            mWindow.addLoadHistoryHeader(false);
                        }
                        return;
                    }
                    loadMessage(cursor,messageId,loadedId,count);
                    // if have next message set header visible
                    if (judgeUnLoadedHistory(cursor, messageId)) {
                        mWindow.addLoadHistoryHeader(true);
                    } else {
                        mWindow.addLoadHistoryHeader(false);
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        /**
         * Find the message from cursor with a id loadedId and move the cursor to next
         * @param cursor A cursor
         * @param loadedId message id
         * @return True if the message with a loadedId exist in cursor,
         *         otherwise return false.
         */
        private boolean findLoadedMessage(Cursor cursor, Object loadedId) {
            do {
                String id = cursor.getString(cursor
                        .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                if (id != null && id.equals(loadedId)) {
                    if (!cursor.isAfterLast()) {
                        cursor.moveToNext();
                    }
                    return true;
                }
            } while (cursor.moveToNext());
            return false;
        }

        /**
         * Load message from cursor to memory
         * @param cursor A cursor
         * @param messageId A message Id, it is a output parameter
         * @param loadedId The loaded message Id
         * @param count The number of message to be loaded
         * @return True, always return true.
         */
        private boolean loadMessage(Cursor cursor, String messageId, Object loadedId, int count) {
            Logger.v(TAG, "loadMessage(): messageId = " + messageId + ", loadedId = " + loadedId
                    + ", count = " + count);
            int i = 0;
            if (!cursor.isAfterLast()) {
                do {
                    ChatMessage message = null;
                    messageId = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                    int messageType = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.DIRECTION));
                    String messageData = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.BODY));
                    int messageStatus = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.MESSAGE_STATUS));
                    Date date = new Date(cursor.getLong(cursor
                            .getColumnIndex(ChatLog.Message.TIMESTAMP)));
                    String remount = cursor.getString(cursor
                            .getColumnIndex(ChatLog.Message.CONTACT_NUMBER));
                    message = new ChatMessage(messageId, remount, messageData, date, true,RcsSettings.getInstance().getJoynUserAlias());
                    if (messageType == ChatLog.Message.Direction.INCOMING) {
                        if (messageId.equals(loadedId)) {
                            continue;
                        }
                       // message.setIsHistory(true); //TODo check this
                        mWindow.addReceivedMessage(message, true);
                        mAllMessages.add(0, new ChatMessageReceived(message));
                        i++;
                    } else if (messageType == ChatLog.Message.Direction.OUTGOING) {
                        if (messageId.equals(loadedId)) {
                            continue;
                        }
                        //message.setIsHistory(true); //TODo check this
                        ISentChatMessage sent = mWindow.addSentMessage(message, -1);
                        mAllMessages.add(0, new ChatMessageSent(message));
                        Logger.d(TAG, "sent = " + sent);
                        sent.updateStatus(getStatusEnum(messageStatus));
                        i++;
                    }
                } while (cursor.moveToNext() && i < count);
            }
            return true;
        }

    }


//TODo Replace al EventsLog APIs and enums with some new APIs andenums.
    /**
     * Get message status from database.
     * .
     * @return Status enum.
     */
    private static Status getStatusEnum(int status) {
        Logger.w(TAG, "getStatusEnum entry, status = " + status);
        switch (status) {
            case ChatLog.Message.Status.Content.READ:
                return Status.DISPLAYED;            
            case ChatLog.Message.Status.Content.SENT	:
        case ChatLog.Message.Status.Content.TO_SEND:
        case ChatLog.Message.Status.Content.FAILED:
                return Status.FAILED;
            case ChatLog.Message.Status.Content.UNREAD_REPORT:
        case ChatLog.Message.Status.Content.UNREAD:
                return Status.DELIVERED;
            default:
                return Status.FAILED;
        }
    }


    /**
     * Generate an sent file transfer instance in a specific chat window
     * 
     * @param filePath The path of the file to be sent
     * @return A sent file transfer instance in a specific chat window
     */
    public SentFileTransfer generateSentFileTransfer(String filePath, Object fileTransferTag) {
        return new SentFileTransfer(mTag, (IOne2OneChatWindow) mChatWindow, filePath, mParticipant,
                fileTransferTag);
    }
    
    

   

    /**
     * Handle a file transfer invitation
     */
    public void addReceiveFileTransfer(FileTransfer fileTransferObject, boolean isAutoAccept,boolean isGroup) {
    	Logger.v(TAG, "M0CFF O2O addReceiveFileTransfer isAutoAccept" + isAutoAccept+" isGroup:"+isGroup);
        mReceiveFileTransferManager.addReceiveFileTransfer(fileTransferObject, isAutoAccept,isGroup);
		if (mIsInBackground) {
			One2OneChat.this.addUnreadFt(fileTransferObject);
		}
    }

    /**
     * Handle reject a file transfer invitation
     */
    public void handleRejectFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleRejectFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.rejectFileTransferInvitation();
        }
    }

    /**
     * Handle accept a file transfer invitation
     */
    public void handleAcceptFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleAcceptFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.acceptFileTransferInvitation();
        }
    }

    /**
     * Handle cancel a file transfer invitation
     */
    public void handleCancelFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleCancelFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
            receiveFileTransfer.cancelFileTransfer();
        }
    }

	/**
     * Handle pause a file transfer 
     */
    public void handlePauseReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handlePauseReceiveFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			Logger.d(TAG,"handlePauseReceiveFileTransfer 1");
            receiveFileTransfer.onPauseReceiveTransfer();
        }
    }
    

	public void handleResumePauseReceiveFileTransfer(){
			Logger.d(TAG,"M0CFF handleResumePauseReceiveFileTransfer 0");
			ReceiveFileTransfer resume_file = null;
			//resume_file = mReceiveFileTransferManager
			try{
                resume_file = mReceiveFileTransferManager.getReceiveTransfer();
			}
			catch (Exception e) {
				Logger.d(TAG, "M0CFF resumeFileSend exception");
			}
			if (null != resume_file) {
				Logger.d(TAG,"M0CFF handleResumePauseReceiveFileTransfer 1");
	            resume_file.onResumeReceiveTransfer();
        	}
	}
    
   /**
     * Handle resume a file transfer 
     */
    public void handleResumeReceiveFileTransfer(Object fileTransferTag) {
        ReceiveFileTransfer receiveFileTransfer =
                mReceiveFileTransferManager.findFileTransferByTag(fileTransferTag);
        Logger.d(TAG,
                "M0CFF handleResumeReceiveFileTransfer entry(): receiveFileTransfer = "
                        + receiveFileTransfer + ", fileTransferTag = "
                        + fileTransferTag);
        if (null != receiveFileTransfer) {
			Logger.d(TAG,"M0CFF handleResumeReceiveFileTransfer 1");
            receiveFileTransfer.onResumeReceiveTransfer();
        }
    }

    @Override
    public void onCapabilitiesReceived(String contact, Capabilities capabilities) {
        Logger.w(TAG, "M0CFF onCapabilityChanged() entry the contact is " + contact + " capabilities is "
                + capabilities);
        if (mParticipant == null) {
            Logger.d(TAG, "M0CFF onCapabilityChanged() mParticipant is  null");
            return;
        }
        String participantNumber = mParticipant.getContact();
        Logger.w(TAG, "M0CFF onCapabilityChanged() participantNumbert is " + participantNumber);
        if (participantNumber.equals(contact)) {
            Logger.d(TAG, "M0CFF onCapabilityChanged() the participant equals the contact and number is "
                    + contact);
            if (capabilities.isImSessionSupported() || ContactsListManager.getInstance().isLocalContact(participantNumber)
                    || ContactsListManager.getInstance().isStranger(participantNumber)) {
                Logger.w(TAG, "M0CFF onCapabilityChanged() the participant is rcse contact " + contact);
                onCapabilityChangedWhenRemoteIsRcse(contact, capabilities);
            } else {
                Logger.w(TAG, "M0CFF onCapabilityChanged() the participant is not rcse contact "
                        + contact);
				if(!RcsSettings.getInstance().isFtAlwaysOn()){
                if (mFileTransferController != null) {
                    mFileTransferController.setRemoteFtCapability(false);
                    mFileTransferController.controlFileTransferIconStatus();
                }
                IChatManager instance = ModelImpl.getInstance();
                if (instance != null) {
                    ((ModelImpl) instance).handleFileTransferNotAvailable(mTag,
                            FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
                }
                mReceiveFileTransferManager.cancelReceiveFileTransfer();
					}
                Logger.w(TAG, "M0CFF mChatWindow = " + mChatWindow);
                if (mChatWindow != null) {
                    ((IOne2OneChatWindow) mChatWindow)
                            .setRemoteOfflineReminder(false);
                }
            }
        }
        Logger.d(TAG, "M0CFF onCapabilityChanged() exit");
    }
    
    private void onCapabilityChangedWhenRemoteIsRcse(String number, Capabilities capabilities) {
        Logger.d(TAG,
                "M0CFF onCapabilityChangedWhenRemoteIsRcse() the participant support rcse "
                        + number + ", mFileTransferController = "
                        + mFileTransferController);
        if (capabilities.isFileTransferSupported()) {
            Logger.d(TAG, "M0CFF onCapabilityChangedWhenRemoteIsRcse() is filetransfersupported");
            if (mFileTransferController != null) {
                mFileTransferController.setRemoteFtCapability(true);
                mFileTransferController.controlFileTransferIconStatus();
            }
        } else if(!RcsSettings.getInstance().isFtAlwaysOn()){
            Logger.d(TAG, "M0CFF onCapabilityChanged isn't filetransfersupported");
            if (mFileTransferController != null) {
                mFileTransferController.setRemoteFtCapability(false);
                mFileTransferController.controlFileTransferIconStatus();
            }
            IChatManager instance = ModelImpl.getInstance();
            if (instance != null) {
                ((ModelImpl) instance).handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
            }
            mReceiveFileTransferManager.cancelReceiveFileTransfer();
        }
        ContactInfo info = ContactsManager.getInstance().getContactInfo(number);
        Logger.d(TAG, "M0CFF onCapabilityChanged() contact info is " + info);
        int registrationState = info.getRegistrationState();
        if (ContactInfo.REGISTRATION_STATUS_ONLINE == registrationState ||  RcsSettings.getInstance().isImAlwaysOn()) {
            Logger.w(TAG, "M0CFF onCapabilityChangedWhenRemoteIsRcse() ,the participant " + number
                    + " is online");
			if (mChatWindow != null) {
            ((IOne2OneChatWindow) mChatWindow).setRemoteOfflineReminder(false);
				}
			mWorkerHandler.post(new Runnable() {
                @Override
                public void run() {
                    mMessageDepot.resendStoredMessages();
                }
            });
        } else {
            boolean isLocalContact = ContactsListManager.getInstance().isLocalContact(number);
            if (isLocalContact) {
                Logger.w(TAG, "M0CFF onCapabilityChangedWhenRemoteIsRcse() ,the participant " + number
                        + " is offline");
                ((IOne2OneChatWindow) mChatWindow).setRemoteOfflineReminder(true);
            }
        }
    }

    public void onCapabilitiesChangedListener(com.orangelabs.rcs.core.ims.service.capability.Capabilities stackCapabilities) {
		Set<String> exts = new HashSet<String>(stackCapabilities.getSupportedExtensions());
		Capabilities capabilities =  new Capabilities(stackCapabilities.isImageSharingSupported(),
				stackCapabilities.isVideoSharingSupported(),
				stackCapabilities.isImSessionSupported(),
				stackCapabilities.isFileTransferSupported(),
				stackCapabilities.isGeolocationPushSupported(),
				stackCapabilities.isIPVoiceCallSupported(),
				stackCapabilities.isIPVideoCallSupported(),
    			exts,
    			stackCapabilities.isSipAutomata(),
    			stackCapabilities.isFileTransferHttpSupported(),
				stackCapabilities.isRCSContact(),
				stackCapabilities.isIntegratedMessagingMode(),
				stackCapabilities.isCsVideoSupported());
		
        Logger.d(TAG, "M0CFF onCapabilitiesChangedListener() entry : capabilites is "
                + capabilities + ", mFileTransferController = "
                + mFileTransferController);
        if (capabilities == null) {
            return;
        }
        if (capabilities.isFileTransferSupported()) {
            Logger.d(TAG, "M0CFF onCapabilitiesChangedListener() self filetransfer support");
            if (mFileTransferController != null) {
                mFileTransferController.setLocalFtCapability(true);
                mFileTransferController.controlFileTransferIconStatus();
            } 
        } else {
            Logger.d(TAG, "M0CFF onCapabilitiesChangedListener() self filetransfer not support");
            if (mFileTransferController != null) {
                mFileTransferController.setLocalFtCapability(false);
                mFileTransferController.controlFileTransferIconStatus();
            }
            ModelImpl instance = (ModelImpl) ModelImpl.getInstance();
			if(!RcsSettings.getInstance().isFtAlwaysOn()){
            if (instance != null) {
                instance.handleFileTransferNotAvailable(mTag,
                        FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
            }
            if (mReceiveFileTransferManager != null) {
                mReceiveFileTransferManager.cancelReceiveFileTransfer();
            }
        }
        }
        Logger.d(TAG, "M0CFF onCapabilitiesChangedListener() exit ");
    }

    private final MessageDepot mMessageDepot = new MessageDepot();
    
    /**
     * Added to notify the user that the sip invite is failed.@{
     */
    private final ArrayList<ISentChatMessage> mLastISentChatMessageList = new ArrayList<ISentChatMessage>();
    /**
     * @}
     */
    
	int messageSent = 0;
	int messageDeleteTag = 0;
    
    /**
     * The class is used to manage the stored message sent in unregistered status
     */
    private class MessageDepot {
        public final String tag = "M0CF MessageDepot@" + mTag;
        private static final String WHERE = UnregMessageProvider.KEY_CHAT_TAG + "=?";
		private static final String WHEREDELETE = UnregMessageProvider.KEY_MESSAGE_TAG + "=?";
        private final String[] mSelectionArg = new String[] {
                mTag.toString()
        };

		

        private final Map<Integer, WeakReference<ISentChatMessage>> mMessageMap = 
                                    new ConcurrentHashMap<Integer, WeakReference<ISentChatMessage>>();

		private final Map<Integer, String> mMessageTagMap = new HashMap<Integer, String>();

        /**
         * Check whether need to add a new message to View
         * @param messageTag
         * @return null if this message is not for resent, otherwise an instance of ISentChatMessage
         */
        public ISentChatMessage checkIsResend(int messageTag) {
            Logger.d(tag, "checkIsResend() entry, messageTag: ");
            WeakReference<ISentChatMessage> reference = mMessageMap.get(messageTag);
            if (null != reference) {
                ISentChatMessage message = reference.get();
                mMessageMap.remove(messageTag);
                Logger.d(tag, "checkIsResend() message: " + message);
                return message;
            } else {
                return null;
            }
        }

		public ISentChatMessage getMessageTag(int messageTag) {
            Logger.d(tag, "checkIsResend() entry, messageTag: ");
            WeakReference<ISentChatMessage> reference = mMessageMap.get(messageTag);
            if (null != reference) {
                ISentChatMessage message = reference.get();
                mMessageMap.remove(messageTag);
                Logger.d(tag, "checkIsResend() message: " + message);
                return message;
            } else {
                return null;
            }
        }

        public void updateStoredMessage(int messageTag, ISentChatMessage message) {
            Logger.d(tag, "updateStoredMessage() messageTag: " + messageTag);
            mMessageMap.put(messageTag, new WeakReference<ISentChatMessage>(message));
        }


		public void addMessageTag(int messageTag, String message) {
            Logger.d(tag, "addMessageTag() messageTag: " + messageTag + " Id " + message);
            mMessageTagMap.put(messageTag, message);
        }

		public String getMsgTag(int msgId) {
            Logger.d(tag, "checkIsResend() entry, messageTag: ");
            String messageTag  = mMessageTagMap.get(msgId);
			Logger.d(tag, "getMsgTag() message: " + messageTag);                        
            return messageTag;          
        } 

		public void removeMessageTag(int messageTag, String msgId) {
            Logger.d(tag, "removeMessageTag() messageTag: " + messageTag + " Id " + msgId);
            mMessageTagMap.remove(msgId);
        }

        public ChatMessage storeMessage(String content, int messageTag) {
            Logger.d(tag, "storeMessage() entry : content =" + content
                    + ",messageTag = " + messageTag);
            if (!TextUtils.isEmpty(content)) {
                ApiManager apiManager = ApiManager.getInstance();
                Logger.d(tag, "storeMessage() : apiManager ="
                        + apiManager);
                Context context = null;
                ContentResolver resolver = null;
                if (apiManager != null) {
                    context = apiManager.getContext();
                    Logger.d(tag, "storeMessage() : context ="
                            + context);
					try{
                    if (context != null && mParticipant != null) {
                        resolver = context.getContentResolver();
						ChatMessage msg = new ChatMessage(ChatUtils.generateMessageId(), mParticipant.getContact(), content, new Date(), true,RcsSettings.getInstance().getJoynUserAlias());
                        ContentValues values = new ContentValues();
                        values.put(UnregMessageProvider.KEY_CHAT_TAG, mTag.toString());
                        values.put(UnregMessageProvider.KEY_MESSAGE_TAG, messageTag);
                        values.put(UnregMessageProvider.KEY_MESSAGE, content);
						//values.put(UnregMessageProvider.KEY_MESSAGE_ID, msg.getMessageId());
                        resolver.insert(UnregMessageProvider.CONTENT_URI, values);
                        Logger.d(tag, "storeMessage() Store messages while the sender is not registered");
                       // return new ChatMessage(ChatUtils.generateMessageId(), mParticipant.getContact(), content, true, new Date());
                       return msg;
                    }
                }
					catch(Exception e){
						 Logger.d(tag, "storeMessage fail");
					}
                }
            }
            return null;
        }

        public void resendStoredMessages() {
			Cursor cursor = null;
			try {
            ContentResolver resolver = ApiManager.getInstance().getContext().getContentResolver();
			Logger.w(tag, "resendStoredMessages() entry");
            // Query
                cursor = resolver.query(UnregMessageProvider.CONTENT_URI, null, WHERE, mSelectionArg, null);
                if (cursor != null) {
                    int messageIndex = cursor.getColumnIndex(UnregMessageProvider.KEY_MESSAGE);
                    int messageTagIndex = cursor.getColumnIndex(UnregMessageProvider.KEY_MESSAGE_TAG);
                    for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
                        String message = cursor.getString(messageIndex);
                        int messageTag = cursor.getInt(messageTagIndex);
						messageSent = 0;
						messageDeleteTag = messageTag;
                        One2OneChat.this.sendMessage(message, messageTag);
                        Logger.w(tag, "resendStoredMessages() chat[" + cursor.getString(0) + "] send message" + message
                                + " with message tag: " + messageTag);
						if(messageSent == 1){
							//Delete current cursor
							Logger.w(tag, "resendStoredMessages() delet"+messageTag);
							resolver.delete(UnregMessageProvider.CONTENT_URI, UnregMessageProvider.KEY_MESSAGE_TAG + " = " +messageTag, null);
							messageSent = 0;
							messageDeleteTag = 0;
						}
                    }
                }
            } catch (SQLiteException e) {
                e.printStackTrace();
                Logger.w(tag, "resendStoredMessages() Query exception");
            } catch (NullPointerException e) {
                e.printStackTrace();
                Logger.w(tag, "resendStoredMessages() null exception");
            } finally {
            	messageSent = 0;
				messageDeleteTag = 0;
                if (cursor != null) {
                    cursor.close();
                }
            }
            // Delete
            /*
            try {
                int counts = resolver.delete(UnregMessageProvider.CONTENT_URI, WHERE, mSelectionArg);
                Logger.i(tag, "resendStoredMessages() Remove " + counts + " messages");
            } catch (SQLiteException e) {
                e.printStackTrace();
                Logger.e(tag, "resendStoredMessages() Query exception");
            }
            */
        }

        public void removeUnregisteredMessage() {
			try{
            ContentResolver resolver = ApiManager.getInstance().getContext().getContentResolver();
            int count = resolver.delete(UnregMessageProvider.CONTENT_URI, WHERE, mSelectionArg);
            Logger.i(tag, "removeUnregisteredMessage() Delete " + count + " unregistered sending messages");
            mMessageMap.clear();
        }
			catch (NullPointerException e) {
                e.printStackTrace();
                Logger.w(tag, "resendStoredMessages() Query exception");
            }
        }
    }

    @Override
    protected void reloadFileTransfer(final FileStruct fileStruct, final int transferDirection, final int messageStatus) {
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "M0CFF reloadFileTransfer()->run() entry, file transfer tag: " + fileStruct.mFileTransferTag
                        + "file transfer path: " + fileStruct.mFilePath + " , transferType: " + transferDirection
                        + ", messageStatus: " + messageStatus);
                IFileTransfer fileTransfer = null;
                if (FileTransfer.Direction.INCOMING == transferDirection) { //TODo check this
                    fileTransfer = ((IOne2OneChatWindow) mChatWindow).addReceivedFileTransfer(fileStruct, false);
                    if (fileTransfer != null) {
                        if (fileStruct.mFilePath == null) {
                            if((mReceiveFileTransferManager != null) &&(messageStatus == 0 || messageStatus == 1)) //TODo check this
                            {   
                                if(mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag) == null) 
                                {
                                	fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                                }
                            	else
                                {
                                Logger.d(TAG,
                                        "M0CFF reloadFileTransfer(), file path is null, set status Transferring!");                           
                                mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag).mFileTransfer = fileTransfer;                               
                                if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING)
                                fileTransfer
                                .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.TRANSFERING);
                                else if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING)
                                    fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING);
                                else if(fileTransferStatus == com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED)
                                    fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.REJECTED);
                               }
                            }
                            else
                            {
                            Logger.d(TAG,
                                    "M0CFF reloadFileTransfer(), file path is null, set status failed!");
                            fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                            }
                        } else {
                            Logger.d(TAG, "M0CFF reloadFileTransfer(), set status finished!");
                            fileTransfer
                                    .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                        }
                    } else {
                        if(mReceiveFileTransferManager!=null)
                        {
                            fileTransfer = mReceiveFileTransferManager.findFileTransferByTag(fileStruct.mFileTransferTag).mFileTransfer;
                            fileTransfer
                            .setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.WAITING);
                        Logger.e(TAG, "M0CFF reloadFileTransfer(), fileTransfer is null!");
                    }
                    }
                } else if (FileTransfer.Direction.OUTGOING == transferDirection) {  //TODo check this
                    fileTransfer = ((IOne2OneChatWindow) mChatWindow).addSentFileTransfer(fileStruct);
                    Logger.d(TAG, "M0CFF reloadFileTransfer(), messageStatus = " + messageStatus);
                    if (fileTransfer != null && ( messageStatus == 4 || messageStatus == 7 || messageStatus == 8)) { //TODo check this
                        fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FINISHED);
                    }
                    /* We can have a single condition which sets Fail in all cases other then terminated */
                    else 
                        fileTransfer.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.FAILED);
                }
            }
        };
        mWorkerHandler.post(worker);
    }

    @Override
    protected void reloadMessage(final ChatMessage message, final int messageType,
            final int messageStatus,int messagetag, final String chatId) {
    	final int messageTag = messagetag;
        Logger.d(TAG, "reloadMessage() message " + message.getId() + " " + this
                + " messageStatus is " + messageStatus + "messageTag" + messagetag + "chatid " + chatId);
        Runnable worker = new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "reloadMessage()->run() entry, message id: " + message.getId()
                        + "message text: " + message.getMessage() + " , messageType: "
                        + messageType);
                if (ChatLog.Message.Direction.INCOMING == messageType && (chatId.equals(message.getContact()))) {
                	if(mChatWindow != null)
                	{
                    Logger.d(TAG, "reloadMessage() the mchatwindow is " + mChatWindow);
					if(messageStatus == 2){
                    mChatWindow.addReceivedMessage(message, true);
					} else {
					    Logger.d(TAG, "mReceivedAfterReloadMessage Id: " + message.getId());
					    mReceivedAfterReloadMessage.add(message.getId());
					    mChatWindow.addReceivedMessage(message, false);
					}
                    mAllMessages.add(new ChatMessageReceived(message));
                	}
                } else if (ChatLog.Message.Direction.OUTGOING == messageType && (chatId.equals(message.getContact()))) {
                    //Check if error condition
                	if(mChatWindow != null)
                	{
                    ISentChatMessage sentMessage = mChatWindow.addSentMessage(message, messageTag);
                    if (sentMessage != null) {
                        sentMessage.updateStatus(getStatusEnum(messageStatus));
						if(getStatusEnum(messageStatus) == Status.FAILED){
							Logger.w(TAG, "reloadMessage() Failed message add to depot text is" + message.getMessage());
							mMessageDepot.storeMessage(message.getMessage(),messageTag);
							mMessageDepot.updateStoredMessage(messageTag, sentMessage);
							Logger.w(TAG, "reloadMessage() Failed message add to depot TAG is" + messageTag);
						}
                    } else {
                        Logger.d(TAG, "reloadMessage() the ISentChatMessage is " + null);
                    }
                    mAllMessages.add(new ChatMessageSent(message));
                }
            }
            }
        };
        mWorkerHandler.post(worker);
    }
}
