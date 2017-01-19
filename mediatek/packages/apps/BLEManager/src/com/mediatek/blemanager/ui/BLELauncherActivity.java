package com.mediatek.blemanager.ui;


import android.app.Activity;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.BluetoothCallback;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;
import com.mediatek.bluetooth.BleProfileServiceManager;

public class BLELauncherActivity extends Activity {

    private static final String TAG = BLEConstants.COMMON_TAG + "[BLELauncherActivity]";

    private ImageButton mImageButton;
    private TextView mToastView;

    private Context mContext;
    private LocalBluetoothLEManager mLocalManager;
    private CachedBluetoothLEDeviceManager mDeviceManager;
    
    private boolean mTurnBTOn;
    
    private boolean mIsShowingScanActivity;
    private boolean mCurrentResumed;
    
    private ProgressDialog mTurnBtDialog;
    
    private ScanAction mScanAction;
    
    private boolean mTurnOnButtonClicked = false;
    
    private boolean mIsEnter3DView = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        mContext = this.getApplicationContext();
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter == null) {
            Toast.makeText(this, "NOT SUPPORT BT", Toast.LENGTH_SHORT).show();
            return;
        }
        if (CachedBluetoothLEDeviceManager.getInstance().getCachedDevicesCopy().size() > 0) {
            startPairedDeviceListActivity();
            finish();
            return;
        }

        Log.d(TAG, "[onCreate] enter!!");
        setContentView(R.layout.ble_launcher_activity);
        mImageButton = (ImageButton)this.findViewById(R.id.bluetooth_switch_button);
        mToastView = (TextView) this.findViewById(R.id.bluetooth_state_text);

        mLocalManager = LocalBluetoothLEManager.getInstance(mContext);
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        mDeviceManager.registerDeviceListChangedListener(mListListener);
        mLocalManager.stopLEScan();
        mLocalManager.registerBluetoothAdapterStateCallback(mAdapterCallback);
        
        mScanAction = new ScanAction(this);
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        Log.d(TAG, "[onDestroy] enter!!");
        if (mLocalManager != null) {
            mLocalManager.unregisterAdaterStateCallback(mAdapterCallback);
        }
        if (mDeviceManager != null) {
            mDeviceManager.unregisterDeviceListChangedListener(mListListener);
        }
        ScanDeviceAlertDialog.dismiss();
        ConnectProgressAlertDialog.dismiss();
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // TODO Auto-generated method stub
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
           if (!mIsEnter3DView) {
                if (mLocalManager != null) {
                    Log.d(TAG, "[onDestroy] call to close LocalBluetoothLEManager");
                    mLocalManager.close();
                }
            }
           this.finish();
           return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "[onStop] enter!!");
    }

    @Override
    protected void onNewIntent(Intent intent) {
        // TODO Auto-generated method stub
        super.onNewIntent(intent);
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO Auto-generated method stub
        super.onOptionsItemSelected(item);
        
        switch (item.getItemId()) {
        case 0:
            if (mLocalManager != null) {
                mLocalManager.setBackgroundMode(true);
            }
            break;
            
        case 1:
            if (mLocalManager != null) {
                mLocalManager.setBackgroundMode(false);
            }
            break;
            
            default:
                break;
        }
        
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub
        super.onPrepareOptionsMenu(menu);
        menu.clear();
        if (mLocalManager != null) {
            if (mLocalManager.getBackgroundMode() == BleProfileServiceManager.RET_ENABLED) {
                menu.add(0, 1, 0, R.string.disable_background_service);
            } else if (mLocalManager.getBackgroundMode() == BleProfileServiceManager.RET_DISABLED){
                menu.add(0, 0, 0, R.string.enable_background_service);
            }
        }
        return true;
    }

    private void startPairedDeviceListActivity() {
        mIsEnter3DView = true;
        Intent intent = new Intent(BLELauncherActivity.this, PairedDeviceListActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        this.startActivity(intent);
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        updateToastTextView();

        mImageButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                /// used to turn bluetooth
                if (mLocalManager.getCurrentState() == BluetoothAdapter.STATE_OFF) {
//                    mLocalManager.turnOnBluetooth();
//                    String str = BLELauncherActivity.this.getResources().
//                          getString(R.string.turning_on_bluetooth);
//                    mTurnBtDialog = ProgressDialog.show(BLELauncherActivity.this, "", str);
//                    mTurnBtDialog.setCancelable(false);
                    TurnOnBTProgressDialog.show(BLELauncherActivity.this);
                    mTurnOnButtonClicked = true;
                } else if (mLocalManager.getCurrentState() == BluetoothAdapter.STATE_ON) {
                    mScanAction.doScanAction(0);
                }
            }
        });
    }
    
    private void updateToastTextView() {
        this.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                int state = mLocalManager.getCurrentState();
                if (state == BluetoothAdapter.STATE_OFF) {
                    mToastView.setText(R.string.tap_to_turn_on_bluetooth);
                } else if (state == BluetoothAdapter.STATE_ON) {
                    mToastView.setText(R.string.tap_to_add_device);
                }
            }
            
        });
        
    }
    
    private CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener mListListener =
            new CachedBluetoothLEDeviceManager.CachedDeviceListChangedListener() {
                
                @Override
                public void onDeviceRemoved(CachedBluetoothLEDevice device) {
                    // TODO Auto-generated method stub
                    
                }

                @Override
                public void onDeviceAdded(CachedBluetoothLEDevice device) {
                    // TODO Auto-generated method stub
                    BLELauncherActivity.this.finish();
                    startPairedDeviceListActivity();
                }
            };

    private BluetoothCallback.BluetoothAdapterState mAdapterCallback =
        new BluetoothCallback.BluetoothAdapterState() {
        
        @Override
        public void onBluetoothStateChanged(int state) {
            // TODO Auto-generated method stub
            if (state == BluetoothAdapter.STATE_ON) {
//                if (mTurnBtDialog != null) {
//                    mTurnBtDialog.setCancelable(true);
//                }
                TurnOnBTProgressDialog.dismiss();
                if (mTurnOnButtonClicked) {
                    Log.d(TAG, "[onBluetoothStateChanged] start scan action");
                    if (mScanAction != null) {
                        mScanAction.doScanAction(0);
                    }
                }
            } else if (state == BluetoothAdapter.STATE_OFF) {
                mTurnOnButtonClicked = false;
            }
            updateToastTextView();
        }
        
        @Override
        public void onBluetoothScanningStateChanged(boolean started) {
            // TODO Auto-generated method stub
            
        }
    };
}
