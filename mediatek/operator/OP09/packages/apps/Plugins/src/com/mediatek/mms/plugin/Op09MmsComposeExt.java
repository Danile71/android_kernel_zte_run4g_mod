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

package com.mediatek.mms.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.AsyncQueryHandler;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Resources.Theme;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Message;
import android.preference.PreferenceManager;
import android.provider.BaseColumns;
import android.provider.Settings;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;
import android.text.TextUtils;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsComposeExt;
import com.mediatek.mms.ext.ViewOnClickListener;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SmsManagerEx;

import java.util.ArrayList;
import java.util.Map;

public class Op09MmsComposeExt extends DefaultMmsComposeExt {
    private static final String TAG = "Mms/Op09MmsComposeExt";

    private static final String[] SMS_PROJECTION = new String[] {
        Sms.ADDRESS,
        Sms.DATE,
        Sms.TYPE,
        Sms.ERROR_CODE,
        Sms._ID,
        Sms.SIM_ID
        };

    private static final int COLUMN_ADDRESS = 0;
    private static final int COLUMN_DATE = 1;
    private static final int COLUMN_TYPE = 2;
    private static final int COLUMN_ERROR_CODE = 3;
    private static final int COLUMN_MSG_ID = 4;
    private static final int COLUMN_SIM_ID = 5;

    private static final String COLUMN_ADDRESS_NAME = "address";
    private static final String COLUMN_DATE_NAME = "date";
    private static final String COLUMN_TYPE_NAME = "type";
    private static final String COLUMN_ERROR_CODE_NAME = "error_code";
    private static final String COLUMN_SIM_ID_NAME = "sim_id";

    private static final int MENU_SPLIT_MESSAGE = 310;
    private static final int MENU_SPLIT_THREAD = 311;

    private static final String PREF_KEY_SHOW_DIALOG = "pref_key_show_dialog";

    private long mMessageGroupId = 0;
    private long mThreadId = 0;
    private Context mContext = null;
    private Activity mActivity = null;
    private Context mPluginContext;
    /// M: For length required mms to sms
    private boolean mNeedConfirmMmsToSms = true;

    public Op09MmsComposeExt(Context context) {
        super(context);
        mPluginContext = context;
    }

    public String getNumberLocation(Context context, String number) {
        PhoneNumberUtils pnu = new PhoneNumberUtils(context);
        String location = pnu.getNumberLocation(number);
        if (location == null || location.trim().equals("")) {
            return number;
        }
        return number + " (" + location + ")";
    }

    public Uri getConverationUri(Uri uriSrc, long threadId) {
        return Uri.parse("content://mms-sms/conversations_distinct/" + threadId);
    }

    @Override
    public boolean deleteMassTextMsg(AsyncQueryHandler backQueryHandler, long msgId, long timeStamp) {
        MmsLog.d(TAG, "deleteMassTextMag: msgId:" + msgId + " timeStamp:" + timeStamp);
        if (timeStamp >= 0) {
            return false;
        }

        if (backQueryHandler != null) {
            backQueryHandler.startDelete(9700, null, android.provider.Telephony.Sms.CONTENT_URI, "ipmsg_id = ?",
                new String[] {timeStamp + ""});
        }
        return true;
    }

    public boolean lockMassTextMsg(final Context context, final long msgId, final long timeStamp,
            final boolean locked) {
        final ContentValues values = new ContentValues(1);
        values.put("locked", locked ? 1 : 0);
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (timeStamp < 0) {
                    context.getContentResolver().update(Sms.CONTENT_URI, values, "ipmsg_id = ?",
                        new String[] {timeStamp + ""});
                } else {
                    context.getContentResolver().update(Sms.CONTENT_URI, values, "_id = ?",
                        new String[] {msgId + ""});
                }
            }
        }).start();
        return true;
    }

    public boolean showMassTextMsgDetail(final Context context, long timeStamp) {
        MmsLog.d(TAG,"showMassTextMsgDetail:" + timeStamp);
        if (context == null || timeStamp >= 0) {
            return false;
        }
        String[] datas = null;
        Cursor cursor = context.getContentResolver().query(Sms.CONTENT_URI, SMS_PROJECTION,
            "ipmsg_id = ?", new String[] {timeStamp + ""}, null);
        long msgIds[][] = null;
        try {
            if (cursor.getCount() < 2) {
                return false;
            }
            msgIds = new long[cursor.getCount()][2];
            datas = new String[cursor.getCount() + 1];

            int index = 0;
            int dataIndex = 1;
            while (cursor.moveToNext()) {
                ///M: set sms's address @{
                String addressStr = this.getResources().getString(
                    R.string.to_address_label)
                      + cursor.getString(COLUMN_ADDRESS);
                ///@}
                ///M: set sms's status @{
                String status = this.getResources().getString(R.string.msg_status);
                msgIds[index][0] = -1;
                int type = cursor.getInt(COLUMN_TYPE);
                MmsLog.d(TAG, "Sms Type:" + type);
                switch (type) {
                    case Sms.MESSAGE_TYPE_SENT:
                        status = status + this.getResources().getString(R.string.msg_status_success);
                        break;
                    case Sms.MESSAGE_TYPE_DRAFT:
                        status = status + this.getResources().getString(R.string.saved_label);
                        break;
                    case Sms.MESSAGE_TYPE_FAILED:
                        status = status + this.getResources().getString(R.string.msg_status_failed);
                        msgIds[index][0] = cursor.getLong(COLUMN_MSG_ID);
                        msgIds[index][1] = cursor.getLong(COLUMN_SIM_ID);
                        break;
                    case Sms.MESSAGE_TYPE_OUTBOX:
                    case Sms.MESSAGE_TYPE_QUEUED:
                        status = status + this.getResources().getString(R.string.msg_status_sending);
                        break;
                    default:
                        break;
                }
                ///@}
                datas[dataIndex] =  addressStr + "\n" + status;
                index++;
                dataIndex++;
            }
            ///M: set MessageListDetails head @{
                String typeLabel = this.getResources().getString(R.string.message_type_label);
            String headStr = typeLabel + this.getResources().getString(R.string.text_message);
                String dateLabel = this.getResources().getString(R.string.sent_label);
            if (cursor.moveToFirst()) {
                headStr = headStr + "\n" + (dateLabel
                        + MessageUtils.formatDateOrTimeStampStringWithSystemSetting(context, cursor
                                .getLong(COLUMN_DATE), true));
            }
            datas[0] = headStr;
            /// @}
            final long[][] msgIDS = msgIds;
            DialogInterface.OnClickListener clickListner = new DialogInterface.OnClickListener() {
                @SuppressWarnings("unchecked")
                public final void onClick(DialogInterface dialog, int which) {
                    Op09ResendSmsExt mcde = new Op09ResendSmsExt(context);
                    if (msgIDS != null && msgIDS.length > 0) {
                        for (int index = 0; index < msgIDS.length; index++) {
                            MmsLog.d(TAG,"msgIDS:" + msgIDS[index][0]);
                            if (msgIDS[index][0] > 0) {
                                Uri resendUri = ContentUris.withAppendedId(Sms.CONTENT_URI,
                                    msgIDS[index][0]);
                                MmsLog.d(TAG, "Resend SMS Uri:" + resendUri + " SMs MsgID:"
                                    + msgIDS[index][1]);
                                mcde.resendMessage(context, resendUri, (int)msgIDS[index][1]);
                            }
                        }
                    }
                    dialog.dismiss();
                }
            };
            String sendButtonStr = this.getResources().getString(R.string.btn_resend_str);
            boolean hasFailed = false;
            if (msgIds == null || msgIds.length < 1) {
                hasFailed = false;
            } else {
                for (int msgIdIndex = 0; msgIdIndex < msgIds.length; msgIdIndex++) {
                    if (msgIds[msgIdIndex][0] > 0) {
                        hasFailed = true;
                        break;
                    }
                }
            }
            if (hasFailed) {
                showMassTextMsgDetails(datas, clickListner, sendButtonStr, hasFailed);
            } else {
                showMassTextMsgDetails(datas, null, sendButtonStr, hasFailed);
            }
        } catch (SQLiteException e) {
            MmsLog.e(TAG, "query mass text message's details failed.", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return true;
    }

    private final class ContextMenuClickListener implements MenuItem.OnMenuItemClickListener {
        public boolean onMenuItemClick(MenuItem item) {
            switch (item.getItemId()) {
                case MENU_SPLIT_MESSAGE:
                    MmsLog.d(TAG, "ContextMenuClickListener: click MENU_SPLIT_MESSAGE, messageGroupId:" + mMessageGroupId);
                    splitSingleMessage(mContext, mMessageGroupId);
                    break;

                default:
                    return false;
            }
            return true;
        }
    }

    /**
     * M: For CT. split single massing text message to separater message.
     * @param messageGroupId
     */
    public void splitSingleMessage(final Context context, final long messageGroupId) {
        if (messageGroupId >= 0) {
            MmsLog.d(TAG, "splitSingleMessage failed: messageGroupId >= 0; messageGroupId = " + messageGroupId);
            return;
        }
        if (context == null) {
            MmsLog.d(TAG, "splitSingleMessage failed: context == null;");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                // /M: First: Get all messages which own the same messageGroupId(ipmessageId)
                Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(), Sms.CONTENT_URI,
                        new String[] {Sms._ID, Sms.ADDRESS, Sms.THREAD_ID, Sms.IPMSG_ID}, Sms.IPMSG_ID + " = ? ",
                        new String[] {messageGroupId + ""}, null);
                long threadId = 0;
                if (cursor != null) {
                    try {
                        int smsCount = cursor.getCount();
                        while (cursor.moveToNext()) {
                            // /M: Second: get or create thread Id
                            String address = cursor.getString(1);
                            long smsId = cursor.getLong(0);
                            threadId = cursor.getLong(2);
                            long newThreadId = Threads.getOrCreateThreadId(context, address);
                            // /M: Third: Update sms's threadId and messageGroupId
                            ContentValues cv = new ContentValues();
                            cv.put(Sms.THREAD_ID, newThreadId);
                            cv.put(Sms.IPMSG_ID, 0);
                            SqliteWrapper.update(context, context.getContentResolver(), ContentUris.withAppendedId(
                                Sms.CONTENT_URI, smsId), cv, null, null);
                        }
                        closeActivity(context, threadId);
                    } finally {
                        cursor.close();
                    }
                }
            }
        }).start();
    }

    /**
     * For : Cloase activity if the activity has no message in compose.
     * @param threadId
     */
    private void closeActivity(Context context, long threadId) {
        if (threadId < 1) {
            return;
        }
        Uri conUri = ContentUris.withAppendedId(MmsSms.CONTENT_CONVERSATIONS_URI, threadId);
        Cursor conCursor = context.getContentResolver().query(conUri, new String[] {BaseColumns._ID}, null, null, null);
        try {
            if (conCursor != null) {
                int msgCount = conCursor.getCount();
                MmsLog.d(TAG,"closeActivity, msgCount: " + msgCount);
                if (msgCount < 1 && mActivity != null) {
                    mActivity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            // TODO Auto-generated method stub
                            mActivity.finish();
                        }
                    });
                }
            }
        } finally {
            if (conCursor != null) {
                conCursor.close();
            }
        }
    }

    private void splitThreadMassingTextMessage(final Context context, final long threadId) {
        if (threadId <= 0) {
            MmsLog.d(TAG, "splitThreadMassingTextMessage failed: threadId <= 0; threadId = " + threadId);
            return;
        }
        if (context == null) {
            MmsLog.d(TAG, "splitThreadMassingTextMessage failed: context == null;");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                ///M: First: Get all messages which own the same messageGroupId(ipmessageId)
                Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(), Sms.CONTENT_URI,
                        new String[] {Sms._ID, Sms.ADDRESS, Sms.THREAD_ID, Sms.IPMSG_ID}, Sms.THREAD_ID + " = ? ",
                        new String[] {threadId + ""}, null);
                if (cursor != null) {
                    try {
                        while (cursor.moveToNext()) {
                            // /M: Second: get or create thread Id
                            String address = cursor.getString(1);
                            long smsId = cursor.getLong(0);
                            long newThreadId = Threads.getOrCreateThreadId(context, address);
                            // /M: Third: Update sms's threadId and messageGroupId
                            ContentValues cv = new ContentValues();
                            cv.put(Sms.THREAD_ID, newThreadId);
                            cv.put(Sms.IPMSG_ID, 0);
                            SqliteWrapper.update(context, context.getContentResolver(), ContentUris.withAppendedId(
                                Sms.CONTENT_URI, smsId), cv, null, null);
                        }
                        closeActivity(context, threadId);
                    } finally {
                        cursor.close();
                    }
                }
            }
        }).start();
    }

    public void addSplitMessageContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo, Activity activity,
            long messageGroupId, int messagesCount) {
        MmsLog.i(TAG, "add Context enu");
        this.mMessageGroupId = messageGroupId;
        this.mContext = activity.getApplicationContext();
        this.mActivity = activity;

        menu.add(0, MENU_SPLIT_MESSAGE, 0, getString(R.string.split_message_apart)).setOnMenuItemClickListener(
            new ContextMenuClickListener());
    }

    public void addSplitThreadOptionMenu(Menu menu, Activity activity, long threadId) {
        this.mThreadId = threadId;
        this.mContext = activity.getBaseContext();
        this.mActivity = activity;
        menu.add(0, MENU_SPLIT_THREAD, 0, this.getResources().getString(R.string.split_thread_apart));
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_SPLIT_THREAD:
                splitThreadMassingTextMessage(mContext, mThreadId);
                return true;
            default:
                return false;
        }
    }

    public void showDisableDRDialog(final Activity activity, final int simId) {
        MmsLog.d(TAG, "showDisableDRDialog() simId = " + simId);
        final Context context = activity.getApplicationContext();
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        if (!sp.getBoolean(PREF_KEY_SHOW_DIALOG + "_" + simId, true)) {
            return;
        }

        /// M: Set theme the same as in ComposeMessageActivity @{
        Theme theme = mPluginContext.getTheme();
        theme.setTo(activity.getTheme());
        /// @}

        final View contents = View.inflate(mPluginContext, R.layout.disable_delivery_report_dialog, null);
        final CheckBox checkbox = (CheckBox)contents.findViewById(R.id.dlg_never_shown);

        activity.runOnUiThread(new Runnable(){
            @Override
            public void run() {
                new AlertDialog.Builder(activity)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setView(contents)
                    .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface arg0, int arg1) {
                                if (checkbox.isChecked()) {
                                    enableDRWarningDialog(context, false, simId);
                                }
                            }
                        }
                    ).create().show();
            }});
    }

    public void enableDRWarningDialog(Context context, boolean isEnable, int simId) {
        SharedPreferences.Editor editor
                = PreferenceManager.getDefaultSharedPreferences(context).edit();
        editor.putBoolean(PREF_KEY_SHOW_DIALOG + "_" + simId, isEnable).apply();
    }

    @Override
    public boolean needConfirmMmsToSms() {
        // TODO Auto-generated method stub
        return mNeedConfirmMmsToSms;
    }

    @Override
    public void setConfirmMmsToSms(boolean needConfirm) {
        // TODO Auto-generated method stub
        mNeedConfirmMmsToSms = needConfirm;
    }

    public int getSmsMessageAndSaveToSim(String[] numbers, String scAddress, ArrayList<String> messages, int smsStatus,
            long timeStamp, int slotId, final int srcResult) {
        MmsLog.d(TAG, "save sms message to sim");
        if (numbers == null || numbers.length < 1) {
            return srcResult;
        }
        int result = -1;
        /// M: choose the first validate number to save to sim card.
        for (String number : numbers) {
            result = SmsManagerEx.getDefault().copyTextMessageToIccCard(scAddress, number, messages, smsStatus,
                timeStamp, slotId == -1 ? 0 : slotId);
            if (result == 0 || result == 7) {
                MmsLog.d(TAG, "save sms message to sim successed: number:" + number);
                return result;
            }
        }
        return srcResult;
    }

/// M: ---------------new OP09Plugin Refactory-------------------------
    /// M: the big button for new dual btns;
    private ImageButton mButtonSmall = null;
    /// M: the small button for new dual btns;
    private ImageButton mButtonBig = null;
    /// M: the original button of default google MMMS;
    private TextView mSendButtonMms = null;
    /// M: the original button of default google MMMS;
    private ImageButton mSendButtonSms = null;
    private TextView mTextCounter = null;
    /// M: the original button of default google MMMS for Slide VIEW.
    private Button mSendButton = null;
    /// M: SIM info map; the key is simId;
    private Map<Integer, SimInfoManager.SimInfoRecord> mSiminfoList = null;
    /// M: the dual button's draw
    private Drawable[] mButtonDrawable = null;
    private int[] mButtonSlotIds ;
    private int[] mButtonColors;

    @Override
    public void initDualSendButtonLayout(Activity activity, LinearLayout buttonWithCounter, TextView newTextCounter) {

        mActivity = activity;
        mContext = activity;
        if (buttonWithCounter == null) {
            return;
        }
        mButtonSmall = (ImageButton) buttonWithCounter.getChildAt(0);
        mButtonBig = (ImageButton) buttonWithCounter.getChildAt(1);
        mTextCounter = newTextCounter;
        mSendButtonMms = (TextView) buttonWithCounter.getChildAt(3);
        mSendButtonSms = (ImageButton) buttonWithCounter.getChildAt(4);
        ImageButton sendButtonIpMessage = (ImageButton) buttonWithCounter.getChildAt(5);

        if (mSendButtonMms != null) {
            mSendButtonMms.setVisibility(View.GONE);
        }
        if (mSendButtonSms != null) {
            mSendButtonSms.setVisibility(View.GONE);
        }
        if (sendButtonIpMessage != null) {
            sendButtonIpMessage.setVisibility(View.GONE);
        }
        mButtonSmall.setVisibility(View.VISIBLE);
        mButtonBig.setVisibility(View.VISIBLE);
    }

    public void initDualSendBtnForDialogMode(Context context, LinearLayout buttonWithCounter, TextView newTextCounter) {
        mContext = context;
        if (buttonWithCounter == null) {
            return;
        }
        mButtonSmall = (ImageButton) buttonWithCounter.getChildAt(0);
        mButtonBig = (ImageButton) buttonWithCounter.getChildAt(1);
        buttonWithCounter.getChildAt(2).setVisibility(View.GONE);
        mTextCounter = newTextCounter;
        mSendButtonMms = null;
        mSendButtonSms = (ImageButton) buttonWithCounter.getChildAt(3);

        if (mSendButtonMms != null) {
            mSendButtonMms.setVisibility(View.GONE);
        }
        if (mSendButtonSms != null) {
            mSendButtonSms.setVisibility(View.GONE);
        }
        mButtonSmall.setVisibility(View.VISIBLE);
        mButtonBig.setVisibility(View.VISIBLE);
    }

    @Override
    public void initDualSendBtnForAttachment(Context context, LinearLayout attachmentBtnLayout, Button sendButton,
            ViewOnClickListener listener) {
        MmsLog.d(TAG, "initDualSendBtnForAttachment");
        mContext = context;
        if (attachmentBtnLayout == null) {
            return;
        }
        mSendButton = sendButton;
        attachmentBtnLayout.setVisibility(View.VISIBLE);
        mButtonBig = (ImageButton) attachmentBtnLayout.getChildAt(0);
        mButtonBig.setVisibility(View.VISIBLE);
        mButtonSmall = (ImageButton) attachmentBtnLayout.getChildAt(1);
        mSiminfoList = MessageUtils.getSimInfoFromCache(mContext);
        int simCardSize = 0;
        if (mSiminfoList != null) {
            simCardSize = mSiminfoList.size();
        }
        if (simCardSize == 2) {
            mButtonSmall.setVisibility(View.VISIBLE);
        } else {
            mButtonSmall.setVisibility(View.GONE);
            android.view.ViewGroup.LayoutParams lp = mButtonBig.getLayoutParams();
            lp.width = getResources().getDimensionPixelOffset(R.dimen.attchment_view_send_button_length);
            mButtonBig.setLayoutParams(lp);
        }
        setDualSendButtonType(listener);
        MmsLog.d(TAG, "initDualSendBtnForAttachment END");
    }

    @Override
    public void hideDualButtonPanel() {

        if (mButtonBig != null) {
            mButtonBig.setVisibility(View.GONE);
        }

        if (mButtonSmall != null) {
            mButtonSmall.setVisibility(View.GONE);
        }
    }

    @Override
    public void showDualButtonPanel() {
        if (mButtonBig != null) {
            mButtonBig.setVisibility(View.VISIBLE);
        }

        if (mButtonSmall != null) {
            mButtonSmall.setVisibility(View.VISIBLE);
        }

    }

    @Override
    public void hideDualButtonAndShowSrcButton() {
        hideDualButtonPanel();
        if (mSendButtonSms != null) {
            mSendButtonSms.setVisibility(View.VISIBLE);
            mSendButtonSms.setEnabled(false);
        }
        if (mSendButtonMms != null) {
            mSendButtonMms.setText("");
            mSendButtonMms.setClickable(false);
            mSendButtonMms.setEnabled(false);
        }
        if (mSendButton != null) {
            mSendButton.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void setDualSendButtonType(final ViewOnClickListener btnListener) {
        if (mContext == null) {
            return;
        }
        mSiminfoList = MessageUtils.getSimInfoFromCache(mContext);
        int simCardSize = 0;
        if (mSiminfoList != null) {
            simCardSize = mSiminfoList.size();
        }
        Drawable bigImageId = null;
        Drawable smallImageId = null;
        Object[][] resIds = null;
        mButtonDrawable = new Drawable[2];
        Op09MmsUtilsExt mmsUtilExt = new Op09MmsUtilsExt(mPluginContext);
        MmsLog.d(TAG, "setDualSendButtonType-simCardSize:" + simCardSize);
        if (simCardSize == 0) {
            MmsLog.d(TAG, "setDualSendButtonType Failed, as simCardSize = 0;");
            return;
        } else if (simCardSize == 1) {
            int slotId = mSiminfoList.get(0).mSimSlotId;
            /// M: For get disable button for slot @{
            resIds = mmsUtilExt.getSendButtonResourceIdBySlotId(mContext, slotId, false);
            if (resIds != null) {
                mButtonDrawable[0] = (Drawable) resIds[0][0];
                mButtonDrawable[1] = (Drawable) resIds[1][0];
            }
            /// @}
            if (mButtonBig.isEnabled()) {
                resIds = mmsUtilExt.getSendButtonResourceIdBySlotId(mContext, slotId, true);
            }
            if (resIds == null) {
                return;
            }
            bigImageId = (Drawable) resIds[0][0];
            smallImageId = (Drawable) resIds[1][0];
        } else if (simCardSize == 2) {
            int defaultSimId = (int) Settings.System.getLong(mContext.getContentResolver(),
                Settings.System.SMS_SIM_SETTING, Settings.System.DEFAULT_SIM_NOT_SET);
            int slotId = 0;
            if (defaultSimId == Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK) {
                slotId = 0;
            } else if (defaultSimId == Settings.System.DEFAULT_SIM_NOT_SET) {
                slotId = 0;
            } else {
                slotId = MessageUtils.getSimInfoBySimId(mContext, defaultSimId).mSimSlotId;
            }
            MmsLog.d(TAG, "default SIM Slot Id:" + slotId);
            /// M: For get disable button for slot @{
            resIds = mmsUtilExt.getSendButtonResourceIdBySlotId(mContext, slotId, false);
            if (resIds != null) {
                mButtonDrawable[0] = (Drawable) resIds[0][0];
                mButtonDrawable[1] = (Drawable) resIds[1][0];
            }
            /// @}
            if (mButtonBig.isEnabled()) {
                resIds = mmsUtilExt.getSendButtonResourceIdBySlotId(mContext, slotId, mButtonBig.isEnabled());
            }
            if (resIds == null) {
                return;
            }
            bigImageId = (Drawable) resIds[0][0];
            smallImageId = (Drawable) resIds[1][0];
        }
        if (resIds == null) {
            return;
        }
        mButtonSlotIds = new int[2];
        mButtonColors = new int[2];

        mButtonSlotIds[0] = (Integer) resIds[0][1];
        mButtonSlotIds[1] = (Integer) resIds[1][1];
        mButtonColors[0] = (Integer) resIds[0][2];
        mButtonColors[1] = (Integer) resIds[1][2];

        View.OnClickListener buttonClickListener = new View.OnClickListener() {
            public void onClick(View v) {
                int send_sim_id = -1;
                if (v == mButtonBig) {
                    SimInfoManager.SimInfoRecord sm = MessageUtils.getSimInfoBySlotId(mContext, mButtonSlotIds[0]);
                    send_sim_id = (int) sm.mSimInfoId;
                } else if (v == mButtonSmall) {
                    SimInfoManager.SimInfoRecord sm = MessageUtils.getSimInfoBySlotId(mContext, mButtonSlotIds[1]);
                    send_sim_id = (int) sm.mSimInfoId;
                }
                btnListener.setSelectedSimId(send_sim_id);
                btnListener.onClick(v);
            }
        };

        mButtonBig.setImageDrawable(bigImageId);
        mButtonSmall.setImageDrawable(smallImageId);
        mButtonBig.setOnClickListener(buttonClickListener);
        mButtonSmall.setOnClickListener(buttonClickListener);

    }

    @Override
    public void updateNewTextCounter(int textLineCount, boolean isMms, int remainingInCurrentMessage, int msgCount) {
        if (mTextCounter == null) {
            return;
        }
        if (isMms) {
            mTextCounter.setText(this.getResources().getString(R.string.mms));
            mTextCounter.setVisibility(View.VISIBLE);
            return;
        }
        if (textLineCount <= 1) {
            mTextCounter.setVisibility(View.GONE);
            return;
        }
        String counterText = remainingInCurrentMessage + "/" + msgCount;
        mTextCounter.setText(counterText);
        mTextCounter.setVisibility(View.VISIBLE);
    }

    private void hideAllSrcButton(){
        if (mSendButtonMms != null) {
            mSendButtonMms.setVisibility(View.GONE);
        }
        if (mSendButtonSms != null) {
            mSendButtonSms.setVisibility(View.GONE);
        }
        if (mSendButton != null) {
            mSendButton.setVisibility(View.GONE);
        }
    }

    @Override
    public void updateDualSendButtonStatue(boolean enable, boolean isMms) {
        MmsLog.d(TAG, "updateDualSendButtonStatue  enable: " +enable + " isMms:"+isMms);
        if (mButtonBig == null || mButtonSmall == null) {
            return;
        }
        Map<Integer, SimInfoManager.SimInfoRecord> simInfoList = MessageUtils.getSimInfoFromCache(mContext);
        int simCount = 0;
        if (simInfoList == null) {
            simCount = 0;
        } else {
            simCount = simInfoList.size();
        }
        if (simCount > 0) {
            hideAllSrcButton();
        }
        /// M: correct Button's status picture @{
        boolean bigEnable = mButtonBig.isEnabled();
        boolean smallEnable = mButtonSmall.isEnabled();
        Op09MmsUtilsExt mmsUtilsExt = new Op09MmsUtilsExt(mPluginContext);
        if (bigEnable != enable && mButtonSlotIds != null && mButtonColors != null) {
            Drawable drawable = null;
            if (enable) {
                drawable = mmsUtilsExt.getActivatedButtonIconBySlotId(mButtonSlotIds[0], false, mButtonColors[0]);
            } else {
                if (mButtonDrawable != null) {
                    drawable = mButtonDrawable[0];
                }
            }
            if (drawable != null) {
                mButtonBig.setImageDrawable(drawable);
            }
        }
        if (smallEnable != enable && mButtonSlotIds != null && mButtonColors != null) {
            Drawable drawable = null;
            if (enable) {
                drawable = mmsUtilsExt.getActivatedButtonIconBySlotId(mButtonSlotIds[1], true, mButtonColors[1]);
            } else {
                if (mButtonDrawable != null) {
                    drawable = mButtonDrawable[1];
                }
            }
            if (drawable != null) {
                mButtonSmall.setImageDrawable(drawable);
            }
        }
        /// @}
        MmsLog.d(TAG, "updateDualSendButtonStatue simCount: " + simCount);
        if (simCount == 0) {
            mButtonBig.setEnabled(false);
            mButtonSmall.setEnabled(false);
            hideDualButtonPanel();
        } else if (simCount == 1) {
            mButtonBig.setEnabled(enable);
            mButtonBig.setVisibility(View.VISIBLE);
            mButtonSmall.setEnabled(false);
            mButtonSmall.setVisibility(View.GONE);
        } else if (simCount == 2) {
            mButtonBig.setEnabled(enable);
            mButtonSmall.setEnabled(enable);
            showDualButtonPanel();
        }
        mButtonBig.setFocusable(enable);
        if (isMms && mTextCounter != null) {
            mTextCounter.setText(getResources().getString(R.string.mms));
            mTextCounter.setVisibility(View.VISIBLE);
        }
        MmsLog.d(TAG, "updateDualSendButtonStatue END");
    }

    private void showMassTextMsgDetails(String[] items, OnClickListener clickListener, String btnStr, boolean showButton) {
        AlertDialog.Builder detailDialog = new AlertDialog.Builder(mContext);
        detailDialog.setTitle(this.getResources().getString(R.string.message_details_title));
        detailDialog.setCancelable(true);
        if (clickListener != null && showButton) {
            detailDialog.setPositiveButton(btnStr, clickListener);
        }
        detailDialog.setItems(items, null);
        detailDialog.show();
    }
}
