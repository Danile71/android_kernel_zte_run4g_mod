package com.mediatek.systemui.plugin;

import android.app.ActivityManagerNative;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.internal.statusbar.IStatusBarService;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.systemui.statusbar.phone.QuickSettingsTileView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.op09.plugin.R;
import com.mediatek.systemui.ext.DefaultQuickSettingsPlugin;
import com.mediatek.systemui.ext.QuickSettingsTileViewId;
import com.mediatek.systemui.statusbar.util.SIMHelper;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;

import java.util.List;

/**
 * M: OP09 implementation of Plug-in definition of Quick Settings.
 */
public class Op09QuickSettingsPlugin extends DefaultQuickSettingsPlugin {
    private static final String TAG = "Op09QuickSettingsPlugin";
    private static final String TRANSACTION_START = "com.android.mms.transaction.START";
    private static final String TRANSACTION_STOP = "com.android.mms.transaction.STOP";
    private static final String APN_URI = "content://telephony/carriers_sim1/preferapn";
    private static final boolean DEBUG = true;

    private Context mContext;
    private LayoutInflater mInflater;
    private IStatusBarService mStatusbarService;

    private APNStateTracker mAPNStateTracker;
    private SimConnStateTracker mSimConnStateTracker;
    private boolean mSimConnStateTrackerReady;
    private QuickSettingsTileView mNetSwapTile;
    private QuickSettingsTileView mDualSimSettingsTile;
    private QuickSettingsBasicTile mSimConnTile;
    private int mNextTileIndex;
    private boolean mDeviceProvisioned;

    private final APNChangeObserver mAPNChangeObserver;
    private final Handler mHandler;

    //Network Selection And Settings Broadcast receive to determine if there is SIM State Change.
    private BroadcastReceiver mSimStateIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Xlog.d(TAG, "onReceive action is " + action);
            if (action.equals(TelephonyIntents.ACTION_SIM_INSERTED_STATUS)
                    || action.equals(TelephonyIntents.ACTION_SIM_INFO_UPDATE)
                    || action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                updateDualSimSettingsTile();
                updateNetSwapTile();
                updateSimConnTile();
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean enabled = intent.getBooleanExtra("state", false);
                Xlog.d(TAG, "airline mode changed: state is " + enabled);
                if (mSimConnStateTrackerReady) {
                    mSimConnStateTracker.setAirlineMode(enabled);
                }
                updateSimConnTile();
            } else if (action.equals(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                PhoneConstants.DataState state = getMobileDataState(intent);
                boolean isApnTypeChange = false;
                String types = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
                if (types != null) {
                    String[] typeArray = types.split(",");
                    for (String type : typeArray) {
                        if (PhoneConstants.APN_TYPE_DEFAULT.equals(type)) {
                            isApnTypeChange = true;
                            break;
                        }
                    }
                }
                if (isApnTypeChange && ((state == PhoneConstants.DataState.CONNECTED) || 
                        (state == PhoneConstants.DataState.DISCONNECTED))
                        && !mSimConnStateTracker.mIsMmsOngoing) {
                    updateSimConnTile();
                }
            } else if (action.equals(TRANSACTION_START)) {
                /// M: only apply if NOT wifi-only device @{
                if (!isWifiOnlyDevice() && mSimConnStateTrackerReady) {
                    mSimConnStateTracker.setIsMmsOngoing(true);
                    mSimConnTile.setEnabled(mSimConnStateTracker.isClickable());
                    mSimConnStateTracker.setImageViewResources(mContext);
                
                }
                /// M: }@
            } else if (action.equals(TRANSACTION_STOP)) {
                /// M: only apply if NOT wifi-only device @{
                if (!isWifiOnlyDevice() && mSimConnStateTrackerReady) {
                    mSimConnStateTracker.setIsMmsOngoing(false);
                    mSimConnTile.setEnabled(mSimConnStateTracker.isClickable());
                    mSimConnStateTracker.setImageViewResources(mContext);
                }
                /// M: }@
            } else if (action.equals(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED)) {
                if (!isWifiOnlyDevice() && mSimConnStateTrackerReady) {
                    SIMHelper.updateSIMInfos(context);
                    SIMHelper.updateSimInsertedStatus();
                    Xlog.d(TAG, "Panel sim ready called");
                    /// M: only apply if NOT wifi-only device @{
                    if (!isWifiOnlyDevice()) {
                    /// M: }@
                        List<SimInfoManager.SimInfoRecord> simInfos = SIMHelper.getSIMInfoList(mContext);
                        if (simInfos == null || simInfos.size() <= 0) {
                            mSimConnStateTracker.setHasSim(false);
                        } else {
                            mSimConnStateTracker.setHasSim(true);
                        }
                        updateSimConnTile();
                    }
                }
            }
        }
    };

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onServiceStateChanged(ServiceState state) {
            updateNetSwapTile();
        }
    };

    private ContentObserver mProvisioningObserver = new ContentObserver(
            new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            final boolean provisioned = 0 != Settings.Global.getInt(
                    mContext.getContentResolver(),
                    Settings.Global.DEVICE_PROVISIONED, 0);
            if (provisioned != mDeviceProvisioned) {
                mDeviceProvisioned = provisioned;
            }
        }
    };

    private ContentObserver mSimConnStateChangeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            if (mSimConnStateTrackerReady) {
                ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                boolean cmConn = false;
                if (cm != null) {
                    cmConn = cm.getMobileDataEnabled();
                }
                /// M: only apply if NOT wifi-only device @{
                if (!isWifiOnlyDevice() && !cmConn) {
                /// M: }@
                    /// M: Add for op09 SIM Conn.
                    Xlog.d(TAG, "mSimConnStateChangeObserver onChange...");
                    mSimConnStateTracker.onActualStateChange(mContext, null);
                    mSimConnTile.setEnabled(mSimConnStateTracker.isClickable());
                    mSimConnStateTracker.setImageViewResources(mContext);
                /// M: only apply if NOT wifi-only device @{
                }
                /// M: }@
            }
        }
    };

    public Op09QuickSettingsPlugin(Context context) {
        super(context);
        mContext = context;
        mInflater = LayoutInflater.from(mContext);
        mHandler = new Handler();

        mAPNStateTracker = new APNStateTracker(mContext);
        mAPNChangeObserver = new APNChangeObserver(mHandler);

        mAPNChangeObserver.startObserving();
        mContext.getContentResolver().registerContentObserver(
                Settings.System.getUriFor(Settings.System.GPRS_CONNECTION_SETTING)
                , true, mSimConnStateChangeObserver);

        mProvisioningObserver.onChange(false); // set up
        mContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.DEVICE_PROVISIONED), true,
                mProvisioningObserver);

        SIMHelper.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_SERVICE_STATE,
                PhoneConstants.GEMINI_SIM_1);

        IntentFilter simIntentFilter = new IntentFilter();
        simIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        simIntentFilter.addAction(TRANSACTION_START);
        simIntentFilter.addAction(TRANSACTION_STOP);
        simIntentFilter.addAction(TelephonyIntents.ACTION_SIM_INSERTED_STATUS);
        simIntentFilter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        simIntentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        simIntentFilter.addAction(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
        simIntentFilter.addAction(TelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);

        mContext.registerReceiver(mSimStateIntentReceiver, simIntentFilter);
    }

    @Override
    public void customizeTileViews(ViewGroup parent) {
        if (DEBUG) {
            Xlog.d(TAG, "customizeTileViews parent= " + parent);
        }

        // Remove the unused quicksettings icon view
        removeTileById(parent, QuickSettingsTileViewId.ID_Datausage);

        // Add the additional quicksettings icon view
        addDualSimSettingTile(parent);
        addNetSwapTile(parent);
        addSimConnTile(parent);

        //Reorder the quicksettings icon view
        int wifiTileIndex = getTileIndexById(parent, QuickSettingsTileViewId.ID_Wifi);
        Xlog.d(TAG, "customizeTileViews, index = " + getTileIndexById(parent, QuickSettingsTileViewId.ID_Wifi));
        if (wifiTileIndex != -1) {
            mNextTileIndex = ++wifiTileIndex;
        } else {
            int batteryTileIndex = getTileIndexById(parent,
                    QuickSettingsTileViewId.ID_Battery);
            mNextTileIndex = ++batteryTileIndex;
        }

        setNextViewByTileId(parent, QuickSettingsTileViewId.ID_DataConnection);
        setNextViewByTileId(parent, QuickSettingsTileViewId.ID_SimConnection);
        if (FeatureOption.MTK_AUDIO_PROFILES) {
            setNextViewByTileId(parent, QuickSettingsTileViewId.ID_AudioProfile);
        }
        if (FeatureOption.MTK_GPS_SUPPORT) {
            setNextViewByTileId(parent, QuickSettingsTileViewId.ID_Location);
        }
    }

    @Override
    public void updateResources() {
        updateDualSimSettingsTile();
        updateNetSwapTile();
        updateSimConnTile();
    }

    private void addSimConnTile(ViewGroup parent) {
        mSimConnTile = new QuickSettingsBasicTile(mContext);
        mSimConnTile.setTileViewId(QuickSettingsTileViewId.ID_SimConnection);
        mSimConnStateTracker = new SimConnStateTracker(mContext, mSimConnTile);
        mSimConnStateTrackerReady = true;
        mSimConnTile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSimConnStateTracker.toggleState(null);
            }
        });
        updateSimConnTile();
        parent.addView(mSimConnTile);
    }

    private void updateSimConnTile() {
        /// M: only apply if NOT wifi-only device.
        if (!isWifiOnlyDevice() && mSimConnStateTrackerReady) {
            mSimConnStateTracker.onActualStateChange(mContext, null);
            mSimConnTile.setEnabled(mSimConnStateTracker.isClickable());
            mSimConnStateTracker.setImageViewResources(mContext);
        }
    }
    /**
     * M: Used to check weather this device is wifi only.
     */
    private boolean isWifiOnlyDevice() {
      ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(mContext.CONNECTIVITY_SERVICE);
      return  !(cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE));
    }

    private void setNextViewByTileId(ViewGroup parent,
            QuickSettingsTileViewId id) {
        View addingTileView = getTileViewById(parent, id);
        if (addingTileView != null) {
            parent.removeView(addingTileView);
            parent.addView(addingTileView, mNextTileIndex++);
        }
    }

    private void addNetSwapTile(ViewGroup parent) {
        // APN Settings
        mNetSwapTile = new QuickSettingsBasicTile(mContext,null, R.layout.quick_settings_tile_basic_common);
        mNetSwapTile.setTileViewId(QuickSettingsTileViewId.ID_NetSwap);
        mNetSwapTile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mAPNStateTracker.toggleState();
            }
        });
        mNetSwapTile.setOnLongClickListener(new OnLongClickListener() {
            public boolean onLongClick(View v) {
                Intent intent = new Intent();
                intent.setAction("android.settings.APN_SETTINGS");
                intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY,
                        PhoneConstants.GEMINI_SIM_1);
                startSettingsActivity(intent);
                return true;
            }
        });
        updateNetSwapTile();
        parent.addView(mNetSwapTile);
        Xlog.d(TAG,
                "addNetSwapTile, index = "
                        + getTileIndexById(parent,
                                QuickSettingsTileViewId.ID_NetSwap));
    }

    private void updateNetSwapTile() {
        int state = SIMHelper
                .getSimIndicatorStateGemini(PhoneConstants.GEMINI_SIM_1);
        TileState netSwapState = new TileState();
        if (!SIMHelper.isSimInserted(PhoneConstants.GEMINI_SIM_1)
                || PhoneConstants.SIM_INDICATOR_LOCKED == state
                || PhoneConstants.SIM_INDICATOR_SEARCHING == state
                || PhoneConstants.SIM_INDICATOR_INVALID == state
                || PhoneConstants.SIM_INDICATOR_RADIOOFF == state) {
            netSwapState.mEnable = false;
        } else {
            netSwapState.mEnable = true;
        }
        TextView tv = (TextView) mNetSwapTile.findViewById(R.id.text);
        tv.setText(R.string.apn);
        ImageView imageView = (ImageView) mNetSwapTile.findViewById(R.id.image);
        imageView.setImageResource(mAPNStateTracker.getResource());
        mNetSwapTile.setEnabled(netSwapState.mEnable);
        if (FeatureOption.EVDO_IR_SUPPORT
                && SIMHelper.isInternationalRoamingStatus(mContext)) {
            mNetSwapTile.setVisibility(View.GONE);
        } else {
            mNetSwapTile.setVisibility(View.VISIBLE);
        }
    }

    private void addDualSimSettingTile(ViewGroup parent) {
        mDualSimSettingsTile = new QuickSettingsBasicTile(mContext,null, R.layout.quick_settings_tile_basic_common);
        mDualSimSettingsTile.setTileViewId(QuickSettingsTileViewId.ID_DualSim);
        mDualSimSettingsTile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setComponent(new ComponentName("com.android.settings",
                        "com.android.settings.Settings$SimManagementActivity"));
                startSettingsActivity(intent);
            }
        });
        updateDualSimSettingsTile();
        parent.addView(mDualSimSettingsTile);
        Xlog.d(TAG,
                "addDualSimSettingTile, index = "
                        + getTileIndexById(parent,
                                QuickSettingsTileViewId.ID_DualSim));
    }

    private void updateDualSimSettingsTile() {
        TileState simSettingsState = new TileState();
        List<SimInfoManager.SimInfoRecord> infos = SIMHelper
                .getSIMInfoList(mContext);
        if (infos == null || infos.size() == 0) {
            simSettingsState.mIconId = R.drawable.ic_qs_dual_sim_settings_disable;
            simSettingsState.mEnable = false;
        } else {
            simSettingsState.mIconId = R.drawable.ic_qs_dual_sim_settings_enable;
            simSettingsState.mEnable = true;
        }
        TextView tv = (TextView) mDualSimSettingsTile.findViewById(R.id.text);
        tv.setText(R.string.dual_sim_settings);
        ImageView imageView = (ImageView) mDualSimSettingsTile
                .findViewById(R.id.image);
        imageView.setImageResource(simSettingsState.mIconId);
        mDualSimSettingsTile.setEnabled(simSettingsState.mEnable);
    }

    private void startSettingsActivity(Intent intent) {
        if (!mDeviceProvisioned) {
            return;
        }
        try {
            // Dismiss the lock screen when Settings starts.
            ActivityManagerNative.getDefault().dismissKeyguardOnNextActivity();
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }
        mContext.startActivityAsUser(intent, new UserHandle(
                UserHandle.USER_CURRENT));
        try {
            IStatusBarService statusbarService = getStatusbarService();
            if (statusbarService != null) {
                statusbarService.collapsePanels();
            }
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }
    }

    private IStatusBarService getStatusbarService() {
        if (mStatusbarService == null) {
            mStatusbarService = IStatusBarService.Stub
                    .asInterface(ServiceManager.getService("statusbar"));
        }
        return mStatusbarService;
    }

    /** ContentObserver to determine the APN net */
    private class APNChangeObserver extends ContentObserver {
        public APNChangeObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            updateNetSwapTile();
        }

        public void startObserving() {
            final ContentResolver cr = mContext.getContentResolver();
            cr.registerContentObserver(Uri.parse(APN_URI), false, this);
        }
    };

    public int getDataConnectionIcon() {
        return -1;
    }

    private static PhoneConstants.DataState getMobileDataState(Intent intent) {
        String str = intent.getStringExtra(PhoneConstants.STATE_KEY);
        if (str != null) {
            return Enum.valueOf(PhoneConstants.DataState.class, str);
        } else {
            return PhoneConstants.DataState.DISCONNECTED;
        }
    }

    static class TileState {
        int mIconId;
        String mLabel;
        boolean mEnable;
    }
}
