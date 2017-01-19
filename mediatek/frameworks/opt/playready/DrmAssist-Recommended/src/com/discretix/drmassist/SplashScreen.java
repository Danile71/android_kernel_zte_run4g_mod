package com.discretix.drmassist;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

public class SplashScreen extends Activity {
	private Handler mHandler = new Handler();
	private final int mDelay = 3500; //msec
	private final Runnable mLauncher = new DrmAssistLauncher();
	private volatile Boolean isCannseled = false;
	
	private class DrmAssistLauncher implements Runnable{
		public void run() {
			if (isCannseled) {
				return;
			}
			Intent intent = new Intent(SplashScreen.this, FileNavigatorActivity.class);
			SplashScreen.this.startActivity(intent);
			SplashScreen.this.finish();
			
		}
	}
	
	@Override
	public void onBackPressed() {
		isCannseled = true;
		mHandler.removeCallbacks(mLauncher);
		Intent intent = new Intent(SplashScreen.this, FileNavigatorActivity.class);
		SplashScreen.this.startActivity(intent);
		SplashScreen.this.finish();
		
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		setContentView(R.layout.splashscreen);
		super.onCreate(savedInstanceState);
		mHandler.postDelayed(mLauncher, mDelay);
	}

}
