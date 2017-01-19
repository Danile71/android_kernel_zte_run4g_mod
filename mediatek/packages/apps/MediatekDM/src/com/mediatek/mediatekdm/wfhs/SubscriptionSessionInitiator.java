package com.mediatek.mediatekdm.wfhs;

import android.util.Log;

import com.mediatek.mediatekdm.mdm.SessionInitiator;

public class SubscriptionSessionInitiator implements SessionInitiator {

    private static final String TAG = "DM/Wifi.SubscriptionSessionInitiator";

    public SubscriptionSessionInitiator(int type) {
        mSessionID = type;
    }

    @Override
    public String getId() {
        switch (mSessionID) {

        case MdmWifi.SessionID.PROVISION_USERNAME_PASSWORD:
            return MdmWifi.SessionType.PROVISION_USERNAME_PASSWORD;

        case MdmWifi.SessionID.PROVISION_CERTIFICATE:
            return MdmWifi.SessionType.PROVISION_CERTIFICATE;

        case MdmWifi.SessionID.PROVISION_DEVICE_CERTIFICATE:
            return MdmWifi.SessionType.PROVISION_DEVICE_CERTIFICATE;

        case MdmWifi.SessionID.REMEDIATION_MACHINE_USERNAME_PASSWORD:
            return MdmWifi.SessionType.REMEDIATION_MACHINE_USERNAME_PASSWORD;

        case MdmWifi.SessionID.REMEDIATION_USER_USERNAME_PASSWORD:
            return MdmWifi.SessionType.REMEDIATION_USER_USERNAME_PASSWORD;

        case MdmWifi.SessionID.REMEDIATION_MACHINE_CERTIFICATE:
            return MdmWifi.SessionType.REMEDIATION_MACHINE_CERTIFICATE;

        case MdmWifi.SessionID.REMEDIATION_USER_CERTIFICATE:
            return MdmWifi.SessionType.REMEDIATION_USER_CERTIFICATE;

        case MdmWifi.SessionID.REMEDIATION_UPDATE_CERTIFICATE:
            return MdmWifi.SessionType.REMEDIATION_UPDATE_CERTIFICATE;

        case MdmWifi.SessionID.REMEDIATION_UPDATE_CERTIFICATE_UNPW:
            return MdmWifi.SessionType.REMEDIATION_UPDATE_CERTIFICATE_USERNAME_PASSWORD;

        case MdmWifi.SessionID.POLICY_UPDATE_USERNAME_PASSWORD:
            return MdmWifi.SessionType.POLICY_UPDATE_USERNAME_PASSWORD;

        case MdmWifi.SessionID.POLICY_UPDATE_CERTIFICATE:
            return MdmWifi.SessionType.POLICY_UPDATE_SERTIFICATE;

        default:
            Log.e(TAG, "error session id : " + mSessionID);
            return "";
        }
    }

    private int mSessionID;
}
