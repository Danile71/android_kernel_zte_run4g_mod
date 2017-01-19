package com.mediatek.aaltool;

import android.os.Bundle;
import android.provider.Settings;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Button;
import android.content.ContentResolver;
import android.content.SharedPreferences;
import android.content.Context;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.Map;

public class AALTuning extends Activity implements OnSeekBarChangeListener {
    private static final String TAG = "AALTool";
    private static final String PREFS_NAME = "aal";
    private static final String FILE_NAME = "aal.cfg";
    private SeekBar mBrightnessBar;
    private SeekBar mDarkeningSpeedBar;
    private SeekBar mBrighteningSpeedBar;
    private SeekBar mSmartBLStrengthBar;
    private SeekBar mSmartBLRangeBar;
    private SeekBar mReadabilityBar;
    private TextView mBrightnessText;
    private TextView mDarkeningSpeedText;
    private TextView mBrighteningSpeedText;
    private TextView mSmartBLStrengthText;
    private TextView mSmartBLRangeText;
    private TextView mReadabilityText;
    private Button mSaveButton;
    private SharedPreferences mPreferences;
    private int mBrightness = 255;
    private int mBrightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC;
    private int mPreBrightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;
    
    //the content resolver used as a handle to the system's settings  
    private ContentResolver mContentResolver;  
    //a window object, that will store a reference to the current window  
    private Window mWindow;

    private static class AALParameters {
        public int brightnessLevel;
        public int darkeningSpeedLevel;
        public int brighteningSpeedLevel;
        public int smartBacklightStrength;
        public int smartBacklightRange;
        public int readabilityLevel;

        public AALParameters() {
            brightnessLevel = R.integer.MAX_BRIGHTNESS_LEVEL / 2;
            darkeningSpeedLevel = R.integer.MAX_DARKENING_SPEED_LEVEL / 2;
            brighteningSpeedLevel = R.integer.MAX_BRIGHTENING_SPEED_LEVEL / 2;
            smartBacklightStrength = R.integer.MAX_SMARTBACKLIGHT_STRENGTH / 2;
            smartBacklightRange = R.integer.MAX_SMARTBACKLIGHT_RANGE / 2;
            readabilityLevel = R.integer.MAX_READABILITY_LEVEL / 2;
        }
    }
    
    private AALParameters mParam;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.aal_tuning);        
        
        Log.d(TAG, "onCreate...");
        
        
        //get the content resolver  
        mContentResolver = getContentResolver();
        //get the current window  
        mWindow = getWindow();
        try {
             mPreBrightnessMode = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, mBrightnessMode);
		
        
        mSaveButton = (Button)findViewById(R.id.buttonSave);
        mSaveButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                saveToFile();
            }
        });
        
        mPreferences = getSharedPreferences(PREFS_NAME, 0);
        loadPreference();

        mBrightnessText = (TextView) this.findViewById(R.id.textBrightnessLevel); 
        mBrightnessText.setText(Integer.toString(mParam.brightnessLevel));
        mBrightnessBar = (SeekBar)findViewById(R.id.seekBarBrightness); // make seekbar object
        mBrightnessBar.setOnSeekBarChangeListener(this); // set seekbar listener.
        
        mDarkeningSpeedText = (TextView) this.findViewById(R.id.textDarkeningSpeedLevel); 
        mDarkeningSpeedText.setText(Integer.toString(mParam.darkeningSpeedLevel));
        mDarkeningSpeedBar = (SeekBar)findViewById(R.id.seekBarDarkeningSpeed); // make seekbar object
        mDarkeningSpeedBar.setOnSeekBarChangeListener(this); // set seekbar listener. 
        
        mBrighteningSpeedText = (TextView) this.findViewById(R.id.textBrighteningSpeedLevel); 
        mBrighteningSpeedText.setText(Integer.toString(mParam.brighteningSpeedLevel));
        mBrighteningSpeedBar = (SeekBar)findViewById(R.id.seekBarBrighteningSpeed); // make seekbar object
        mBrighteningSpeedBar.setOnSeekBarChangeListener(this); // set seekbar listener.       

        mSmartBLStrengthText = (TextView) this.findViewById(R.id.textSmartBLStrengthLevel); 
        mSmartBLStrengthText.setText(Integer.toString(mParam.smartBacklightStrength));
        mSmartBLStrengthBar = (SeekBar)findViewById(R.id.seekBarSmartBLStrength); // make seekbar object
        mSmartBLStrengthBar.setOnSeekBarChangeListener(this); // set seekbar listener.

        mSmartBLRangeText = (TextView) this.findViewById(R.id.textSmartBLRangeLevel); 
        mSmartBLRangeText.setText(Integer.toString(mParam.smartBacklightRange));
        mSmartBLRangeBar = (SeekBar)findViewById(R.id.seekBarSmartBLRange); // make seekbar object
        mSmartBLRangeBar.setOnSeekBarChangeListener(this); // set seekbar listener.

        mReadabilityText = (TextView) this.findViewById(R.id.textReadabilityLevel); 
        mReadabilityText.setText(Integer.toString(mParam.readabilityLevel));
        mReadabilityBar = (SeekBar)findViewById(R.id.seekBarReadability); // make seekbar object
        mReadabilityBar.setOnSeekBarChangeListener(this); // set seekbar listener.

        mBrightnessBar.setProgress(mParam.brightnessLevel);
        mDarkeningSpeedBar.setProgress(mParam.darkeningSpeedLevel);
        mBrighteningSpeedBar.setProgress(mParam.brighteningSpeedLevel);
        mSmartBLStrengthBar.setProgress(mParam.smartBacklightStrength);
        mSmartBLRangeBar.setProgress(mParam.smartBacklightRange);
        mReadabilityBar.setProgress(mParam.readabilityLevel);
    }

    private void setScreenBrightness() {
        try {
             mBrightness = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS);
        } catch (Exception e) {
            e.printStackTrace();
        }
        nSetBacklight(mBrightness);
    }
    
    private void loadPreference() {
        mParam = new AALParameters();
    
        Map<String,?> keys = mPreferences.getAll();
        for(Map.Entry<String,?> entry : keys.entrySet()) {
            Log.d(TAG, "map values " + entry.getKey() + ": " + entry.getValue().toString());
            int value = Integer.parseInt(entry.getValue().toString());
            if (entry.getKey().equals("Brightness"))
                mParam.brightnessLevel = value;
            if (entry.getKey().equals("DarkeningSpeed")) 
                mParam.darkeningSpeedLevel = value;
            if (entry.getKey().equals("BrighteningSpeed")) 
                mParam.brighteningSpeedLevel = value;
            if (entry.getKey().equals("SmartBacklightStrength")) 
                mParam.smartBacklightStrength = value;
            if (entry.getKey().equals("SmartBacklightRange")) 
                mParam.smartBacklightRange = value;
            if (entry.getKey().equals("Readability"))
                mParam.readabilityLevel = value;
        }
        
        // Get latest parameters from AALService
        nGetParameters(mParam);
    }
    private void saveToFile() {
        try {
            
            FileOutputStream fos = openFileOutput(FILE_NAME, Context.MODE_PRIVATE);
            PrintWriter pw = new PrintWriter(fos); 
            pw.println("Brightness=" + mParam.brightnessLevel);
            pw.println("DarkeningSpeed=" + mParam.darkeningSpeedLevel);
            pw.println("BrighteningSpeed=" + mParam.brighteningSpeedLevel);
            pw.println("SmartBacklightStrength=" + mParam.smartBacklightStrength);
            pw.println("SmartBacklightRange=" + mParam.smartBacklightRange);
            pw.println("Readability=" + mParam.readabilityLevel);
            pw.close();
            fos.close();
        } catch(Exception e){
            e.printStackTrace();
        }
    }
    	
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.aal_tuning, menu);
        return true;
    }

    @Override
    public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {        
        String key = "";
        if (arg0 == mBrightnessBar) {
            Log.d(TAG, "Brightness level = " + arg1);
            if (nSetBrightnessLevel(arg1)) {
                key = "Brightness";
                mParam.brightnessLevel = arg1;
                mBrightnessText.setText(Integer.toString(mParam.brightnessLevel));
            }
        }
        if (arg0 == mDarkeningSpeedBar) {
            Log.d(TAG, "set Darkening Speed level = " + arg1);
            if (nSetDarkeningSpeedLevel(arg1)) {
                key = "DarkeningSpeed";
                mParam.darkeningSpeedLevel = arg1;
                mDarkeningSpeedText.setText(Integer.toString(mParam.darkeningSpeedLevel));
            }
        }
        if (arg0 == mBrighteningSpeedBar) {
            Log.d(TAG, "set Brightening Speed level = " + arg1);
            if (nSetBrighteningSpeedLevel(arg1)) {
                key = "BrighteningSpeed";
                mParam.brighteningSpeedLevel = arg1;
                mBrighteningSpeedText.setText(Integer.toString(mParam.brighteningSpeedLevel));
            }
        }
        if (arg0 == mSmartBLStrengthBar) {
            Log.d(TAG, "set SmartBacklight strength = " + arg1);
            if (nSetSmartBacklightStrength(arg1)) {
                key = "SmartBacklightStrength";
                mParam.smartBacklightStrength = arg1;
                mSmartBLStrengthText.setText(Integer.toString(mParam.smartBacklightStrength));
            }
        }
        if (arg0 == mSmartBLRangeBar) {
            Log.d(TAG, "set SmartBacklight range = " + arg1);
            if (nSetSmartBacklightRange(arg1)) {
                key = "SmartBacklightRange";
                mParam.smartBacklightRange = arg1;
                mSmartBLRangeText.setText(Integer.toString(mParam.smartBacklightRange));
            }
        }
        if (arg0 == mReadabilityBar) {
            Log.d(TAG, "set Readability level = " + arg1);
            if (nSetReadabilityLevel(arg1)) {
                key = "Readability";
                mParam.readabilityLevel = arg1;
                mReadabilityText.setText(Integer.toString(mParam.readabilityLevel));
            }
        }
		
        setScreenBrightness();

        if (key.length() > 0) {
            SharedPreferences.Editor editor = mPreferences.edit();
            editor.putInt(key, arg1);
            editor.commit();
        }

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
        
    }
    
    @Override
    protected void onResume() {
        Log.d(TAG, "onResume..., restore brightness mode to " + mBrightnessMode);
        try {
             mPreBrightnessMode = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, mBrightnessMode);
		
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause..., restore brightness mode to " + mPreBrightnessMode);
        Settings.System.putInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, mPreBrightnessMode);
        super.onPause();
    }
	
    static {
        System.loadLibrary("aaltool_jni");
    }

    private native boolean nSetBrightnessLevel(int level);
    private native boolean nSetDarkeningSpeedLevel(int level);
    private native boolean nSetBrighteningSpeedLevel(int level);
    private native boolean nSetSmartBacklightStrength(int level);
    private native boolean nSetSmartBacklightRange(int level);
    private native boolean nSetReadabilityLevel(int level);
    private native boolean nSetBacklight(int level);
    private native void nGetParameters(AALParameters param);
}
