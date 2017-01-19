package com.mediatek.settings.plugin;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.preference.EditTextPreference;
import android.preference.PreferenceScreen;
import android.provider.Telephony;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.op09.plugin.R;
import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.xlog.Xlog;

public class OP09ApnSettingsImp extends DefaultApnSettingsExt {

    private static final String TAG = "OP09ApnSettingsImp";
    private EditTextPreference mPppEditTextPreference;
    private Context mContext;

    private static final String CHINA_NW_MCC = "460";
    private static final String MACOO_NW_MCC = "455";

    public OP09ApnSettingsImp(Context context) {
        mContext = context;
    }

    /**
     * Add a preference in the prefernce screen according to the slotId
     * CT need PPP dialog for connect internet, so add PPP,
     * Customize apn titles 
     */
    @Override
    public void customizePreference(int slotId, PreferenceScreen root) {
        if (PhoneConstants.GEMINI_SIM_1 == slotId) {
            mPppEditTextPreference = new EditTextPreference(root.getContext(),
                    null);
            mPppEditTextPreference.setKey(mContext.getResources().getString(
                    R.string.ppp_dialing));
            mPppEditTextPreference.setTitle(mContext.getResources().getString(
                    R.string.ppp_dialing));
            mPppEditTextPreference.setDialogTitle(mContext.getResources()
                    .getString(R.string.ppp_dialing));
            root.addPreference(mPppEditTextPreference);

            // PPP Phone number already added.
            root.findPreference("apn_name").setTitle(
                    mContext.getString(R.string.apn_name));
            root.findPreference("apn_apn").setTitle(
                    mContext.getString(R.string.apn_apn));
            root.findPreference("auth_type").setTitle(
                    mContext.getString(R.string.apn_auth_type));
            root.findPreference("apn_user").setTitle(
                    mContext.getString(R.string.apn_user));
            root.findPreference("apn_password").setTitle(
                    mContext.getString(R.string.apn_password));
        }
    }

    /**
     * Customize apn projection, such as add Telephony.Carriers.PPP Called at
     * onCreate in ApnEditor.
     */
    @Override
    public String[] customizeApnProjection(String[] projection) {
        if (Telephony.Carriers.PPP.equals(projection[projection.length - 1])) {
            return projection;
        }
        String[] newStrs = new String[projection.length + 1];
        for (int i = 0; i < projection.length; i++) {
            newStrs[i] = projection[i];
        }
        newStrs[newStrs.length - 1] = Telephony.Carriers.PPP;
        return newStrs;
    }

    /**
     * Save the added apn values called when save the added apn vaule in
     * apnEditor.
     */
    @Override
    public void saveApnValues(ContentValues contentValues) {
        contentValues.put(Telephony.Carriers.PPP, getPppDialingText());
    }

    /**
     * Set the preference text and summary according to the slotId. called at
     * update UI, in ApnEditor.
     */
    @Override
    public void setPreferenceTextAndSummary(int slotId, String text) {
        if (PhoneConstants.GEMINI_SIM_1 == slotId) {
            mPppEditTextPreference.setText(text);
            mPppEditTextPreference.setSummary(text);
        }
    }

    /**
     * Update the customized status(enable , disable)
     * Called at update screen status
     */
    @Override
    public void updateFieldsStatus(int slotId, PreferenceScreen root) {
        if (PhoneConstants.GEMINI_SIM_1 == slotId) {
            root.findPreference("apn_mms_proxy").setEnabled(false);
            root.findPreference("apn_mms_port").setEnabled(false);
            root.findPreference("apn_mmsc").setEnabled(false);
            root.findPreference("apn_mcc").setEnabled(false);
            root.findPreference("apn_mnc").setEnabled(false);
        }
    }

    /**
     * M: For CT fature :add ppp_dialing use class
     * ApnEditor return the ppp_dialing's text
     */
    private String getPppDialingText() {
        if (mPppEditTextPreference != null) {
            return mPppEditTextPreference.getText();
        } else {
            return mContext.getResources().getString(R.string.apn_not_set);
        }
    }

    // For CT-A feature to hide CTWAP in roaming state
    @Override
    public String getFillListQuery(String numeric, int slotId) {
        String result;
        String sqlStr = "";
        String apn = "";
        String sourceType = "";
        try {
            ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
            String mvnoType = telephony.getMvnoMatchType(slotId);
            String mvnoPattern = telephony.getMvnoPattern(mvnoType,slotId);
            // If mvnoType or mvnoPattern is null, should replace with ''
            sqlStr = " mvno_type=\'" + replaceNullString(mvnoType) + "\'" +
            " and mvno_match_data=\"" + replaceNullString(mvnoPattern) + "\"";
        }  catch (android.os.RemoteException e) {
            Xlog.d(TAG, "RemoteException " + e);
        }

        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            String plmnNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_OPERATOR_NUMERIC);
            if (plmnNumeric != null && plmnNumeric.length() >= 3 && !plmnNumeric.startsWith(CHINA_NW_MCC)
                    && !numeric.startsWith(MACOO_NW_MCC)) {
                apn += " and apn <> \"ctwap\"";
                sourceType += " or sourceType = \"1\"";
                result = "numeric=\"" + numeric + "\" and " + 
                "((" + sqlStr + apn + ")" + " or (sourceType = \"1\"))" ;
                Xlog.d(TAG, "getFillListQuery result=" + result);
                return result;
            }
        }
        result = "numeric=\"" + numeric + "\" and " + sqlStr ;
        Xlog.d(TAG, "getFillListQuery result=" + result);
        return result;
    }

    private String replaceNullString(String origString) {
        if (origString == null) {
            return "";
        } else {
            return origString;
        }
    }

    @Override
    public Cursor customizeQueryResult(Activity activity, Cursor cursor, Uri uri, String numeric) {
        return cursor;
    }

    @Override
    public boolean defaultApnCanDelete() {
        Xlog.d(TAG, "defaultApnCanDelete true");
        return true;
    }
}
