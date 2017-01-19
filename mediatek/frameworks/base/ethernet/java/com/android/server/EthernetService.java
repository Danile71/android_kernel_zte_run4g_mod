/*
 * Copyright (C) 2010 The Android-x86 Open Source Project
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

package com.android.server;

import android.app.ActivityManagerNative;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.DhcpResults;
import android.net.DhcpInfo;
import android.net.LinkCapabilities;
import android.net.LinkProperties;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.NetworkUtils;
import android.net.RouteInfo;

import android.net.ethernet.EthernetManager;
import android.net.ethernet.EthernetDevInfo;
import android.net.ethernet.EthernetNative;
import android.net.ethernet.IEthernetManager;


import android.os.Handler;
import android.os.Message;
import android.os.Looper;
import android.os.HandlerThread;
import android.os.ServiceManager;

import android.provider.Settings;
import android.text.TextUtils;
import android.util.Slog;

import java.net.InetAddress;
import java.net.Inet4Address;

import java.net.UnknownHostException;


/**
 * EthernetService handles remote Ethernet operation requests by implementing
 * the IEthernetManager interface. It also creates a EtherentMonitor to listen
 * for Etherent-related events.
 * {@hide}
 */
public class EthernetService<syncronized> extends IEthernetManager.Stub {
    public static final int EVENT_DHCP_START = 0;

    private static final String TAG = "EthernetService";
    private static final int ETHERNET_HAS_CONFIG = 1;
    private static final boolean localLOGV = true;

    private int mEthState = EthernetManager.ETHERNET_STATE_UNKNOWN;
    private Context mContext;
    private String[] DevName;
    private String mInterfaceName;	
    private DhcpResults mDhcpResult;
    private boolean mStartingDhcp;

    private NetworkInfo mNetworkInfo;
    private LinkProperties mLinkProperties;
    private LinkCapabilities mLinkCapabilities;

    private boolean mStackConnected;
    private boolean mHWConnected;
    private boolean mInterfaceStopped;

    private EventHandler handler;
    private DhcpHandler mDhcpTarget;

    private EthernetMonitor mMonitor;

    class EthernetMonitor {
        private static final String TAG = "EthernetMonitor";
        private static final boolean localLOGV = true;

        private static final int CONNECTED = 1;
        private static final int DISCONNECTED = 2;
        private static final int PHYUP = 3;

        private static final int ADD_ADDR = 20;
        private static final int RM_ADDR = 21;
        private static final int NEW_LINK = 16;
        private static final int DEL_LINK = 17;

        public EthernetMonitor() {

        }

        public void startMonitoring() {
            new MonitorThread().start();
        }

        class MonitorThread extends Thread {
            public MonitorThread() {
                super("EthMonitor");
            }

            public void run() {
                //noinspection InfiniteLoopStatement
                for (; ; ) {
                    int index;
                    int i;					
                    int cmd;
                    String dev;

                    if (localLOGV) Slog.v(TAG, "go poll events");

                    String eventName = EthernetNative.waitForEvent();

                    if (eventName == null) {
                        continue;
                    }

                    if (localLOGV) Slog.v(TAG, "get event " + eventName);

                    i = 0;
                    while (i < eventName.length()) {
                        index = eventName.substring(i).indexOf(":");
                        if (index == -1)
                            break;
                        dev = eventName.substring(i, index);
                        i += index + 1;
                        index = eventName.substring(i).indexOf(":");
                        if (index == -1)
                            break;
                        cmd = Integer.parseInt(eventName.substring(i, i + index));
                        i += index + 1;
                        if (localLOGV) Slog.v(TAG, "dev: " + dev + " ev " + cmd);
                        switch (cmd) {
                            case DEL_LINK:
                                handleEvent(dev, DISCONNECTED);
                                break;
                            case ADD_ADDR:
                                handleEvent(dev, CONNECTED);
                                break;
                            case NEW_LINK:
                                handleEvent(dev, PHYUP);
                                break;
                        }
                    }
                }
            }

            /**
             * Handle all supplicant events except STATE-CHANGE
             *
             * @param event     the event type
             */
            void handleEvent(String ifname, int event) {
                switch (event) {
                    case DISCONNECTED:
                        notifyStateChange(ifname, NetworkInfo.DetailedState.DISCONNECTED);
                        break;
                    case CONNECTED:
                        notifyStateChange(ifname, NetworkInfo.DetailedState.CONNECTED);
                        break;
                    case PHYUP:
                        notifyPhyConnected(ifname);
                        break;
                    default:
                        notifyStateChange(ifname, NetworkInfo.DetailedState.FAILED);
                        break;
                }
            }
        }
    }

    public void notifyPhyConnected(String ifname) {
        if (localLOGV) Slog.v(TAG, "report interface is up for " + ifname);
        synchronized (this) {
            handler.sendEmptyMessage(EthernetManager.EVENT_HW_PHYCONNECTED);
        }
    }

    public void notifyStateChange(String ifname, NetworkInfo.DetailedState state) {
        if (localLOGV) Slog.i(TAG, "report new state " + state.toString() + " on dev " + ifname);
        if (ifname.equals(mInterfaceName)) {
            if (localLOGV) Slog.v(TAG, "update network state tracker");
            synchronized (this) {
                handler.sendEmptyMessage(state.equals(NetworkInfo.DetailedState.CONNECTED)
                        ? EthernetManager.EVENT_HW_CONNECTED : EthernetManager.EVENT_HW_DISCONNECTED);
            }
        }
    }

    public EthernetService(Context context) {
        if (localLOGV) Slog.v(TAG, "EthernetService constructor");

        mContext = context;
        mNetworkInfo = new NetworkInfo(ConnectivityManager.TYPE_ETHERNET, 0, "ETH", "");
        mLinkProperties = new LinkProperties();
        mLinkCapabilities = new LinkCapabilities();

        HandlerThread dhcpThread = new HandlerThread("DHCP Handler Thread");
        dhcpThread.start();
        handler = new EventHandler();
        mDhcpTarget = new DhcpHandler(dhcpThread.getLooper(), handler);		
        mDhcpResult = new DhcpResults();

        if (EthernetNative.initEthernetNative() != 0) {
            Slog.e(TAG,"Can not init ethernet device layers");
        }

        getDeviceNameList();
  //    setState(getPersistedState());
        setState(2);
    
        mMonitor = new EthernetMonitor();
        mMonitor.startMonitoring();

    }


    /**
     * check if the ethernet service has been configured.
     *
     * @return {@code true} if configured {@code false} otherwise
     */
    public boolean isConfigured() {
        final ContentResolver cr = mContext.getContentResolver();
        return (Settings.Secure.getInt(cr, EthernetManager.ETHERNET_CONF, 0) == ETHERNET_HAS_CONFIG);

    }

    /**
     * Return the saved ethernet configuration
     *
     * @return ethernet interface configuration on success, {@code null} on failure
     */
    public synchronized EthernetDevInfo getSavedConfig() {
        if (!isConfigured())
            return null;
        Slog.v(TAG, "getSavedConfig");

        EthernetDevInfo info = new EthernetDevInfo();			

        final ContentResolver cr = mContext.getContentResolver();

        info.setConnectMode(Settings.Secure.getString(cr, EthernetManager.ETHERNET_MODE));
        info.setIfName(Settings.Secure.getString(cr, EthernetManager.ETHERNET_IFNAME));
        if (info.getConnectMode().equals(EthernetDevInfo.ETHERNET_CONN_MODE_DHCP))	 {
            DhcpInfo dInfo = getDhcpInfo();
            if(dInfo == null) {        
                Slog.e(TAG, "info is null");
                info.setIpAddress("0.0.0.0");
                info.setDnsAddr("0.0.0.0");
                info.setNetMask("0.0.0.0");
                info.setRouteAddr("0.0.0.0");		
            } else {					 
                String iPtemp = new String(""+((dInfo.ipAddress>>0)&0xff)+"."+((dInfo.ipAddress>>8)&0xff)+"."+((dInfo.ipAddress>>16)&0xff)+"."+((dInfo.ipAddress>>24)&0xff));
                String dnsTemp = new String(""+((dInfo.dns1>>0)&0xff)+"."+((dInfo.dns1>>8)&0xff)+"."+((dInfo.dns1>>16)&0xff)+"."+((dInfo.dns1>>24)&0xff));;
                String NetMaskTemp = new String(""+((dInfo.netmask>>0)&0xff)+"."+((dInfo.netmask>>8)&0xff)+"."+((dInfo.netmask>>16)&0xff)+"."+((dInfo.netmask>>24)&0xff));
                String RoutTemp = new String(""+((dInfo.gateway>>0)&0xff)+"."+((dInfo.gateway>>8)&0xff)+"."+((dInfo.gateway>>16)&0xff)+"."+((dInfo.gateway>>24)&0xff));
                info.setIpAddress(iPtemp);
                info.setDnsAddr(dnsTemp);
                info.setNetMask(NetMaskTemp);
                info.setRouteAddr(RoutTemp);						 
            }
            Slog.v(TAG, "ifName: "+info.getIfName()+" ipaddr:"+info.getIpAddress()+" netmask:"+info.getNetMask()+" getway:"+info.getRouteAddr()+" dns:"+info.getDnsAddr());
        } else {
            info.setIpAddress(Settings.Secure.getString(cr, EthernetManager.ETHERNET_IP));
            info.setDnsAddr(Settings.Secure.getString(cr, EthernetManager.ETHERNET_DNS));
            info.setNetMask(Settings.Secure.getString(cr, EthernetManager.ETHERNET_MASK));
            info.setRouteAddr(Settings.Secure.getString(cr, EthernetManager.ETHERNET_ROUTE));
        }
        return info;

    }

    /**
     * Set the ethernet interface configuration mode
     *
     * @param mode {@code ETHERNET_CONN_MODE_DHCP} for dhcp {@code ETHERNET_CONN_MODE_MANUAL} for manual configure
     */
    public synchronized void setMode(String mode) {
        final ContentResolver cr = mContext.getContentResolver();
        if (DevName != null) {
            Settings.Secure.putString(cr, EthernetManager.ETHERNET_IFNAME, DevName[0]);
            Settings.Secure.putInt(cr, EthernetManager.ETHERNET_CONF, 1);
            Settings.Secure.putString(cr, EthernetManager.ETHERNET_MODE, mode);
        }
    }

    /**
     * update a ethernet interface information
     *
     * @param info the interface infomation
     */
    public synchronized void updateDevInfo(EthernetDevInfo info) {
        final ContentResolver cr = mContext.getContentResolver();
        Settings.Secure.putInt(cr, EthernetManager.ETHERNET_CONF, 1);
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_IFNAME, info.getIfName());
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_IP, info.getIpAddress());
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_MODE, info.getConnectMode());
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_DNS, info.getDnsAddr());
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_DNS2, info.getDns2Addr());
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_ROUTE, info.getRouteAddr());
        Settings.Secure.putString(cr, EthernetManager.ETHERNET_MASK, info.getNetMask());
        if (mEthState == EthernetManager.ETHERNET_STATE_ENABLED) {
            resetInterface();
        }
    }

    /**
     * get the number of ethernet interfaces in the system
     *
     * @return the number of ethernet interfaces
     */
    public int getTotalInterface() {
        return EthernetNative.getInterfaceCnt();
    }

    private int scanDevice() {
        int i, j;
        if ((i = EthernetNative.getInterfaceCnt()) == 0)
            return 0;

        DevName = new String[i];

        for (j = 0; j < i; j++) {
            DevName[j] = EthernetNative.getInterfaceName(j);
            if (DevName[j] == null)
                break;
            if (localLOGV) Slog.v(TAG, "device " + j + " name " + DevName[j]);
        }

        return i;
    }

    /**
     * get all the ethernet device names
     *
     * @return interface name list on success, {@code null} on failure
     */
    public String[] getDeviceNameList() {
        return (scanDevice() > 0) ? DevName : null;
    }

    private int getPersistedState() {
        final ContentResolver cr = mContext.getContentResolver();
        try {
            return Settings.Secure.getInt(cr, EthernetManager.ETHERNET_ON);
        } catch (Settings.SettingNotFoundException e) {
            return EthernetManager.ETHERNET_STATE_UNKNOWN;
        }
    }

    private synchronized void persistEnabled(boolean enabled) {
        final ContentResolver cr = mContext.getContentResolver();
        Settings.Secure.putInt(cr, EthernetManager.ETHERNET_ON, enabled ? EthernetManager.ETHERNET_STATE_ENABLED : EthernetManager.ETHERNET_STATE_DISABLED);
    }

    /**
     * Enable or Disable a ethernet service
     *
     * @param state {@code true} to enable, {@code false} to disable
     */
    public synchronized void setState(int state) {
        if (localLOGV) Slog.v(TAG, "setState, mEthState=" + mEthState + ", state="+state);
        if (mEthState != state) {
            mEthState = state;
            if (state == EthernetManager.ETHERNET_STATE_DISABLED) {
                persistEnabled(false);
                stopInterface(false);
            } else {
                persistEnabled(true);
                if (!isConfigured()) {
                    // If user did not configure any interfaces yet, pick the first one
                    // and enable it.
                    setMode(EthernetDevInfo.ETHERNET_CONN_MODE_DHCP);
                }
                try {
                    initInterface();
                } catch (UnknownHostException e) {
                    Slog.e(TAG, "Wrong ethernet configuration");
                }
            }
        }
    }

    /**
     * Get ethernet service state
     *
     * @return the state of the ethernet service
     */
    public int getState() {
        return mEthState;
    }

    public String getMacAddr() {
        return EthernetNative.getMacAddressCommand();
    }

    public DhcpInfo getDhcpInfo() {
        if (mDhcpResult.linkProperties == null) return null;
        DhcpInfo info = new DhcpInfo();
        for (LinkAddress la : mDhcpResult.linkProperties.getLinkAddresses()) {
            InetAddress addr = la.getAddress();
            if (addr instanceof Inet4Address) {
                info.ipAddress = NetworkUtils.inetAddressToInt((Inet4Address)addr);
                break;
            }
        }
        for (RouteInfo r : mDhcpResult.linkProperties.getRoutes()) {
            if (r.isDefaultRoute()) {
                InetAddress gateway = r.getGateway();
                if (gateway instanceof Inet4Address) {
                    info.gateway = NetworkUtils.inetAddressToInt((Inet4Address)gateway);
                }
            } else if (r.hasGateway() == false) {
                LinkAddress dest = r.getDestination();
                if (dest.getAddress() instanceof Inet4Address) {
                    info.netmask = NetworkUtils.prefixLengthToNetmaskInt(
                            dest.getNetworkPrefixLength());
                }
            }
        }
        int dnsFound = 0;
        for (InetAddress dns : mDhcpResult.linkProperties.getDnses()) {
            if (dns instanceof Inet4Address) {
                if (dnsFound == 0) {
                    info.dns1 = NetworkUtils.inetAddressToInt((Inet4Address)dns);
                } else {
                    info.dns2 = NetworkUtils.inetAddressToInt((Inet4Address)dns);
                }
                if (++dnsFound > 1) break;
            }
        }
        InetAddress serverAddress = mDhcpResult.serverAddress;
        if (serverAddress instanceof Inet4Address) {
            info.serverAddress = NetworkUtils.inetAddressToInt((Inet4Address)serverAddress);
        }
        info.leaseDuration = mDhcpResult.leaseDuration;

        return info;
    }

    public boolean getLinkStatus(String nic) {
        return (EthernetNative.getLinkStatus() != 0);
    }

    public boolean teardown(){
        synchronized (this) {
            if (!mHWConnected)
                return true;
        }

        return stopInterface(false);
    }

    public boolean reconnect() {
        synchronized (this) {
            if (mHWConnected && mStackConnected)
                return true;
        }

        if (getState() != EthernetManager.ETHERNET_STATE_DISABLED) {
            // maybe this is the first time we run, so set it to enabled
            setState(EthernetManager.ETHERNET_STATE_ENABLED);
            if (!isConfigured()) {
                setMode(EthernetDevInfo.ETHERNET_CONN_MODE_DHCP);
            }

            return resetInterface();
        }

        return false;
    }

    private static int lookupHost(String hostname) {
        InetAddress inetAddress;
        try {
            inetAddress = InetAddress.getByName(hostname);
        } catch (UnknownHostException e) {
            return -1;
        }
        byte[] addrBytes;
        int addr;
        addrBytes = inetAddress.getAddress();
        addr = ((addrBytes[3] & 0xff) << 24)
                | ((addrBytes[2] & 0xff) << 16)
                | ((addrBytes[1] & 0xff) << 8)
                | (addrBytes[0] & 0xff);
        return addr;
    }

    /**
     * init ethernet interface
     *
     * @return true
     * @throws UnknownHostException
     */
    private boolean initInterface() throws UnknownHostException {
        /*
         * This will guide us to enabled the enabled device
         */
        EthernetDevInfo info = getSavedConfig();
        if (info != null && isConfigured()) {
            synchronized (this) {
                mInterfaceName = info.getIfName();
                if (localLOGV) Slog.i(TAG, "initInterface " + mInterfaceName);
                NetworkUtils.resetConnections(mInterfaceName, NetworkUtils.RESET_ALL_ADDRESSES);
                NetworkUtils.enableInterface(mInterfaceName);

                if (mDhcpTarget != null) {
                    mDhcpTarget.removeMessages(EVENT_DHCP_START);
                }

                if (ActivityManagerNative.isSystemReady()) {
                    if (localLOGV) Slog.i(TAG, " Activity Manager native is System Ready.");
                    Intent intent = new Intent(EthernetManager.ETHERNET_START_INTERFACE_ACTION);
                    intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                    mContext.sendBroadcast(intent);
                }

                configureInterface(info);
            }
        }

        return true;
    }

    private boolean resetInterface() {

        EthernetDevInfo info = getSavedConfig();
        if (info != null && isConfigured()) {
            synchronized (this) {
                if (localLOGV) Slog.i(TAG, "reset device " + mInterfaceName);

                mLinkProperties.clear();
                mInterfaceName = info.getIfName();
                NetworkUtils.clearIpAddress(mInterfaceName);
                NetworkUtils.resetConnections(mInterfaceName, NetworkUtils.RESET_ALL_ADDRESSES);
                NetworkUtils.enableInterface(mInterfaceName);
                // Stop DHCP
                if (mDhcpTarget != null) {
                    mDhcpTarget.removeMessages(EVENT_DHCP_START);
                }
                if (!NetworkUtils.stopDhcp(mInterfaceName)) {
                    if (localLOGV) Slog.w(TAG, "Could not stop DHCP");
                }

                try {
                    configureInterface(info);
                } catch (UnknownHostException e) {
                    Slog.e(TAG, "Wrong ethernet configuration");
                    return false;
                }
            }
        }

        return true;
    }


    /**
     * Stop etherent interface
     *
     * @param suspend {@code false} disable the interface {@code true} only reset the connection without disable the interface
     * @return true
     */
    private boolean stopInterface(boolean suspend) {

        EthernetDevInfo info = getSavedConfig();
        if (info != null && isConfigured()) {
            synchronized (mDhcpTarget) {
                mInterfaceStopped = true;
                if (localLOGV) Slog.i(TAG, "stop dhcp and interface");
                mDhcpTarget.removeMessages(EVENT_DHCP_START);
                String ifname = info.getIfName();

                if (!NetworkUtils.stopDhcp(ifname)) {
                    if (localLOGV) Slog.w(TAG, "Could not stop DHCP");
                }
                mLinkProperties.clear();
                NetworkUtils.clearIpAddress(ifname);
                NetworkUtils.resetConnections(ifname, NetworkUtils.RESET_ALL_ADDRESSES);
                if (!suspend)
                    NetworkUtils.disableInterface(ifname);
            }
        }

        if (ActivityManagerNative.isSystemReady()) {
            Intent intent = new Intent(EthernetManager.ETHERNET_STOP_INTERFACE_ACTION);
            intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
            mContext.sendBroadcast(intent);
        }

        return true;
    }

    private boolean configureInterface(EthernetDevInfo info) throws UnknownHostException {
        String mode = info.getConnectMode();
        if (localLOGV) Slog.i(TAG, "configureInterface switch mode to: " + mode + " for interface: " + info.getIfName());

        if (mode.equals(EthernetDevInfo.ETHERNET_CONN_MODE_DHCP)) {
            ///M:@}
            mStartingDhcp = true;
            mDhcpTarget.sendEmptyMessage(EVENT_DHCP_START);
        } else if (mode.equals(EthernetDevInfo.ETHERNET_CONN_MODE_MANUAL)) { 
            DhcpInfo tTmpDhcpInfo = new DhcpInfo();
            tTmpDhcpInfo.ipAddress = lookupHost(info.getIpAddress()); 
            tTmpDhcpInfo.gateway = lookupHost(info.getRouteAddr());
            tTmpDhcpInfo.netmask = lookupHost(info.getNetMask());
            tTmpDhcpInfo.dns1 = lookupHost(info.getDnsAddr());
            tTmpDhcpInfo.dns2 = lookupHost(info.getDns2Addr());

            mLinkProperties.clear();
            if (NetworkUtils.configureInterface(info.getIfName(), tTmpDhcpInfo)) {
                if (localLOGV) Slog.v(TAG, "Static IP configuration succeeded");
                updateLinkPropertiesFromDevInfo(info);
                handler.sendEmptyMessage(EthernetManager.EVENT_INTERFACE_CONFIGURATION_SUCCEEDED);

            } else {
                if (localLOGV) Slog.w(TAG, "Static IP configuration failed");
                handler.sendEmptyMessage(EthernetManager.EVENT_INTERFACE_CONFIGURATION_FAILED);
            }
        }else if (mode.equals(EthernetDevInfo.ETHERNET_CONN_MODE_PPPOE)) {
            if (mDhcpTarget != null) {
                mDhcpTarget.removeMessages(EVENT_DHCP_START);
            }
            if(mStartingDhcp)
            {
                if (!NetworkUtils.stopDhcp(info.getIfName())) {
                    if (localLOGV) Slog.w(TAG, "Could not stop DHCP");
                }
            }
        }

        return true;
    }

    private class DhcpHandler extends Handler {
        private Handler mTargetHandler;

        public DhcpHandler(Looper looper, Handler target) {
            super(looper);
            mTargetHandler = target;
        }

        public void handleMessage(Message msg) {
            int event;

            switch (msg.what) {
                case EVENT_DHCP_START:
                    if (!mInterfaceStopped) {
                        if (!getLinkStatus(mInterfaceName)) {
                            if (localLOGV) Slog.d(TAG, "I/F is not up yet, not to do dhcp");
                            mStartingDhcp = false;
                            return;
                        }
                        if (localLOGV) Slog.d(TAG, "DhcpHandler: DHCP request started");

		        DhcpResults dhcpResults = new DhcpResults();
						
                        if (NetworkUtils.runDhcp(mInterfaceName, dhcpResults)) {
                            event = EthernetManager.EVENT_INTERFACE_CONFIGURATION_SUCCEEDED;
                            Slog.d(TAG, "DhcpHandler:hank DHCP request succeeded: " + dhcpResults.toString());

                        } else {
                            event = EthernetManager.EVENT_INTERFACE_CONFIGURATION_FAILED;
                            Slog.e(TAG, "DhcpHandler: DHCP request failed: " + NetworkUtils.getDhcpError());
                        }

                        updateLinkPropertiesFromDhcpInfoInternal(dhcpResults);
                        mTargetHandler.sendEmptyMessage(event);
                    } else {
                        mInterfaceStopped = false;
                    }
                    mStartingDhcp = false;
                    break;
            }
        }
    }

    private void updateLinkPropertiesFromDevInfo(EthernetDevInfo info) {
        mLinkProperties.setInterfaceName(mInterfaceName);

        InetAddress destAddr = NetworkUtils.numericToInetAddress(info.getIpAddress());
        InetAddress maskAddr = NetworkUtils.numericToInetAddress(info.getNetMask());
        int prefixLength = NetworkUtils.netmaskIntToPrefixLength(NetworkUtils.inetAddressToInt((Inet4Address)maskAddr));
        LinkAddress linkAddress = new LinkAddress(destAddr, prefixLength);
        mLinkProperties.addLinkAddress(linkAddress);

        if (!TextUtils.isEmpty(info.getDnsAddr())) {
            InetAddress dns1Addr = NetworkUtils.numericToInetAddress(info.getDnsAddr());
            mLinkProperties.addDns(dns1Addr);
        }

        if (!TextUtils.isEmpty(info.getDns2Addr())) {
            InetAddress dns2Addr = NetworkUtils.numericToInetAddress(info.getDns2Addr());
            mLinkProperties.addDns(dns2Addr);
        }

        InetAddress gatewayAddr = NetworkUtils.numericToInetAddress(info.getRouteAddr());
        RouteInfo route = new RouteInfo(null, gatewayAddr);
        mLinkProperties.addRoute(route);
    }

    private void updateLinkPropertiesFromDhcpInfoInternal(DhcpResults result) {      
        mDhcpResult = result;

        mLinkProperties = mDhcpResult.linkProperties;
        mLinkProperties.setInterfaceName(mInterfaceName);
        
    }
	
    private void broadcastConnectionInfo(boolean state) {
        boolean isNeedUpdate = false;
        if (mNetworkInfo.isConnected() != state) {
            if (state) {
                if(getLinkStatus(null)){
                    mNetworkInfo.setDetailedState(NetworkInfo.DetailedState.CONNECTED, null, null);
                }
            } else {
                mNetworkInfo.setDetailedState(NetworkInfo.DetailedState.DISCONNECTED, null, null);
                stopInterface(true); 
            }
            mNetworkInfo.setIsAvailable(state);
            isNeedUpdate = true;
        }else if(state){
            /* may change mode from dhcp to static, in this case, also need broadcast information */
            isNeedUpdate = true;
        }

        if(isNeedUpdate){
            if (localLOGV) Slog.i(TAG," broadcastConnectionInfo: " + state);

            final Intent intent = new Intent(EthernetManager.NETWORK_STATE_CHANGED_ACTION);
            intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);

            intent.putExtra(EthernetManager.EXTRA_NETWORK_INFO, mNetworkInfo);
            intent.putExtra(EthernetManager.EXTRA_LINK_PROPERTIES, mLinkProperties);
            intent.putExtra(EthernetManager.EXTRA_LINK_CAPABILITIES, mLinkCapabilities);

            mContext.sendStickyBroadcast(intent);
        }
    }

    private void broadcastConnectionEvent(int event) {
        if (localLOGV) Slog.i(TAG," broadcastConnectionEvent : " + event);

        final Intent intent = new Intent(EthernetManager.ETHERNET_STATE_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        intent.putExtra(EthernetManager.EXTRA_ETHERNET_STATE, event);
        mContext.sendStickyBroadcast(intent);
    }

    class EventHandler extends Handler {
        public void handleMessage(Message msg) {

            synchronized (this) {
                switch (msg.what) {
                    case EthernetManager.EVENT_INTERFACE_CONFIGURATION_SUCCEEDED:
                        if (localLOGV) Slog.i(TAG, "received configured succeeded(1), stack=" + mStackConnected + " HW=" + mHWConnected);
                        mStackConnected = true;
                        broadcastConnectionInfo(true);
                        broadcastConnectionEvent(msg.what);
                        break;
                    case EthernetManager.EVENT_INTERFACE_CONFIGURATION_FAILED:
                        if (localLOGV) Slog.i(TAG, "received configured failed(2), stack=" + mStackConnected + " HW=" + mHWConnected);
                        mStackConnected = false;
                        broadcastConnectionEvent(msg.what);						
                        //start to retry ?
                        break;
                    case EthernetManager.EVENT_HW_CONNECTED:
                        if (localLOGV) Slog.i(TAG, "received HW connected(3), stack=" + mStackConnected + " HW=" + mHWConnected);
                        mHWConnected = true;
                        if (mStackConnected)
                        {
                            broadcastConnectionInfo(true);
                            broadcastConnectionEvent(msg.what);
                        }
                        break;
                    case EthernetManager.EVENT_HW_DISCONNECTED:
                        if (localLOGV) Slog.i(TAG, "received disconnected events(4), stack=" + mStackConnected + " HW=" + mHWConnected);
                        mStackConnected = false;
                        mHWConnected = false;
                        broadcastConnectionInfo(false);
                        broadcastConnectionEvent(msg.what);
                        break;
                    case EthernetManager.EVENT_HW_PHYCONNECTED:
                        if (localLOGV)  Slog.i(TAG, "interface up event, kick off connection request(5)");
                        broadcastConnectionEvent(msg.what);	                        
                        mStackConnected = false;
                        mHWConnected = false;
                        if (!mStartingDhcp) {
                            if (mEthState != EthernetManager.ETHERNET_STATE_DISABLED) {
                                EthernetDevInfo info = getSavedConfig();
                                if (info != null && isConfigured()) {
                                    try {
                                        mInterfaceStopped = false;
                                        configureInterface(info);
                                    } catch (UnknownHostException e) {
                                        // TODO Auto-generated catch block
                                        //e.printStackTrace();
                                        Slog.e(TAG, "Cannot configure interface");
                                    }
                                }
                            }
                        }
                        break;
                }
            }
        }
    }
}
