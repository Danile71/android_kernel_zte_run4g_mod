package com.mediatek.settings.plugin;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.provider.Settings;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.RadioButton;

import com.mediatek.op09.plugin.R;
import com.mediatek.xlog.Xlog;

public class RoamingNotify implements View.OnClickListener {

    private static final String TAG = "RoamingNotify";

    private Context mContext;
    private static RoamingNotify sInstance;

    private NotificationManager mNM = null;

    private RadioButton mRadioBtn1 = null;
    private RadioButton mRadioBtn2 = null;
    // Test Current time settings:
    private int mCurrentTime = 0;
    private int mSelectTimeMode = 0;
    private static final int BEIJING_MODE = 0;
    private static final int LOCAL_MODE = 1;

    public RoamingNotify(Context context) {
        mContext = context;
    }

    public static RoamingNotify getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new RoamingNotify(context);
        }
        return sInstance;
    }

    /**
     * Show time zone recommend in notification bar
     */
    public void showTimeZoneRecommendInNotificationBar() {
        String notificationTitle = mContext.getString(R.string.time_zone_recommend_title);
        String notificationSummary = mContext.getString(R.string.time_zone_recommend_notify_summary);
        if (mNM == null) {
            mNM = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        }
        Notification notification = new Notification(R.drawable.ic_notification_time_zone_recommendation, notificationTitle,
                System.currentTimeMillis());
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        /** The PendingIntent to launch our activity if the user selects this notification */
        PendingIntent contentIntent = null;
        Intent backIntent = new Intent();
        backIntent.setComponent(
                new ComponentName("com.mediatek.op09.plugin", "com.mediatek.settings.plugin.TimeZoneActivity"));
        backIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);

        if (mContext.getPackageManager().resolveActivity(backIntent, 0) != null) {
            contentIntent = PendingIntent.getActivity(mContext, 0, backIntent, 0);
        } else {
            Xlog.e(TAG, "Could not find OP09 plugin.TimeZoneActivity.");
        }
        notification.setLatestEventInfo(mContext, notificationTitle, notificationSummary, contentIntent);
        if (mNM != null) {
            mNM.notify(R.drawable.ic_notification_time_zone_recommendation, notification);
        }
    }

    public void showTimeZoneChooseDialog() {
        LayoutInflater inflater = LayoutInflater.from(mContext);
        View view = inflater.inflate(R.layout.time_display_mode, null);
        mRadioBtn1 = (RadioButton) view.findViewById(R.id.beijing_radio);
        mRadioBtn2 = (RadioButton) view.findViewById(R.id.local_radio);
        mRadioBtn1.setOnClickListener(this);
        mRadioBtn2.setOnClickListener(this);
        // init radio button:
        mCurrentTime = Settings.System.getInt(mContext.getContentResolver(), Settings.System.CT_TIME_DISPLAY_MODE,
                BEIJING_MODE);
        Xlog.i(TAG, "get ct init time display mode:" + mCurrentTime);
        if (mCurrentTime == BEIJING_MODE) {
            mRadioBtn1.setChecked(true);
            mRadioBtn2.setChecked(false);
        } else if (mCurrentTime == LOCAL_MODE) {
            mRadioBtn1.setChecked(false);
            mRadioBtn2.setChecked(true);
        }
        AlertDialog timeZoneDialog = new AlertDialog.Builder(mContext).setTitle(R.string.time_settings_title).setView(view)
                .setPositiveButton(com.android.internal.R.string.ok, new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        Xlog.d(TAG, "Pressed OK");
                        Settings.System.putInt(mContext.getContentResolver(), Settings.System.CT_TIME_DISPLAY_MODE,
                                mCurrentTime);
                        // Send broadcast that Call App can receive it.
                        Intent intent = new Intent("com.mediatek.ct.TIME_DISPLAY_MODE");
                        intent.putExtra("time_display_mode", mCurrentTime);
                        mContext.sendBroadcast(intent);
                    }
                }).setNegativeButton(com.android.internal.R.string.cancel, null).create();
        Window win = timeZoneDialog.getWindow();
        win.setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        timeZoneDialog.show();
    }

    public void onClick(View view) {
        // TODO Auto-generated method stub
        boolean checked = ((RadioButton) view).isChecked();
        switch (view.getId()) {
        case R.id.beijing_radio:
            if (checked) {
                if (mRadioBtn2 != null) {
                    mRadioBtn2.setChecked(false);
                    mCurrentTime = BEIJING_MODE;
                }
            }
            break;
        case R.id.local_radio:
            if (checked) {
                if (mRadioBtn1 != null) {
                    mRadioBtn1.setChecked(false);
                    mCurrentTime = LOCAL_MODE;
                }
            }
            break;
        default:
            break;
        }
    }
}
