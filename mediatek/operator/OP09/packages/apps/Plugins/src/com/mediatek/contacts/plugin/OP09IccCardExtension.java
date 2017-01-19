
package com.mediatek.contacts.plugin;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
//import android.provider.Telephony.SIMInfo;
import android.util.Log;

import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.IccCardExtension;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

public class OP09IccCardExtension extends IccCardExtension {

    private static final String TAG = OP09IccCardExtension.class.getSimpleName();

    private Context mPluginContext;

    
    public OP09IccCardExtension() {
    }

    public OP09IccCardExtension(Context context) {
        mPluginContext = context;
    }

    public String getCommond() {
        return ContactPluginDefault.COMMD_FOR_OP09;
    }

    public interface SimPhotoIdAndUri {

        int SIM_PHOTO_ID_1_ORANGE_SDN = -31;
        int SIM_PHOTO_ID_2_BLUE_SDN = -32;

        int SIM_PHOTO_ID_1_ORANGE = -33;
        int SIM_PHOTO_ID_2_BLUE = -34;

        String SIM_PHOTO_URI_1_ORANGE_SDN = "content://sdn-31";
        String SIM_PHOTO_URI_2_BLUE_SDN = "content://sdn-32";

        String SIM_PHOTO_URI_1_ORANGE = "content://sdn-33";
        String SIM_PHOTO_URI_2_BLUE = "content://sdn-34";

    }
    public Drawable getIconDrawableBySimInfoRecord(SimInfoRecord simInfo, String commd){//(SIMInfo simInfo) {

        Log.d(TAG, "getIconDrawableBySimInfoRecord commd = " + commd);
        if (!ContactPluginDefault.COMMD_FOR_OP09.equals(commd)) {
            return null;
        }
        int simInfoColor = -1;
        if (simInfo != null) {
            simInfoColor = simInfo.mColor;
        }

        int res = 0;

        if (simInfoColor == 0) {
            res = R.drawable.ic_contact_picture_sim_contact_2;
        } else if (simInfoColor == 1) {
            res = R.drawable.ic_contact_picture_sim_contact_1;
        }
        if (res != 0) {
            return mPluginContext.getResources().getDrawable(res);
        }
        return null;

    }

    public String getIccPhotoUriString(Bundle args, String commd) {

        Log.d(TAG, "getIccPhotoUriString commd = " + commd);
        String photoUri = null;

        boolean isSdn = args.getBoolean(IccCardExtension.KEY_IS_ICC_CONTACT_SDN);
        int colorId = args.getInt(IccCardExtension.KEY_ICC_COLOR_ID);// color//slot
        if (com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT) {

            Log.i(TAG, "[getIccPhotoUriString] colorId = " + colorId + " | isSdn : " + isSdn);
            if (colorId == 1) {
                if (isSdn) {
                    photoUri = SimPhotoIdAndUri.SIM_PHOTO_URI_1_ORANGE_SDN;
                } else {
                    photoUri = SimPhotoIdAndUri.SIM_PHOTO_URI_1_ORANGE;
                }
            } else if (colorId == 0) {
                if (isSdn) {
                    photoUri = SimPhotoIdAndUri.SIM_PHOTO_URI_2_BLUE_SDN;
                } else {
                    photoUri = SimPhotoIdAndUri.SIM_PHOTO_URI_2_BLUE;
                }
            }
        }
        return photoUri;
    }

    public long getIccPhotoId(Bundle args, String commd) {

        Log.d(TAG, "getIccPhotoId commd = " + commd);

        long photoId = 0;
        if (com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT) {
            boolean isSdn = args.getBoolean(IccCardExtension.KEY_IS_ICC_CONTACT_SDN);
            int colorId = args.getInt(IccCardExtension.KEY_ICC_COLOR_ID);// color//slot

            Log.i(TAG, "[getIccPhotoId] colorId = " + colorId + " | isSdn : " + isSdn);

            if (colorId == 1) {// if (color == 1)//slot == SLOT_ID1
                if (isSdn) {
                    photoId = SimPhotoIdAndUri.SIM_PHOTO_ID_1_ORANGE_SDN;
                } else {
                    photoId = SimPhotoIdAndUri.SIM_PHOTO_ID_1_ORANGE;
                }
            } else if (colorId == 0) {// if (color ==0)
                if (isSdn) {
                    photoId = SimPhotoIdAndUri.SIM_PHOTO_ID_2_BLUE_SDN;
                } else {
                    photoId = SimPhotoIdAndUri.SIM_PHOTO_ID_2_BLUE;
                }
            }
        }
        return photoId;
    }

    public Drawable getIccPhotoDrawable(Bundle args, String commd) {

        Log.d(TAG, "getIccPhotoDrawable commd = " + commd);
        if (!ContactPluginDefault.COMMD_FOR_OP09.equals(commd)) {
            return null;
        }

        long photoId = args.getLong(IccCardExtension.KEY_PHOTO_ID);

        if (photoId == 0) {
            Uri uri = args.getParcelable(IccCardExtension.KEY_PHOTO_URI);
            photoId = getPhotoIdByPhotoUri(uri);
        }
        boolean darkTheme = args.getBoolean(IccCardExtension.KEY_IS_DARK_THEME);
        int id = (int) photoId;
        int res = 0;
        Log.d(TAG, "applySimDefaultImage photoId = " + photoId);

        switch (id) {
            case SimPhotoIdAndUri.SIM_PHOTO_ID_1_ORANGE:
                if (darkTheme) {
                    res = R.drawable.ic_contact_picture_sim_hold_dark_1;
                } else {
                    res = R.drawable.ic_contact_picture_sim_contact_1;
                }
                break;

            case SimPhotoIdAndUri.SIM_PHOTO_ID_2_BLUE:
                if (darkTheme) {
                    res = R.drawable.ic_contact_picture_sim_hold_dark_2;
                } else {
                    res = R.drawable.ic_contact_picture_sim_contact_2;
                }
                break;

            case SimPhotoIdAndUri.SIM_PHOTO_ID_1_ORANGE_SDN:
                if (darkTheme) {
                    res = R.drawable.ic_contact_picture_sdn_hold_dark_1;
                } else {
                    res = R.drawable.ic_contact_picture_sdn_contact_1;
                }
                break;

            case SimPhotoIdAndUri.SIM_PHOTO_ID_2_BLUE_SDN:
                if (darkTheme) {
                    res = R.drawable.ic_contact_picture_sdn_hold_dark_2;
                } else {
                    res = R.drawable.ic_contact_picture_sdn_contact_2;
                }
                break;

            default:
                break;
        }
        Log.d(TAG, "applySimDefaultImage res = " + res);
        if (res != 0) {
            return mPluginContext.getResources().getDrawable(res);
        }
        return null;
    }

    public static long getPhotoIdByPhotoUri(Uri uri) {
        long id = 0;

        if (uri == null) {
            Log.e(TAG, "getPhotoIdByPhotoUri uri is null");
            return id;
        }

        String photoUri = uri.toString();
        Log.i(TAG, "getPhotoIdByPhotoUri uri : " + photoUri);

        if (SimPhotoIdAndUri.SIM_PHOTO_URI_1_ORANGE.equals(photoUri)) {
            id = SimPhotoIdAndUri.SIM_PHOTO_ID_1_ORANGE;
        } else if (SimPhotoIdAndUri.SIM_PHOTO_URI_2_BLUE.equals(photoUri)) {
            id = SimPhotoIdAndUri.SIM_PHOTO_ID_2_BLUE;
        } else if (SimPhotoIdAndUri.SIM_PHOTO_URI_1_ORANGE_SDN.equals(photoUri)) {
            id = SimPhotoIdAndUri.SIM_PHOTO_ID_1_ORANGE_SDN;
        } else if (SimPhotoIdAndUri.SIM_PHOTO_URI_2_BLUE_SDN.equals(photoUri)) {
            id = SimPhotoIdAndUri.SIM_PHOTO_ID_2_BLUE_SDN;
        }

        Log.i(TAG, "getSimIdByUri id : " + id);
        return id;
    }

}
