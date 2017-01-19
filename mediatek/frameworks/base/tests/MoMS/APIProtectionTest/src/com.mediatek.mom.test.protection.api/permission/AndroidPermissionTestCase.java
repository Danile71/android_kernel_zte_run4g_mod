/*
 * Copyright (C) 2008 The Android Open Source Project
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
 */

package com.mediatek.mom.test.protection.api.permission;

import java.lang.reflect.Method;
import java.net.URI;

import org.apache.http.HttpHost;
import org.apache.http.HttpRequest;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.hardware.Camera;
import android.location.LocationManager;
import android.media.MediaRecorder;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.net.http.AndroidHttpClient;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ServiceManager;
import android.provider.CallLog;
import android.provider.ContactsContract;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;
import android.test.AndroidTestCase;

public class AndroidPermissionTestCase extends AndroidTestCase {
    private static final Uri URI_SMS_INBOX = Uri.parse("content://sms/inbox");
    private static final Uri URI_MMS_INBOX = Uri.parse("content://mms/inbox");
    private static final Uri URI_CALLLOG = Uri.parse("content://call_log/calls");
    private static final Uri URI_CONTACTS = ContactsContract.CommonDataKinds.Phone.CONTENT_URI;
    private static final String OUTPUT_RECORD_MIC = "/sdcard/api_test.3gpp";
    private static final String OUTPUT_RECORD_PHONE = "/sdcard/cta_test.amr";
    private static final String PHONE_NUMBER = "012345678";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * The following APIs monitored by MoMS
     * should be also protected by android permission checking
     */
    public void testSendSMS() throws Exception {
        try {
            //ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService("isms"));
            SmsManager smsManager = SmsManager.getDefault();
            smsManager.sendTextMessage(PHONE_NUMBER, null, "sms message", null, null);
            fail("Send SMS should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testReadSMS() throws Exception {
        try {
            Cursor c = mContext.getContentResolver().query(URI_SMS_INBOX, null, null, null, null);
            fail("Read SMS should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testSendMMS() throws Exception {
        try {
            AndroidHttpClient localAndroidHttpClient = AndroidHttpClient.newInstance("MMS 1.0");
            URI localURI = new URI("http://www.baidu.com");
            HttpHost localHttpHost = new HttpHost(localURI.getHost(), localURI.getPort(), "http");
            HttpPost localHttpPost = new HttpPost("http://www.baidu.com");
            byte[] arrayOfByte = new byte[2];
            arrayOfByte[0] = -116;
            arrayOfByte[1] = -128;
            ByteArrayEntity localByteArrayEntity = new ByteArrayEntity(arrayOfByte);
            localByteArrayEntity.setContentType("application/vnd.wap.mms-message");
            localHttpPost.setEntity(localByteArrayEntity);
            localAndroidHttpClient.execute(localHttpHost, localHttpPost);

            fail("Send MMS should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testReadMMS() throws Exception {
        try {
            Cursor c = mContext.getContentResolver().query(URI_MMS_INBOX, null, null, null, null);
            fail("Read MMS should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testReadContacts() throws Exception {
        try {
            Cursor c = mContext.getContentResolver().query(URI_CONTACTS, null, null, null, null);
            fail("Read contacts should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testReadCalllog() throws Exception {
        try {
            String columns[]=new String[] {
                    CallLog.Calls._ID,
                    CallLog.Calls.NUMBER, 
                    CallLog.Calls.DATE, 
                    CallLog.Calls.DURATION, 
                    CallLog.Calls.TYPE};
            Cursor c = mContext.getContentResolver().query(URI_CALLLOG, columns, null, null, "Calls._ID DESC");
            fail("Read Calllog should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testDeleteSMS() {
        try {
            mContext.getContentResolver().delete(URI_SMS_INBOX, null, null);
            fail("Write SMS should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testDeleteMMS() {
        try {
            mContext.getContentResolver().delete(URI_MMS_INBOX, null, null);
            fail("Write MMS should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testDeleteContacts() {
        try {
            mContext.getContentResolver().delete(URI_CONTACTS, null, null);
            fail("Write contacts should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testDeleteCallLog() {
        try {
            mContext.getContentResolver().delete(URI_CALLLOG, null, null);
            fail("Write calllog should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testReadIMEI() throws Exception {
        try {
            TelephonyManager localTelephonyManager = (TelephonyManager)mContext.getSystemService("phone");
            localTelephonyManager.getDeviceId();
            fail("Read IMEI should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testOpenCamera() throws Exception {
        try {
            Camera.open();
            fail("Open camera should be protected by android permission checking!");
        } catch (RuntimeException e) {
            // Expected a RuntimeException throw by Camera.
        }
    }

    public void testNetworkOn() throws Exception {
        try {
            ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            cm.setMobileDataEnabled(true);
            fail("Turn on network should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testBluetoothOn() throws Exception {
        try {
            BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
            btAdapter.enable();
            fail("Turn on bluetooth should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testWifiOn() throws Exception {
        try {
            WifiManager localWifiManager = (WifiManager)mContext.getSystemService("wifi");
            localWifiManager.setWifiEnabled(true);
            fail("Turn on wifi should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testGetLocation() throws Exception {
        try {
            LocationManager lm = ((LocationManager)mContext.getSystemService("location"));
            lm.getLastLocation();
            fail("Get location should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }

    public void testRecordMic() throws Exception {
        try {
            MediaRecorder r = new MediaRecorder();
            r.setAudioSource(MediaRecorder.AudioSource.MIC);
            r.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
            r.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
            r.setOutputFile(OUTPUT_RECORD_MIC);
            r.prepare();
            r.start();
            fail("Record microphone should be protected by android permission checking!");
        } catch (RuntimeException e) {
            // Expected a RuntimeException throw by MediaRecorder.
        }
    }

    public void testRecordPhone() throws Exception {
        try {
            MediaRecorder r = new MediaRecorder();
            r.setAudioSource(MediaRecorder.AudioSource.DEFAULT);
            r.setOutputFormat(MediaRecorder.OutputFormat.DEFAULT);
            r.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
            r.setOutputFile(OUTPUT_RECORD_PHONE);
            r.prepare();
            r.start();
            fail("Record phone call should be protected by android permission checking!");
        } catch (RuntimeException e) {
            // Expected a RuntimeException throw by MediaRecorder.
        }
    }

    public void testMakeCall() throws Exception {
        try {
            String posted_by = PHONE_NUMBER;
            String uri = "tel:" + posted_by.trim() ;
            Intent intent = new Intent(Intent.ACTION_CALL);
            intent.setData(Uri.parse(uri));
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mContext.startActivity(intent);
            fail("Make phone call should be protected by android permission checking!");
        } catch (SecurityException e) {
            // Expected
        }
    }
}
