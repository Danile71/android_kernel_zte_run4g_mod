package com.mediatek.ppl.test.ui;

import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.ppl.PplApplication;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;
import com.mediatek.ppl.test.util.MockDataUtil;
import com.mediatek.ppl.test.util.MockPplUtil;
import com.mediatek.ppl.ui.LoginPplActivity;

public class LoginTest extends ActivityInstrumentationTestCase2<LoginPplActivity> {
    private static final String TAG = "PPL/LoginTest";

    private Solo mSolo;
    private LoginPplActivity mActivity;

    private CheckBox mShowPassword;
    private EditText mPassword;
    private Button mConfirmButton;

    public LoginTest() {
        super(LoginPplActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        MockDataUtil.preparePlatformManager((PplApplication) getActivity().getApplication());
        MockDataUtil.writeSampleControlData();

        mActivity = getActivity();
        mSolo = new Solo(getInstrumentation(), mActivity);

        mPassword = (EditText) mActivity.findViewById(R.id.et_login_ppl_input);
        mShowPassword = (CheckBox) mActivity.findViewById(R.id.cb_login_ppl_show_pw);
        mConfirmButton = (Button) mActivity.findViewById(R.id.btn_bottom_confirm);

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
        assertNotNull(mPassword);
        assertNotNull(mShowPassword);
        assertNotNull(mConfirmButton);

        assertFalse(mConfirmButton.isEnabled());
        assertFalse(mShowPassword.isChecked());
        mSolo.clickOnActionBarHomeButton();
    }

    /**
     * Operation: input and click check box
     * Check: state of check box
     * */
    public void test01() {
        MockPplUtil.formatLog(TAG, "test01");
        inputText("0");
        mSolo.clickOnButton(mShowPassword.getText().toString());
        assertTrue(mShowPassword.isChecked());

        inputText("1");
        mSolo.clickOnButton(mShowPassword.getText().toString());
        assertFalse(mShowPassword.isChecked());

        MockPplUtil.testUiReceiver(mActivity, "LoginPplActivity", PplService.Intents.UI_QUIT_SETUP_WIZARD);
    }

    /**
     * Operation: input a wrong password, and click next
     * Check: NA
     * */
    public void test02() {
        MockPplUtil.formatLog(TAG, "test02");
        inputText("1000010000");
        assertTrue(mConfirmButton.isEnabled());
        mSolo.clickOnButton(mConfirmButton.getText().toString());

        MockPplUtil.testUiReceiver(mActivity, "LoginPplActivity", PplService.Intents.UI_NO_SIM);
    }

    /**
     * Operation: input password
     * Check: content of edit text
     * */
    private void inputText(String text) {
        mSolo.clearEditText(mPassword);
        mSolo.enterText(mPassword, text);
        Log.i(TAG, "Entered password is " + mPassword.getText());
        assertTrue(text.equals(mPassword.getText().toString()));
    }

}
