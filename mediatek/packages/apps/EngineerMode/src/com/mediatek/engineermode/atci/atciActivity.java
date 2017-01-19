package com.mediatek.engineermode.atci;

import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;
import android.provider.Settings;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;

public class atciActivity extends Activity {
    /** Called when the activity is first created. */

    private static final String TAG = "EM/ATCI";

    private Button mEnableButton;

    private static final String ADB_ENABLE = "persist.sys.usb.config";
    private static final String ATCI_USERMODE = "persist.service.atci.usermode";
    private static final String RO_BUILD_TYPE = "ro.build.type";

    private void enable_ATCI(){
        SystemProperties.set(ATCI_USERMODE, "1");        
        //SystemProperties.set(ADB_ENABLE, "mass_storage,adb,acm");
        SystemProperties.set("persist.radio.port_index", "1");
        Settings.Global.putInt(getContentResolver(), Settings.Global.ACM_ENABLED, 0);
        Settings.Global.putInt(getContentResolver(), Settings.Global.ACM_ENABLED, 1);
        
        String type = SystemProperties.get(RO_BUILD_TYPE, "unknown");
        Elog.v(TAG, "build type: " + type);
        if (!type.equals("eng")) {
            try {
                Elog.v(TAG, "start atcid-daemon-u");
                Process proc1 = Runtime.getRuntime().exec("start atcid-daemon-u");
                Process proc2 = Runtime.getRuntime().exec("start atci_service");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        Toast.makeText(atciActivity.this, "Enable ATCI Success", Toast.LENGTH_LONG).show();
    }

    private OnClickListener mEnableListener = new Button.OnClickListener(){

        @Override
        public void onClick(View v) {
            enable_ATCI();
        }
    };

    protected void findViews(){
        this.mEnableButton = (Button) this.findViewById(R.id.ATCI_enable);
    }

    protected void setActionListener() {
        this.mEnableButton.setOnClickListener(this.mEnableListener);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.atci);
        this.findViews();
        this.setActionListener();
    }

}
