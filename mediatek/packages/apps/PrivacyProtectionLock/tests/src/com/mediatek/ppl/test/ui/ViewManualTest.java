package com.mediatek.ppl.test.ui;

import android.content.Intent;
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
import com.mediatek.ppl.ui.ViewManualActivity;

public class ViewManualTest extends ActivityInstrumentationTestCase2<ViewManualActivity> {

    private Solo mSolo;
    private ViewManualActivity mActivity;
    private Button mConfirmButton;
    private CheckBox mSendMessage;

    public ViewManualTest() {
        super(ViewManualActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        MockDataUtil.preparePlatformManager((PplApplication) getActivity().getApplication());
        MockDataUtil.writeSampleControlData();

        mActivity = getActivity();
        mSolo = new Solo(getInstrumentation(), mActivity);
        mConfirmButton = (Button) mActivity.findViewById(R.id.btn_bottom_next);
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
        assertNotNull(mActivity);
        assertNotNull(mConfirmButton);
        assertFalse(mSendMessage.isChecked());
        assertTrue(mConfirmButton.isEnabled());
        assertTrue(mSolo.getString(R.string.button_finish).equals(mConfirmButton.getText().toString()));
    }

    /**
     * Operation: click check box, confirm button and turn on airplane mode
     * Check: NA
     * */
    public void test01() {
        mSolo.clickOnButton(mSendMessage.getText().toString());
        assertTrue(mSendMessage.isChecked());
        assertTrue(mSolo.getString(R.string.button_send_sms).equals(mConfirmButton.getText().toString()));

        mSolo.clickOnButton(mConfirmButton.getText().toString());

        MockPplUtil.turnonAirplaneMode(mActivity);
    }

    /**
     * Operation: Cancel to turn off airplane mode
     * Check: NA
     * */
    public void test02() {
        mSolo.clickOnButton(mSendMessage.getText().toString());
        mSolo.clickOnButton(mConfirmButton.getText().toString());

        if (MockPplUtil.isAirplaneModeEnabled(mActivity)) {
            mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));
        }
        MockPplUtil.testUiReceiver(mActivity, "ViewManualActivity", Intent.ACTION_SCREEN_OFF);
    }

    /**
     * Operation: turn off airplane mode
     * Check: NA
     * */
    public void test03() {
        mSolo.clickOnButton(mSendMessage.getText().toString());
        mSolo.clickOnButton(mConfirmButton.getText().toString());

        if (MockPplUtil.isAirplaneModeEnabled(mActivity)) {
            mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
        }
        MockPplUtil.testUiReceiver(mActivity, "ViewManualActivity", PplService.Intents.UI_NO_SIM);
    }

    /**
     * Operation: display chooseSimDialog (only sim2)
     * Check: NA
     * */
    public void test04() {
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
        int[] insertedSim = {0, 1};
        MockPplUtil.displaySimDiag(mActivity, insertedSim);

        ListView listView = (ListView) mSolo.getView(ListView.class, 0);
        if (listView != null) {
            mSolo.clickOnView(listView.getChildAt(0));
        }
    }

}
