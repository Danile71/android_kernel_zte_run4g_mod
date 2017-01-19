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
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.provider.Settings;
import android.provider.Telephony.Mms;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.Sms;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduPersister;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsMessageListItemExt;
import com.mediatek.mms.ext.IMmsCancelDownloadExt;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;

import java.util.Calendar;

public class Op09MmsMessageListItemExt extends DefaultMmsMessageListItemExt {
    private static final String TAG = "Mms/Op09MmsMessageListItemExt";

    private static final String[] MASS_SMS_PROJECTION = new String[] {Sms.THREAD_ID, Sms._ID, Sms.TYPE, Sms.DATE};

    private static final int COLUMN_MSG_SMS_THREAD_ID = 0;
    private static final int COLUMN_MSG_SMS_ID = 1;
    private static final int COLUMN_MSG_SMS_TYPE = 2;
    private static final int COLUMN_MSG_SMS_DATE = 3;

    private static final int MSG_TYPE_SMS = 0;

    public Op09MmsMessageListItemExt(Context context) {
        super(context);
        MessageUtils.initSiminfoList(context);
    }

    /**
     * M: show Sim Type Indicator
     * 
     * @param context
     * @param simId
     * @param textView
     */
    public void showSimType(Context context, int simId, TextView textView) {
        Drawable simTypeDraw = null;

        SimInfoManager.SimInfoRecord simInfo = MessageUtils.getSimInfoBySimId(context, simId);
        if (simInfo != null) {
            simTypeDraw = getResources().getDrawable(simInfo.mSimBackgroundLightSmallRes);
        } else {
            simTypeDraw = getResources().getDrawable(R.drawable.sim_light_not_activated);
        }
        MmsLog.d(TAG, "showSimType for MessageListItem");

        if (textView != null) {
            textView.setText("");
            textView.setBackgroundDrawable(simTypeDraw);
            MmsLog.d(TAG, "showSimType for MessageListItem :" + textView.getText());
        }
    }

    public String getSentDateStr(Context context, String srcTxt, long msgId, int msgType, long smsSentDate, int boxId) {
        MmsLog.d(TAG, "smsSentDate:" + smsSentDate + " msgType:" + msgType);
        if (msgType == PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
            return srcTxt;
        }
        if (boxId == Mms.MESSAGE_BOX_INBOX) {
            if (smsSentDate > 0) {
                /// M: this is sms date
                return MessageUtils.getShortTimeString(context, smsSentDate);
            }
            if (msgType != MSG_TYPE_SMS) {
                /// M: this is mms date
                Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, msgId);
                MultimediaMessagePdu msg;
                try {
                    GenericPdu pdu = PduPersister.getPduPersister(context).load(uri);
                    if (pdu.getMessageType() == PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                        return srcTxt;
                    }
                    msg = (MultimediaMessagePdu) PduPersister.getPduPersister(context).load(uri);
                } catch (MmsException e) {
                    MmsLog.e(TAG, "Failed to load the message: " + uri, e);
                    return srcTxt;
                }
                return MessageUtils.getShortTimeString(context, msg.getDateSent() * 1000L);
            }
        }
        return srcTxt;
    }

    public void drawMassTextMsgStatus(Context context, LinearLayout statusLayout, boolean isSms, long timestamp) {

        MmsLog.d(TAG, "drawMassTextMsgStatus; context:" + context + "\tisSMS:" + isSms + "\ttimeStamp:" + timestamp);
        if (context == null || !isSms || timestamp >= 0) {
            drawMassTextMsgStatus(statusLayout, 0, 0, 0, "");
            return;
        }
        int sendingCount = 0;
        int sendSuccess = 0;
        int sendFailed = 0;
        long sendDate = 0;
        Cursor cursor = context.getContentResolver().query(Sms.CONTENT_URI, MASS_SMS_PROJECTION, " ipmsg_id = ? ",
            new String[] {timestamp + ""}, null);
        try {
            if (cursor == null || cursor.getCount() < 2) {
                sendingCount = 0;
                sendSuccess = 0;
                sendFailed = 0;
            } else {
                while (cursor.moveToNext()) {
                    int type = cursor.getInt(COLUMN_MSG_SMS_TYPE);
                    sendDate = cursor.getLong(COLUMN_MSG_SMS_DATE);
                    switch (type) {
                        case Sms.MESSAGE_TYPE_OUTBOX:
                        case Sms.MESSAGE_TYPE_QUEUED:
                            sendingCount++;
                            break;
                        case Sms.MESSAGE_TYPE_SENT:
                            sendSuccess++;
                            break;
                        case Sms.MESSAGE_TYPE_FAILED:
                            sendFailed++;
                            break;
                        default:
                            break;
                    }
                }
            }
        } catch (SQLiteException e) {
            MmsLog.e(TAG, "drawMassTextMsgStatus error.", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        MmsLog.d(TAG, "sendingCount:" + sendingCount + " sendSuccess:" + sendSuccess + " sendFailed:" + sendFailed);
        drawMassTextMsgStatus(statusLayout, sendingCount, sendSuccess, sendFailed, MessageUtils.getShortTimeString(
            context, sendDate));
    }

    public boolean needEditFailedMessge(Context context, long msgId, long timeStamp) {
        if (timeStamp >= 0) {
            return true;
        }
        Cursor cursor = context.getContentResolver().query(Sms.CONTENT_URI, MASS_SMS_PROJECTION,
            " ipmsg_id < 0 and ipmsg_id = ? ", new String[] {timeStamp + ""}, null);
        try {
            if (cursor == null || cursor.getCount() < 2) {
                return true;
            }
        } catch (SQLiteException e) {
            MmsLog.e(TAG, "needEditFailedMessge error.", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return false;
    }

    public boolean setNotifyContent(String address, String subject, String msgSizeText, String expireText,
            TextView expireTextView) {
        if (expireTextView == null) {
            return true;
        }
        expireTextView.setText(address + "\n" + subject + "\n" + msgSizeText + "\n" + expireText);
        return true;
    }

    public void setDualTime(Context context, boolean isRecievedMsg, int simId, TextView dateView,
            LinearLayout linearLayout, String timeDate) {
        if (!isRecievedMsg || linearLayout == null) {
            MmsLog.d(TAG, "Do not set dual time, just return");
            return;
        }

        /// M: Set sent time text
        dateView.setVisibility(View.VISIBLE);
        dateView.setText(getResources().getString(R.string.time_send));

        linearLayout.setVisibility(View.VISIBLE);
        /// M:Set text for receive time
        TextView tv = (TextView) linearLayout.getChildAt(0);
        tv.setText(getResources().getString(R.string.time_receive));

        /// M: set time for receive time
        TextView tv2 = (TextView) linearLayout.getChildAt(1);
        tv2.setText(timeDate);

        /// M: Get the sim card's status of roaming.
        boolean isInternational = MessageUtils.isInternationalRoamingStatusBySimid(context, simId);
        MmsLog.d(TAG, "ternational status:" + isInternational);

        /// M: set timeZone
        TextView timeZone = (TextView) linearLayout.getChildAt(2);
        Calendar cal = Calendar.getInstance();
        String displayName = cal.getTimeZone().getDisplayName();
        if (isInternational) {
            /// M: Get the status of auto time zone. 1: auto get time zone; 0: no;
            int autoTimeZone = Settings.System.getInt(context.getContentResolver(), Settings.Global.AUTO_TIME_ZONE, 1);
            MmsLog.d(TAG, "Auto time zone:" + autoTimeZone);
            String date = (String) dateView.getText();
            if (autoTimeZone == 0) {
                timeZone.setText(displayName);
            } else {
                timeZone.setText(getResources().getString(R.string.local_time_msg));
            }
        } else {
            timeZone.setVisibility(View.GONE);
        }
    }

    /// M: --------------------------OP09 Plug-in Re-factory-------------------------
    private Button mDownloadButton;
    private TextView mDownloadingLabel;
    /// M: the host context;
    private Context mContext;
    private Activity mActivity;
    private TextView mExpireText;

    @Override
    public void initDownloadLayout(Activity activity, LinearLayout downladBtnLayout, TextView expireText) {
        mContext = activity;
        mActivity = activity;
        mExpireText = expireText;
        if (downladBtnLayout == null) {
            return;
        }
        
        downladBtnLayout.setVisibility(View.VISIBLE);
        mDownloadButton = (Button) downladBtnLayout.getChildAt(0);
        mDownloadingLabel = (TextView) downladBtnLayout.getChildAt(1);
    }

    @Override
    public void showDownloadButton(final Uri messageUri, final CheckBox selectedBox, final long msgId,
            final boolean deviceStorageIsFull, final OnClickListener downloadBtnListener,final OnClickListener canceldownloadListener) {
        if (mDownloadButton == null) {
            return;
        }
        mDownloadButton.setVisibility(View.VISIBLE);

        Uri notificationUri = ContentUris.withAppendedId(Mms.CONTENT_URI, msgId);
        final Op09MmsCancelDownloadExt mmsCancelDownload = new Op09MmsCancelDownloadExt(mContext);
        int actionStatus = mmsCancelDownload.getStateExt(notificationUri);
        MmsLog.d(TAG, "MessageListItemHost. show download BUTOON;" + actionStatus);
        if (actionStatus == IMmsCancelDownloadExt.STATE_DOWNLOADING
            || actionStatus == IMmsCancelDownloadExt.STATE_CANCELLING
            || actionStatus == IMmsCancelDownloadExt.STATE_ABORTED) {
            hideDownloadButton(messageUri, canceldownloadListener, selectedBox, msgId);
            return;
        }

        if (mDownloadButton != null) {
            mDownloadButton.setVisibility(View.VISIBLE);
            mDownloadButton.setEnabled(true);
            mDownloadButton.setText(this.getResources().getString(R.string.download));
            mDownloadButton.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    // TODO Auto-generated method stub
                    if (selectedBox != null && selectedBox.getVisibility() == View.VISIBLE) {
                        return;
                    }
                    Op09MmsUtilsExt mmsUtilsExt = new Op09MmsUtilsExt(mContext);
                    if (!mmsUtilsExt.allowSafeDraft(mActivity, deviceStorageIsFull, true,
                        Op09MmsUtilsExt.TOAST_TYPE_FOR_DOWNLOAD_MMS)) {
                        return;
                    }

                    int simId = 0;
                    Cursor cursor = SqliteWrapper.query(mContext, mContext.getContentResolver(), messageUri,
                        new String[] {"sim_id"}, null, null, null);
                    if (cursor != null) {
                        try {
                            if (cursor.getCount() == 1 && cursor.moveToFirst()) {
                                simId = cursor.getInt(0);
                            }
                        } finally {

                            cursor.close();
                        }
                    }
                    if (deviceStorageIsFull && showStorageFullToast(mContext)) {
                        return;
                    }

                    mmsCancelDownload.markStateExt(messageUri, Op09MmsCancelDownloadExt.STATE_DOWNLOADING);
                    mDownloadingLabel.setVisibility(View.VISIBLE);
                    mDownloadButton.setVisibility(View.GONE);
                    mDownloadButton.setEnabled(false);
                    downloadBtnListener.onClick(v);
                    v.setVisibility(View.GONE);
                    hideDownloadButton(messageUri, canceldownloadListener, selectedBox, msgId);
                }
            });
        }
        if (mDownloadingLabel != null) {
            mDownloadingLabel.setVisibility(View.GONE);
        }
    }

    @Override
    public void hideDownloadButton(final Uri messageUri,final OnClickListener canceldownloadListener, final CheckBox selectedBox, long msgId) {
        MmsLog.d(TAG, "hideDownloadButton");
        /**
         * Hide the download button and show the cancel download button. Before show cancel download button should check
         * the Mms setting of downloading Mms
         */
        if (mDownloadButton != null) {
            mDownloadButton.setVisibility(View.GONE);
        }
        Uri notificationUri = ContentUris.withAppendedId(Mms.CONTENT_URI, msgId);
        final Op09MmsCancelDownloadExt mmsCancelDownloadExt = new Op09MmsCancelDownloadExt(mContext);
        int actionStatus = mmsCancelDownloadExt.getStateExt(notificationUri);
        MmsLog.d(TAG, "MessageListItemHost. Mms CacenlingStatus:" + actionStatus);
        if (actionStatus == IMmsCancelDownloadExt.STATE_DOWNLOADING) {
            if (mDownloadButton != null) {
                mDownloadButton.setText(this.getResources().getString(R.string.mms_cancel));
                mDownloadButton.setEnabled(true);
                mDownloadButton.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        // add for multi-delete
                        v.setEnabled(false);
                        if (selectedBox != null && selectedBox.getVisibility() == View.VISIBLE) {
                            return;
                        }
                        canceldownloadListener.onClick(v);
                    }
                });
                mDownloadButton.setVisibility(View.VISIBLE);
            }
        }

        if (mDownloadingLabel != null) {
            mDownloadingLabel.setVisibility(View.VISIBLE);
        }
    }

    public void hideAllButton() {
        if (mDownloadButton != null) {
            mDownloadButton.setVisibility(View.GONE);
        }
        if (mDownloadingLabel != null) {
            mDownloadingLabel.setVisibility(View.GONE);
        }
        if (mExpireText != null) {
            mExpireText.setVisibility(View.GONE);
        }
    }

    public void drawMassTextMsgStatus(LinearLayout statusLayout, int sendingMsgCount, int sendMsgSuccessCount,
            int sendMsgFailedCount, String smsDate) {
        MmsLog.d(TAG, "sendingMsgCount:" + sendingMsgCount + " sendMsgSuccessCount:" + sendMsgSuccessCount
            + " sendMsgFailedCount:" + sendMsgFailedCount);
        // .findViewById(R.id.delivered_sending);
        ImageView sendingImg = (ImageView) statusLayout.getChildAt(2);
        // this.findViewById(R.id.delivered_sending_txt)
        TextView sendingTxt = (TextView) statusLayout.getChildAt(3);

        // this.findViewById(R.id.delivered_success);
        ImageView successImg = (ImageView) statusLayout.getChildAt(4);
        // this.findViewById(R.id.delivered_success_txt);
        TextView successTxt = (TextView) statusLayout.getChildAt(5);

        // this.findViewById(R.id.delivered_failed);
        ImageView failedImg = (ImageView) statusLayout.getChildAt(6);
        // this.findViewById(R.id.delivered_failed_txt);
        TextView failedTxt = (TextView) statusLayout.getChildAt(7);

        ImageView deliveredIndicator = (ImageView) statusLayout.getChildAt(8);
        TextView dateView = (TextView) statusLayout.getChildAt(1);
        if ((sendMsgSuccessCount + sendMsgFailedCount + sendingMsgCount) < 2) {
            if (sendingImg != null) {
                sendingImg.setVisibility(View.GONE);
                sendingTxt.setVisibility(View.GONE);
            }
            if (successImg != null) {
                successImg.setVisibility(View.GONE);
                successTxt.setVisibility(View.GONE);
            }
            if (failedImg != null) {
                failedImg.setVisibility(View.GONE);
                failedTxt.setVisibility(View.GONE);
            }
            return;
        }
        if (deliveredIndicator != null) {
            deliveredIndicator.setVisibility(View.GONE);
        }
        if (sendingMsgCount <= 0) {
            if (sendingImg != null) {
                sendingImg.setVisibility(View.GONE);
            }
            if (sendingTxt != null) {
                sendingTxt.setVisibility(View.GONE);
            }
        } else {
            if (sendingImg != null) {
                /// M: keep the same action with the common.
                sendingImg.setVisibility(View.VISIBLE);
                sendingTxt.setVisibility(View.GONE);
            }
            if (successImg != null) {
                successImg.setVisibility(View.GONE);
                successTxt.setVisibility(View.GONE);
            }
            if (failedImg != null) {
                failedImg.setVisibility(View.GONE);
                failedTxt.setVisibility(View.GONE);
            }
            return;
        }
        if (sendMsgSuccessCount <= 0) {
            if (successImg != null) {
                successImg.setVisibility(View.GONE);
            }
            if (successTxt != null) {
                successTxt.setVisibility(View.GONE);
            }
        } else {
            if (successImg != null) {
                if (sendMsgFailedCount > 0) {
                    successImg.setVisibility(View.VISIBLE);
                    successTxt.setVisibility(View.VISIBLE);
                    successTxt.setText(sendMsgSuccessCount + "");
                } else {
                    successImg.setVisibility(View.GONE);
                    successTxt.setVisibility(View.GONE);
                    if (deliveredIndicator != null) {
                        deliveredIndicator.setVisibility(View.VISIBLE);
                    }
                }
            }
        }

        if (sendMsgFailedCount <= 0) {
            if (failedImg != null) {
                failedImg.setVisibility(View.GONE);
            }
            if (failedTxt != null) {
                failedTxt.setVisibility(View.GONE);
            }
        } else {
            if (failedImg != null) {
                failedImg.setVisibility(View.VISIBLE);
                if (sendMsgSuccessCount > 0) {
                    failedTxt.setVisibility(View.VISIBLE);
                    failedTxt.setText(sendMsgFailedCount + "");
                } else {
                    failedTxt.setVisibility(View.GONE);
                }
            }
        }

        /// M: always show sent time;
        if (dateView != null && ((sendingMsgCount + sendMsgFailedCount + sendMsgSuccessCount) > 1)) {
            dateView.setVisibility(View.VISIBLE);
            dateView.setText(smsDate);
        }
    }

}
