package com.mediatek.settings.plugin;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.preference.DialogPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.view.Menu;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.op03.plugin.R;
import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.List;

public class ApnSettingsExt extends DefaultApnSettingsExt {
    
    private static final String TAG = "OP03ApnSettingsExt";
    private static final String TETHER_TYPE = "tethering";
    private static final String TYPE_MMS = "mms";
    private static final String APN_TYPE = "apn_type";

    private static final String ORANGE_CARD_1 = "20801";
    private static final String ORANGE_CARD_2 = "23430";
    private static final String ORANGE_CARD_3 = "23431";
    private static final String ORANGE_CARD_4 = "23432";
    private static final String ORANGE_CARD_5 = "23433";
    private static final String ORANGE_CARD_6 = "21403";
    
    private boolean mIsTetherApn = false;

    private static final Uri PREFER_APN_TETHER_URI = Uri.parse("content://telephony/carriers/prefertetheringapn");
    private static final String TETHER_APN_SETTING = "tether_apn_settings";
    private static final String BLUETOOTH_TETHERING = "enable_bluetooth_tethering";

    private boolean mIsSwitching = false;
    private boolean mIsSIMReady = true;
    private boolean mIsTethering = false;
    private boolean mAirplaneModeEnabled = false;
    private ConnectivityManager mConnManager;
    private String[] mUsbRegexs;
    private PreferenceActivity mActivity;
    private Context mContext;

    private final BroadcastReceiver mTetheringStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
        
            String action = intent.getAction(); 
            if (action.equals(ConnectivityManager.TETHER_CHANGED_DONE_ACTION)) {
                Xlog.d(TAG, "onReceive:ConnectivityManager.TETHER_CHANGED_DONE_ACTION");
                mIsSwitching = false;
                mActivity.getPreferenceScreen().setEnabled(getScreenEnableState());                
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                Xlog.d(TAG, "onReceive:AIRPLANE_MODE state changed: " + mAirplaneModeEnabled);
                mAirplaneModeEnabled = intent.getBooleanExtra("state", false);
                mActivity.getPreferenceScreen().setEnabled(getScreenEnableState());
            } else if (action.equals(ConnectivityManager.ACTION_TETHER_STATE_CHANGED)) {
                Xlog.d(TAG, "onReceive: ConnectivityManager.ACTION_TETHER_STATE_CHANGED");
                ArrayList<String> active = intent.getStringArrayListExtra(
                        ConnectivityManager.EXTRA_ACTIVE_TETHER);
                if (active != null) {
                    updateTetheredState(active.toArray());  
                } else {
                    Xlog.d(TAG, "active tether is null , not update tether state.");
                } 
            }
        }
    };

    public ApnSettingsExt(Context context) {
        super();
        mContext = context;
    }

    public boolean isAllowEditPresetApn(String type ,String apn, String numeric, int sourcetype){
        Xlog.d(TAG,"isAllowEditPresetApn");
        boolean isAllowEdit = true;
        isAllowEdit = !TETHER_TYPE.equals(type);
        return isAllowEdit && !isOrangeCard(numeric) || sourcetype!= 0;
    }

    public void customizeTetherApnSettings(PreferenceScreen root) {
        Xlog.d(TAG, "customizeTetherApnSettings");
        Preference tetherPreference = new Preference(mContext);
        tetherPreference.setKey(TETHER_APN_SETTING);
        tetherPreference.setTitle(mContext.getText(R.string.tethering_apn_settings_title));
        tetherPreference.setSummary(mContext.getText(R.string.tethering_apn_settings_summary));
 	
        Intent intent = new Intent("android.settings.APN_SETTINGS");
        intent.putExtra("isTether", true);
        /* must add the flag, or will have exception: calling startActivity()
         * from outside of an Activity context requires the FLAG_ACTIVITY_NEW_TASK
       	 * flag.
         */
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        tetherPreference.setIntent(intent);
        
        Preference bluetoothPreference = root.findPreference(BLUETOOTH_TETHERING);
        if (bluetoothPreference != null) {
        	int order = bluetoothPreference.getOrder();
            tetherPreference.setOrder(order + 1);
        }
        root.addPreference(tetherPreference);
        
        /* Check if SIM is present or not. If not present,("APN Setting" pref added but)disable it */
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(mContext);
        if (simList != null){
           tetherPreference.setEnabled(simList.size() > 0);
        }

    }
    
    private boolean isOrangeCard(String numeric){
        return (ORANGE_CARD_1.equals(numeric) || ORANGE_CARD_5.equals(numeric) || ORANGE_CARD_6.equals(numeric));
    }
  
    private boolean isOrangeCardForTether(String numeric){
        return (ORANGE_CARD_1.equals(numeric) || ORANGE_CARD_2.equals(numeric) 
                || ORANGE_CARD_3.equals(numeric) || ORANGE_CARD_4.equals(numeric));
    }

    public boolean isSelectable(String type) {
        boolean isSelect = false;
        if (TETHER_TYPE.equals(type)) {
            isSelect = mIsTetherApn;
        } else {
            isSelect = super.isSelectable(type);
        }
        return isSelect;
    }

    public IntentFilter getIntentFilter() {
        IntentFilter filter = null;
        if (mIsTetherApn) {
            filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);       
            filter.addAction(ConnectivityManager.TETHER_CHANGED_DONE_ACTION); 
            filter.addAction(ConnectivityManager.ACTION_TETHER_STATE_CHANGED); 
        } else {
            filter = super.getIntentFilter();
        }
        return filter;
    }

    public BroadcastReceiver getBroadcastReceiver(BroadcastReceiver receiver) {
        BroadcastReceiver rece;
        if (mIsTetherApn) {
            rece = mTetheringStateReceiver;
        } else {
            rece = receiver;
        }
        return rece;
    }

    public boolean getScreenEnableState() {        
        Xlog.d(TAG, " mAirplaneModeEnabled : " + 
            mAirplaneModeEnabled + " mIsSIMReady :" + mIsSIMReady + " mIsSwitching: " + 
            mIsSwitching + " mIsTethering: " + mIsTethering);
        return !mIsTethering && !mAirplaneModeEnabled && mIsSIMReady && !mIsSwitching;
    }

    public boolean getScreenEnableState(int slotId, Activity activity) {        
        if (mIsTetherApn) {
            return getScreenEnableState();
        } else {
            return super.getScreenEnableState(slotId, activity);
        }
    }

    public String getFillListQuery(String numeric,int slotId) {
        if (mIsTetherApn) {
            return "numeric=\"" + numeric + "\" AND type=\"" + TETHER_TYPE + "\"";
        } else {
            return super.getFillListQuery(numeric, slotId);
        }
    }

    public void addMenu(Menu menu, Activity activity, int add, int restore, String numeric) {

        if (mIsTetherApn) {
            if (isOrangeCardForTether(numeric)) {
                return;
            }
            menu.add(0, MENU_NEW, 0,
                activity.getResources().getString(add))
                .setIcon(android.R.drawable.ic_menu_add);
        } else {
            super.addMenu(menu, activity, add, restore, numeric);
        }
    }

    public void addApnTypeExtra(Intent it) {
        if (mIsTetherApn) {
            it.putExtra(APN_TYPE, TETHER_TYPE);
        }
    }

    public void updateTetherState(Activity activity) {
        if (mIsTetherApn) {
            if (mConnManager != null) {
                mIsSwitching = !mConnManager.isTetheringChangeDone();
                String[] tethered = mConnManager.getTetheredIfaces(); 
                updateTetheredState(tethered);
            }
        } else {
            super.updateTetherState(activity);
        }
    }

    public void initTetherField(Activity activity) {
        Intent intent = activity.getIntent();
        mIsTetherApn = intent.getBooleanExtra("isTether", false);
        Xlog.d(TAG, "mIsTetherApn = " + mIsTetherApn);
        mConnManager = (ConnectivityManager)activity.getSystemService(Context.CONNECTIVITY_SERVICE);
 
        if (mConnManager != null) {
            mUsbRegexs = mConnManager.getTetherableUsbRegexs();
        }
        TelephonyManager telManager = TelephonyManager.getDefault();
        
        if (telManager != null) {
            mIsSIMReady = TelephonyManager.SIM_STATE_READY == telManager.getSimState();
        }
        mActivity = (PreferenceActivity)activity;
    }
    
    public Uri getRestoreCarrierUri(int slotId) {
        if (mIsTetherApn) {
            return PREFER_APN_TETHER_URI;
        } else {
            return super.getRestoreCarrierUri(slotId);
        }
    }

   private void updateTetheredState(Object[] tethered) {
        mIsTethering = false;
        for (Object o : tethered) {
               String s = (String)o;
               for (String regex : mUsbRegexs) {
                   if (s.matches(regex)) {
                        mIsTethering = true;
                   }
               }
        }
        mActivity.getPreferenceScreen().setEnabled(getScreenEnableState());
   } 
   
   public void setApnTypePreferenceState(Preference preference) {
       Xlog.d(TAG,"setPreferenceState:false");
       preference.setEnabled(false);
   }
   
   public Uri getUriFromIntent(Context context, Intent intent) {
       Xlog.d(TAG,"getUriFromIntent , intent = " + intent);
       ContentValues value = new ContentValues();
       String strType = intent.getStringExtra(APN_TYPE);                   
       if ((strType != null) && (strType.equals(TETHER_TYPE))) {
           value.put(Telephony.Carriers.TYPE, TETHER_TYPE);
       }
       return context.getContentResolver().insert(intent.getData(), value);
   }

   public String[] getApnTypeArray(Context context, int defResId,boolean isTether) {
       Xlog.d(TAG,"getApnTypeArray : orange array");
       String[] resArray = null;
       if (isTether) {
           resArray = mContext.getResources().getStringArray(R.array.apn_type_orange_tethering_only);
       } else {
           resArray = mContext.getResources().getStringArray(R.array.apn_type_orange);
       }
       return resArray;
   }

    public Cursor customizeQueryResult(Activity activity,Cursor cursor,Uri uri,String numeric) {
      // need to do nothing if tethering
      Xlog.d(TAG,"customizeQueryResult(), mIsTetherApn = " + mIsTetherApn);
      if (mIsTetherApn) {
          return cursor;
      } else {
         return super.customizeQueryResult(activity,cursor,uri,numeric);
      }
    }
}

