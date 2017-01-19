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

import java.net.UnknownHostException;
import java.util.List;
import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.net.wifi.IWifiManager;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Slog;
import android.net.DhcpInfo;
import android.os.IBinder;
import android.os.ServiceManager;
import android.net.ethernet.IEthernetManager;



/**
 * This class provides the primary API for managing all aspects of Ethernet
 * connectivity. Get an instance of this class by calling
 * {@link android.content.Context#getSystemService(String) Context.getSystemService(Context.ETHERNET_SERVICE)}.
 *
 * This is the API to use when performing Ethernet specific operations. To
 * perform operations that pertain to network connectivity at an abstract
 * level, use {@link android.net.ConnectivityManager}.
 * {@hide}
 */
public class EthernetManager {
    public static final String TAG = "EthernetManager";
    public static final int ETHERNET_DEVICE_SCAN_RESULT_READY = 0;
    public static final String ETHERNET_STATE_CHANGED_ACTION =
            "android.net.ethernet.ETHERNET_STATE_CHANGED";
    public static final String NETWORK_STATE_CHANGED_ACTION =
            "android.net.ethernet.STATE_CHANGE";

    public static final String ETHERNET_STOP_INTERFACE_ACTION = "android.net.ethernet.STOP_INTERFACE_ACTION";
    public static final String ETHERNET_START_INTERFACE_ACTION = "android.net.ethernet.START_INTERFACE_ACTION";
    public static final String EXTRA_NETWORK_INFO = "networkInfo";
    public static final String EXTRA_ETHERNET_STATE = "ETHERNET_state";
    public static final String EXTRA_PREVIOUS_ETHERNET_STATE = "previous_ETHERNET_state";
    /**
     * The lookup key for a {@link android.net.LinkProperties} object associated with the
     * Ethernet network. Retrieve with
     * {@link android.content.Intent#getParcelableExtra(String)}.
     * @hide
     */
    public static final String EXTRA_LINK_PROPERTIES = "linkProperties";

    private static EthernetManager ethernet;

    /**
     * The lookup key for a {@link android.net.LinkCapabilities} object associated with the
     * Ethernet network. Retrieve with
     * {@link android.content.Intent#getParcelableExtra(String)}.
     * @hide
     */
    public static final String EXTRA_LINK_CAPABILITIES = "linkCapabilities";

    public static final int ETHERNET_STATE_UNKNOWN = 0;
    public static final int ETHERNET_STATE_DISABLED = 1;
    public static final int ETHERNET_STATE_ENABLED = 2;
    public static final int EVENT_INTERFACE_CONFIGURATION_SUCCEEDED = 1;
    public static final int EVENT_INTERFACE_CONFIGURATION_FAILED = 2;
    public static final int EVENT_HW_CONNECTED = 3;
    public static final int EVENT_HW_DISCONNECTED = 4;
    public static final int EVENT_HW_PHYCONNECTED = 5;

    public static final String ETHERNET_ON = "ethernet_on";
    public static final String ETHERNET_MODE = "ethernet_mode";
    public static final String ETHERNET_IP = "ethernet_ip";
    public static final String ETHERNET_MASK = "ethernet_netmask";
    public static final String ETHERNET_DNS = "ethernet_dns";
    public static final String ETHERNET_DNS2 = "ethernet_dns2";
    public static final String ETHERNET_ROUTE = "ethernet_iproute";
    public static final String ETHERNET_CONF = "ethernet_conf";
    public static final String ETHERNET_IFNAME = "ethernet_ifname";

    IEthernetManager mService;

    public EthernetManager(IEthernetManager service) {
        Slog.i(TAG, "Init Ethernet Manager, service: " +service);
        mService = service;	
    }

    public static EthernetManager getInstance() { 
        if(ethernet == null)
        {
            IBinder b = ServiceManager.getService("ethernetservice");
            if (b != null) {
                IEthernetManager service = IEthernetManager.Stub.asInterface(b);
                ethernet = new EthernetManager(service);			
            }else{
        	Slog.w(TAG, "Error getting service name:" + "ethernetservice");
            }
        }         
        return ethernet;
    }

    /**
     * check if the ethernet service has been configured.
     * @return {@code true} if configured {@code false} otherwise
     */
    public boolean isConfigured() {
        try {
            return mService.isConfigured();
        } catch (RemoteException e) {
            Slog.e(TAG, "Can not check eth config state");
        }
        return false;
    }

    /**
     * Return the saved ethernet configuration
     * @return ethernet interface configuration on success, {@code null} on failure
     */
    public EthernetDevInfo getSavedConfig() {
        try {
            return mService.getSavedConfig();
        } catch (RemoteException e) {
            Slog.e(TAG, "Can not get eth config");
        }
        return null;
    }

    /**
     * update a ethernet interface information
     * @param info  the interface infomation
     */
    public void updateDevInfo(EthernetDevInfo info) {
        try {
            mService.updateDevInfo(info);
        } catch (RemoteException e) {
            Slog.e(TAG, "Can not update ethernet device info");
        }
    }

    /**
     * get all the ethernet device names
     * @return interface name list on success, {@code null} on failure
     */
    public String[] getDeviceNameList() {
        try {
            return mService.getDeviceNameList();
        } catch (RemoteException e) {
            return null;
        }
    }
    /**
     * Enable or Disable a ethernet service
     * @param enable {@code true} to enable, {@code false} to disable
     * @hide
     */
    public void setEnabled(boolean enable) {
        try {
            mService.setState(enable ? ETHERNET_STATE_ENABLED:ETHERNET_STATE_DISABLED);
        } catch (RemoteException e) {
            Slog.e(TAG,"Can not set new state");
        }
    }

    /**
     * Get ethernet service state
     * @return the state of the ethernet service
     */
    public int getState( ) {
        try {
            return mService.getState();
        } catch (RemoteException e) {
            return 0;
        }
    }

    /**
     * get the number of ethernet interfaces in the system
     * @return the number of ethernet interfaces
     */
    public int getTotalInterface() {
        try {
            return mService.getTotalInterface();
        } catch (RemoteException e) {
            return 0;
        }
    }

    /**
     * @hide
     */
    public void setDefaultConf() {
        try {
            mService.setMode(EthernetDevInfo.ETHERNET_CONN_MODE_DHCP);
        } catch (RemoteException e) {
        }
    }
    public DhcpInfo getDhcpInfo() {
        try {
            return mService.getDhcpInfo();
        } catch (RemoteException e) {
            return null;
        }
    }
    public String getMacAddr() {
        try {
            return mService.getMacAddr();
        } catch (RemoteException e) {
            return null;
        }
    }
    public boolean getLinkStatus(String nic) {
        try {
            return mService.getLinkStatus(nic);
        } catch (RemoteException e) {
            return false;
        }
    }
    public boolean teardown() {
        try {
            return mService.teardown();
        } catch (RemoteException e) {
            return false;
        }
    }
    public boolean reconnect() {
        try {
            return mService.reconnect();
        } catch (RemoteException e) {
            return false;
        }
    }
}
