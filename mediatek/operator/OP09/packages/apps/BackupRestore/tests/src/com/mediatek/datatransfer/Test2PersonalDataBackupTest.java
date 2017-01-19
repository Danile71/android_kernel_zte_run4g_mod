package com.mediatek.datatransfer;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.content.Intent;
import android.os.Bundle;
import android.test.ActivityInstrumentationTestCase2;
import android.test.mock.MockDialogInterface;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.Toast;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.datatransfer.MainActivity;
import com.mediatek.datatransfer.PersonalDataBackupActivity;
import com.mediatek.datatransfer.PersonalDataRestoreActivity;
import com.mediatek.datatransfer.R;
import com.mediatek.datatransfer.utils.Constants.DialogID;


/**
 * Describe class BackupRestoreTest here.
 *
 *
 * Created: Sun Jul 15 15:01:46 2012
 *
 * @author <a href="mailto:mtk80359@mbjswglx259">Zhanying Liu (MTK80359)</a>
 * @version 1.0
 */
public class Test2PersonalDataBackupTest extends ActivityInstrumentationTestCase2<PersonalDataBackupActivity> {
	private static final String TAG = "PersonalDataBackupTest";
    private Solo mSolo = null;
    PersonalDataBackupActivity activity = null;
    /**
     * Creates a new <code>BackupRestoreTest</code> instance.
     *
     */
    public Test2PersonalDataBackupTest() {
        super(PersonalDataBackupActivity.class);
    }

    /**
     * Describe <code>setUp</code> method here.
     *
     * @exception Exception if an error occurs
     */
    public final void setUp() throws Exception {
        super.setUp();
        mSolo = new Solo(getInstrumentation(), getActivity());
        activity = getActivity();
        Log.d(TAG, "setUp");
    }

    /**
     * Describe <code>tearDown</code> method here.
     *
     * @exception Exception if an error occurs
     */
    public final void tearDown() throws Exception {
        //
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }

        if(activity != null){
        	 Log.d(TAG, "tearDown : activity = "+ activity);
        	activity.finish();
        	activity = null;
        }
        super.tearDown();
        Log.d(TAG, "tearDown");
        sleep(5000);
    }
    
    public void test3ConfigDialog(){
    	Log.d(TAG, "test1 : test for testConfigDialog");
    	boolean result = false;
    	sleep(500);
    	ArrayList<ListView> listViews = mSolo.getCurrentListViews();
    	sleep(500);
    	assertEquals(1, listViews.size());
    	sleep(500);
    	mSolo.clickOnView(listViews.get(0).getChildAt(0).findViewById(R.id.item_config));
    	mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN);
    	mSolo.sendKey(KeyEvent.KEYCODE_DPAD_CENTER);
    	mSolo.sendKey(KeyEvent.KEYCODE_DPAD_CENTER);
    	sleep(500);
    	result = mSolo.searchButton(getActivity().getString(R.string.btn_ok));
    	sleep(500);
    	assertTrue(result);
    	sleep(500);
    	mSolo.clickOnButton(getActivity().getString(R.string.btn_ok));
    	sleep(500);
    	if(listViews.get(0).getChildAt(1).isEnabled()){
    		mSolo.clickOnView(listViews.get(0).getChildAt(1).findViewById(R.id.item_config));
    		sleep(500);
    		mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN);
        	mSolo.sendKey(KeyEvent.KEYCODE_DPAD_CENTER);
        	mSolo.sendKey(KeyEvent.KEYCODE_DPAD_CENTER);
        	result = mSolo.searchButton(getActivity().getString(R.string.btn_ok));
        	sleep(500);
        	assertTrue(result);
        	sleep(500);
        	mSolo.clickOnButton(getActivity().getString(R.string.btn_ok));
        	sleep(500);
    	}
		mSolo.finishOpenedActivities();
    }
    
    /** 
     * Describe <code>test1</code> method here.
     * test for backup app_module
     */
    public void test1PersonalDataBackup(){
    	Log.d(TAG, "test1 : test for backup all personalData");
    	boolean result = false;
    	sleep(500);
        mSolo.clickOnCheckBox(0);
        sleep(1000);
        mSolo.clickOnCheckBox(0);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(200);
        int count = 0;
        while(count++ < 100) {
            result = mSolo.searchText(getActivity().getText(R.string.backup_result).toString());
            if(result) {
                break;
            }
            sleep(1000);
        }
        result = mSolo.searchText(getActivity().getText(R.string.result_fail).toString());
        assertFalse(result);
        sleep(2000);
        mSolo.clickOnButton(getActivity().getText(R.string.btn_ok).toString());
        sleep(5000);
    }
    /**
     * Describe <code>test3</code> method here.
     * test for backuping cancel
     */
    public void test2CancelPersonalData(){
    	Log.d(TAG, "test3 : test for backuping cancel");
    	boolean result = false;
    	mSolo.clickOnCheckBox(0);
        sleep(1000);
        mSolo.clickOnCheckBox(0);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(R.string.backuptosd).toString());
        sleep(1000);
        result = mSolo.searchText(getActivity().getText(R.string.edit_folder_name).toString());
        assertTrue(result);
        sleep(1000);
        mSolo.clickOnButton(getActivity().getText(android.R.string.ok).toString());
        sleep(2000);
        mSolo.goBack();
        sleep(3000);
        mSolo.clickOnText(getActivity().getText(android.R.string.ok).toString());
        sleep(200);
        result = mSolo.searchButton(getActivity().getText(R.string.cancel_backup_confirm).toString());
        sleep(500);
    }
    
    /** 
     * Describe <code>sleep</code> method here.
     * 
     * @param time a <code>int</code> value
     */
    public void sleep(int time) {
        try {
            Thread.sleep(time);
        } catch(InterruptedException e) {
        }
    }

}
