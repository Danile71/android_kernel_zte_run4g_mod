package com.discretix.drmassist;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.widget.MediaController;
import android.widget.Toast;
import android.widget.VideoView;

/**
 * Video playback activity
 */
public class VideoPlayer extends Activity implements OnErrorListener,
OnCompletionListener{
	
	private VideoView mVideoView;
	private AudioManager audio;
	private static int mLastPosition = 0;
	private boolean isInFront=true;
	
	private static final String contentToPlay = "content2Play";
	 
	/*
	 * (non-Javadoc)
	 * @see android.app.Activity#onCreate(android.os.Bundle)
	 */
	public void onCreate(Bundle savedInstanceState) {
		
		super.onCreate(savedInstanceState);
		audio = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
		Log.i("VideoPlayer", "Video playback activity created");
		setContentView(R.layout.video);
		mVideoView = (VideoView) findViewById(R.id.videoView1);

		final String path = this.getIntent().getExtras().getString(contentToPlay);
		
		mVideoView.setOnErrorListener(this);
		mVideoView.setOnCompletionListener(this);

		MediaController ctlr=new MediaController(this);
		//ctlr.setMediaPlayer(mVideoView);
		mVideoView.setMediaController(ctlr);

		mVideoView.setVideoPath(path);
		mVideoView.requestFocus();
		//// 
		//add by rui.hu
		final Bundle instanceState = savedInstanceState;
		mVideoView.getHolder().addCallback(new Callback() {
                     
                     public void surfaceDestroyed(SurfaceHolder holder) {
                           Log.d("Rui","surfaceDestroyed videoview stopPlayback");
                           // Stop playback ...
                           mVideoView.stopPlayback();
                     }
                     
                     public void surfaceCreated(SurfaceHolder holder) {
                           Log.d("Rui","surfaceCreated");
                           // Start/continue playback
                           if (null == instanceState){ 
			                           Log.d("Rui","surfaceCreated videoview start");
			                           mVideoView.start();
		                       } else if (isInFront){
			                           Log.d("Rui","surfaceCreated videoview resume");
			                           mVideoView.resume();
		                       }
                     }
                     
                     public void surfaceChanged(SurfaceHolder holder, int format, int width,
                                  int height) {
                           Log.d("Rui","surfaceChanged");
                           
                     }
              });
		
		/////
		/*
		if (null == savedInstanceState){
			mVideoView.start();
		} else if (isInFront){
			mVideoView.resume();
		}
		*/
		
	}

		
	/**
	 * play video file
	 * @param context current context
	 * @param file path to file to be played
	 */
	public static void play(Context context, String file){
		Bundle bundle = new Bundle();
		
		bundle.putString(contentToPlay, file);
		Intent videoPlaybackIntent = new Intent(context, VideoPlayer.class);
		videoPlaybackIntent.putExtras(bundle);
		context.startActivity(videoPlaybackIntent);
		
	}

	/* (non-Javadoc)
	 * @see android.app.Activity#onKeyDown(int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		switch (keyCode) {
			case KeyEvent.KEYCODE_VOLUME_UP:
				audio.adjustStreamVolume(AudioManager.STREAM_MUSIC,
										 AudioManager.ADJUST_RAISE, AudioManager.FLAG_SHOW_UI);
		        return true;
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				audio.adjustStreamVolume(AudioManager.STREAM_MUSIC,
										 AudioManager.ADJUST_LOWER, AudioManager.FLAG_SHOW_UI);
		        return true;
			case KeyEvent.KEYCODE_BACK: //If the back button was pushed
				onCompletion(null);
				return true;
			default:
		        return false;
		    }
	}

	public boolean onError(MediaPlayer arg0, int arg1, int arg2) {
		Log.e("VideoPlayer","Error ocured during video playback");
		Toast.makeText(this, "Error ocured during video playback", Toast.LENGTH_LONG).show();
		
		finish();
		return true;
	}

	public void onCompletion(MediaPlayer arg0) {
		Log.i("VideoPlayer","Playback has finished");
		mLastPosition = 0;
		Toast.makeText(this, "Playback has finished", Toast.LENGTH_LONG).show();

		// Finish should be called in order to exit the activity after playback
		// completes.
		finish();
	}

	@Override
	protected void onPause() {
		isInFront = false;
		if (mVideoView.isPlaying()){
			mLastPosition = mVideoView.getCurrentPosition();
		}
		
		//mVideoView.pause();
		Log.d("Rui","onPause videoview stopPlayback");
    // Stop playback ...
    mVideoView.stopPlayback();
		
		super.onPause();
	}

	@Override
	protected void onResume() {
		isInFront = true;
		if (mLastPosition != 0){
			mVideoView.seekTo(mLastPosition);
			mVideoView.start();
		}
		super.onResume();
	}
	



	
	
	
}