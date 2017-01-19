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

import android.net.ethernet.EthernetDevInfo;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.Parcelable.Creator;

/**
 * Describes the state of any Ethernet connection that is active or
 * is in the process of being set up.
 * {@hide}
 */

public class EthernetDevInfo implements Parcelable {
    /**
     * The ethernet interface is configured by dhcp
     */
    public static final String ETHERNET_CONN_MODE_DHCP= "dhcp";
    /**
     * The ethernet interface is configured manually
     */
    public static final String ETHERNET_CONN_MODE_MANUAL = "manual";

    public static final String ETHERNET_CONN_MODE_PPPOE = "pppoe";

    private String dev_name;
    private String ipaddr;
    private String netmask;
    private String route;
    private String dns;
    private String dns2;
    private String mode;

    public EthernetDevInfo () {
        dev_name = null;
        ipaddr = null;
        dns = null;
        dns2 = null;
        route = null;
        netmask = null;
        mode = ETHERNET_CONN_MODE_DHCP;
    }

    /**
     * save interface name into the configuration
     */
    public void setIfName(String ifname) {
        this.dev_name = ifname;
    }

    /**
     * Returns the interface name from the saved configuration
     * @return interface name
     */
    public String getIfName() {
        return this.dev_name;
    }

    public void setIpAddress(String ip) {
        this.ipaddr = ip;
    }

    public String getIpAddress( ) {
        return this.ipaddr;
    }

    public void setNetMask(String ip) {
        this.netmask = ip;
    }

    public String getNetMask( ) {
        return this.netmask;
    }

    public void setRouteAddr(String route) {
        this.route = route;
    }

    public String getRouteAddr() {
        return this.route;
    }

    public void setDnsAddr(String dns) {
        this.dns = dns;
    }

    public String getDnsAddr( ) {
        return this.dns;
    }
    public void setDns2Addr(String dns2) {
        this.dns2 = dns2;
    }

    public String getDns2Addr( ) {
        return this.dns2;
    }

    /**
     * Set ethernet configuration mode
     * @param mode {@code ETHERNET_CONN_MODE_DHCP} for dhcp {@code ETHERNET_CONN_MODE_MANUAL} for manual configure
     * @return
     */
    public boolean setConnectMode(String mode) {
        if (mode.equals(ETHERNET_CONN_MODE_DHCP) 
				|| mode.equals(ETHERNET_CONN_MODE_MANUAL)
				|| mode.equals(ETHERNET_CONN_MODE_PPPOE)) {
            this.mode = mode;
            return true;
        }
        return false;
    }

    public String getConnectMode() {
        return this.mode;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.dev_name);
        dest.writeString(this.ipaddr);
        dest.writeString(this.netmask);
        dest.writeString(this.route);
        dest.writeString(this.dns);
        dest.writeString(this.dns2);
        dest.writeString(this.mode);
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<EthernetDevInfo> CREATOR = new Creator<EthernetDevInfo>() {
        public EthernetDevInfo createFromParcel(Parcel in) {
            EthernetDevInfo info = new EthernetDevInfo();
            info.setIfName(in.readString());
            info.setIpAddress(in.readString());
            info.setNetMask(in.readString());
            info.setRouteAddr(in.readString());
            info.setDnsAddr(in.readString());
            info.setDns2Addr(in.readString());
            info.setConnectMode(in.readString());
            return info;
        }

        public EthernetDevInfo[] newArray(int size) {
            return new EthernetDevInfo[size];
        }
    };
}
