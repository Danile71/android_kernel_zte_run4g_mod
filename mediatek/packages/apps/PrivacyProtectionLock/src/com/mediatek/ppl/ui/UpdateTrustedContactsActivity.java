package com.mediatek.ppl.ui;

import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;
import com.mediatek.ppl.ui.SetupTrustedContactsActivity;

public class UpdateTrustedContactsActivity extends SetupTrustedContactsActivity {

    private TextView mTitle;

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
        mTitle = (TextView)findViewById(R.id.tv_setup_trusted_title);   
    }
    
    @Override
    protected void onInitLayout() {
        mTitle.setText(R.string.title_update_trusted_contact);
        mNextButton.setText(R.string.button_confirm);
        super.onInitLayout();
    }
    
    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mBinder.registerSensitiveActivity(this);
        super.onPplServiceConnected(saveInstanceState);
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
    protected void onButtonClicked(){
        
        mBinder.saveTustedNumberList(getNumberList(), PplService.EDIT_TYPE_UPDATE);
        gotoActivity(this, ControlPanelActivity.class);
        finish();
    }
}
