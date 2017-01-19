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
import com.mediatek.datatransfer.BootActivity;
import com.mediatek.datatransfer.MainActivity;
import com.mediatek.datatransfer.PersonalDataBackupActivity;
import com.mediatek.datatransfer.PersonalDataRestoreActivity;
import com.mediatek.datatransfer.R;
import com.mediatek.datatransfer.utils.Constants.DialogID;


public class Test3MainActivityTest extends ActivityInstrumentationTestCase2<BootActivity> {
	private static final String TAG = "MainActivitytest";
    private Solo mSolo = null;
    BootActivity activity = null;
    /**
     * Creates a new <code>BackupRestoreTest</code> instance.
     *
     */
    public Test3MainActivityTest() {
        super(BootActivity.class);
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
    
    public void testMount(){
    	sleep(6000);
    	Intent mountIntent = new Intent("com.mediatek.autotest.unmount");
		getActivity().sendBroadcast(mountIntent);
		Log.d(TAG, "MainActivity unmount SDCard");
		sleep(6000);
		mountIntent = new Intent("com.mediatek.autotest.mount");
		getActivity().sendBroadcast(mountIntent);
		Log.d(TAG, "MainActivity mount SDCard");
		sleep(6000);
		mountIntent = new Intent("com.mediatek.SD_SWAP");
		getActivity().sendBroadcast(mountIntent);
		Log.d(TAG, "MainActivity SD_SWAP");
		sleep(6000);
		mSolo.finishOpenedActivities();
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
