package com.mediatek.blemanager.ui;

import android.bluetooth.BluetoothGatt;
import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.common.CachedBluetoothLEDevice;
import com.mediatek.blemanager.provider.BLEConstants;


public class ComposeDeviceImage extends RelativeLayout {

    private static final String TAG = BLEConstants.COMMON_TAG + "[ComposeDeviceImage]";
    
    private ImageView mDeviceImage;
    private ImageView mBackGroundImage;
    private ImageView mConnectionStateImage;
    private TextView mDeviceNameTextView;
    
    private Uri mDeviceImageUri;
    private int mDeviceSignal;
    private int mDeviceConnectionState;
    private String mDeviceName;
    private boolean mAlertState;
    
//    private Handler mHandler;
    private Runnable mRunnable;
    
    public ComposeDeviceImage(Context context, AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater.from(context).inflate(R.layout.find_all_compose_device_image, this, true);
    }

    public ComposeDeviceImage(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public ComposeDeviceImage(Context context) {
        super(context);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mDeviceImage = (ImageView)this.findViewById(R.id.compose_device_image);
        mBackGroundImage = (ImageView)this.findViewById(R.id.compose_background_image);
        mConnectionStateImage = (ImageView)this.findViewById(R.id.compose_state_image);
        mDeviceNameTextView = (TextView)this.findViewById(R.id.device_name_text_view);
        
        updateDeviceImage();
        updateDeviceName();
        updateDeviceStateImage();
    }

    private void updateDeviceImage() {
        if (mDeviceImage != null) {
            mDeviceImage.setImageURI(mDeviceImageUri);
            if (mDeviceConnectionState == BluetoothGatt.STATE_DISCONNECTED) {
                mDeviceImage.setAlpha((float)125.0);
            } else if (mDeviceConnectionState == BluetoothGatt.STATE_CONNECTED) {
                mDeviceImage.setAlpha((float)255);
            }
        }
    }
    
    private void updateDeviceName() {
        if (mDeviceNameTextView != null) {
            mDeviceNameTextView.setText(mDeviceName);
        }
    }
    
    private void updateDeviceStateImage() {
        int resId = Integer.MAX_VALUE;
        if (mDeviceConnectionState == BluetoothGatt.STATE_DISCONNECTED) {
            resId = R.drawable.bt_bar_disconnected;
        } else if (mDeviceConnectionState == BluetoothGatt.STATE_CONNECTED) {
            if (mDeviceSignal == CachedBluetoothLEDevice.PXP_DISTANCE_FAR) {
                resId = R.drawable.ic_bt_combine_signal_1;
            } else if (mDeviceSignal == CachedBluetoothLEDevice.PXP_DISTANCE_MIDDLE) {
                resId = R.drawable.ic_bt_combine_signal_2;
            } else if (mDeviceSignal == CachedBluetoothLEDevice.PXP_DISTANCE_NEAR) {
                resId = R.drawable.ic_bt_combine_signal_3;
            } else if (mDeviceSignal == CachedBluetoothLEDevice.PXP_DISTANCE_NO_SIGNAL){
                resId = R.drawable.ic_bt_combine_signal_0;
            } else {
                resId = R.drawable.bt_bar_connected;
            }
        }
        if (mConnectionStateImage != null) {
            mConnectionStateImage.setImageResource(resId);
        }
    }
    
    private Handler mPostHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            int what = msg.what;
            switch (what) {
            case 1:
                int curr = mBackGroundImage.getVisibility();
                if (curr == View.VISIBLE) {
                    Log.d(TAG, "[hanleMessage] set INVISIBLE");
                    mBackGroundImage.setVisibility(View.INVISIBLE);
                } else if (curr == View.INVISIBLE) {
                    Log.d(TAG, "[hanleMessage] set VISIBLE");
                    mBackGroundImage.setVisibility(View.VISIBLE);
                } else {
                    mBackGroundImage.setVisibility(View.INVISIBLE);
                }
                break;
            case 2:
                Log.d(TAG, "[hanleMessage] 2 set INVISIBLE");
                mBackGroundImage.setVisibility(View.INVISIBLE);
                break;
                default:
                    break;
            }
        }
        
    };
    
    private void flashingBg() {

        if (mRunnable == null) {
            mRunnable = new Runnable() {
    
                @Override
                public void run() {
                    // TODO Auto-generated method stub
                    while (mAlertState) {
                        try {
                            Thread.sleep(400);
                            Message msg = mPostHandler.obtainMessage();
                            msg.what = 1;
                            mPostHandler.sendMessage(msg);
                        } catch (InterruptedException ex) {
                            Log.d(TAG, ex.toString());
                            mBackGroundImage.setVisibility(View.INVISIBLE);
                        }
                    }
                    Message msg = mPostHandler.obtainMessage();
                    msg.what = 2;
                    mPostHandler.sendMessage(msg);
                    mRunnable = null;
                }
                
            };
            new Thread(mRunnable).start();
        }
    }
    
    public void setDeviceImage(Uri uri) {
        if (uri == null) {
            Log.d(TAG, "[setDeviceImage] uri is null!!");
            return;
        }
        if (mDeviceImageUri == uri) {
            return;
        }
        mDeviceImageUri = uri;
        updateDeviceImage();
    }

    public void setDeviceName(String name) {
        if (name == null || name.trim().length() == 0) {
            Log.d(TAG, "[setDeviceName] name is null or empty");
            return;
        }
        if (name.equals(mDeviceName)) {
            return;
        }
        mDeviceName = name;
        updateDeviceName();
    }
    
    public void setDeviceConnectionState(int state) {
        if (mDeviceConnectionState == state) {
            return;
        }
        mDeviceConnectionState = state;
        updateDeviceStateImage();
        updateDeviceImage();
    }
    
    public void setDeviceSignal(int distance) {
        if (mDeviceSignal == distance) {
            return;
        }
        mDeviceSignal = distance;
        updateDeviceStateImage();
    }
    
    public void setDeviceAlertState(Handler handler, boolean state) {
        if (handler == null) {
            return;
        }
        if (mAlertState == state) {
            return;
        }
        mAlertState = state;
//        mHandler = handler;
        flashingBg();
    }
}
