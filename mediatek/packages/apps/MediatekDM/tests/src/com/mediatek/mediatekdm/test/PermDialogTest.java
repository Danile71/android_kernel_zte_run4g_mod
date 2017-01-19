package com.mediatek.mediatekdm.test;

import android.app.NotificationManager;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.mediatekdm.CollectSetPermissionControl;
import com.mediatek.mediatekdm.CollectSetPermissionDialog;
import com.mediatek.mediatekdm.DmConst;

public class PermDialogTest extends ActivityInstrumentationTestCase2<CollectSetPermissionDialog> {
    private static final String TAG = "MDMTest/PermDialogTest";

    private Solo mSolo;
    private CollectSetPermissionDialog mActivity;

    public PermDialogTest() {
        super(CollectSetPermissionDialog.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
        mSolo = new Solo(getInstrumentation(), mActivity);
    }

    @Override
    protected void tearDown() throws Exception {
        try {
            mSolo.finishOpenedActivities();
            clearNotification();
        } catch (Throwable e) {
            e.printStackTrace();
        }
        super.tearDown();
    }

    /**
     * Operation: rotate
     * Check: NA
     * */
    public void test00() {
        CollectSetPermissionControl.getInstance().isPermFileReady();
        CollectSetPermissionControl.getInstance().resetKeyValue();

        Log.i(TAG, "test00.");
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
    }

    /**
     * Operation: click ok
     * Check: NA
     * */
    public void test01() {
        Log.i(TAG, "test01.");
        mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
    }

    /**
     * Operation: click check box and ok
     * Check: NA
     * */
    public void test02() {
        Log.i(TAG, "test02.");
        mSolo.clickOnCheckBox(0);
        mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
    }

    /**
     * Operation: click cancel -> ok
     * Check: NA
     * */
    public void test03() {
        Log.i(TAG, "test03.");
        Log.i(TAG, "Click cancle button.");
        mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));

        Log.i(TAG, "Click ok button of 2nd dialogue");
        mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
    }

    /**
     * Operation: click cancel -> cancel
     * Check: NA
     * */
    public void test04() {
        Log.i(TAG, "test04.");
        Log.i(TAG, "Click cancle button.");
        mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));

        Log.i(TAG, "Click cancel button of 2nd dialogue");
        mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));

        CollectSetPermissionControl.getInstance().resetKeyValue();
    }

    private void clearNotification() {
        NotificationManager notificationManager = (NotificationManager) mActivity.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(DmConst.NotificationInteractionType.TYPE_COLLECT_SET_PERM_NOTIFICATION);
    }
}
