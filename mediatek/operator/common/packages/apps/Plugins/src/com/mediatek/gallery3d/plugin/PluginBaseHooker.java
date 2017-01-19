package com.mediatek.gallery3d.plugin;

import android.content.Context;

import com.mediatek.gallery3d.ext.DefaultActivityHooker;
import com.mediatek.gallery3d.video.IMoviePlayer;
import android.util.Log;

public class PluginBaseHooker extends DefaultActivityHooker {
    private static final String TAG = "Gallery2/VideoPlayer/PluginBaseHooker";
    private static final boolean LOG = true;
    protected Context mPluginContext;
    private IMoviePlayer mPlayer;

    public PluginBaseHooker() {
        super();
        mPluginContext = null;
    }
    
    public PluginBaseHooker(Context context) {
        super();
        mPluginContext = context;
    }

    @Override
    public void setParameter(String key, Object value) {
        super.setParameter(key, value);
        Log.i(TAG, "setParameter() value = " + value);
        if (value instanceof IMoviePlayer) {
            mPlayer = (IMoviePlayer) value;
            onMoviePlayerChanged(mPlayer);
            Log.i(TAG, "setParameter() mPlayer = " + mPlayer);
        }
    }

    public IMoviePlayer getPlayer() {
        return mPlayer;
    }

    public void onMoviePlayerChanged(final IMoviePlayer player){}
}
