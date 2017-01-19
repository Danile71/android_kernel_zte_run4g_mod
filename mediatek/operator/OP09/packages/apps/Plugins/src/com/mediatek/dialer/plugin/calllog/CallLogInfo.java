package com.mediatek.dialer.plugin.calllog;

import android.content.Intent;

import com.mediatek.dialer.calllogex.ContactInfoEx;

public final class CallLogInfo {

    public ContactInfoEx mContactInfo;
    public Intent mCallDetailIntent;

    public CallLogInfo(ContactInfoEx contactInfo, Intent callDetailIntent) {
        mContactInfo = contactInfo;
        mCallDetailIntent = callDetailIntent;
    }
}
