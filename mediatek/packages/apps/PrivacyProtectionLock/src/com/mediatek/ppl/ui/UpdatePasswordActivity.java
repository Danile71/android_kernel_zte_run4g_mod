package com.mediatek.ppl.ui;

import android.content.Intent;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class UpdatePasswordActivity extends SetupPasswordActivity implements PplRelativeLayout.IOnResizeListener {
    
    private TextView mTitle;
    private TextView mSummary;
    private TextView mDesc;
    
    @Override 
    public void onDestroy() {
        if (null != mBinder) {
            mBinder.unregisterSensitiveActivity(this);
        }
        super.onDestroy();
    }

    @Override
    protected void onPropertyConfig() {
        setProperty(PROPERTY_NEED_SERVICE|PROPERTY_HAS_ACTIONBAR|PROPERTY_QUIT_BACKGROUND);
    }
    
    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(Intent.ACTION_SCREEN_OFF);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {
        
        super.onPrepareLayout();        
        
        mTitle = (TextView)findViewById(R.id.tv_setup_password_title);   
        mSummary = (TextView)findViewById(R.id.tv_setup_password_summary);        
        mDesc = (TextView)findViewById(R.id.tv_setup_password_desc);
    }


    @Override
    protected void onInitLayout() {
        mTitle.setText(R.string.title_change_password);
        mSummary.setVisibility(View.GONE);
        mDesc.setVisibility(View.GONE);
        mNextButton.setText(R.string.button_confirm);

        super.onInitLayout();
    }
    
    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mBinder.registerSensitiveActivity(this);
        mProgressBar.setVisibility(View.GONE);
        mLayoutUp.setVisibility(View.VISIBLE);
        mLayoutDown.setVisibility(View.VISIBLE);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (android.R.id.home == item.getItemId()) {
            gotoActivity(this, ControlPanelActivity.class);
            finish();
            return true;
        }
        return false;
    }
    
    @Override
    public void onBackPressed() {
        gotoActivity(this, ControlPanelActivity.class);
        finish();
    }
    
    @Override
    protected void onButtonClicked() {
        if (!mFirstPassword.getText().toString().equals(mSecondPassword.getText().toString())) {
            Toast.makeText(this, R.string.toast_passwords_do_not_match, Toast.LENGTH_SHORT).show();
        } else {
            mBinder.savePassword(mFirstPassword.getText().toString(), PplService.EDIT_TYPE_UPDATE);
            gotoActivity(this, ControlPanelActivity.class);
            finish();
        }
    }
}
