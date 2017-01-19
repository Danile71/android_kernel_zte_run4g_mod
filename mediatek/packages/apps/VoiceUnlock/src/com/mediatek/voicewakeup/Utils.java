package com.mediatek.voicewakeup;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.provider.Settings;
import android.widget.Toast;

import java.util.HashMap;

import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voiceunlock.SettingsPreferenceFragment;
import com.mediatek.voiceunlock.VoiceUnlockPreference;
import com.mediatek.voicewakeup.VowNoSpeaker.VowNoSpeakerFragment;
import com.mediatek.voiceunlock.R;
import com.mediatek.xlog.Xlog;

public class Utils {

    static final String KEY_COMMAND_ID = "command_id";
    static final String KEY_COMMAND_SUMMARY = "command_summary";
    static final String KEY_COMMAND_TITLE = "command_title";
    // launched app component string
    static final String KEY_COMMAND_VALUE = "command_value";
    // 0: record, 1: modify
    static final String KEY_COMMAND_TYPE = "command_type";
    static final int COMMAND_TYPE_RECORD = 0;
    static final int COMMAND_TYPE_MODIFY = 2;
    // 0: voice unlock, 1: no speaker id, 2: with speak id
    static final String KEY_COMMAND_MODE = "command_mode";
    static final int VOW_NO_SPEAKER_MODE = 1;
    static final int VOW_WITH_SPEAKER_MODE = 2;

    public static final String VOICE_WAKEUP_MODE = Settings.System.VOICE_WAKEUP_MODE;
    public static final int VOICE_WAKEUP_ANYONE = 1;
    public static final int VOICE_WAKEUP_COMMAND = 2;
    
    private static final String TAG = "VowUtils";
    private Context mContext;
    private String mPkgName;
    private static final Object sLock = new Object();
    private static Utils sInstance;

    final HashMap<Integer, VowCommandInfo> mKeyCommandInfoMap = new HashMap<Integer, VowCommandInfo>();
    private VoiceServiceListener mVoiceServiceListener;

    public interface VoiceServiceListener {
        public void onVoiceServiceConnect();

        public void handleVoiceCommandNotified(int mainAction, int subAction, Bundle extraData);
    }

    IVoiceCommandManagerService mVCmdMgrService;
    private boolean mIsAttachedService = false;

    private IVoiceCommandListener mVoiceCallback = new IVoiceCommandListener.Stub() {
        public void onVoiceCommandNotified(int mainAction, int subAction, Bundle extraData)
                throws RemoteException {
            Message.obtain(mVoiceCommandHandler, mainAction, subAction, 0, extraData)
                    .sendToTarget();
        }
    };

    private Handler mVoiceCommandHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (mVoiceServiceListener != null) {
                mVoiceServiceListener.handleVoiceCommandNotified(msg.what, msg.arg1,
                        (Bundle) msg.obj);
            }
        }
    };

   public static Utils getInstance() {
        synchronized (sLock) {
            if (sInstance == null) {
                sInstance = new Utils();
            }
            return sInstance;
        }
    }

    void setContext(Context context) {
        mContext = context;
        Xlog.d(TAG, "mContext = " + mContext);
        if (mContext != null) {
            mPkgName = mContext.getPackageName();
        }
    }

    void setOnChangedListener(VoiceServiceListener l) {
        mVoiceServiceListener = l;
    }

    void onResume() {
        Xlog.d(TAG, "register to service");
        if (mVCmdMgrService == null) {
            bindVoiceService(mContext);
        } else {
            registerVoiceCommand(mPkgName);
        }
    }

    void onPause() {
        if (mVCmdMgrService != null) {
            Xlog.d(TAG, "sendCommand TRAINING_STOP");
            sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                    VoiceCommandListener.ACTION_VOICE_TRAINING_STOP, null);

            Xlog.d(TAG, "unregister to service");
            unregisterVoicecommand(mPkgName);
        }
    }

    boolean isLastResetCommand() {
        int count = 0;
        for (int i = 0; i < mKeyCommandInfoMap.size(); i++) {
            Xlog.d(TAG, "updateCommandStatus key: " + i);
            if (Settings.System.getVoiceCommandValue(mContext.getContentResolver(),
                    Settings.System.BASE_VOICE_WAKEUP_COMMAND_KEY, mKeyCommandInfoMap.get(i).mId) != null) {
                count++;
            }
        }
        return count == 1;
    }

    String getAppLabel(ComponentName componentName) {
        ActivityInfo info;
        try {
            info = mContext.getPackageManager().getActivityInfo(componentName,
                    PackageManager.GET_SHARED_LIBRARY_FILES);
        } catch (NameNotFoundException e) {
            return null;
        }

        CharSequence name = info.loadLabel(mContext.getPackageManager());
        return name.toString();
    }

    void sendVoiceCommand(String pkgName, int mainAction, int subAction, Bundle extraData) {
        if (mIsAttachedService) {
            try {
                int errorid = mVCmdMgrService
                        .sendCommand(pkgName, mainAction, subAction, extraData);
                Xlog.e(TAG, "send voice Command errorid: " + errorid);
            } catch (RemoteException e) {
                mIsAttachedService = false;
                mVCmdMgrService = null;
                Xlog.e(TAG, "send voice Command RemoteException =  " + e.getMessage());
            }
        } else {
            Xlog.d(TAG, "didn't register , can not send voice Command  ");
        }
    }

    void playCommand(SettingsPreferenceFragment fragment, int commandId, String commandSummary,
            int vowMode) {
        Xlog.d(TAG, "playCommand commandId = " + commandId + " summary = " + commandSummary);
        Intent intent = new Intent("com.mediatek.voicecommand.VOICE_UNLOCK_PSWPREVIEW");
        intent.putExtra(KEY_COMMAND_SUMMARY, commandSummary);
        intent.putExtra(KEY_COMMAND_ID, commandId);
        intent.putExtra(KEY_COMMAND_MODE, vowMode);
        try {
            fragment.startActivity(intent);
        } catch (android.content.ActivityNotFoundException e) {
            Toast.makeText(mContext, R.string.pass_word_file_missing, Toast.LENGTH_SHORT).show();
        }
    }

    void updateCommand(SettingsPreferenceFragment fragment, int commandId, String commandValue,
            int commandType, int vowMode, String[] commandKeywordArray) {
        Xlog.d(TAG, "recordOrModifyCommand commandId = " + commandId + " commandValue = "
                + commandValue + " commandType = " + commandType + " vowMode = " + vowMode);
        Xlog.d(TAG, "recordOrModifyCommand commandKeyword = " + getKeywords(commandKeywordArray, ","));
        Intent intent = new Intent();
        intent.setClass(mContext, VowCommandRecord.class);
        intent.putExtra(KEY_COMMAND_ID, commandId);
        intent.putExtra(KEY_COMMAND_VALUE, commandValue);
        intent.putExtra(KEY_COMMAND_TYPE, commandType); // modify or
        // record
        intent.putExtra(KEY_COMMAND_MODE, vowMode); // with or no speaker id
        // mode
        if (commandKeywordArray != null) {
            intent.putExtra(VowNoSpeakerFragment.KEY_COMMAND_KEYWORD, commandKeywordArray);
        }
        fragment.startActivity(intent);
    }

    private void registerVoiceCommand(String pkgName) {
        if (!mIsAttachedService) {
            try {
                int errorid = mVCmdMgrService.registerListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    mIsAttachedService = true;
                } else {
                    Xlog.e(TAG, "register voiceCommand fail ");
                }
            } catch (RemoteException e) {
                mIsAttachedService = false;
                mVCmdMgrService = null;
                Xlog.e(TAG, "register voiceCommand RemoteException =  " + e.getMessage());
            }
        } else {
            Xlog.d(TAG, "register voiceCommand success ");
        }
        Xlog.d(TAG, "register voiceCommand end ");
    }

    private void unregisterVoicecommand(String pkgName) {
        if (mVCmdMgrService != null) {
            try {
                int errorid = mVCmdMgrService.unregisterListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    mIsAttachedService = false;
                }
            } catch (RemoteException e) {
                Xlog.e(TAG, "unregisteVoiceCmd voiceCommand RemoteException = " + e.getMessage());
                mIsAttachedService = false;
                mVCmdMgrService = null;
            }
            Xlog.d(TAG, "unregisteVoiceCmd end ");
            // ALPS01523837 , as NE happened firstly,CR:ALPS01509601 
            try {
                mContext.unbindService(mVoiceSerConnection);
            } catch (IllegalArgumentException illegalArgExp) {
            	Xlog.e(TAG,"happen exception , maybe as NE leads to service is killed");
            }
            mVCmdMgrService = null;
            mIsAttachedService = false;
        }
    }

    private void bindVoiceService(Context context) {
        Xlog.d(TAG, "bindVoiceService begin  ");
        Intent mVoiceServiceIntent = new Intent();
        mVoiceServiceIntent.setAction(VoiceCommandListener.VOICE_SERVICE_ACTION);
        mVoiceServiceIntent.addCategory(VoiceCommandListener.VOICE_SERVICE_CATEGORY);
        context.bindService(mVoiceServiceIntent, mVoiceSerConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mVoiceSerConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mVCmdMgrService = IVoiceCommandManagerService.Stub.asInterface(service);
            registerVoiceCommand(mPkgName);
            Xlog.d(TAG, "onServiceConnected   ");
            if (mVoiceServiceListener != null) {
                mVoiceServiceListener.onVoiceServiceConnect();
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            Xlog.d(TAG, "onServiceDisconnected   ");
            mIsAttachedService = false;
            mVCmdMgrService = null;
        }
    };

    static class VowCommandInfo {

        int mId;
        String mPreferTitle;
        String mPreferSummary;
        ComponentName mLaunchedApp;

        public VowCommandInfo(int id) {
            mId = id;
        }
    }
    
    /*
     * Query wakeup mode from setting provider
     * * @return
     */
    public  int getWakeupMode(Context context) {
    	return Settings.System.getInt(context.getContentResolver(),
                VOICE_WAKEUP_MODE, 1);
    }
    
    /*
     * convert the array to string for keywords
     */
	public String getKeywords(String[] keywordArray, String split) {
		if (keywordArray == null) {
			return null;
		}
		
		String keywords;
		int len = keywordArray.length;
        Xlog.d(TAG, "len =  " + len);
		if (1 == len) {
			keywords = keywordArray[0];
		} else {
			StringBuffer sb = new StringBuffer();
			for (int i = 0; i < len - 1; i++) {
				sb.append(keywordArray[i]);
				sb.append(split);
			}
			sb.append(keywordArray[len - 1]);
			keywords = sb.toString();
		}
		Xlog.d(TAG,"keywords = " + keywords);
		return keywords;
	}
}
