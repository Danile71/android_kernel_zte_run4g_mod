package com.mediatek.systemui.plugin;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.SystemProperties;
import android.provider.Telephony;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.op09.plugin.R;
import com.mediatek.systemui.statusbar.util.SIMHelper;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.List;

/**
 * Support for APN state change tracker.
 *
 */
public final class APNStateTracker {
    private static final String TAG = "APNStateTracker";
    private static final String APN_URI = "content://telephony/carriers_sim1/preferapn";
    private static final String APN_ID = "apn_id";

    //The APN ID for CTWAP and CTNET in database.
    private static final String APN_CTWAP = "888";
    private static final String APN_CTNET = "889";
    private static final int ID_INDEX = 0;
    private static final int NAME_INDEX = 1;
    private static final int APN_INDEX = 2;
    private static final int TYPES_INDEX = 3;
    private static final int SOURCE_TYPE_INDEX = 4;
    private Uri mRestoreCarrierUri;
    private Uri mUri;
    private Context mContext;
    private String mSelectedKey;
    private String mNextKey;
    private String mImsi;
    private String mSpn;
    private List<String> mApnList;
    private String mNumeric = "-1";
    private boolean mFilled;

    APNStateTracker(Context context) {
        mContext = context;
        mNumeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
        mUri = Uri.parse("content://telephony/carriers_sim1");
        mRestoreCarrierUri = Uri.parse(APN_URI);
        mApnList = new ArrayList<String>();
        mApnList.add(APN_CTWAP);
        mApnList.add(APN_CTNET);
        fillList();
        mNextKey = getNextKey(mSelectedKey);
        Xlog.d(TAG, "APNStateTracker init : mSelectedKey = " + mSelectedKey
                + " mNextKey = " + mNextKey + " mNumeric = " + mNumeric);
    }

    public void toggleState() {
        Xlog.d(TAG, "APNStateTracker toggleState : mSelectedKey = "
                + mSelectedKey + " mNextKey = " + mNextKey);
        fillList();
        setSelectedApnKey(mNextKey);
    }

    public int getResource() {
        fillList();
        mSelectedKey = getSelectedApnKey();
        mNextKey = getNextKey(mSelectedKey);
        int resId = -1;
        Xlog.d(TAG, " mSelectedKey = " + mSelectedKey);
        if (mSelectedKey != null && mSelectedKey.equals(mApnList.get(1))) {
            resId = R.drawable.toolbar_net_ctnet;
        } else if (mSelectedKey != null && mSelectedKey.equals(mApnList.get(0))) {
            resId = R.drawable.toolbar_net_ctwap;
        } else {
            resId = R.drawable.toolbar_net_others;
        }
        int state = SIMHelper
                .getSimIndicatorStateGemini(PhoneConstants.GEMINI_SIM_1);
        if (!SIMHelper.isSimInserted(PhoneConstants.GEMINI_SIM_1)
                || PhoneConstants.SIM_INDICATOR_LOCKED == state
                || PhoneConstants.SIM_INDICATOR_SEARCHING == state
                || PhoneConstants.SIM_INDICATOR_INVALID == state
                || PhoneConstants.SIM_INDICATOR_RADIOOFF == state) {
            resId = R.drawable.toolbar_net_switch_disable;
        }
        return resId;
    }

    private void setSelectedApnKey(String key) {
        if (key != null && key.equals(mSelectedKey) || key == null) {
            return;
        }

        mNextKey = mSelectedKey;
        mSelectedKey = key;

        ContentResolver resolver = mContext.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(APN_ID, mSelectedKey);
        resolver.update(mRestoreCarrierUri, values, null, null);
    }

    private String getSelectedApnKey() {
        String key = mApnList.get(0);
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(mRestoreCarrierUri,
                    new String[] { "_id" }, null, null,
                    Telephony.Carriers.DEFAULT_SORT_ORDER);
            if (cursor != null && cursor.getCount() > 0) {
                cursor.moveToFirst();
                key = cursor.getString(ID_INDEX);
            }
        } finally {
            cursor.close();
        }
        Xlog.d(TAG, "APNStateTracker getSelectedApnKey called: Selectedkey is "
                + key);
        return key;
    }

    private String getNextKey(String selected) {
        String next = null;
        if (selected != null && !selected.equals(mApnList.get(0))) {
            next = mApnList.get(0);
        } else {
            next = mApnList.get(1);
        }
        return next;
    }

    private void fillList() {
        if (mFilled) {
            return;
        }
        if (mUri == null) {
            Xlog.d(TAG, "fillList, mUri null !");
            return;
        }

        String where = getFillListQuery();
        ContentResolver cr = mContext.getContentResolver();
        Cursor cursor = null;
        try {
            cursor = cr.query(mUri, new String[] { "_id", "name", "apn",
                    "type", "sourcetype" }, where, null, null);

            mSelectedKey = getSelectedApnKey();
            if (mSelectedKey == null) {
                mSelectedKey = mApnList.get(0);
            }
            cursor.moveToFirst();

            int i = 0;
            while (!cursor.isAfterLast()) {
                String name = cursor.getString(NAME_INDEX);
                String apn = cursor.getString(APN_INDEX);
                String key = cursor.getString(ID_INDEX);
                mApnList.set(i++, key);
                if (i > 1) {
                    break;
                }

                int sourcetype = cursor.getInt(SOURCE_TYPE_INDEX);
                if (!"cmmail".equals(apn) || sourcetype != 0) {
                    mFilled = true;
                }
                cursor.moveToNext();
            }
        } finally {
            cursor.close();
        }
    }

    private String getFillListQuery() {
        if (mNumeric.equals("-1")) {
            mNumeric = SystemProperties.get(
                    TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
        }
        return "numeric=\"" + mNumeric + "\"";
    }
}
