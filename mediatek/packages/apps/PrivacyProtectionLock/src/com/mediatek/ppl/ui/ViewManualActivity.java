package com.mediatek.ppl.ui;

import android.content.Intent;
import android.os.Bundle;
import android.view.MenuItem;
import android.widget.TextView;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class ViewManualActivity extends SetupManualActivity {
    
    private TextView mTitle;
    
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
        mTitle = (TextView)findViewById(R.id.tv_setup_manual_title);   
    }

    @Override
    protected void onInitLayout() {
        mTitle.setText(R.string.title_view_instruction);
        super.onInitLayout();
    }
    
    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mBinder.registerSensitiveActivity(ViewManualActivity.this);
        super.onPplServiceConnected(saveInstanceState);   
    }
    
    @Override
    public void onBackPressed() {
        gotoActivity(this, ControlPanelActivity.class);
        finish();
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
    public void onDestroy() {
        if (null != mBinder) {
            mBinder.unregisterSensitiveActivity(this);
        }
        super.onDestroy();
    }
    
    @Override
    protected void onConfirm() {
        
        gotoActivity(this, ControlPanelActivity.class);
        finish();
    }
}
