package com.mediatek.ppl.test.ui;

import android.test.ActivityInstrumentationTestCase2;
import android.widget.Button;
import android.widget.EditText;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.ppl.PplApplication;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;
import com.mediatek.ppl.test.util.MockDataUtil;
import com.mediatek.ppl.test.util.MockPplUtil;
import com.mediatek.ppl.ui.SetupPasswordActivity;

public class SetupPasswordTest extends ActivityInstrumentationTestCase2<SetupPasswordActivity> {
    private static final String TAG = "PPL/SetupPasswordTest";

    private Solo mSolo;
    private SetupPasswordActivity mActivity;
    private Button mNextButton;
    private EditText mFirstPassword;
    private EditText mSecondPassword;

    public SetupPasswordTest() {
        super(SetupPasswordActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        MockDataUtil.preparePlatformManager((PplApplication) getActivity().getApplication());

        mActivity = getActivity();
        mSolo = new Solo(getInstrumentation(), mActivity);
        mNextButton = (Button) mActivity.findViewById(R.id.btn_bottom_next);
        mFirstPassword = (EditText) mActivity.findViewById(R.id.et_setup_password_input1);
        mSecondPassword = (EditText) mActivity.findViewById(R.id.et_setup_password_input2);
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
     * Check: state of button
     * */
    public void test00() {
        MockPplUtil.formatLog(TAG, "test00");
        assertNotNull(mActivity);
        assertNotNull(mNextButton);
        assertFalse(mNextButton.isEnabled());
        mSolo.clickOnActionBarHomeButton();
    }

    /**
     * Operation: input password
     * Check: input and state of button
     * */
    public void test01() {
        MockPplUtil.formatLog(TAG, "test01");
        String shortPass = "11111";
        mSolo.enterText(0, shortPass);
        mSolo.enterText(1, shortPass);
        assertFalse(mNextButton.isEnabled());
        assertTrue(mFirstPassword.getText().toString().equals(shortPass));
        assertTrue(mSecondPassword.getText().toString().equals(shortPass));

        mSolo.enterText(0, "0");
        assertFalse(mNextButton.isEnabled());
        assertTrue(mFirstPassword.getText().toString().equals(shortPass + "0"));

        mSolo.enterText(1, "1");
        assertTrue(mNextButton.isEnabled());
        assertTrue(mSecondPassword.getText().toString().equals(shortPass + "1"));

        mSolo.clickOnButton(mSolo.getString(R.string.button_next));
        MockPplUtil.testUiReceiver(mActivity, "SetupPasswordActivity", PplService.Intents.UI_QUIT_SETUP_WIZARD);
    }

    /**
     * Operation: only input first EditText
     * Check: state of button
     * */
    public void test02() {
        MockPplUtil.formatLog(TAG, "test02");
        mSolo.enterText(0, "111111");
        assertFalse(mNextButton.isEnabled());
        MockPplUtil.testUiReceiver(mActivity, "SetupPasswordActivity", PplService.Intents.UI_NO_SIM);
    }
}
