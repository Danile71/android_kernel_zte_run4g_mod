package com.mediatek.ppl.test.ui;

import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.ppl.PplApplication;
import com.mediatek.ppl.R;
import com.mediatek.ppl.test.util.MockDataUtil;
import com.mediatek.ppl.test.util.MockPplUtil;
import com.mediatek.ppl.ui.LaunchPplActivity;
import com.mediatek.ppl.ui.SetupManualActivity;
import com.mediatek.ppl.ui.SetupTrustedContactsActivity;

import java.util.List;

public class NewSettingTest extends ActivityInstrumentationTestCase2<LaunchPplActivity> {
    private static final String TAG = "PPL/NewSettingTest";

    private Solo mSolo;
    private LaunchPplActivity mActivity;
    private String[] mModeList = null;

    public NewSettingTest() {
        super(LaunchPplActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        prepareEnvironment();

        mActivity = getActivity();
        mSolo = new Solo(getInstrumentation(), mActivity);
        mModeList = mActivity.getApplication().getResources().getStringArray(R.array.enable_mode_list);
    }

    @Override
    protected void tearDown() throws Exception {
        try {
            mSolo.finishOpenedActivities();
        } catch (Throwable e) {
            e.printStackTrace();
        }

        super.tearDown();
    }

    /**
     * Operation: Write sample data & disable feature
     * Check: NA
     * */
    public void test00() {
        ControlData controlData = MockDataUtil.buildSampleControlData();
        controlData.setEnable(false);
        MockDataUtil.writeControlData(controlData);
    }

    /**
     * Operation: click second item (use new setting)
     * Check: UI set up flow & state of feature, contact number
     * */
    public void test01() {
        int index = 1;
        Log.i(TAG, "mModeList[" + index + "] is " + mModeList[index]);
        mSolo.clickOnText(mModeList[index]);

        // 1/3: SetupPasswordActivity
        mSolo.enterText(0, MockPplUtil.PASSWORD_CHANGED);
        mSolo.enterText(1, MockPplUtil.PASSWORD_CHANGED);
        mSolo.clickOnButton(mSolo.getString(R.string.button_next));
        mSolo.waitForActivity(SetupTrustedContactsActivity.class);

        // 2/3: SetupTrustedContactsActivity
        mSolo.enterText(0, MockPplUtil.SERVICE_NUMBER_2nd);
        mSolo.clickOnButton(mSolo.getString(R.string.button_next));
        mSolo.waitForActivity(SetupManualActivity.class);

        // 3/3: SetupManualActivity -> ControlPanelActivity
        mSolo.clickOnButton(mSolo.getString(R.string.button_finish));

        ControlData controlData = MockDataUtil.loadControlData();
        Log.i(TAG, "Control data is " + controlData);

        assertTrue(controlData.isEnabled());
        assertTrue(controlData.isProvisioned());
        assertFalse(controlData.isLocked());
        assertFalse(controlData.isSimLocked());
        assertFalse(controlData.hasWipeFlag());

        assertTrue(MockDataUtil.checkPassword(MockPplUtil.PASSWORD_CHANGED.getBytes(),
                controlData.salt, controlData.secret));

        List<String> trustedList = controlData.TrustedNumberList;
        assertTrue(trustedList.size() == 1);
        assertTrue(trustedList.get(0).equals(MockPplUtil.SERVICE_NUMBER_2nd));

        List<PendingMessage> msgList = controlData.PendingMessageList;
        assertTrue(msgList == null || msgList.size() == 0);

    }

    private void prepareEnvironment() {
        MockDataUtil.preparePlatformManager((PplApplication) getActivity().getApplication());

        // Write the sample data
        ControlData controlData = MockDataUtil.buildSampleControlData();
        controlData.setEnable(false);
        MockDataUtil.writeControlData(controlData);
    }
}
