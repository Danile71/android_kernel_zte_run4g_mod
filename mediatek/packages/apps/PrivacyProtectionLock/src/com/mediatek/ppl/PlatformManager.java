package com.mediatek.ppl;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.storage.IMountService;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.PhoneLookup;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.SmsManagerEx;
import com.mediatek.telephony.TelephonyManagerEx;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Random;

public class PlatformManager {
    protected static final String TAG = "PPL/PlatformManager";
    private final Context mContext;
    private final IPplAgent mAgent;
    private final SmsManagerEx mSmsManager;
    private final IMountService mMountService;
    private final ConnectivityManager mConnectivityManager;
    private final WakeLock mWakeLock;

    public static final int SIM_NUMBER;

    static {
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                SIM_NUMBER = 4;
            } else if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                SIM_NUMBER = 3;
            } else {
                SIM_NUMBER = 2;
            }
        } else {
            SIM_NUMBER = 1;
        }
    }

    public PlatformManager(Context context) {
        mContext = context;
        IBinder binder = ServiceManager.getService("PPLAgent");
        if (binder == null) {
            throw new Error("Failed to get PPLAgent");
        }
        mAgent = IPplAgent.Stub.asInterface(binder);
        if (mAgent == null) {
            throw new Error("mAgent is null!");
        }
        mSmsManager = SmsManagerEx.getDefault();
        mMountService = IMountService.Stub.asInterface(ServiceManager.getService("mount"));
        mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "PPL_WAKE_LOCK");
    }

    public IPplAgent getPPLAgent() {
        return mAgent;
    }

    public void sendTextMessage(String destinationAddress, long id, String text, Intent sentIntent, int simId) {
        Log.d(TAG, "sendTextMessage(" + destinationAddress + ", " + id + ", " + text + ", " + simId + ")");
        ArrayList<String> segments = divideMessage(text);
        ArrayList<PendingIntent> pis = new ArrayList<PendingIntent>(segments.size());
        final int total = segments.size();
        for (int i = 0; i < total; ++i) {
            Intent intent = new Intent(sentIntent);
            Uri.Builder builder = new Uri.Builder();
            builder.authority(MessageManager.SMS_PENDING_INTENT_DATA_AUTH)
                    .scheme(MessageManager.SMS_PENDING_INTENT_DATA_SCHEME).appendPath(Long.toString(id))
                    .appendPath(Integer.toString(total)).appendPath(Integer.toString(i));
            Log.d(TAG, "sendTextMessage: uri string is " + builder.toString());
            intent.setData(builder.build());

            byte type = intent.getByteExtra(PendingMessage.KEY_TYPE, MessageManager.Type.INVALID);
            String number = intent.getStringExtra(PendingMessage.KEY_NUMBER);
            Log.d(TAG, "id is " + id + ", type is " + type + ", number is " + number);

            PendingIntent pi = PendingIntent.getBroadcast(mContext, 0, intent, PendingIntent.FLAG_ONE_SHOT);
            pis.add(pi);
        }
        sendMultipartTextMessage(destinationAddress, null, segments, pis, simId);
    }

    protected ArrayList<String> divideMessage(String text) {
        return mSmsManager.divideMessage(text);
    }

    protected void sendMultipartTextMessage(String destinationAddress, String scAddress, ArrayList<String> parts,
            ArrayList<PendingIntent> sentIntents, int simId) {
        mSmsManager.sendMultipartTextMessage(destinationAddress, scAddress, parts, sentIntents, null, simId);
    }

    public boolean isUsbMassStorageEnabled() {
        try {
            return mMountService.isUsbMassStorageEnabled();
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }

    public void setMobileDataEnabled(boolean enable) {
        mConnectivityManager.setMobileDataEnabled(enable);
    }
    
    public void acquireWakeLock() {
        mWakeLock.acquire();
    }
    
    public void releaseWakeLock() {
        mWakeLock.release();
    }

    public void stayForeground(Service service) {
        Log.i(TAG, "Bring service to foreground");
        Notification notification = new Notification();
        notification.flags |= Notification.FLAG_HIDE_NOTIFICATION;
        service.startForeground(1, notification);
    }

    public void leaveForeground(Service service) {
        Log.d(TAG, "Exec stopForeground with para true.");
        service.stopForeground(true);
    }

    /**
     * TelephonyManager may be unavailable in certain circumstance such as data encrypting process of the phone.
     * @return  Whether TelephonyManager is available.
     */
    @SuppressWarnings("unused")
    public static boolean isTelephonyReady(Context context){
        boolean teleEnable = true;

        try {
            TelephonyManagerEx telephonyManagerEx = new TelephonyManagerEx(context);
            if (telephonyManagerEx == null) {
                teleEnable = false;
                Log.e(TAG, "TelephonyManagerEx is null");
            }
            telephonyManagerEx.hasIccCard(0);

        } catch (Throwable t) {
            Log.e(TAG, "TelephonyManager(Ex) is not ready");
            teleEnable = false;
        }

        return teleEnable;
    }


   public static List<PplSimInfo> buildSimInfo(Context context) {
        List<SimInfoRecord> simItem =  SimInfoManager.getInsertedSimInfoList(context);
        Log.i(TAG, "simItem: " + simItem.size());

        // sort the unordered list
        Collections.sort(simItem, new SimInfoComparable());

        List<PplSimInfo> data = new ArrayList<PplSimInfo>();
        for (int i = 0; i < simItem.size(); i++) {
            

            int indicator = getSimIndicator(context, simItem.get(i).mSimSlotId);
            if (indicator == PhoneConstants.SIM_INDICATOR_UNKNOWN ||
                    indicator == PhoneConstants.SIM_INDICATOR_NORMAL) {
                indicator = -1;
            }

            PplSimInfo map = new PplSimInfo(simItem.get(i).mDisplayName, indicator, simItem.get(i).mColor);

            Log.i(TAG, "mSimSlotId: " + simItem.get(i).mSimSlotId);
            Log.i(TAG, "mDisplayName: " + simItem.get(i).mDisplayName);
            Log.i(TAG, "mColor: " + simItem.get(i).mColor);

            data.add(map);
        }

        return data;
    }

    private static class SimInfoComparable implements Comparator<SimInfoRecord> {
        @Override
         public int compare(SimInfoRecord sim1, SimInfoRecord sim2) {
            return sim1.mSimSlotId - sim2.mSimSlotId;
         }
     }

    private static int getSimIndicator(Context context, int slotId) {
        if (isAllRadioOff(context)) {
            Log.i(TAG, "isAllRadioOff=" + isAllRadioOff(context) + "slotId=" + slotId);
            return PhoneConstants.SIM_INDICATOR_RADIOOFF;
        }

        ITelephonyEx iTelephonyEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
        int indicator = PhoneConstants.SIM_INDICATOR_UNKNOWN;
        if (iTelephonyEx != null) {
            try {
                indicator = iTelephonyEx.getSimIndicatorState(slotId);
                Log.i(TAG, "indicator is " + indicator);
            } catch (RemoteException e) {
                Log.i(TAG, "RemoteException");
            } catch (NullPointerException ex) {
                Log.i(TAG, "NullPointerException");
            }
        }
        return indicator;
    }

    /**
     * Get the pic from framework according to sim indicator state 
     * @param state sim indicator state
     * @return the pic res from mediatek framework
     */
    public static int getStatusResource(int state) {
        int resId;
        switch (state) {
        case PhoneConstants.SIM_INDICATOR_RADIOOFF:
            resId = com.mediatek.internal.R.drawable.sim_radio_off;
            break;
        case PhoneConstants.SIM_INDICATOR_LOCKED:
            resId = com.mediatek.internal.R.drawable.sim_locked;
            break;
        case PhoneConstants.SIM_INDICATOR_INVALID:
            resId = com.mediatek.internal.R.drawable.sim_invalid;
            break;
        case PhoneConstants.SIM_INDICATOR_SEARCHING:
            resId = com.mediatek.internal.R.drawable.sim_searching;
            break;
        case PhoneConstants.SIM_INDICATOR_ROAMING:
            resId = com.mediatek.internal.R.drawable.sim_roaming;
            break;
        case PhoneConstants.SIM_INDICATOR_CONNECTED:
            resId = com.mediatek.internal.R.drawable.sim_connected;
            break;
        case PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
            resId = com.mediatek.internal.R.drawable.sim_roaming_connected;
            break;
        default:
            resId = PhoneConstants.SIM_INDICATOR_UNKNOWN;
            break;
        }
        return resId;
    }

    /**
     * Get sim color resources
     * @param colorId sim color id
     * @return the color resource 
     */
    public static int getSimColorResource(int colorId) {
        int bgColor = -1;
        if ((colorId >= 0) && (colorId < SimInfoManager.SimBackgroundDarkRes.length)) {
            bgColor = SimInfoManager.SimBackgroundDarkRes[colorId];
        } else if (colorId == SimInfoManager.SimBackgroundDarkRes.length) {
            bgColor = com.mediatek.internal.R.drawable.sim_background_sip;
        }

        return bgColor;
    }

    private static boolean isAllRadioOff(Context context) {
        int airMode = Settings.Global.getInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1);
        int dualMode = Settings.System.getInt(context.getContentResolver(), Settings.System.DUAL_SIM_MODE_SETTING, -1);
        return airMode == 1 || dualMode == 0;
    }

    /**
     * Query name for number.
     * 
     * @param context
     * @param number
     * @return
     */
    public static String getContactNameByPhoneNumber(Context context, String number) {
        ContentResolver contentResolver = context.getContentResolver();
        Uri uri = Uri.withAppendedPath(ContactsContract.PhoneLookup.CONTENT_FILTER_URI, Uri.encode(number));
        String[] projection = new String[] { PhoneLookup.DISPLAY_NAME, PhoneLookup._ID };
        Cursor cursor = contentResolver.query(uri, projection, null, null, null);
        if (null == cursor) {
            return null;
        }
        String id = null;
        if (cursor.moveToFirst()) {
            id = cursor.getString(cursor.getColumnIndexOrThrow(PhoneLookup._ID));
        }
        cursor.close();
        if (null == id) {
            return null;
        }

        // Build the Entity URI.
        Uri.Builder b = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id).buildUpon();
        b.appendPath(ContactsContract.Contacts.Entity.CONTENT_DIRECTORY);
        Uri contactUri = b.build();
        Log.d(TAG, "XXX: contactUri is " + contactUri);
        // Create the projection (SQL fields) and sort order.
        projection = new String[] {
                ContactsContract.Contacts.Entity.RAW_CONTACT_ID,
                ContactsContract.Contacts.Entity.DATA1,
                ContactsContract.Contacts.Entity.MIMETYPE
        };
        String sortOrder = ContactsContract.Contacts.Entity.RAW_CONTACT_ID + " ASC";
        cursor = context.getContentResolver().query(contactUri, projection, null, null, sortOrder);
        if (null == cursor) {
            return null;
        }
        String name = null;
        int mimeIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.MIMETYPE);
        int dataIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.DATA1);
        if (cursor.moveToFirst()) {
            do {
                String mime = cursor.getString(mimeIdx);
                if (mime.equalsIgnoreCase(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                    name = cursor.getString(dataIdx);
                    break;
                }
            } while (cursor.moveToNext());
        }
        cursor.close();

        return name;
    }

    public static class ContactQueryResult {
        public String name;
        public ArrayList<String> phones;

        public ContactQueryResult() {
            name = null;
            phones = new ArrayList<String>();
        }
    }


    /**
     * Query name and numbers for specified contact.
     * 
     * @param context
     * @param uri       URI of contact.
     * @return
     */
    public static ContactQueryResult getContactInfo(Context context, Uri uri) {
        ContactQueryResult result = new ContactQueryResult();
        String id = null;
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (null == cursor) {
            return result;
        }
        if (cursor.moveToFirst()) {
            id = cursor.getString(cursor.getColumnIndex(ContactsContract.Contacts._ID));
        }
        cursor.close();
        // Build the Entity URI.
        Uri.Builder b = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id).buildUpon();
        b.appendPath(ContactsContract.Contacts.Entity.CONTENT_DIRECTORY);
        Uri contactUri = b.build();
        // Create the projection (SQL fields) and sort order.
        String[] projection = { ContactsContract.Contacts.Entity.RAW_CONTACT_ID,
                ContactsContract.Contacts.Entity.DATA1, ContactsContract.Contacts.Entity.MIMETYPE };
        String sortOrder = ContactsContract.Contacts.Entity.RAW_CONTACT_ID + " ASC";
        cursor = context.getContentResolver().query(contactUri, projection, null, null, sortOrder);
        if (null == cursor) {
            return result;
        }
        String mime;
        int mimeIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.MIMETYPE);
        int dataIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.DATA1);
        if (cursor.moveToFirst()) {
            do {
                mime = cursor.getString(mimeIdx);
                if (mime.equalsIgnoreCase(CommonDataKinds.Phone.CONTENT_ITEM_TYPE)) {
                    result.phones.add(cursor.getString(dataIdx));
                } else if (mime.equalsIgnoreCase(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                    result.name = cursor.getString(dataIdx);
                }
            } while (cursor.moveToNext());
        }
        cursor.close();
        return result;
    }

    /**
     * Generate new secrets, including new password and new salt.
     * 
     * @param password
     * @param salt
     * @return
     */
    public static byte[] generateSecrets(final byte[] password, byte[] salt) {
        // generate salts
        Random random = new Random();
        random.nextBytes(salt);
        // generate secret
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance("SHA-1");
            byte[] buffer = new byte[salt.length + password.length];
            System.arraycopy(password, 0, buffer, 0, password.length);
            System.arraycopy(salt, 0, buffer, password.length, salt.length);
            return md.digest(buffer);
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        }
    }

    
    /**
     * Check whether the password is correct.
     *
     * SHA1(password:salt) == secret
     *
     * @param password
     * @param salt
     * @param secret
     * @return
     */
    public static boolean checkPassword(final byte[] password, final byte[] salt, final byte[] secret) {
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance("SHA-1");
            byte[] buffer = new byte[salt.length + password.length];
            System.arraycopy(password, 0, buffer, 0, password.length);
            System.arraycopy(salt, 0, buffer, password.length, salt.length);
            byte[] digest = md.digest(buffer);
            if (secret.length != digest.length) {
                return false;
            }
            for (int i = 0; i < secret.length; ++i) {
                if (secret[i] != digest[i]) {
                    return false;
                }
            }
            return true;
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        }
    }

    public static boolean isAirplaneModeEnabled(Context context) {
        String state = Settings.Global.getString(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON);
        return "1".equals(state);
    }
    
    public static void turnOffAirplaneMode(Context context) {
        Settings.Global.putString(context.getContentResolver(), 
                Settings.Global.AIRPLANE_MODE_ON, "0");
        context.sendBroadcast(new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED));
    }
}
