package com.mediatek.contacts.plugin.util;

import android.net.Uri;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.content.ContentResolver;
import android.content.Context;
import android.graphics.drawable.Drawable;

import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.op09.plugin.R;
import com.android.internal.telephony.PhoneConstants;

public class OP09ContactsDetailCallColor {
    private static final String TAG = OP09ContactsDetailCallColor.class.getSimpleName();

    int DEFAULT_SIM_PHOTO_ID = -1;
    int DEFAULT_SIM_PHOTO_ID_SDN = -9;

    String DEFAULT_SIM_PHOTO_URI_SDN = "content://sdn";
    String DEFAULT_SIM_PHOTO_URI = "content://sim";

    // M:alps00531578 & alps00512904
    int SIM_PHOTO_ID_1_ORANGE_SDN = -31;
    int SIM_PHOTO_ID_2_BLUE_SDN = -32;

    int SIM_PHOTO_ID_1_ORANGE = -33;
    int SIM_PHOTO_ID_2_BLUE = -34;

    String SIM_PHOTO_URI_1_ORANGE_SDN = "content://sdn-31";
    String SIM_PHOTO_URI_2_BLUE_SDN = "content://sdn-32";

    String SIM_PHOTO_URI_1_ORANGE = "content://sdn-33";
    String SIM_PHOTO_URI_2_BLUE = "content://sdn-34";

    // get slot1 and slot2
    private static final int SLOT_ID1 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1;
    private static final int SLOT_ID2 = com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2;

    private static OP09ContactsDetailCallColor instance = new OP09ContactsDetailCallColor();

    private OP09ContactsDetailCallColor() {

    }

    public static OP09ContactsDetailCallColor getInstance() {
        if (instance == null) {
            return new OP09ContactsDetailCallColor();
        }
        return instance;
    }

    public int getDefaultSlot(ContentResolver contentResolver) {
        SimInfoRecord mSimInfoOfDefaultSim = getDefaultSiminfo(contentResolver);

        return mSimInfoOfDefaultSim.mSimSlotId;
    }

    public int getNotDefaultSlot(ContentResolver contentResolver){
        SimInfoRecord mSimInfoOfDefaultSim = getDefaultSiminfo(contentResolver);
        
        return mSimInfoOfDefaultSim.mSimSlotId == PhoneConstants.GEMINI_SIM_1 ? PhoneConstants.GEMINI_SIM_2
                : PhoneConstants.GEMINI_SIM_1;
    }

    public Uri getCallUri(String number) {
        // !!!!! need to check below code again
        /*if (FeatureOption.MTK_GEMINI_SUPPORT) {
            return Uri.fromParts(Constants.SCHEME_TEL, number, null);
        } else {
            if (PhoneNumberUtils.isUriNumber(number)) {
                return Uri.fromParts(Constants.SCHEME_SIP, number, null);
            }*/
        return Uri.fromParts("tel", number, null);
        //}
    }

    public int getDrawableCorG(SimInfoRecord simInfo) {
        // M:alps00531578 & alps00512904
        int slot = simInfo.mSimSlotId;
        if (slot == SLOT_ID1) {
            return R.drawable.ic_contact_detail_call_1;
        } else {
            return R.drawable.ic_contact_detail_call_2;
        }

    }

    public SimInfoRecord getDefaultSiminfo(ContentResolver contentResolver) {
        final long mDefaultSim = Settings.System.getLong(contentResolver,
                Settings.System.VOICE_CALL_SIM_SETTING,
                Settings.System.DEFAULT_SIM_NOT_SET);
        SimInfoRecord simInfoOfDefaultSim = SIMInfoWrapper.getDefault().getSimInfoById((int)mDefaultSim);
        return simInfoOfDefaultSim;
    }

    public SimInfoRecord getNotDefaultSiminfo(ContentResolver contentResolver) {
        SimInfoRecord defaultInfo = getDefaultSiminfo(contentResolver);
        if (defaultInfo.mSimSlotId == PhoneConstants.GEMINI_SIM_1) {
            return SIMInfoWrapper.getDefault().getSimInfoBySlot(PhoneConstants.GEMINI_SIM_2);
        } else {
            return SIMInfoWrapper.getDefault().getSimInfoBySlot(PhoneConstants.GEMINI_SIM_1);
        }
    }

    public boolean isCDMAPhoneTypeBySlot(final int slot) {
        TelephonyManagerEx telephony = TelephonyManagerEx.getDefault();
        return telephony.getPhoneType(slot) == PhoneConstants.PHONE_TYPE_CDMA;
    }

    public Drawable getEnhancementAccountSimIndicator(int i, int slot, Context mPluginContext) {

        int photoId = 0;
        // M:alps00531578 & alps00512904
        if (slot == SLOT_ID1) {
            photoId = R.drawable.ic_contact_account_sim_1;
        } else if (slot == SLOT_ID2) {
            photoId = R.drawable.ic_contact_account_sim_2;
        }
        if (photoId == 0) {
            return null;
        }
        return mPluginContext.getResources().getDrawable(photoId);
    }

    /**
     * FOR CT NEW FEATURE
     */
    public long getEnhancementPhotoId(int isSdnContact, int colorId, int slot) {
        boolean isCDMA = isCDMAPhoneTypeBySlot(slot);
        long photoId = 0;
        if (com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT) {
            boolean isSdn = (isSdnContact > 0);
            Log.i(TAG, "[getSimType] i = " + colorId + " | isSdn : " + isSdn);
            // M:alps00531578 & alps00512904
            if (slot == SLOT_ID1) {
                if (isSdn) {
                    photoId = SIM_PHOTO_ID_1_ORANGE_SDN;
                } else {
                    photoId = SIM_PHOTO_ID_1_ORANGE;
                }
            } else if (slot == SLOT_ID2) {
                if (isSdn) {
                    photoId = SIM_PHOTO_ID_2_BLUE_SDN;
                } else {
                    photoId = SIM_PHOTO_ID_2_BLUE;
                }
            }
        } else {
            if (isSdnContact > 0) {
                photoId = DEFAULT_SIM_PHOTO_ID_SDN;
            } else {
                photoId = DEFAULT_SIM_PHOTO_ID;
            }
        }
        return photoId;
    }

    public String getEnhancementPhotoUri(int isSdnContact, int colorId, int slot) {
        boolean isCDMA = isCDMAPhoneTypeBySlot(slot);
        String photoUri = null;
        if (com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT) {

            boolean isSdn = (isSdnContact > 0);
            Log.i(TAG, "[onLoadFinished] i = " + colorId + " | isSdn : " + isSdn);
            // M:alps00531578 & alps00512904
            if (slot == SLOT_ID1) {
                if (isSdn) {
                    photoUri = SIM_PHOTO_URI_1_ORANGE_SDN;
                } else {
                    photoUri = SIM_PHOTO_URI_1_ORANGE;
                }
            } else if (slot == SLOT_ID2) {
                if (isSdn) {
                    photoUri = SIM_PHOTO_URI_2_BLUE_SDN;
                } else {
                    photoUri = SIM_PHOTO_URI_2_BLUE;
                }
            }
        } else {
            if (isSdnContact > 0) {
                photoUri = DEFAULT_SIM_PHOTO_URI_SDN;
            } else {
                photoUri = DEFAULT_SIM_PHOTO_URI;
            }
        }
        return photoUri;
    }

}
