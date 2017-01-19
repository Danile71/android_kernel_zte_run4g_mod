package com.mediatek.ppl.ui;

import android.content.Intent;
import android.os.Bundle;
import android.text.Html;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;
import com.mediatek.ppl.ui.DialogChooseSimFragment.IChooseSim;
import com.mediatek.ppl.ui.PplBasicActivity;

public class SetupManualActivity extends PplBasicActivity implements IChooseSim{

    private LinearLayout mLayoutUp;
    private LinearLayout mLayoutDown;
    private ProgressBar mProgressBar;
    private CheckBox    mCheckBox;
    private Button      mBtnNext;
    private TextView    mContent;
    
    @Override
    protected void onResume() {
        mCheckBox.setText(R.string.checkbox_send_sms);
        super.onResume();
    }
    
    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }
    
    @Override
    protected void onPrepareLayout() {        
     
        setContentView(R.layout.setup_manual);
        
        mLayoutUp = (LinearLayout)findViewById(R.id.layout_setup_manual_up);
        mLayoutDown = (LinearLayout)findViewById(R.id.layout_setup_manual_down);
        mProgressBar = (ProgressBar)findViewById(R.id.common_progress);        
        mCheckBox = (CheckBox)findViewById(R.id.cb_send_manual_sendSms);
        mBtnNext = (Button)findViewById(R.id.btn_bottom_next);
        mContent = (TextView)findViewById(R.id.tv_setup_manual_content);
        mContent.setText(Html.fromHtml(getString(R.string.content_setup_send_manual)));
        
                
        mCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener(){

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    mBtnNext.setText(R.string.button_send_sms);  
                } else {
                    mBtnNext.setText(R.string.button_finish);
                }
            }
            
        });
        
        mBtnNext.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                onButtonClicked();
            }            
        });
        
    }

    @Override
    protected void onInitLayout() {

        mProgressBar.setVisibility(View.VISIBLE);
        mLayoutUp.setVisibility(View.GONE);
        mLayoutDown.setVisibility(View.GONE);
        mCheckBox.setChecked(false);
        mBtnNext.setText(R.string.button_finish);
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mProgressBar.setVisibility(View.GONE);
        mLayoutUp.setVisibility(View.VISIBLE);
        mLayoutDown.setVisibility(View.VISIBLE);     
    }
    
    protected void onButtonClicked() {
        
        if (mCheckBox.isChecked()) {
            onConfirmWithMessage();
        } else {            
            onConfirm();
        }
    }
    
    protected void onConfirm() {
    
        mBinder.setProvision(true);
        mBinder.enable(true);
        
        gotoActivity(this, ControlPanelActivity.class);
        
        Intent intent = new Intent(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        sendBroadcast(intent);  
    }
    
    protected void onConfirmWithMessage() {
        if (PlatformManager.isAirplaneModeEnabled(this)) {
            DialogTurnoffAirplaneFragment frg = DialogTurnoffAirplaneFragment.newInstance();
            frg.show(getFragmentManager(), "turnOff_airplane");
        } else {
            int[] insertedSimSlots = mBinder.getInsertedSim();
            if (1 < insertedSimSlots.length) {
                String[] itemList = new String[insertedSimSlots.length];
                String itemTemplate = getResources().getString(R.string.item_sim_n);
                for (int i = 0; i < insertedSimSlots.length; ++i) {
                    itemList[i] = itemTemplate + i;
                }
                DialogChooseSimFragment frg = DialogChooseSimFragment.
                        newInstance(itemList, insertedSimSlots);
                frg.show(getFragmentManager(), "choose_sim");
            } else if (1 == insertedSimSlots.length) {
                onSimSelected(insertedSimSlots[0]);
            } else {
                Toast.makeText(this, R.string.toast_no_sim, Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public void onSimSelected(int simId) {
        onConfirm();
        mBinder.sendInstructionDescriptionMessage(simId);
    }    
}
