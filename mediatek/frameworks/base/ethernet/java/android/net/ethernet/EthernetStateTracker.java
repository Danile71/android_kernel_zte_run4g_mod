/*
 * Copyright (C) 2010 The Android-X86 Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Yi Sun <beyounn@gmail.com>
 */

package android.net.ethernet;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.DhcpResults;
import android.net.DhcpInfo;
import android.net.LinkCapabilities;
import android.net.LinkProperties;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.NetworkUtils;
import android.net.RouteInfo;
import android.net.LinkQualityInfo;
import android.net.NetworkStateTracker;
import android.net.SamplingDataTracker;

import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.ServiceManager;
import com.android.server.EthernetService;
import android.util.Slog;

import java.util.concurrent.atomic.AtomicBoolean;
   
/**
 * Track the state of Ethernet connectivity. All event handling is done here,
 * and all changes in connectivity state are initiated here.
 *
 * @hide
 */

public class EthernetStateTracker implements NetworkStateTracker {
    private static final String TAG                                 = "EthernetStateTracker";
    public static final int EVENT_DHCP_START                        = 0;
    public static final int EVENT_INTERFACE_CONFIGURATION_SUCCEEDED = 1;
    public static final int EVENT_INTERFACE_CONFIGURATION_FAILED    = 2;
    public static final int EVENT_HW_CONNECTED                      = 3;
    public static final int EVENT_HW_DISCONNECTED                   = 4;
    public static final int EVENT_HW_PHYCONNECTED                   = 5;
    private static final int NOTIFY_ID                              = 6;
    private static final boolean localLOGV = true;

    private AtomicBoolean mTeardownRequested = new AtomicBoolean(false);
    private AtomicBoolean mPrivateDnsRouteSet = new AtomicBoolean(false);
    private AtomicBoolean mDefaultRouteSet = new AtomicBoolean(false);

    private NetworkInfo mNetworkInfo;
    private LinkProperties mLinkProperties;
    private LinkCapabilities mLinkCapabilities;
	private LinkQualityInfo mLinkQualityInfo;

    private EthernetManager mService;
    private BroadcastReceiver mEthernetStateReceiver;

    /* For sending events to connectivity service handler */
    private Handler mCsHandler;
    private Context mContext;

    public EthernetStateTracker(int netType, Context context) {
        if (localLOGV) Slog.v(TAG, "EthernetStateTracker Starts...");
        mContext = context;
        mNetworkInfo = new NetworkInfo(ConnectivityManager.TYPE_ETHERNET, 0, "ETH", "");
        mLinkProperties = new LinkProperties();
        mLinkCapabilities = new LinkCapabilities();
	mLinkQualityInfo = new LinkQualityInfo();
        EthernetService ethService = new EthernetService(mContext);
        ServiceManager.addService("ethernetservice", ethService);
    }

    @Override
    public void startMonitoring(Context context, Handler target) {
        if (localLOGV) Slog.v(TAG,"start to monitor the ethernet devices");
        mService = EthernetManager.getInstance();
        mCsHandler = target;

        IntentFilter filter = new IntentFilter();
        filter.addAction(EthernetManager.NETWORK_STATE_CHANGED_ACTION);

        mEthernetStateReceiver = new EthernetStateReceiver();
        mContext.registerReceiver(mEthernetStateReceiver, filter);
    }



    @Override
    public String getTcpBufferSizesPropName() {
        return "net.tcp.buffersize.default";
    }

    @Override
    public boolean isAvailable() {
        // Only say available if we have interfaces and user did not disable us.
        return ((mService.getTotalInterface() != 0) && (mService.getState() != EthernetManager.ETHERNET_STATE_DISABLED));
    }

    @Override
    public boolean teardown() {
        return (mService != null) && mService.teardown();
    }

    @Override
    public boolean reconnect() {
        return (mService != null) && mService.reconnect();
    }

    @Override
    public boolean setRadio(boolean turnOn) {
        return false;
    }

/* HFM
    @Override
    public int startUsingNetworkFeature(String feature, int callingPid, int callingUid) {
        return 0;
    }

    @Override
    public int stopUsingNetworkFeature(String feature, int callingPid, int callingUid) {
        return 0;
    }
*/
    @Override
    public void setUserDataEnable(boolean enabled) {
        Slog.w(TAG, "ignoring setUserDataEnable(" + enabled + ")");
    }

    @Override
    public void setPolicyDataEnable(boolean enabled) {
        Slog.w(TAG, "ignoring setPolicyDataEnable(" + enabled + ")");
    }

    public void setDependencyMet(boolean met) {
        // not supported on this network
    }

    public void setTeardownRequested(boolean isRequested) {
        mTeardownRequested.set(isRequested);
    }

    public boolean isTeardownRequested() {
        return mTeardownRequested.get();
    }
    /**
     * Check if private DNS route is set for the network
     */
    public boolean isPrivateDnsRouteSet() {
        return mPrivateDnsRouteSet.get();
    }

    /**
     * Set a flag indicating private DNS route is set
     */
    public void privateDnsRouteSet(boolean enabled) {
        mPrivateDnsRouteSet.set(enabled);
    }

    /**
     * Fetch NetworkInfo for the network
     */
    public NetworkInfo getNetworkInfo() {
        return new NetworkInfo(mNetworkInfo);
    }
	
    public void getDhcpInfo(EthernetDevInfo info)
    {
    }

    /**
     * Fetch LinkProperties for the network
     */
    public LinkProperties getLinkProperties() {
        return new LinkProperties(mLinkProperties);
    }

    /**
     * A capability is an Integer/String pair, the capabilities
     * are defined in the class LinkSocket#Key.
     *
     * @return a copy of this connections capabilities, may be empty but never null.
     */
    public LinkCapabilities getLinkCapabilities() {
        return new LinkCapabilities(mLinkCapabilities);
    }

    /**
     * Get interesting information about this network link
     * @return a copy of link information, null if not available
     */
    public LinkQualityInfo getLinkQualityInfo()	 {
        return null;
    }	

    /**
     * Ready to switch on to the network after captive portal check
     */
    public void captivePortalCheckComplete() {
          // not support on this network
    }

    /**
     * Captive portal check has completed
     */
    public void captivePortalCheckCompleted(boolean isCaptive) {
         // not support on this network
    }

    /**
     * Check if default route is set
     */
    public boolean isDefaultRouteSet() {
        return mDefaultRouteSet.get();
    }

    /**
     * Set a flag indicating default route is set for the network
     */
    public void defaultRouteSet(boolean enabled) {
        mDefaultRouteSet.set(enabled);
    }

    /**
     * Informs the state tracker that another interface is stacked on top of it.
     **/
    public void addStackedLink(LinkProperties link) {
    
    }

    /**
     * Informs the state tracker that a stacked interface has been removed.
     **/
    public void removeStackedLink(LinkProperties link) {

    }

    /*
     * Called once to setup async channel between this and
     * the underlying network specific code.
     */
    public void supplyMessenger(Messenger messenger) {

    }

    /*
     * Network interface name that we'll lookup for sampling data
     */
    public String getNetworkInterfaceName() {
        return "eth0" ;
    }

    /*
     * Save the starting sample
     */
    public void startSampling(SamplingDataTracker.SamplingSnapshot s) {
       // do nothing
    }

    /*
     * Save the ending sample
     */
    public void stopSampling(SamplingDataTracker.SamplingSnapshot s) {
      // do nothing
    }	
    

    class EthernetStateReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(EthernetManager.NETWORK_STATE_CHANGED_ACTION)) {

                if (localLOGV) Slog.v(TAG,"receive NETWORK_STATE_CHANGED_ACTION");

                mNetworkInfo = intent.getParcelableExtra(EthernetManager.EXTRA_NETWORK_INFO);
                mLinkProperties = intent.getParcelableExtra(EthernetManager.EXTRA_LINK_PROPERTIES);
                mLinkCapabilities = intent.getParcelableExtra(EthernetManager.EXTRA_LINK_CAPABILITIES);

                Message msg = mCsHandler.obtainMessage(EVENT_STATE_CHANGED, new NetworkInfo(mNetworkInfo));
                msg.sendToTarget();
            }
        }
    }
}
