package com.hissage.ui.activity;

import java.util.Timer;
import java.util.TimerTask;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.jni.engineadapter;
//M: Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.receiver.system.NmsSMSReceiver;
import com.hissage.struct.SNmsMsgType;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;
import com.hissage.util.statistics.NmsRegInfoStatistics;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;

public class NmsActivationActivity extends NmsBaseActivity implements OnClickListener {

    private static final String TAG = "NmsActivationActivity";

    private TextView mMainTitle = null;
    private TextView mTitle = null;

    private TextView mCountryTitle = null;
    private Spinner mCountryContent = null;

    private TextView mPhoneTitle = null;
    private EditText mPhoneEdit = null;

    private ProgressBar mProgressWait = null;
    private TextView mWaitVerify = null;

    private TextView mVerifyFailTitle = null;
    private TextView mVerifyFailContent = null;

    private TextView mWorkingSmsTip = null;
    private View mLine = null;

    private Button mSendOrEditNumber = null;
    private Button mCancel = null;

    private RegBroadCast mReceiver = null;
    private String mNumber = null;
    private String mUnifyNumber = null;
    private long mSim_id = NmsConsts.INVALID_SIM_ID;
    private int countryIndex = 41;
    private boolean mRegSmsSent = false;
    // private boolean mVerifyFlag = false;
    private static boolean mIsDlgShown = false;
    private int mActivateType;

    private Timer mTimer = null;

    private class CountDownTimerTask extends TimerTask {

        private int mTotalTime = -1;
        private Timer mScheduleTimer = null;

        CountDownTimerTask(Timer timer, int time) {
            mScheduleTimer = timer;
            mTotalTime = time;
        }

        @Override
        public void run() {
            try {
                if (mTotalTime < 0) {
                    mScheduleTimer.cancel();
                    return;
                }
                runOnUiThread(new Runnable() {
                    public void run() {

                        mTotalTime--;
                        if (mPhoneEdit.getVisibility() == View.GONE) {
                            if (mTotalTime < 0) {
                                initMode(getString(R.string.STR_NMS_CONNECTING));
                            } else {
                                initMode(getCountDownTimeString(mTotalTime));
                            }
                        }
                    }
                });
            } catch (Exception e) {
                NmsLog.nmsPrintStackTraceByTag(TAG, e);
            }
        }

    }

    private void initSpinner() {
        long currentSimId = 0;
        if (mSim_id < 0) {
            currentSimId = NmsPlatformAdapter.getInstance(this).getCurrentSimId();
        } else {
            currentSimId = mSim_id;
        }
        int slot = NmsPlatformAdapter.getInstance(this).getSlotIdBySimId(currentSimId);
        String currentImsi = NmsPlatformAdapter.getInstance(this).getImsi(slot);

        if (TextUtils.isEmpty(currentImsi))
            return;

        String[] imsiPreArray = this.getResources().getStringArray(R.array.imsi_pre_list);
        // String[] phoneArray = NmsService.getInstance().getResources()
        // .getStringArray(R.array.phone_number_pre_list);
        for (int i = 0; i < imsiPreArray.length; ++i) {
            if (currentImsi.startsWith(imsiPreArray[i])) {
                countryIndex = i;
                return;
            }
        }
    }

    private void init() {

        initSpinner();

        mMainTitle = (TextView) findViewById(R.id.main_title);
        mTitle = (TextView) findViewById(R.id.title);

        mCountryTitle = (TextView) findViewById(R.id.country_title);
        mCountryContent = (Spinner) findViewById(R.id.country_content);
        mCountryContent.setSelection(countryIndex);
        mCountryContent.setEnabled(false);

        mPhoneTitle = (TextView) findViewById(R.id.phone_title);
        mPhoneEdit = (EditText) findViewById(R.id.phone);
        if (!TextUtils.isEmpty(mNumber)) {
            mPhoneEdit.setText(mNumber);
        }

        mProgressWait = (ProgressBar) findViewById(R.id.progress_wait);
        mWaitVerify = (TextView) findViewById(R.id.wait_for_verify);

        mVerifyFailTitle = (TextView) findViewById(R.id.verify_fail_title);
        mVerifyFailContent = (TextView) findViewById(R.id.verify_fail_content);

        mWorkingSmsTip = (TextView) findViewById(R.id.working_sms_tip);
        mLine = (View) findViewById(R.id.divider);

        mSendOrEditNumber = (Button) findViewById(R.id.send_or_edit);
        mSendOrEditNumber.setOnClickListener(this);
        mCancel = (Button) findViewById(R.id.cancel);
        mCancel.setOnClickListener(this);

        initMode(getString(R.string.STR_NMS_CONNECTING));
    }

    private void initMode(String waitText) {
        mMainTitle.setText(R.string.STR_NMS_ACTIVE);
        mTitle.setVisibility(View.GONE);

        mCountryTitle.setVisibility(View.INVISIBLE);
        mCountryContent.setVisibility(View.GONE);

        mPhoneTitle.setVisibility(View.GONE);
        mPhoneEdit.setVisibility(View.GONE);

        mProgressWait.setVisibility(View.VISIBLE);
        mWaitVerify.setVisibility(View.VISIBLE);
        mWaitVerify.setText(waitText);

        mVerifyFailTitle.setVisibility(View.GONE);
        mVerifyFailContent.setVisibility(View.GONE);

        mWorkingSmsTip.setVisibility(View.INVISIBLE);
        mSendOrEditNumber.setVisibility(View.GONE);

        mLine.setVisibility(View.GONE);

        mCancel.setVisibility(View.GONE);
        mCancel.setText(R.string.STR_NMS_CANCEL);
    }

    private void inputMode() {

        if (mTimer != null)
            mTimer.cancel();

        mMainTitle.setText(R.string.STR_NMS_ACTIVAT_MAIN_TITLE);
        mTitle.setVisibility(View.VISIBLE);

        mCountryTitle.setVisibility(View.VISIBLE);
        mCountryContent.setVisibility(View.VISIBLE);

        mPhoneTitle.setVisibility(View.VISIBLE);
        mPhoneEdit.setVisibility(View.VISIBLE);
        mPhoneEdit.setEnabled(true);

        mProgressWait.setVisibility(View.GONE);
        mWaitVerify.setVisibility(View.GONE);

        mVerifyFailTitle.setVisibility(View.GONE);
        mVerifyFailContent.setVisibility(View.GONE);

        mWorkingSmsTip.setVisibility(View.GONE);
        mSendOrEditNumber.setVisibility(View.VISIBLE);
        mSendOrEditNumber.setText(R.string.STR_NMS_SEND);

        mCancel.setText(R.string.STR_NMS_CANCEL);

        mLine.setVisibility(View.VISIBLE);

        NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                NmsRegInfoStatistics.NMS_REGS_SHOW_INPUT_DLG, null);
    }

    private void verifingMode() {
        mMainTitle.setText(R.string.STR_NMS_ACTIVAT_MAIN_TITLE2);
        mTitle.setVisibility(View.GONE);

        mCountryTitle.setVisibility(View.GONE);
        mCountryContent.setVisibility(View.GONE);

        mPhoneTitle.setVisibility(View.GONE);
        mPhoneEdit.setVisibility(View.VISIBLE);
        mPhoneEdit.setEnabled(false);
        mPhoneEdit.clearFocus();

        mProgressWait.setVisibility(View.VISIBLE);
        mWaitVerify.setVisibility(View.VISIBLE);
        mWaitVerify.setText(R.string.STR_NMS_VERIFYING);

        mVerifyFailTitle.setVisibility(View.GONE);
        mVerifyFailContent.setVisibility(View.GONE);

        mWorkingSmsTip.setVisibility(View.VISIBLE);

        mSendOrEditNumber.setVisibility(View.GONE);
        mSendOrEditNumber.setText(R.string.STR_NMS_EDIT_NUMBER);

        mCancel.setText(R.string.STR_NMS_CANCEL);
        mLine.setVisibility(View.GONE);

    }

    private void verifyFailMode() {
        mMainTitle.setText(R.string.STR_NMS_ACTIVAT_MAIN_TITLE2);
        mTitle.setVisibility(View.GONE);

        mCountryTitle.setVisibility(View.GONE);
        mCountryContent.setVisibility(View.GONE);

        mPhoneTitle.setVisibility(View.GONE);
        mPhoneEdit.setVisibility(View.VISIBLE);
        mPhoneEdit.setEnabled(false);
        mPhoneEdit.clearFocus();

        mProgressWait.setVisibility(View.GONE);
        mWaitVerify.setVisibility(View.GONE);

        mVerifyFailTitle.setVisibility(View.VISIBLE);
        mVerifyFailContent.setVisibility(View.VISIBLE);

        mWorkingSmsTip.setVisibility(View.VISIBLE);
        mSendOrEditNumber.setVisibility(View.VISIBLE);
        mSendOrEditNumber.setText(R.string.STR_NMS_EDIT_NUMBER);

        mCancel.setText(R.string.STR_NMS_RETRY);
        mLine.setVisibility(View.VISIBLE);
        NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                NmsRegInfoStatistics.NMS_REGS_SHOW_EDIT_DLG, null);
    }

    void initPhoneNumber() {
        if (mSim_id > 0) {
            int slotId = NmsPlatformAdapter.getInstance(this).getSlotIdBySimId(mSim_id);
            if (slotId >= 0) {
                mNumber = NmsPlatformAdapter.getInstance(this).getLine1Number(slotId);
            }
        }
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activition_layout);
        setFinishOnTouchOutside(false);

        Intent i = getIntent();
        if (i == null
                || NmsConsts.INVALID_SIM_ID == (mSim_id = i.getLongExtra(NmsConsts.SIM_ID,
                        NmsConsts.INVALID_SIM_ID))) {
            NmsLog.error(TAG, "not find sim_id at intent: " + i);
            finish();
            return;
        }

        if (!isSimCardReady()) {
            Toast.makeText(getApplicationContext(), R.string.STR_NMS_NO_SIM, Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }
        //fix bug ISMS-1197
        SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId((int)mSim_id);
        if (info != null) {
            if (info.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                NmsLog.error(TAG, String.format("error that the status: %d of mSim_id: %d is invalid", info.status, mSim_id));
                finish() ;
                return ;
            }
        } else {
            NmsLog.error(TAG, "fatal error that get SNmsSimInfo is null,  mSim_id: " + mSim_id);
        }

        initPhoneNumber();

        init();

        NmsSMSReceiver.getInstance().setRegPhone(mNumber);
        if (null == mReceiver) {
            regResRecver();
        }
        // M: Add new feature: ISMS-214
        mActivateType = i.getIntExtra(NmsConsts.ACTIVATE_TYPE,
                NmsIpMessageConsts.NmsUIActivateType.OTHER);

        int keyIndex = NmsStatistics.KEY_OTHER_ACTIVATE_TRY;
        if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.EMOTION)
            keyIndex = NmsStatistics.KEY_EMO_ACTIVATE_TRY;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.MULTI_MEDIA)
            keyIndex = NmsStatistics.KEY_MEDIA_ACTIVATE_TRY;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.SETTING)
            keyIndex = NmsStatistics.KEY_SETTING_ACTIVATE_TRY;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.DIALOG)
            keyIndex = NmsStatistics.KEY_DLG_ACTIVATE_TRY;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.PROMPT)
            keyIndex = NmsStatistics.TIPS_ACTIVATE_TRY;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.MESSAGE)
            keyIndex = NmsStatistics.SMS_ACTIVATE_TRY;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.WAKEUP)
            keyIndex = NmsStatistics.KEY_WAKEUP_ACTIVATE_TRY;
        NmsStatistics.incKeyVal(keyIndex);
        engineadapter.get().nmsUISetUserActivteType(mActivateType);
        engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
    }

    @Override
    protected void onPause() {
        /*
         * if (mTimer != null) mTimer.cancel() ;
         */

        super.onPause();
    }

    private void regResRecver() {
        mReceiver = new RegBroadCast();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsIntentStrId.NMS_REG_STATUS);
        filter.addAction(NmsIntentStrId.NMS_REG_SMS_ERROR);
        filter.addAction(NmsIntentStrId.NMS_REG_INPUT_PHONENUM);
        filter.addAction(NmsIntentStrId.NMS_REG_SEND_NOC_TIME);
        filter.addAction(NmsIntentStrId.NMS_REG_SEND_SELF_TIME);
        registerReceiver(mReceiver, filter);
    }

    @Override
    public void finish() {
        if (null != mReceiver) {
            unregisterReceiver(mReceiver);
        }
        mReceiver = null;
        super.finish();
    }

    private void sendInputNumberMsgToEngine() {
        mNumber = mPhoneEdit.getText().toString();
        if (TextUtils.isEmpty(mNumber) || mNumber.length() < 5) {
            Toast.makeText(this, R.string.STR_NMS_ILLEGAL_ADDRESS, Toast.LENGTH_SHORT).show();
            return;
        }
        mRegSmsSent = true;
        mUnifyNumber = engineadapter.get().nmsUIGetUnifyPhoneNumber(mNumber);
        NmsIpMessageApiNative.nmsInputNumberForActivation(mUnifyNumber, (int) mSim_id);
        verifingMode();
    }

    private void showChooseNetworkDlg() {

        if (mIsDlgShown) {
            NmsLog.error(TAG, "this dlg was shown, so ignore this.");
            return;
        }
        mIsDlgShown = true;

        new AlertDialog.Builder(NmsActivationActivity.this).setTitle(R.string.STR_NMS_MAIN)
                .setMessage(R.string.STR_NMS_NETWORK_UNAVL)
                .setPositiveButton(R.string.STR_NMS_RETRY, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        engineadapter.get().nmsUISetUserActivteType(mActivateType);
                        engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
                        mIsDlgShown = false;
                        dialog.cancel();
                    }
                })
                .setNegativeButton(R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mIsDlgShown = false;
                        finish();
                    }
                }).setCancelable(false).create().show();

    }

    private void regFail(Context context) {
        if (mRegSmsSent) {
            verifyFailMode();
        } else {
            showChooseNetworkDlg();
        }
    }
    
    private String getCountDownTimeString(int time) {
        return time + getString(R.string.STR_NMS_SECOND) + "..." ;
    }

    private void startCountdown(boolean isNoc, final int totalTime) {

        if (totalTime <= 0) {
            NmsLog.error(TAG, "startCountdown got invalid time: " + totalTime);
            return;
        }

        mTimer = new Timer();

        TimerTask timerTask = new CountDownTimerTask(mTimer, totalTime);

        mTimer.schedule(timerTask, 1000, 1000);
        initMode(getCountDownTimeString(totalTime));
    }

    class RegBroadCast extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();
            NmsLog.trace(TAG, "The Activation hissage action:" + action);

            if (action.equals(NmsIntentStrId.NMS_REG_SMS_ERROR)) {
                new AlertDialog.Builder(context)
                        .setTitle(getString(R.string.STR_NMS_HISSAGE))
                        .setMessage(getString(R.string.STR_NMS_SEND_SMS_FAIL))
                        .setPositiveButton(R.string.STR_NMS_OK,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        dialog.cancel();
                                    }
                                }).create().show();
                verifyFailMode();
            } else if (NmsIntentStrId.NMS_REG_INPUT_PHONENUM.equals(action)) {
                /*
                 * if (mVerifyFlag) { mVerifyFlag = false; sendSMS(); } else {
                 * inputMode(); }
                 */

                inputMode();
            } else if (action.equals(NmsIntentStrId.NMS_REG_STATUS)) {
                int regStatus = intent.getIntExtra("regStatus", -1);

                switch (regStatus) {
                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER:
                    Toast.makeText(getApplicationContext(), R.string.STR_NMS_ENABLE_SUCCESS,
                            Toast.LENGTH_SHORT).show();
                    finish();
                    break;

                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_FAIL:
                    regFail(context);
                    break;

                case SNmsMsgType.NMS_UI_MSG_CHOOSE_IAP:
                    showChooseNetworkDlg();
                    break;

                case SNmsMsgType.NMS_UI_MSG_REMIND_USER_TO_SET_NAME:
                    Toast.makeText(context, R.string.STR_NMS_REMIND_USER_SET_NAME,
                            Toast.LENGTH_LONG).show();
                    break;

                case SNmsMsgType.NMS_UI_MSG_VERIF_ING:
                    break;

                default:
                    NmsLog.trace(TAG, "some case not necessary, case: " + regStatus);
                }

            } else if (action.equals(NmsIntentStrId.NMS_REG_SEND_NOC_TIME)) {
                int remainTime = intent.getIntExtra(NmsIntentStrId.NMS_REG_SEND_NOC_TIME, -1);
                NmsLog.trace(TAG, "got noc reg time:" + remainTime);
                startCountdown(true, remainTime);
            } else if (action.equals(NmsIntentStrId.NMS_REG_SEND_SELF_TIME)) {
                int remainTime = intent.getIntExtra(NmsIntentStrId.NMS_REG_SEND_SELF_TIME, -1);
                NmsLog.trace(TAG, "got self reg time:" + remainTime);
                startCountdown(false, remainTime);
            }
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mCancel) {

            String tip = (String) mCancel.getText();
            if (tip.equals(getString(R.string.STR_NMS_RETRY))) {
                if (TextUtils.isEmpty(mNumber)) {
                    initMode(getString(R.string.STR_NMS_CONNECTING));
                } else {
                    /*
                     * mVerifyFlag = true; verifingMode();
                     */
                    sendInputNumberMsgToEngine();
                }
                engineadapter.get().nmsUISetUserActivteType(mActivateType);
                engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
            } else {
                NmsLog.trace(TAG, "user press the cancel button, just cancel the activation");
                engineadapter.get().nmsUICancelActivation();
                if (mTimer != null)
                    mTimer.cancel();
                this.finish();
            }

        } else if (v == mSendOrEditNumber) {
            String tip = (String) mSendOrEditNumber.getText();
            if (tip.equals(getString(R.string.STR_NMS_SEND))) {
                sendInputNumberMsgToEngine();
            } else {
                /*
                 * initMode();
                 * engineadapter.get().nmsUISetUserActivteType(mActivateType);
                 * engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
                 */

                inputMode();
            }
        }

    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mProgressWait.getVisibility() == View.VISIBLE && !mRegSmsSent) {
                NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                        NmsRegInfoStatistics.NMS_REGS_NOC_BACK, null);
            } else if (mProgressWait.getVisibility() == View.VISIBLE && mRegSmsSent) {
                NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                        NmsRegInfoStatistics.NMS_REGS_SELF_REG_BACK, null);
            } else if (mPhoneEdit.getVisibility() == View.VISIBLE
                    && mPhoneEdit.isEnabled()
                    && getWindow().getAttributes().softInputMode != WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE) {
                String number = mPhoneEdit.getText().toString();
                if (TextUtils.isEmpty(number)) {
                    NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                            NmsRegInfoStatistics.NMS_REGS_INPUT_DLG_BACK, null);
                } else {
                    String unifyNumber = engineadapter.get().nmsUIGetUnifyPhoneNumber(number);
                    NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                            NmsRegInfoStatistics.NMS_REGS_INPUT_DLG_BACK, unifyNumber);
                }
            } else if (mPhoneEdit.getVisibility() == View.VISIBLE
                    && !mPhoneEdit.isEnabled()
                    && getWindow().getAttributes().softInputMode != WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE) {
                NmsRegInfoStatistics.sendNewStatistics((int) mSim_id,
                        NmsRegInfoStatistics.NMS_REGS_SHOW_EDIT_BACK, null);
            }
        }
        return super.onKeyUp(keyCode, event);
    }
}
