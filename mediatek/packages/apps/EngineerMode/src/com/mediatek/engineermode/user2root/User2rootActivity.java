package com.mediatek.engineermode.user2root;

import java.io.IOException;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;

/**
 * 
 * This is a demo for User To Root.
 * If you use this app, you MUST open system properties write security.
 * 
 * Update for android ics: persist.sys.usb.config
 * 
 * @author mtk71029 Yanghui Li <yanghui.li@mediatek.com>
 *
 */
public class User2rootActivity extends Activity {
    /** Called when the activity is first created. */

    private static final String TAG = "EM/USER2ROOT";

    private Button mRootButton;
    private Button mUserButton;
    private LinearLayout mDetails;

    private static final String READ_PROP_FAILED = "Read system property \"%s\" failed.";

    private static final String ANDROID_BUILD_VERSION = "ro.build.version.sdk";
    private static final int ANDROID_BUILD_ICS = 14;

    private static final String RO_ADB_SECURE = "ro.adb.secure";
    private static final String RO_SECURE = "ro.secure";
    private static final String RO_DEBUG = "ro.debuggable";
    private static final String USB_CONFIG = "persist.sys.usb.config";
    private static final String ATCI_USERMODE = "persist.service.atci.usermode";
    private static final String RO_BUILD_TYPE = "ro.build.type";

    private static final String[][] MODIFY_ITEMS = {
            // { item,        root_value,               user_value }
            { USB_CONFIG,     "none",                   null }, 
            { RO_SECURE,      "0",                      "1" },
            { RO_ADB_SECURE,  "0",                      "1" }, 
            { RO_DEBUG,       "1",                      "0" },
            { USB_CONFIG,     "mass_storage,adb,acm",   "mass_storage" },
            { ATCI_USERMODE,  "1",                      "0" }, };

    private void toRoot() {
        for (int i = 0; i < MODIFY_ITEMS.length; i++) {
            SystemProperties.set(MODIFY_ITEMS[i][0], MODIFY_ITEMS[i][1]);
        }
        String type = SystemProperties.get(RO_BUILD_TYPE, "unknown");
        Elog.v(TAG, "build type: " + type);
        if (!type.equals("eng")) {
            try {
                Elog.v(TAG, "user2root start atcid-daemon-u");
                Process proc = Runtime.getRuntime().exec("start atcid-daemon-u");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        displayDetails(1);
        Toast.makeText(User2rootActivity.this, "Update to Root Success", Toast.LENGTH_LONG).show();
    }

    private void toUser() {
        String type = SystemProperties.get(RO_BUILD_TYPE, "unknown");
        if (!type.equals("eng")) {
            try {
                Elog.v(TAG, "user2root stop atcid-daemon-u");
                Process proc = Runtime.getRuntime().exec("stop atcid-daemon-u");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        for (int i = MODIFY_ITEMS.length - 1; i >= 0; i--) {
            if (MODIFY_ITEMS[i][2] != null) {
                SystemProperties.set(MODIFY_ITEMS[i][0], MODIFY_ITEMS[i][2]);
            }
        }
        displayDetails(2);
        Toast.makeText(User2rootActivity.this, "Update to User Success", Toast.LENGTH_LONG).show();
    }

    
    /**
     * @param check 0-Does not check the properties; 1-Check the properties for "root" result; 2-Check the properties for "user" result
     */
    private void displayDetails(int check) {
        if (mDetails.getChildCount() > 1) {
            mDetails.removeViews(1, mDetails.getChildCount() - 1);
        }

        for (String[] item : MODIFY_ITEMS) {
            if (item[1] == null || item[2] == null) {
                continue;
            }

            TextView v = new TextView(this);

            String value = SystemProperties.get(item[0]);
            Boolean result = true;
            if (check != 0) {
                result = value.equals(item[check]);
            }

            if (value.equals("")) {
                v.setText(String.format(READ_PROP_FAILED, item[0]));
                result = false;
            } else {
                v.setText(item[0] + ": " + value);
            }
            if (!result) {
                v.setTextColor(Color.RED);
            }

            mDetails.addView(v);
        }
    }

    private OnClickListener mRootListener = new Button.OnClickListener() {

        @Override
        public void onClick(View v) {
            int sdkVersion = SystemProperties.getInt(ANDROID_BUILD_VERSION, 10);
            if (sdkVersion >= ANDROID_BUILD_ICS) {
                toRoot();
            }
        }
    };

    private OnClickListener mUserListener = new Button.OnClickListener() {

        @Override
        public void onClick(View v) {
            int sdkVersion = SystemProperties.getInt(ANDROID_BUILD_VERSION, 10);
            if (sdkVersion >= ANDROID_BUILD_ICS) {
                toUser();
            }
        }
    };

    protected void findViews() {
        this.mRootButton = (Button) this.findViewById(R.id.root);
        this.mUserButton = (Button) this.findViewById(R.id.user);
        this.mDetails = (LinearLayout) this.findViewById(R.id.details);
    }

    protected void setActionListener() {
        this.mRootButton.setOnClickListener(this.mRootListener);
        this.mUserButton.setOnClickListener(this.mUserListener);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.user2root);
        this.findViews();
        displayDetails(0);
        this.setActionListener();
    }

}
