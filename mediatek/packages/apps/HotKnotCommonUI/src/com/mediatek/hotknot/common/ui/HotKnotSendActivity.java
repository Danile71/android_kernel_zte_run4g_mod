package com.mediatek.hotknot.common.ui;

import java.util.List;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningTaskInfo;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ComponentName;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import com.mediatek.hotknot.HotKnotAdapter;
import com.mediatek.hotknot.HotKnotMessage;

public class HotKnotSendActivity extends Activity implements HotKnotAdapter.OnHotKnotCompleteCallback, Handler.Callback {
    
    public static final String ACTION_SHARE = "com.mediatek.hotknot.action.SHARE";
    public static final String EXTRA_SHARE_MSG = "com.mediatek.hotknot.extra.SHARE_MSG";
    public static final String EXTRA_SHARE_URIS = "com.mediatek.hotknot.extra.SHARE_URIS";	

    private static final int MSG_HOTKNOT_SEND_COMPLETE = 0;
    private static HotKnotAdapter sAdapter;
    private HotKnotMessage mMessage;
	private Uri[] mUris;
    private Handler mHandler;
        
    private boolean createHotKnotAdapter(Context context) {
        if (sAdapter == null) {
            sAdapter = HotKnotAdapter.getDefaultAdapter(context);
        }
        return sAdapter != null;
    }
    
    private boolean retrieveShareDataFromIntent(Intent intent) {
        try {
            mMessage = (HotKnotMessage) intent.getParcelableExtra(EXTRA_SHARE_MSG);
            Parcelable parcels[] = intent.getParcelableArrayExtra(EXTRA_SHARE_URIS);
            mUris = null;
            if (parcels != null && parcels.length > 0) {
                mUris = new Uri[parcels.length];
                for (int i = 0; i < parcels.length; i++) {
                    mUris[i] = (Uri)parcels[i];
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return (mMessage != null || mUris != null);
    }
    
    private void closeCurrentActivity() {
        finish();
    }
    
    private void registerShareData() {
        if (mMessage != null) {
            sAdapter.setHotKnotMessage(mMessage, this);
        }
        if (mUris != null) {
            sAdapter.setHotKnotBeamUris(mUris, this);
        }
        sAdapter.setOnHotKnotCompleteCallback(this, this);
    }
    
    private void onConfirmEnableHotKnot(boolean enable) {
    	if (enable) {
        	sAdapter.enable();
    	} else {
    		finish();
    	}
    }
    
    private void onHotKnotEnabled(boolean enabled) {
    	if (enabled) {
    		registerShareData();
    	}
    }
    
    private class EnableDialog implements DialogInterface.OnClickListener, DialogInterface.OnDismissListener {
    	private boolean mIsAllow;
    	private AlertDialog mDialog;
    	
    	EnableDialog(Context context) {
    		mDialog = new AlertDialog.Builder(context, AlertDialog.THEME_HOLO_DARK).setTitle(R.string.hotknot)
                        .setIcon(R.drawable.ic_settings_hotknot).setMessage(R.string.use_hotknot)
    			.setPositiveButton(R.string.allow, this).setNegativeButton(R.string.deny, this)
                        .setOnDismissListener(this).create();
    	}
    	
    	void show() {
    		mDialog.show();
    	}
    	
    	@Override
    	public void onDismiss(DialogInterface dialog) {
    		if (!mIsAllow) {
    			finish();
    		}
    	}

		@Override
		public void onClick(DialogInterface dialog, int which) {
			if (which == DialogInterface.BUTTON_POSITIVE) {
				mIsAllow = true;
                onConfirmEnableHotKnot(true);
            } else {
                onConfirmEnableHotKnot(false);
            }
		}
    }
    
    private class RetryDialog implements DialogInterface.OnClickListener, DialogInterface.OnDismissListener {
    	private AlertDialog mDialog;
    	private boolean mShouldRetry;
    	
    	RetryDialog(Context context) {
    		mDialog = new AlertDialog.Builder(context, AlertDialog.THEME_HOLO_DARK).setTitle(R.string.fail_to_share)
                        .setIcon(R.drawable.ic_settings_hotknot).setMessage(R.string.fail_to_share_conn_error)
    			.setPositiveButton(R.string.try_again, this).setNegativeButton(android.R.string.cancel, this).setOnDismissListener(this).create();
    	}
    	
    	void show() {
    		mDialog.show();
    	}
    	
    	@Override
    	public void onDismiss(DialogInterface dialog) {
    		if (!mShouldRetry) {
    			finish();
    		}
    	}

		@Override
		public void onClick(DialogInterface dialog, int which) {
			if (which == DialogInterface.BUTTON_POSITIVE) {
				mShouldRetry = true;
            } else {
				finish();
            }
		}
    }
        
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(HotKnotAdapter.ACTION_ADAPTER_STATE_CHANGED)) {
				int state = intent.getIntExtra(HotKnotAdapter.EXTRA_ADAPTER_STATE, HotKnotAdapter.STATE_DISABLED);
				onHotKnotEnabled(state == HotKnotAdapter.STATE_ENABLED);
			} else if (action.equals(Intent.ACTION_USER_PRESENT)) {
				Log.d(HotKnotCommonUi.TAG, "resume from keyguard");
				finish();
			}
		}
    	
    };

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        
		mHandler = new Handler(this);
        
       	setContentView(R.layout.activity_hotknotsend);

       	IntentFilter filter = new IntentFilter();
       	filter.addAction(HotKnotAdapter.ACTION_ADAPTER_STATE_CHANGED);
       	filter.addAction(Intent.ACTION_USER_PRESENT);
       	registerReceiver(mReceiver, filter);

        if (!createHotKnotAdapter(this)) {
        	closeCurrentActivity();
        }
        
        if (!retrieveShareDataFromIntent(getIntent())) {
        	closeCurrentActivity();
        } 
    }
    
    @Override
    public void onNewIntent(Intent intent) {
        if (!retrieveShareDataFromIntent(intent)) {
            closeCurrentActivity();
        }
    }
    	
    @Override
	public void onResume() {
		super.onResume();
        if (sAdapter.isEnabled()) {
            registerShareData();
        } else {
        	new EnableDialog(this).show();
        }
	}
    
    @Override
    public void onStop() {
    	super.onStop();
        Log.d(HotKnotCommonUi.TAG, "onStop");
        if (isApplicationSentToBackground(this)) {
            Log.d(HotKnotCommonUi.TAG, "activity is sent to back");
            finish();
        }
    }
    
    @Override
    public void onDestroy() {
    	super.onDestroy();
    	unregisterReceiver(mReceiver);
    }
        
    @Override
    public void onHotKnotComplete(int reason) {
        Log.d(HotKnotCommonUi.TAG, "onHotKnotComplete, reason = " + reason);
        mHandler.sendMessage(mHandler.obtainMessage(MSG_HOTKNOT_SEND_COMPLETE, reason, 0));
    }
    
    @Override
    public boolean handleMessage(Message msg) {
        if (msg.what == MSG_HOTKNOT_SEND_COMPLETE) {
        	int sendResult = msg.arg1;
        	if (sendResult == HotKnotAdapter.ERROR_SUCCESS) {
        		finish();
        	} else {
        		new RetryDialog(this).show();
        	}
        } else {
            return false;
        }
        return true;
    }
    
    public boolean isApplicationSentToBackground(final Context context) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<RunningTaskInfo> tasks = am.getRunningTasks(1);
        if (!tasks.isEmpty()) {
            ComponentName topActivity = tasks.get(0).topActivity;
            if (!topActivity.getPackageName().equals(context.getPackageName())) {
                return true;
            }
        }
        return false;
    }

}
