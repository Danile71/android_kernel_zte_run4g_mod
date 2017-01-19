package com.mediatek.ppl.ui;

import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.Toast;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class SetupPasswordActivity extends PplBasicActivity implements PplRelativeLayout.IOnResizeListener {
    
    private static final String TAG = "PPL/SetupPasswordActivity";
    private static final String KEY_FIRST_INPUT = "first_input";
    private static final String KEY_SECOND_INPUT = "second_input";
   
    private Intent mLaunchIntent;
    
    protected ScrollView mScrollView;
    protected ProgressBar mProgressBar;
    protected PplRelativeLayout mLayoutOuter;
    protected LinearLayout mLayoutUp;
    protected LinearLayout mLayoutDown;
    protected Button mNextButton;
    protected EditText mFirstPassword;
    protected EditText mSecondPassword;

    @Override
    protected void onCreate(Bundle saveInstanceState) {
        super.onCreate(saveInstanceState);
        mLaunchIntent = getIntent();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putString(KEY_FIRST_INPUT, mFirstPassword.getText().toString());
        outState.putString(KEY_SECOND_INPUT, mSecondPassword.getText().toString());
        super.onSaveInstanceState(outState);
    }
        
	@Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        if (null != savedInstanceState) {
            mFirstPassword.setText(savedInstanceState.getString(KEY_FIRST_INPUT));
            mSecondPassword.setText(savedInstanceState.getString(KEY_SECOND_INPUT));
        }
        super.onRestoreInstanceState(savedInstanceState);
    }

	@Override
	protected void onRegisterEvent() {
	    mEventReceiver.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
	}

    @Override
	protected void onPrepareLayout() {
        
        setContentView(R.layout.setup_password);

        mScrollView = (ScrollView)findViewById(R.id.scrollview_setup_password);
        mLayoutOuter = (PplRelativeLayout)findViewById(R.id.layout_setup_password_outer);
        mLayoutUp = (LinearLayout)findViewById(R.id.layout_setup_password_up);
        mLayoutDown = (LinearLayout)findViewById(R.id.layout_setup_password_down);
        
        mProgressBar = (ProgressBar)findViewById(R.id.common_progress);
        mNextButton = (Button)findViewById(R.id.btn_bottom_next);
        mFirstPassword = (EditText)findViewById(R.id.et_setup_password_input1);
        mSecondPassword = (EditText)findViewById(R.id.et_setup_password_input2);
        
        mLayoutOuter.setOnResizeListener(this);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                onButtonClicked();                
            }            
        });
        
        mFirstPassword.setTypeface(Typeface.SANS_SERIF);
        mFirstPassword.addTextChangedListener(mTextWather);
        mSecondPassword.setTypeface(Typeface.SANS_SERIF);
        mSecondPassword.addTextChangedListener(mTextWather);
	}


    @Override
    protected void onInitLayout() {
        mNextButton.setEnabled(false);
        mProgressBar.setVisibility(View.VISIBLE);
        mLayoutUp.setVisibility(View.GONE);
        mLayoutDown.setVisibility(View.GONE);
        mFirstPassword.requestFocus();
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        if (null != mLaunchIntent  &&
            (0 != (mLaunchIntent.getFlags() & Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY))){

            if (mBinder.isEnabled()) {
                gotoActivity(this, LoginPplActivity.class);
                finish();
            } else if (mBinder.isProvisioned()) {
                gotoActivity(this, LaunchPplActivity.class);
                finish();
            } else {
                mProgressBar.setVisibility(View.GONE);
                mLayoutUp.setVisibility(View.VISIBLE);
                mLayoutDown.setVisibility(View.VISIBLE);
            }
        } else {
            mProgressBar.setVisibility(View.GONE);
            mLayoutUp.setVisibility(View.VISIBLE);
            mLayoutDown.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onPanelChange(int h) {
        mScrollView.scrollTo(0,h);
    }

    private TextWatcher mTextWather = new TextWatcher() {

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            if (mFirstPassword.getText().length() >= PplService.MIN_PASSWORD_LENGTH && 
                mSecondPassword.getText().length() >= PplService.MIN_PASSWORD_LENGTH) {
                    mNextButton.setEnabled(true);
                } else {
                    mNextButton.setEnabled(false);
                }
        }
        
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
           
        }

        @Override
        public void afterTextChanged(Editable s) {
            
        }
    };

    protected void onButtonClicked() {
        if (!mFirstPassword.getText().toString().equals(mSecondPassword.getText().toString())) {
            Toast.makeText(this, R.string.toast_passwords_do_not_match, Toast.LENGTH_SHORT).show();
        } else {
            mBinder.savePassword(mFirstPassword.getText().toString(), PplService.EDIT_TYPE_SETUP);
            gotoActivity(this, SetupTrustedContactsActivity.class);
            //gotoActivity(this, LaunchPplActivity.class);
        }
    }

}
