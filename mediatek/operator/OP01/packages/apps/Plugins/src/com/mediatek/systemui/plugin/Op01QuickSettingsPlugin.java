package com.mediatek.systemui.plugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.util.Log;
import android.widget.TextView;

import com.mediatek.systemui.ext.DefaultQuickSettingsPlugin;

import java.util.List;

/**
 * Customize the quick settings.
 *
 */
public class Op01QuickSettingsPlugin extends DefaultQuickSettingsPlugin {
    private static final boolean DEBUG = true;
    private static final String TAG = "Op01QuickSettingsPlugin";

    private TextView mWifiLabel;
    // wifi
    final WifiManager mWifiManager;
    private String mConnectedWifiString;
    private String mDisconnectedWifiString;
    private boolean mIsWifiConnected;

    /**
     * Constructor.
     * @param context The Context.
     */
    public Op01QuickSettingsPlugin(Context context) {
        super(context);

        if (DEBUG) {
            Log.d(TAG, "Op01QuickSettingsPlugin, mContext=" + context
                    + " register intent filter: " + WifiManager.NETWORK_STATE_CHANGED_ACTION);
        }
        // wifi
        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        registerFilter();
    }

    private void registerFilter() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mContext.registerReceiver(mIntentReceiver, filter);
    }

    @Override
    public void customizeWifiTileSSID(TextView wifiLabel, String wifiString) {
        if (DEBUG) {
            Log.d(TAG, "customizeWifiTileSSID,"
                    + " mIsWifiConnected=" + mIsWifiConnected
                    + " wifiLabel.getText()=" + wifiLabel.getText()
                    + " wifiString=" + wifiString
                    + " mConnectedWifiString=" + mConnectedWifiString);
        }
        mWifiLabel = wifiLabel;
        mDisconnectedWifiString = wifiString;
        if (mIsWifiConnected) {
            wifiLabel.setText(mConnectedWifiString);
        } else {
            wifiLabel.setText(mDisconnectedWifiString);
        }
        mWifiLabel.setAllCaps(false);
    }

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (DEBUG) {
                Log.d(TAG, "onReceive called, action is " + action);
            }
            if (action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                final android.net.NetworkInfo networkInfo = (NetworkInfo)
                        intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                mIsWifiConnected = (networkInfo != null && networkInfo.isConnected());

                final WifiInfo wifiInfo = (WifiInfo) intent
                        .getParcelableExtra(WifiManager.EXTRA_WIFI_INFO);
                String wifiSsid = null;
                if (wifiInfo != null) {
                    wifiSsid = huntForSsid(wifiInfo);
                    wifiSsid = removeDoubleQuotes(wifiSsid);
                    mConnectedWifiString = wifiSsid;
                }

                if (DEBUG) {
                    Log.d(TAG, "onReceive, NETWORK_STATE_CHANGED_ACTION: mIsWifiConnected="
                            + mIsWifiConnected + " wifiInfo=" + wifiInfo
                            + " wifiSsid=" + wifiSsid
                            + " mConnectedWifiString=" + mConnectedWifiString
                            + " mDisconnectedWifiString=" + mDisconnectedWifiString);
                }
                if (mIsWifiConnected && wifiSsid != null) {
                    mWifiLabel.setText(wifiSsid);
                } else {
                    mWifiLabel.setText(mDisconnectedWifiString);
                }
            }
        }
    };

    private String huntForSsid(WifiInfo info) {
        String ssid = info.getSSID();
        if (ssid != null) {
            return ssid;
        }
        // OK, it's not in the connectionInfo; we have to go hunting for it
        List<WifiConfiguration> networks = mWifiManager.getConfiguredNetworks();
        for (WifiConfiguration net : networks) {
            if (net.networkId == info.getNetworkId()) {
                return net.SSID;
            }
        }
        return null;
    }

    private String removeDoubleQuotes(String string) {
        if (string != null) {
            int length = string.length();
            if ((length > 1) && (string.charAt(0) == '"')
                && (string.charAt(length - 1) == '"')) {
                return string.substring(1, length - 1);
            }
        }
        return string;
    }
}
