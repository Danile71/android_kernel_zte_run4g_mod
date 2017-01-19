package com.mediatek.miravision.ui;

import android.app.ActionBar;
import android.app.Fragment;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;

import java.io.IOException;

import com.mediatek.miravision.setting.MiraVisionJni;

public class DynamicContrastFragment extends Fragment implements
        CompoundButton.OnCheckedChangeListener, OnCompletionListener, OnPreparedListener,
        SurfaceHolder.Callback {

    private static final String TAG = "Miravision/DynamicContrastFragment";
    private static final String DYNAMIC_VIDEO_NAME = "dynamic_contrast.mp4";

    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private MediaPlayer mMediaPlayer;
    protected Switch mActionBarSwitch;
    protected String mVideoName;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.video_view, container, false);
        mSurfaceView = (SurfaceView) rootView.findViewById(R.id.surface_view);
        mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
        mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mVideoName = DYNAMIC_VIDEO_NAME;

        // Action bar
        mActionBarSwitch = (Switch) inflater.inflate(
                com.mediatek.internal.R.layout.imageswitch_layout, null);
        final int padding = getResources().getDimensionPixelSize(R.dimen.action_bar_switch_padding);
        mActionBarSwitch.setPaddingRelative(0, 0, padding, 0);
        getActivity().getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM);
        getActivity().getActionBar()
                .setCustomView(
                        mActionBarSwitch,
                        new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                                        | Gravity.END));
        mActionBarSwitch.setOnCheckedChangeListener(this);

        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        // For OverDriveFragment, it has updated way by own.
        if (!(this instanceof OverDriveFragment)) {
            updateStatus();
        }
    }

    @Override
    public void onDestroy() {
        getActivity().getActionBar().setCustomView(null);
        super.onDestroy();
    }

    private void updateStatus() {
        Log.d(TAG, "updateStatus");
        if (mActionBarSwitch != null) {
            mActionBarSwitch.setChecked(MiraVisionJni.getDynamicContrastIndex() == 1);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Log.d(TAG, "onCheckedChanged " + isChecked);
        MiraVisionJni.setDynamicContrastIndex(isChecked ? 1 : 0);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated " + holder);
        mSurfaceHolder = holder;
        initMediaPlayer();
        mMediaPlayer.setDisplay(mSurfaceHolder);
        prepareVideo();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d(TAG, "surfaceChanged " + holder);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "surfaceDestroyed " + holder);
        releaseMediaPlayer();
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        Log.d(TAG, "onPrepared ");
        mMediaPlayer.start();
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        Log.d(TAG, "onCompletion ");
        // cycle play
        mp.seekTo(0);
        mp.start();
    }

    /**
     * error lister, stop play video.
     */
    private OnErrorListener mErrorListener = new OnErrorListener() {
        @Override
        public boolean onError(MediaPlayer mp, int what, int extra) {
            Log.d(TAG, "play error: " + what);
            releaseMediaPlayer();
            return false;
        }
    };

    private void prepareVideo() {
        try {
            if (mMediaPlayer != null) {
                mMediaPlayer.reset();
                AssetFileDescriptor afd = getActivity().getAssets().openFd(mVideoName);
                Log.d(TAG, "video path = " + afd.getFileDescriptor());
                mMediaPlayer.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd
                        .getLength());
                afd.close();
                mMediaPlayer.prepare();
                Log.d(TAG, "mMediaPlayer prepare()");
            }
        } catch (IOException e) {
            Log.e(TAG, "unable to open file; error: " + e.getMessage(), e);
            releaseMediaPlayer();
        } catch (IllegalStateException e) {
            Log.e(TAG, "media player is in illegal state; error: " + e.getMessage(), e);
            releaseMediaPlayer();
        }
    }

    private void initMediaPlayer() {
        if (mMediaPlayer == null) {
            Log.d(TAG, "initMediaPlayer");
            // init media player
            mMediaPlayer = new MediaPlayer();
            mMediaPlayer.setOnCompletionListener(this);
            mMediaPlayer.setOnPreparedListener(this);
            mMediaPlayer.setOnErrorListener(mErrorListener);
        }
    }

    private void releaseMediaPlayer() {
        Log.d(TAG, "releaseMediaPlayer");
        if (mMediaPlayer != null) {
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }
}
