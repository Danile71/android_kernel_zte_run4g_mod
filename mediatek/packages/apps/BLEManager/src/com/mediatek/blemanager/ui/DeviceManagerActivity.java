package com.mediatek.blemanager.ui;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.common.CachedBluetoothLEDeviceManager;
import com.mediatek.blemanager.common.LocalBluetoothLEManager;
import com.mediatek.blemanager.provider.BLEConstants;

public class DeviceManagerActivity extends PreferenceActivity {

    private static final String TAG = BLEConstants.COMMON_TAG + "[DeviceManagerActivity]";
    
    private static final String INCOMING_CALL_PREFERENCE = "incoming_call_notification_preference";
    private static final String MISSED_CALL_PREFERENCE = "missed_call_notification_preference";
    private static final String NEW_MESSAGE_PREFERENCE = "new_message_notification_preference";
    private static final String NEW_EMAIL_PREFERENCE = "new_email_notification_preference";
    private static final String CATEGORY_PREFERENCE_KEY = "alert_notification_preference_category";
    
    private static final int MENU_DISCONNECT_FLAG = 0;
    private static final int MENU_CONNECT_FLAG = 1;
    private static final int MENU_DELETE_FLAG = 2;
    
    private static final int GET_PIC_FROM_DEFAULT = 0;
    private static final int CAPTURE_PIC_FROM_CAMERA = 1;
    private static final int GET_PIC_FROM_GALLERY = 2;
    private static final int CROP_IMAGE = 10;
    
    private static final String EXTRA = "current_device";
    
    private static final int CONNECT_ACTION = 100;
    private static final int DISCONNECT_ACTION = 101;
    private static final int CONNECT_TIMEOUT = 102;
    private static final int DIALOG_SHOW = 103;
    private static final int DIALOG_DISMISS = 104;
    private static final int SHOW_TOAST = 105;
    private static final int DELETE_DEVICE = 106;
    
    private static final int CONNECT_ACTION_TIMEOUT = 30 * 1000; // 30s
    
    private boolean mAnpServiceConnected;
    
    private PreferenceCategory mCategory;
    private CheckBoxPreference mIncomingCallPreference;
    private CheckBoxPreference mMissedCallPreference;
    private CheckBoxPreference mNewMsgPreference;
    private CheckBoxPreference mNewEmailPreference;
    
    private ImageView mRenameImageView;
    private ImageView mDeviceImageView;
    private TextView mDeviceNameText;
    
    private AlertDialog mRenameDialog;

    private String mDeviceName;
    private Uri mDeviceImageUri;
    
    private CachedBluetoothLEDeviceManager mDeviceManager;
    private CachedBluetoothLEDevice mCachedDevice;
    private LocalBluetoothLEManager mManager;
    
    private ConnectAction mConnectAction;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        Intent intent = this.getIntent();
        int current = intent.getIntExtra(EXTRA, -1);
        if (current == -1) {
            finish();
            return;
        }
        Log.d(TAG, "[onCreate] enter");
        mDeviceManager = CachedBluetoothLEDeviceManager.getInstance();
        mCachedDevice = mDeviceManager.getCachedDeviceFromDisOrder(current);
        if (mCachedDevice == null) {
            finish();
            Log.d(TAG, "[onCreate] enter, device is null");
            return;
        }
        
        // Add preference to activity and set content view for it.
        this.addPreferencesFromResource(R.xml.device_manager_activity_preference);
        this.setContentView(R.layout.device_manager_activity_layout);

        mDeviceImageView = (ImageView)this.findViewById(R.id.dm_top_device_pic);
        mRenameImageView = (ImageView)this.findViewById(R.id.dm_device_rename_image);
        mDeviceNameText = (TextView)this.findViewById(R.id.dm_device_name_text_view);

        mCategory = (PreferenceCategory)this.findPreference(CATEGORY_PREFERENCE_KEY);
        mIncomingCallPreference = (CheckBoxPreference)this.findPreference(INCOMING_CALL_PREFERENCE);
        mMissedCallPreference = (CheckBoxPreference)this.findPreference(MISSED_CALL_PREFERENCE);
        mNewMsgPreference = (CheckBoxPreference)this.findPreference(NEW_MESSAGE_PREFERENCE);
        mNewEmailPreference = (CheckBoxPreference)this.findPreference(NEW_EMAIL_PREFERENCE);
        
        mManager = LocalBluetoothLEManager.getInstance(this.getApplicationContext());
        mConnectAction = new ConnectAction(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mCachedDevice.unregisterAttributeChangeListener(mAttributeListener);
        mManager.unregisterServiceConnectionListener(mServiceConnectionListener);
    }

    @Override
    protected void onStart() {
        super.onStart();
        
        this.mAnpServiceConnected = mManager.getServiceConnectionState(LocalBluetoothLEManager.PROFILE_ANP_ID);
        Log.d(TAG, "[onStart] mAnpServiceConnected : " + mAnpServiceConnected);
        
        initActivity();
        initActionBar();
        mCachedDevice.registerAttributeChangeListener(mAttributeListener);
        mManager.registerServiceConnectionListener(mServiceConnectionListener);
    }

    private void initActionBar() {
        ActionBar bar = this.getActionBar();
//        Drawable d = ActivityUtils.getDrawableIcon(mCachedDevice.getDeviceTagId());
//        bar.setIcon(d);
        bar.setTitle(mCachedDevice.getDeviceName());
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub
        super.onPrepareOptionsMenu(menu);
        menu.clear();
        if (mManager.getCurrentState() == BluetoothAdapter.STATE_ON) {
            if (mCachedDevice != null) {
                if (mCachedDevice.getConnectionState() == BluetoothGatt.STATE_CONNECTED) {
                    menu.add(0, MENU_DISCONNECT_FLAG, 0, R.string.disconnect_text);
                }
                if (mCachedDevice.getConnectionState() == BluetoothGatt.STATE_DISCONNECTED) {
                    menu.add(0, MENU_CONNECT_FLAG, 0, R.string.connect_text);
                }
            }
            menu.add(0, MENU_DELETE_FLAG, 0, R.string.delete_text);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        switch (item.getItemId()) {
        case MENU_DISCONNECT_FLAG:
            // do disconnect action
            AlertDialog.Builder builder1 = new AlertDialog.Builder(this);
            builder1.setTitle(R.string.disconnect_text);
            builder1.setMessage(R.string.device_disconnect_dialog_message);
            builder1.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    // TODO do delete action
                    sendMessageToHandler(DISCONNECT_ACTION, 0);
                }
            });
            builder1.setNegativeButton(R.string.no, null);
            builder1.create().show();
            break;

        case MENU_DELETE_FLAG:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.delete_text);
            builder.setMessage(R.string.device_delete_dialog_message);
            builder.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    // TODO do delete action
                    sendMessageToHandler(DELETE_DEVICE, 0);
                }
            });
            builder.setNegativeButton(R.string.no, null);
            builder.create().show();
            break;

        case MENU_CONNECT_FLAG:
            final BluetoothDevice device = mCachedDevice.getDevice();
            if (device == null) {
                break;
            }
            mConnectAction.connect(device, mCachedDevice.getDeviceLocationIndex(), true);
//            sendMessageToHandler(CONNECT_ACTION, 0);
            break;

        case android.R.id.home:
            this.finish();
            break;

            default:
                break;
        }
        return true;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
//        Uri deviceUri = null;
        Bitmap bitmap = null;
        if (resultCode == RESULT_OK) {
            switch(requestCode) {
            case GET_PIC_FROM_DEFAULT:
                int id = data.getIntExtra("which", -1);
//                deviceUri = ActivityUtils.getDrawableUri(this, id,
//                        mCachedDevice.getDeviceImage().toString());
                bitmap = ActivityUtils.getDrawbleBitmap(this, id);
                break;

            case CAPTURE_PIC_FROM_CAMERA:
                if (data == null) {
                    ActivityUtils.handlePhotoCrop(this, CROP_IMAGE, ActivityUtils.getTempFileUri());
                }
                break;

            case GET_PIC_FROM_GALLERY:
                Uri uri = data.getData();
                ActivityUtils.handlePhotoCrop(this, CROP_IMAGE, uri);
                break;

            case CROP_IMAGE:
                Log.d(TAG, "[onActivityResult] crop back");
                Uri cropUri = data.getData();
                Log.d(TAG, "[onActivityResult] cropUri : " + cropUri);
                bitmap = ActivityUtils.saveImageFromCustom(this, cropUri,
                        mCachedDevice.getDeviceImage().toString());
                break;

            default:
                break;
            }
//            Log.d(TAG, "[onActivityResult] deviceUri : " + deviceUri);
            if (bitmap != null) {
                mCachedDevice.updateDeviceImage(ActivityUtils.comproseBitmapToByteArray(bitmap));
                updateUi();
            } else {
                Log.d(TAG, "[onActivityResult] bitmap is null");
            }
        }
    }

    private void initActivity() {
        updatePreferenceState();
        mDeviceNameText.setText(mCachedDevice.getDeviceName());
        mRenameImageView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showRenameDialog();
            }
        });
        
        mDeviceImageView.setImageURI(mCachedDevice.getDeviceImage());
        mDeviceImageView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                AlertDialog.Builder builder = new AlertDialog.Builder(DeviceManagerActivity.this);
                builder.setItems(R.array.device_image_chooser_items,
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        ActivityUtils.startImageChooser(DeviceManagerActivity.this, which);
                    }
                });
                builder.create().show();
            }
        });
    }
    
    private void updatePreferenceState() {
        this.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mIncomingCallPreference.setTitle(R.string.incoming_call_notification_text);
                mMissedCallPreference.setTitle(R.string.missed_call_notificaiotn_text);
                mNewMsgPreference.setTitle(R.string.new_message_notification_text);
                mNewEmailPreference.setTitle(R.string.new_email_notification_text);
                
                mIncomingCallPreference.setChecked(
                        mCachedDevice.getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG));
                mMissedCallPreference.setChecked(
                        mCachedDevice.getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG));
                mNewMsgPreference.setChecked(
                        mCachedDevice.getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG));
                mNewEmailPreference.setChecked(
                        mCachedDevice.getBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG));

                if (mAnpServiceConnected) {
	                mIncomingCallPreference.setEnabled(
	                        mCachedDevice.getBooleanAttribute(
	                                CachedBluetoothLEDevice.DEVICE_REMOTE_INCOMING_CALL_FLAGE));
	                mMissedCallPreference.setEnabled(
	                        mCachedDevice.getBooleanAttribute(
	                                CachedBluetoothLEDevice.DEVICE_REMOTE_MISSED_CALL_FLAGE));
	                mNewMsgPreference.setEnabled(
	                        mCachedDevice.getBooleanAttribute(
	                                CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_MESSAGE_FLAGE));
	                mNewEmailPreference.setEnabled(
	                        mCachedDevice.getBooleanAttribute(
	                                CachedBluetoothLEDevice.DEVICE_REMOTE_NEW_EMAIL_FLAGE));
                } else {
                	mIncomingCallPreference.setEnabled(false);
        			mMissedCallPreference.setEnabled(false);
        			mNewMsgPreference.setEnabled(false);
        			mNewEmailPreference.setEnabled(false);
                }
                
                mIncomingCallPreference.setOnPreferenceClickListener(mPreferenceClickListener);
                mMissedCallPreference.setOnPreferenceClickListener(mPreferenceClickListener);
                mNewMsgPreference.setOnPreferenceClickListener(mPreferenceClickListener);
                mNewEmailPreference.setOnPreferenceClickListener(mPreferenceClickListener);
            }
            
        });
        
    }

    /**
     * Show rename dialog which used to rename the device name.
     */
    private void showRenameDialog() {
        
        if (mRenameDialog != null && mRenameDialog.isShowing()) {
            mRenameDialog.dismiss();
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        final EditText editor = new EditText(this);
        final String befText = mDeviceNameText.getText().toString();
        editor.setText(befText);
        editor.setSingleLine(true);
        editor.addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable arg0) {
                if (arg0.toString().trim().length() == 0) {
                    mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setClickable(false);
                    mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
                    return;
                }
                if (arg0.toString().equals(befText)) {
                    mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setClickable(false);
                    mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
                    return;
                }
                mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setClickable(true);
                mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(true);
            }

            @Override
            public void beforeTextChanged(CharSequence arg0, int arg1,
                    int arg2, int arg3) {
                
            }

            @Override
            public void onTextChanged(CharSequence arg0, int arg1, int arg2,
                    int arg3) {
                
            }
            
        });
        builder.setView(editor);
        builder.setTitle(R.string.device_name_change_dialog_title);
        builder.setNegativeButton(R.string.cancel, null);
        builder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                if (editor.getText().toString().trim().length() != 0) {
                    if (!editor.getText().toString().equals(befText)) {
                        mCachedDevice.setDeviceName(editor.getText().toString());
                        updateUi();
                        arg0.dismiss();
                    }
                }
            }
        });
        mRenameDialog = builder.create();
        mRenameDialog.show();
        mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setClickable(false);
        mRenameDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
    }
    
    private void updateUi() {
        this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDeviceImageView.setImageURI(null);
                mDeviceNameText.setText(mCachedDevice.getDeviceName());
                mDeviceImageView.setImageURI(mCachedDevice.getDeviceImage());
                ActionBar bar = DeviceManagerActivity.this.getActionBar();
                bar.setTitle(mCachedDevice.getDeviceName());
//                Drawable d = ActivityUtils.getDrawableIcon(mCachedDevice.getDeviceTagId());
//                bar.setIcon(d);
            }
        });
    }
    
    private void sendMessageToHandler(int what, long delay) {
        Message msg = mHandler.obtainMessage();
        msg.what = what;
        mHandler.sendMessageDelayed(msg, delay);
    }
    
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            int what = msg.what;
            switch (what) {
//            case CONNECT_ACTION:
//                mHandler.removeMessages(CONNECT_TIMEOUT);
//                sendMessageToHandler(DIALOG_SHOW, 0);
//                Log.d(TAG, "[mHandler] CONNECT_ACTION, start to connect gatt device");
//                mManager.connectGattDevice(mCachedDevice.getDevice(),
//                false, mCachedDevice.getDeviceLocationIndex());
//                sendMessageToHandler(CONNECT_TIMEOUT, CONNECT_ACTION_TIMEOUT);
//                break;
//                
            case DISCONNECT_ACTION:
                mHandler.removeMessages(CONNECT_TIMEOUT);
                Log.d(TAG, "[mHandler] DISCONNECT_ACTION, start to disconnect gatt device");
                mManager.disconnectGattDevice(mCachedDevice.getDevice());
                mCachedDevice.setBooleanAttribute(
                        CachedBluetoothLEDevice.DEVICE_AUTO_CONNECT_FLAG, false);
                break;
//                
//            case CONNECT_TIMEOUT:
//                Log.d(TAG, "[handleMessage] timeout and call disconnect");
//                mManager.disconnectGattDevice(mCachedDevice.getDevice());
//                ConnectProgressAlertDialog.dismiss();
//                Toast.makeText(DeviceManagerActivity.this,
//                "device connect failed", Toast.LENGTH_SHORT).show();
//                break;
//                
            case DIALOG_SHOW:
                ConnectProgressAlertDialog.show(DeviceManagerActivity.this,
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        // TODO Auto-generated method stub
                        sendMessageToHandler(DISCONNECT_ACTION, 0);
                        sendMessageToHandler(DIALOG_DISMISS, 0);
                    }
                });
                break;
                
            case DIALOG_DISMISS:
                ConnectProgressAlertDialog.dismiss();
                break;
                
            case SHOW_TOAST:
                String str = (String)msg.obj;
                Toast.makeText(DeviceManagerActivity.this, str, Toast.LENGTH_SHORT).show();
                break;
                
            case DELETE_DEVICE:
                BluetoothDevice device = mCachedDevice.getDevice();
                if (device != null) {
                    Log.d(TAG, "[mHandler] DELETE_DEVICE, start to disconnect gatt device");
                    mManager.disconnectGattDevice(device);
                    device.removeBond();
                    mDeviceManager.removeDevice(mCachedDevice);
                    DeviceManagerActivity.this.finish();
                    mHandler.removeMessages(CONNECT_TIMEOUT);
                }
                break;
                
                default:
                    break;
            }
        }
        
    };
    
    private LocalBluetoothLEManager.ServiceConnectionListener mServiceConnectionListener =
    	new LocalBluetoothLEManager.ServiceConnectionListener() {
	
    	@Override
    	public void onServiceConnectionChange(int profileService, int connection) {
    		if (profileService == LocalBluetoothLEManager.PROFILE_ANP_ID) {
    			if (connection == LocalBluetoothLEManager.PROFILE_CONNECTED) {
    				mAnpServiceConnected = true;
    			} else if (connection == LocalBluetoothLEManager.PROFILE_DISCONNECTED) {
    				mAnpServiceConnected = false;
    			}
    			updatePreferenceState();
    		}
			}
	};
    
    private CachedBluetoothLEDevice.DeviceAttributeChangeListener mAttributeListener =
            new CachedBluetoothLEDevice.DeviceAttributeChangeListener() {

                @Override
                public void onDeviceAttributeChange(
                        CachedBluetoothLEDevice device, int which) {
                    // TODO Auto-generated method stub
                    if (device == null || device.getDevice() == null) {
                        Log.d(TAG, "[mAttributeListener] device is null");
                        return;
                    }
//                    if (which == CachedBluetoothLEDevice.DEVICE_SERVICE_LIST_CHANGE_FLAG) {
                        updatePreferenceState();
//                    }
                }
        
    };
    
    private Preference.OnPreferenceClickListener mPreferenceClickListener =
            new Preference.OnPreferenceClickListener() {
                
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    // TODO Auto-generated method stub
                    if (preference.getKey().equals(INCOMING_CALL_PREFERENCE)) {
                        mCachedDevice.setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_INCOMING_CALL_ENABLER_FLAG,
                                mIncomingCallPreference.isChecked());
                    }
                    if (preference.getKey().equals(MISSED_CALL_PREFERENCE)) {
                        mCachedDevice.setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_MISSED_CALL_ENABLER_FLAG,
                                mMissedCallPreference.isChecked());
                    }
                    if (preference.getKey().equals(NEW_EMAIL_PREFERENCE)) {
                        mCachedDevice.setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_NEW_EMAIL_ENABLER_FLAG,
                                mNewEmailPreference.isChecked());
                    }
                    if (preference.getKey().equals(NEW_MESSAGE_PREFERENCE)) {
                        mCachedDevice.setBooleanAttribute(
                                CachedBluetoothLEDevice.DEVICE_NEW_MESSAGE_ENABLER_FLAG,
                                mNewMsgPreference.isChecked());
                    }
                    return true;
                }
            };

}
