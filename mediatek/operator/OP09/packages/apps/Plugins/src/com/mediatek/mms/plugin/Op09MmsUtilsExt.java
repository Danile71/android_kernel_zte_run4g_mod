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
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.preference.PreferenceManager;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.Sms;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ext.DefaultMmsUtilsExt;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;

public class Op09MmsUtilsExt extends DefaultMmsUtilsExt {
    private static final String TAG = "Mms/OP09MmsUtilsExt";
    public Op09MmsUtilsExt(Context context) {
        super(context);
    }

    public String formatDateAndTimeStampString(Context context, long msgDate, long msgDateSent,
            boolean fullFormat, String formatStr) {
        if (msgDateSent > 0) {
            return MessageUtils.formatDateOrTimeStampStringWithSystemSetting(this, msgDateSent,
                fullFormat);
        } else if (msgDate > 0) {
            return MessageUtils.formatDateOrTimeStampStringWithSystemSetting(this, msgDate,
                fullFormat);
        } else {
            return formatStr;
        }
    }

    public void showSimTypeBySimId(Context context, long simId, TextView textView) {
        Drawable simTypeDraw = null;
        MmsLog.d(TAG, "showSimTypeBySimId");
        SimInfoManager.SimInfoRecord simInfo = SimInfoManager.getSimInfoById(context, simId);
        int slotId = simInfo.mSimSlotId;
        if (simInfo != null) {
            simTypeDraw = getResources().getDrawable(simInfo.mSimBackgroundLightSmallRes);
        } else {
            simTypeDraw = getResources().getDrawable(R.drawable.sim_light_not_activated);
        }

        if (textView != null) {
            String text = textView.getText().toString().trim();
            textView.setText("  " + text + "  ");
            textView.setBackgroundDrawable(simTypeDraw);
        }
    }

    public Object[][] getSendButtonResourceIdBySlotId(Context context, int defaultSlotId) {
        /// M: [0][0]:The button's draw; [0][1]:SlotId; [0][2]:The siminfo.color
        Object[][] resourceIds = new Object[2][3];
        if (defaultSlotId == 0) {
            SIMInfo defaultSimInfo = SIMInfo.getSIMInfoBySlot(context, defaultSlotId);
            resourceIds[0][0] = getResources()
                    .getDrawable(MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_C_BIG[defaultSimInfo.mColor]);
            resourceIds[0][1] = 0;
            resourceIds[0][2] = defaultSimInfo.mColor;
            SIMInfo secondSimInfo = SIMInfo.getSIMInfoBySlot(context, 1);
            if (secondSimInfo == null) {
                resourceIds[1][0] = getResources().getDrawable(R.drawable.ct_send_2_small_blue_disable);
                resourceIds[1][2] = -1;
            } else {
                resourceIds[1][0] = getResources()
                        .getDrawable(MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_G_SMALL[secondSimInfo.mColor]);
                resourceIds[1][2] = secondSimInfo.mColor;
            }
            resourceIds[1][1] = 1;
        } else if (defaultSlotId == 1) {
            SIMInfo defaultSimInfo = SIMInfo.getSIMInfoBySlot(context, defaultSlotId);
            resourceIds[0][0] = getResources()
                    .getDrawable(MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_G_BIG[defaultSimInfo.mColor]);
            resourceIds[0][1] = 1;
            resourceIds[0][2] = defaultSimInfo.mColor;
            SIMInfo secondSimInfo = SIMInfo.getSIMInfoBySlot(context, 0);
            if (secondSimInfo == null) {
                resourceIds[1][0] = getResources().getDrawable(R.drawable.ct_send_1_small_orange_disable);
                resourceIds[1][2] = -1;
            } else {
                resourceIds[1][0] = getResources()
                        .getDrawable(MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_C_SMALL[secondSimInfo.mColor]);
                resourceIds[1][2] = secondSimInfo.mColor;
            }
            resourceIds[1][1] = 0;
        }
        return resourceIds;
    }

    public Object[][] getSendButtonResourceIdBySlotId(Context context, int defaultSlotId, boolean enable) {
        MmsLog.d(TAG, "getSendButtonResourceIdBySlotId: defaultSlotId:" + defaultSlotId + " enable:" + enable);
        Object[][] resourceIds = new Object[2][3];
        int resBigId = 0;
        int resSmallId = 0;
        if (defaultSlotId == 0) {
            SIMInfo defaultSimInfo = SIMInfo.getSIMInfoBySlot(context, defaultSlotId);
            if (enable) {
                resBigId = MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_C_BIG[defaultSimInfo.mColor];
            } else {
                resBigId = MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_C_BIG[defaultSimInfo.mColor];
            }
            resourceIds[0][0] = getResources().getDrawable(resBigId);
            resourceIds[0][1] = 0;
            resourceIds[0][2] = defaultSimInfo.mColor;
            SIMInfo secondSimInfo = SIMInfo.getSIMInfoBySlot(context, 1);
            if (secondSimInfo == null) {
                resourceIds[1][0] = getResources().getDrawable(R.drawable.ct_send_2_small_blue_disable);
                resourceIds[1][2] = -1;
            } else {
                if (enable) {
                    resSmallId = MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_G_SMALL[secondSimInfo.mColor];
                } else {
                    resSmallId = MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_G_SMALL[secondSimInfo.mColor];
                }
                resourceIds[1][0] = getResources().getDrawable(resSmallId);
                resourceIds[1][2] = secondSimInfo.mColor;
            }
            resourceIds[1][1] = 1;
        } else if (defaultSlotId == 1) {
            SIMInfo defaultSimInfo = SIMInfo.getSIMInfoBySlot(context, defaultSlotId);
            if (enable) {
                resBigId = MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_G_BIG[defaultSimInfo.mColor];
            } else {
                resBigId = MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_G_BIG[defaultSimInfo.mColor];
            }
            resourceIds[0][0] = getResources().getDrawable(resBigId);
            resourceIds[0][1] = 1;
            resourceIds[0][2] = defaultSimInfo.mColor;
            SIMInfo secondSimInfo = SIMInfo.getSIMInfoBySlot(context, 0);
            if (secondSimInfo == null) {
                resourceIds[1][0] = getResources().getDrawable(R.drawable.ct_send_1_small_orange_disable);
                resourceIds[1][2] = -1;
            } else {
                if (enable) {
                    resSmallId = MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_C_SMALL[secondSimInfo.mColor];
                } else {
                    resSmallId = MessageUtils.SEND_BUTTON_DRAWABLE_RESOURCE_ID_C_SMALL[secondSimInfo.mColor];
                }
                resourceIds[1][0] = getResources().getDrawable(resSmallId);
                resourceIds[1][2] = secondSimInfo.mColor;
            }
            resourceIds[1][1] = 0;
        }
        return resourceIds;
    }

    public boolean allowSafeDraft(final Activity activity, boolean deviceStorageIsFull, boolean isNofityUser,
            int toastType) {
        MmsLog.d(TAG, "allowSafeDraft: deviceStorageIsFull:" + deviceStorageIsFull + " isNotifyUser:" + isNofityUser);
        if (activity == null || !deviceStorageIsFull) {
            return true;
        }
        if (deviceStorageIsFull && !isNofityUser) {
            return false;
        }
        if (deviceStorageIsFull && isNofityUser) {
            final String str;
            switch(toastType) {
                case TOAST_TYPE_FOR_SAVE_DRAFT:
                    str = getResources().getString(R.string.memory_full_cannot_save);
                    break;
                case TOAST_TYPE_FOR_SEND_MSG:
                    str = getResources().getString(R.string.memory_full_cannot_send);
                    break;
                case TOAST_TYPE_FOR_ATTACH:
                    str = getResources().getString(R.string.memory_full_cannot_attach);
                    break;
                case TOAST_TYPE_FOR_DOWNLOAD_MMS:
                    str = getResources().getString(R.string.memory_full_cannot_download_mms);
                    break;
                default:
                    str = "";
                    break;
            }
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(activity.getApplicationContext(), str, Toast.LENGTH_LONG).show();
                }
            });
        }
        return false;
    }

    public String formatDateTime(Context context, long time, int formatFlags) {
        return MessageUtils.formatDateTime(context, time, formatFlags);
    }

    private static final String TEXT_SIZE = "message_font_size";
    private static final float DEFAULT_TEXT_SIZE = 18;
    private static final float MIN_TEXT_SIZE = 10;
    private static final float MAX_TEXT_SIZE = 32;

    public static float getTextSize(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        float size = sp.getFloat(TEXT_SIZE, DEFAULT_TEXT_SIZE);
        MmsLog.v(TAG, "getTextSize = " + size);
        if (size < MIN_TEXT_SIZE) {
            size = MIN_TEXT_SIZE;
        } else if (size > MAX_TEXT_SIZE) {
            size = MAX_TEXT_SIZE;
        }
        return size;
    }

    public static void setTextSize(Context context, float size) {
        float textSize;
        MmsLog.v(TAG, "setTextSize = " + size);

        if (size < MIN_TEXT_SIZE) {
            textSize = MIN_TEXT_SIZE;
        } else if (size > MAX_TEXT_SIZE) {
            textSize = MAX_TEXT_SIZE;
        } else {
            textSize = size;
        }
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = sp.edit();
        editor.putFloat(TEXT_SIZE, textSize);
        editor.commit();
    }

    public boolean isDialableForCT(String address) {
        if (address == null || address.length() <= 0) {
            return false;
        }
        /// M: Judge the first character is dialable.
        char firstC = address.charAt(0);
        if (!(firstC == '+' || (firstC >= '0' && firstC <= '9'))) {
            return false;
        }
        for (int i = 1, count = address.length(); i < count; i++) {
            char c = address.charAt(i);
            if (!(c >= '0' && c <= '9')) {
                return false;
            }
        }
        return true;
    }

    public boolean isWellFormedSmsAddress(String address) {
        if (!isDialableForCT(address)) {
            return false;
        }
        String networkPortion =
                PhoneNumberUtils.extractNetworkPortion(address);
        return (!(networkPortion.equals("+")
                  || TextUtils.isEmpty(networkPortion)))
               && isDialableForCT(networkPortion);
    }

    public Drawable getActivatedButtonIconBySlotId(int slotId, boolean smallIcon, int color) {
        Drawable drawable = null;
        switch (slotId) {
            case 0:
                if (smallIcon) {
                    if (color > -1) {
                        drawable = getResources().getDrawable(
                            MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_C_SMALL[color]);
                    } else {
                        drawable = getResources().getDrawable(R.drawable.ct_send_1_small_orange_disable);
                    }
                } else {
                    if (color > -1) {
                        drawable = getResources().getDrawable(
                            MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_C_BIG[color]);
                    } else {
                        drawable = getResources().getDrawable(R.drawable.ct_send_1_big_orange_disable);
                    }
                }
                break;
            case 1:
                if (smallIcon) {
                    if (color > -1) {
                        drawable = getResources().getDrawable(
                            MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_G_SMALL[color]);
                    } else {
                        drawable = getResources().getDrawable(R.drawable.ct_send_2_small_blue_disable);
                    }
                } else {
                    if (color > -1) {
                        drawable = getResources().getDrawable(
                            MessageUtils.SEND_BUTTON_ACTIVIATE_RESOURCE_ID_G_BIG[color]);
                    } else {
                        drawable = getResources().getDrawable(R.drawable.ct_send_2_big_blue_disable);
                    }
                }
                break;
            default:
                break;
        }
        return drawable;
    }

    public void setIntentDateForMassTextMessage(Intent intent, long groupId) {
        if (intent == null) {
            return;
        }
        intent.putExtra(MASS_TEXT_MESSAGE_GROUP_ID, groupId < 0 ? groupId : -1L);
    }

    public long getGroupIdFromIntent(Intent intent) {
        if (intent == null) {
            return -1L;
        }
        return intent.getLongExtra(MASS_TEXT_MESSAGE_GROUP_ID, -1L);
    }

    public Cursor getReportItemsForMassSMS(Context context, String[] projection, long groupId) {
        if (context == null || (groupId >= 0) || projection == null || projection.length < 1) {
            return null;
        }
        MmsLog.d(TAG, "getReportItemsForMassSMS, groupId:" + groupId);
        return context.getContentResolver().query(Sms.CONTENT_URI, projection, "ipmsg_id = ?",
            new String[] {groupId + ""}, null);
    }

    public boolean isCDMAType(Context context, int slotId) {
        return MessageUtils.isCDMAType(context, slotId);
    }
}
