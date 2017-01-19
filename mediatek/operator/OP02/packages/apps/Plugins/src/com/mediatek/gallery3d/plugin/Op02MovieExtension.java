package com.mediatek.gallery3d.plugin;

import android.content.Context;

import com.mediatek.gallery3d.ext.IMovieExtension;
import com.mediatek.gallery3d.ext.IMovieItem;
import com.mediatek.gallery3d.ext.DefaultMovieExtension;
import com.mediatek.gallery3d.ext.MovieUtils;
import com.mediatek.gallery3d.ext.IActivityHooker;
import com.mediatek.gallery3d.ext.IRewindAndForwardExtension;

import com.mediatek.gallery3d.plugin.MovieListHooker;
import com.android.gallery3d.app.MovieActivity;


import java.util.ArrayList;
import java.util.List;

public class Op02MovieExtension extends DefaultMovieExtension {
    private static final String TAG = "Op02MovieExtension";
    private static final boolean LOG = true;

    public Op02MovieExtension(Context context) {
        super(context);
    }

    @Override
    public boolean shouldEnableCheckLongSleep() {
        return false;
    }

    @Override
    public ArrayList<IActivityHooker> getHookers(Context context) {
        ArrayList<IActivityHooker> list = new ArrayList<IActivityHooker>();
        if (context instanceof MovieActivity) {
            list.add(new MovieListHooker(mContext));
        }
        list.add(new StepOptionSettingsHooker(mContext));
        return list;
    }

    @Override
    public IRewindAndForwardExtension getRewindAndForwardExtension() {
        return new RewindAndForward(mContext);
    }
}
