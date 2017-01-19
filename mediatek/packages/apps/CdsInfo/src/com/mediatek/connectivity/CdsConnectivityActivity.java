/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.connectivity;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.net.ConnectivityManager;
import android.net.IConnectivityManager;
import android.provider.Settings;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;
import android.view.View;

import com.mediatek.connectivity.R;


public class CdsConnectivityActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsConnectivityActivity";
    private Context mContext;
    private Toast mToast;
    private static final String[] DEFAULT_CONN_LIST = new String[] {"Wi-Fi", "Mobile"};
    Spinner mConnSpinner = null;
    private int mSelectConnType = ConnectivityManager.TYPE_MOBILE;
    private TextView mDunApnCurrentSetting = null;
    private EditText mReportPercent = null;
    private EditText mConnChangeDelay = null;
    private ConnectivityManager mConnMgr = null;
    private INetworkManagementService nwService = null;
    private IConnectivityManager connManager = null;
    private Button mReportBtnCmd = null;
    private Button mSetBtnCmd = null;
    private Button mEnableUdpBtn = null;
    private Button mDisableUdpBtn = null;
    private Button mSetDunApnBtn = null;
    private CheckBox mCaptiveCheckBox = null;
    private EditText mUdpIpAddr = null;
    private EditText mDunApnSetting = null;
    private static final String UDP_IP_ADDR_PROPERTY_NAME = "net.rndis.client";


    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_connectivity);

        mContext = this.getBaseContext();

        mConnSpinner = (Spinner) findViewById(R.id.connTypeSpinnner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, DEFAULT_CONN_LIST);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mConnSpinner.setAdapter(adapter);
        mConnSpinner.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View arg1,
            int position, long arg3) {
                // TODO Auto-generated method stub
                try {
                    switch(position) {
                    case 0:
                            mSelectConnType = ConnectivityManager.TYPE_WIFI;
                        break;
                    case 1:
                        mSelectConnType = ConnectivityManager.TYPE_MOBILE;
                        break;
                    default:
                        mSelectConnType = ConnectivityManager.TYPE_MOBILE;
                        break;
                    }
                    mConnSpinner.requestFocus();

                } catch(Exception e) {
                    mSelectConnType = ConnectivityManager.TYPE_MOBILE;
                    e.printStackTrace();
                }
            }
            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub

            }
        });

        mReportPercent = (EditText)  findViewById(R.id.ReportPercent);
        mReportPercent.setText("55");
        mConnMgr = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        nwService = INetworkManagementService.Stub.asInterface(b);
        if (null == nwService) {
            Log.e(TAG, "onCreate:Failed to get the NetworkManagementService!");
        }
        b = ServiceManager.getService(CONNECTIVITY_SERVICE);
        connManager = IConnectivityManager.Stub.asInterface(b);
        if (null == connManager) {
            Log.e(TAG, "onCreate:Failed to get the ConnectivityService!");
        }
        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);
        mReportBtnCmd = (Button) findViewById(R.id.Report);
        mReportBtnCmd.setOnClickListener(this);

        mConnChangeDelay = (EditText)  findViewById(R.id.conn_change_delay);


        mSetBtnCmd = (Button) findViewById(R.id.setBtn);
        mSetBtnCmd.setOnClickListener(this);

        mCaptiveCheckBox = (CheckBox) findViewById(R.id.conn_captive_portal);
        mCaptiveCheckBox.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setCaptivePortalCheckEnabled(isChecked);
            }
        });

        mEnableUdpBtn = (Button) findViewById(R.id.tetherEnableBtn);
        mEnableUdpBtn.setOnClickListener(this);
        mUdpIpAddr = (EditText)  findViewById(R.id.tether_udp_test_ipaddr);
        mUdpIpAddr.setText("");

        mDisableUdpBtn= (Button) findViewById(R.id.tetherDisableBtn);
        mDisableUdpBtn.setOnClickListener(this);

        mSetDunApnBtn = (Button) findViewById(R.id.dun_apn_button);
        mSetDunApnBtn.setOnClickListener(this);

        mDunApnCurrentSetting = (TextView) findViewById(R.id.dun_apn_current_setting);
        mDunApnSetting = (EditText)  findViewById(R.id.dun_apn_setting);
        updateCurrentStatus();
        Log.i(TAG, "CdsConnectivityActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateCurrentStatus();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private void updateCurrentStatus() {
        mConnChangeDelay.setText(getConnectivityChangeDelay());
        mCaptiveCheckBox.setChecked(getIsCaptivePortalCheckEnabled());
        mDunApnSetting.setText(getDunApnSetting());

        try {
            String[] tetherInterfaces = connManager.getTetheredIfacePairs();
            if(tetherInterfaces.length != 2){
               Log.d(TAG, "Wrong tethering state:" + tetherInterfaces.length);
            } else {
                String extInterface = tetherInterfaces[0];
                String inInterface = tetherInterfaces[1];
                Log.d(TAG, "[NS-IOT]getUsbClient(" + inInterface);
                if (null == nwService) {
                    Log.e(TAG, "updateCurrentStatus:Failed to get the NetworkManagementService!");
                } else {
                    nwService.getUsbClient(inInterface);
                    String ipAddr = SystemProperties.get(UDP_IP_ADDR_PROPERTY_NAME);
                    if ( ipAddr == null || ipAddr.length() == 0){
                        Log.d(TAG, "[NS-IOT]There is no HostPC address!");
                    } else {
                        mUdpIpAddr.setText(ipAddr);
                    }
                }
           }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "setStartRequest failed!");
        }


    }

    private void setCaptivePortalCheckEnabled(boolean enabled) {
        int value = (enabled) ? 1 : 0;
        Log.d(TAG, "setCaptivePortalCheckEnabled:" + value);
        Settings.Global.putInt(mContext.getContentResolver(),Settings.Global.CAPTIVE_PORTAL_DETECTION_ENABLED, value);
    }

    private String getConnectivityChangeDelay() {
        final ContentResolver cr = mContext.getContentResolver();

        /** Check system properties for the default value then use secure settings value, if any. */
        int defaultDelay = SystemProperties.getInt(
                               "conn." + Settings.Global.CONNECTIVITY_CHANGE_DELAY,
                               ConnectivityManager.CONNECTIVITY_CHANGE_DELAY_DEFAULT);
        int delayValue = Settings.Global.getInt(cr, Settings.Global.CONNECTIVITY_CHANGE_DELAY,
                                                defaultDelay);

        String delayString = String.valueOf(delayValue);

        return delayString;
    }

    private boolean getIsCaptivePortalCheckEnabled() {
        return (Settings.Global.getInt(mContext.getContentResolver(),
                                       Settings.Global.CAPTIVE_PORTAL_DETECTION_ENABLED, 1) == 1);
    }

    private String getDunApnSetting() {
        String dunSetting = Settings.Global.getString(mContext.getContentResolver(),
                            Settings.Global.TETHER_DUN_APN);

        if(dunSetting == null || dunSetting.length() == 0) {
            dunSetting = "Tethering Internet,mobile.vodafone.it,,,,,,,,,222,10,,DUN";
        } else {
            Resources res = mContext.getResources();
            String str = res.getString(R.string.tethering_dun_apn_label);
            mDunApnCurrentSetting.setText(str + dunSetting);
        }

        String configSetting = SystemProperties.get("tether.dun.apn", "");
        if(configSetting.length() > 0) {
            Log.d(TAG, "Use system properoty to configure:" + configSetting);
            dunSetting = configSetting;
        }
        Log.d(TAG, "getDunApnSetting:" + dunSetting);
        return dunSetting;
    }

    public void onClick(View v) {
        int buttonId = v.getId();

        switch(buttonId) {
        case R.id.Report:
            Log.i(TAG, "Report Inet action");
            reportInetAcction();
            break;
        case R.id.setBtn:
            Log.i(TAG, "setting configuration");
            setConnChangeDelay();
            break;
        case R.id.tetherEnableBtn:
            Log.i(TAG, "configure udp testing");
            setUdpTesting(true);
            break;
        case R.id.tetherDisableBtn:
            setUdpTesting(false);
            break;
        case R.id.dun_apn_button:
            setDunApnSetting();
            updateCurrentStatus();
            break;
        default:
            Log.e(TAG, "Error button");
            break;
        }
    }

    private void setDunApnSetting() {
        String apn = mDunApnSetting.getText().toString();
        if(apn != null) {
            Settings.Global.putString(mContext.getContentResolver(),
                                      Settings.Global.TETHER_DUN_APN, apn);
            mToast.setText("Configure OK");
            mToast.show();
        } else {
            mToast.setText("Configure NOK: the seting is null or empty");
            mToast.show();
        }
    }

    private void setUdpTesting(boolean enabled) {
        try {
            if (null == connManager) {
                Log.e(TAG, "setUdpTesting:Failed to get the ConnectivityService!");
                return;
            }
            if (null == nwService) {
                Log.e(TAG, "setUdpTesting:Failed to get the NetworkManagementService!");
                return;
            }

            String[] tetherInterfaces = connManager.getTetheredIfacePairs();
            if(tetherInterfaces.length != 2){
               mToast.setText("Wrong tethering state:" + tetherInterfaces.length);
               mToast.show();
               return;
            } else if (tetherInterfaces[0] == null) {
                Log.e(TAG, "[NS-IOT]Upstream is null");
                mToast.setText("[NS-IOT]Upstream is null" + tetherInterfaces.length/2);
                mToast.show();
                return;                    
            }

            String extInterface = tetherInterfaces[0];
            String inInterface = tetherInterfaces[1];

            Log.d(TAG, "[NS-IOT]getUsbClient(" + inInterface);
            String ipAddr = mUdpIpAddr.getText().toString();
            if ( ipAddr == null || ipAddr.length() == 0){
                nwService.getUsbClient(inInterface);
                ipAddr = SystemProperties.get(UDP_IP_ADDR_PROPERTY_NAME);
                if ( ipAddr == null || ipAddr.length() == 0){
                    mToast.setText("Please input the destination address");
                    mToast.show();
                    return;
                } else {
                    mUdpIpAddr.setText(ipAddr);
                }
            }
            
            nwService.enableUdpForwarding(enabled, inInterface, extInterface, ipAddr);

        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "setStartRequest failed!");
        }

    }

    private void setConnChangeDelay() {
        String tmpString = mConnChangeDelay.getText().toString();
        int delay = 0;
        try {
            delay = Integer.parseInt(tmpString);
            if(delay < 0 || delay > 60 * 1000) {
                mToast.setText("The range of dealy value should be 0 ~ 60 * 1000");
                mToast.show();
                return;
            }
            final ContentResolver cr = mContext.getContentResolver();
            Settings.Global.putInt(cr, Settings.Global.CONNECTIVITY_CHANGE_DELAY,
                                   delay);
            String msg = "The dealy value (" + delay + ") has been configured successfully";
            mToast.setText(msg);
            mToast.show();
        } catch(Exception e) {
            e.printStackTrace();
        }
        mConnChangeDelay.setText(getConnectivityChangeDelay());
    }

    private void reportInetAcction() {

        String percentText = mReportPercent.getText().toString();

        try {
            if(percentText.length() == 0) {
                mToast.setText("The percent value is empty. This is not allowed");
                mToast.show();
                return;
            }
            int percentValue = Integer.parseInt(percentText);
            if(percentValue > 100 || percentValue < 0) {
                mToast.setText("The range fo report percent should be 1 ~ 100");
                mToast.show();
                return;
            }
            Log.i(TAG, "Report nw:" + mSelectConnType + "-" + percentValue);
            mConnMgr.reportInetCondition(mSelectConnType, percentValue);
        } catch(Exception e) {
            mToast.setText("ERROR:" + e.toString());
            e.printStackTrace();
        }
    }

}