package com.mediatek.ppl.test.ui;

import android.test.ActivityInstrumentationTestCase2;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.ppl.PplApplication;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;
import com.mediatek.ppl.test.util.MockDataUtil;
import com.mediatek.ppl.test.util.MockPplUtil;
import com.mediatek.ppl.ui.SetupManualActivity;

public class SetupManualTest extends ActivityInstrumentationTestCase2<SetupManualActivity> {
    private static final String TAG = "PPL/SetupManualTest";

    private Solo mSolo;
    private SetupManualActivity mActivity;
    private Button mNextButton;
    private CheckBox mSendMessage;

    public SetupManualTest() {
        super(SetupManualActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        MockDataUtil.preparePlatformManager((PplApplication) getActivity().getApplication());

        mActivity = getActivity();
        mSolo = new Solo(getInstrumentation(), mActivity);
        mNextButton = (Button) mActivity.findViewById(R.id.btn_bottom_next);
        mSendMessage = (CheckBox) mActivity.findViewById(R.id.cb_send_manual_sendSms);
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
     * Operation: NA
     * Check: text and state of button, then state of check box
     * */
    public void test00() {
        MockPplUtil.formatLog(TAG, "test00");
        assertNotNull(mActivity);
        assertNotNull(mNextButton);
        assertFalse(mSendMessage.isChecked());
        assertTrue(mNextButton.isEnabled());
        assertTrue(mSolo.getString(R.string.button_finish).equals(mNextButton.getText().toString()));
    }

    /**
     * Operation: click check box, confirm button and turn on airplane mode
     * Check: NA
     * */
    public void test01() {
        MockPplUtil.formatLog(TAG, "test01");
        mSolo.clickOnButton(mSendMessage.getText().toString());
        assertTrue(mSendMessage.isChecked());
        assertTrue(mSolo.getString(R.string.button_send_sms).equals(mNextButton.getText().toString()));

        mSolo.clickOnButton(mNextButton.getText().toString());

        MockPplUtil.turnonAirplaneMode(mActivity);
    }

    /**
     * Operation: Cancel to turn off airplane mode
     * Check: NA
     * */
    public void test02() {
        MockPplUtil.formatLog(TAG, "test02");
        mSolo.clickOnButton(mSendMessage.getText().toString());
        mSolo.clickOnButton(mNextButton.getText().toString());

        if (MockPplUtil.isAirplaneModeEnabled(mActivity)) {
            mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));
        }

        MockPplUtil.testUiReceiver(mActivity, "SetupManualActivity", PplService.Intents.UI_QUIT_SETUP_WIZARD);
    }

    /**
     * Operation: turn off airplane mode
     * Check: NA
     * */
    public void test03() {
        MockPplUtil.formatLog(TAG, "test03");
        mSolo.clickOnButton(mSendMessage.getText().toString());
        mSolo.clickOnButton(mNextButton.getText().toString());

        if (MockPplUtil.isAirplaneModeEnabled(mActivity)) {
            mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
        }

        MockPplUtil.testUiReceiver(mActivity, "SetupManualActivity", PplService.Intents.UI_NO_SIM);
    }

    /**
     * Operation: display chooseSimDialog (only sim2)
     * Check: NA
     * */
    public void test04() {
        MockPplUtil.formatLog(TAG, "test04");
        int[] insertedSim = {1};
        MockPplUtil.displaySimDiag(mActivity, insertedSim);

        ListView listView = (ListView) mSolo.getView(ListView.class, 0);
        if (listView != null) {
            mSolo.clickOnView(listView.getChildAt(0));
        }
    }

    /**
     * Operation: display chooseSimDialog (sim1 & sim2)
     * Check: NA
     * */
    public void test05() {
        MockPplUtil.formatLog(TAG, "test05");
        int[] insertedSim = {0, 1};
        MockPplUtil.displaySimDiag(mActivity, insertedSim);

        ListView listView = (ListView) mSolo.getView(ListView.class, 0);
        if (listView != null) {
            mSolo.clickOnView(listView.getChildAt(0));
        }
    }
}
