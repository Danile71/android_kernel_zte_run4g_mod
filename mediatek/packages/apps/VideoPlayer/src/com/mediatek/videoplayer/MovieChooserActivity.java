/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2013. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.videoplayer;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.PixelFormat;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.BaseColumns;
import android.provider.MediaStore;
import android.provider.MediaStore.MediaColumns;
import android.provider.MediaStore.Video.Media;
import android.provider.MediaStore.Video.VideoColumns;
import android.text.Selection;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;

import com.mediatek.drm.OmaDrmStore;

public class MovieChooserActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "Videos/MovieChooserActivity";
    private static final boolean LOG = true;

    private static final Uri VIDEO_URI = Media.EXTERNAL_CONTENT_URI;
    private static final String[] PROJECTION = new String[]{
        BaseColumns._ID,
        MediaColumns.DISPLAY_NAME,
        VideoColumns.DATE_TAKEN,
        VideoColumns.DURATION,
        MediaColumns.MIME_TYPE,
        MediaColumns.DATA,
        MediaColumns.SIZE,
        Media.IS_DRM,
        MediaColumns.DATE_MODIFIED,
        MediaStore.Video.Media.TAGS,
    };
    private static final int INDEX_ID = 0;
    private static final int INDEX_DISPLAY_NAME = 1;
    private static final int INDEX_TAKEN_DATE = 2;
    private static final int INDEX_DRUATION = 3;
    private static final int INDEX_MIME_TYPE = 4;
    private static final int INDEX_DATA = 5;
    private static final int INDEX_FILE_SIZE = 6;
    private static final int INDEX_IS_DRM = 7;
    private static final int INDEX_DATE_MODIFIED = 8;
    private static final int INDEX_SUPPORT_3D = 9;
    
    private static final String ORDER_COLUMN =
        VideoColumns.DATE_TAKEN + " DESC, " + 
        BaseColumns._ID + " DESC ";
    
    private ListView mListView;
    private TextView mEmptyView;
    private ViewGroup mNoSdView;
    private MovieListAdapter mAdapter;
    
    private static final int MENU_DELETE_ALL = 1;
    private static final int MENU_DELETE_ONE = 2;
    private static final int MENU_PROPERTY = 3;
    private static final int MENU_DRM_DETAIL = 4;
    
    private static final String KEY_LOGO_BITMAP = "logo-bitmap";
    private static final String KEY_TREAT_UP_AS_BACK = "treat-up-as-back";
    private static final String EXTRA_ALL_VIDEO_FOLDER = "mediatek.intent.extra.ALL_VIDEO_FOLDER";
    private static final String EXTRA_ENABLE_VIDEO_LIST = "mediatek.intent.extra.ENABLE_VIDEO_LIST";
    private ProgressDialog mProgressDialog;
    private static String[] sExternalStoragePaths;
    
    private ThumbnailCache mThumbnailCache;
    private CachedVideoInfo mCachedVideoInfo;
    
    private String[] mInVideos;
    private String mSelection = null;
    private String[] mSelectionArgs = null;
    private int mMaxVideo = 8;

    // Save for show toast only once if click too much
    private Toast mMaxToast = null;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        MtkLog.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);

        Bundle bundle = null;
        if (null == savedInstanceState) {
            Intent intent = getIntent();
            bundle = intent.getExtras();
        } else {
            bundle = savedInstanceState;
        }
        if (bundle != null) {
            mInVideos = bundle.getStringArray("videos");
            mSelection = bundle.getString("selection", null);
            mSelectionArgs = bundle.getStringArray("selectionArgs");
            mMaxVideo = bundle.getInt("maxVideo");
            MtkLog.d(TAG, "onCreate bundle:" + bundle);
        } else {
            MtkLog.d(TAG, "onCreate bundle is null");
        }

        requestWindowFeature(Window.FEATURE_PROGRESS);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);

        setContentView(R.layout.movielist);
        final StorageManager storageManager =
            (StorageManager)getSystemService(Context.STORAGE_SERVICE);
        sExternalStoragePaths = storageManager.getVolumePaths();

        mListView = (ListView) findViewById(android.R.id.list);
        mEmptyView = (TextView) findViewById(android.R.id.empty);
        mNoSdView = (ViewGroup) findViewById(R.id.no_sdcard);
        mAdapter = new MovieListAdapter(this, R.layout.moviechooser_item, null, new String[]{}, new int[]{});
        mListView.setAdapter(mAdapter);
        mListView.setOnScrollListener(mAdapter);

        mListView.setOnItemClickListener(this);
        registerForContextMenu(mListView);
        registerStorageListener();
        refreshSdStatus(MtkUtils.isMediaMounted(MovieChooserActivity.this));
        
        mThumbnailCache = new ThumbnailCache(this);
        mThumbnailCache.addListener(mAdapter);
        mCachedVideoInfo = new CachedVideoInfo();
        MtkLog.v(TAG, "onCreate(" + savedInstanceState + ")");
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.moviechooser_actionbar, menu);
        return true;
    }
    
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem okMenu = menu.findItem(R.id.menu_ok);
        boolean enableOk = !mAdapter.isSelectEmpty();
        MtkLog.d(TAG, "onPrepareOptionsMenu set ok button enable:" + enableOk);
        okMenu.setEnabled(enableOk);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_cancel:
                setResult(RESULT_CANCELED);
                MtkLog.d(TAG, "onBackPressed videos:" + mAdapter.getSelectedVideos());
                finish();
                break;
            case R.id.menu_ok:
                Intent intent = new Intent();
                Bundle bundle = new Bundle();
                bundle.putStringArray("videos", mAdapter.getSelectedVideos());
                intent.putExtras(bundle);
                setResult(RESULT_OK, intent);
                MtkLog.d(TAG, "onBackPressed videos:" + mAdapter.getSelectedVideos());
                finish();
                break;
            default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putStringArray("videos", mAdapter.getSelectedVideos());
        outState.putString("selection", mSelection);
        outState.putStringArray("selectionArgs", mSelectionArgs);
        outState.putInt("maxVideo", mMaxVideo);
    }

    private void refreshMovieList() {
        MtkLog.d(TAG, "refreshMovieList() mSelection:" + mSelection + ", mSelectionArgs:" + mSelectionArgs);
        mAdapter.getQueryHandler().removeCallbacks(null);
        mAdapter.getQueryHandler().startQuery(0, null,
                VIDEO_URI,
                PROJECTION,
                mSelection,
                mSelectionArgs,
                ORDER_COLUMN);
    }
    
    private void registerStorageListener() {
        final IntentFilter iFilter = new IntentFilter();
        iFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        iFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        iFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        iFilter.addDataScheme("file");
        registerReceiver(mStorageListener, iFilter);
    }
    
    private final BroadcastReceiver mStorageListener = new BroadcastReceiver() {

        @Override
        public void onReceive(final Context context, final Intent intent) {
            MtkLog.v(TAG, "mStorageListener.onReceive(" + intent + ")");
            final String action = intent.getAction();
            if (Intent.ACTION_MEDIA_SCANNER_STARTED.equals(action)) {
                refreshSdStatus(MtkUtils.isMediaMounted(MovieChooserActivity.this));
            } else if (Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                refreshSdStatus(MtkUtils.isMediaMounted(MovieChooserActivity.this));
            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action) || Intent.ACTION_MEDIA_EJECT.equals(action)) {
                if (intent.hasExtra(StorageVolume.EXTRA_STORAGE_VOLUME)) {
                    final StorageVolume storage = (StorageVolume)intent.getParcelableExtra(
                            StorageVolume.EXTRA_STORAGE_VOLUME);
                    if (storage != null && storage.getPath().equals(sExternalStoragePaths[0])) {
                        refreshSdStatus(false);
                        mAdapter.changeCursor(null);
                    } // else contentObserver will listen it.
                    MtkLog.v(TAG, "mStorageListener.onReceive() eject storage="
                            + (storage == null ? "null" : storage.getPath()));
                }
            }
        };
    };

    private void refreshSdStatus(final boolean mounted) {
        MtkLog.v(TAG, "refreshSdStatus(" + mounted + ")");
        if (mounted) {
            if (MtkUtils.isMediaScanning(this)) {
                MtkLog.v(TAG, "refreshSdStatus() isMediaScanning true");
                showList();
                refreshMovieList(); // ALPS01539609 Need to refresh when scanning
            } else {
                MtkLog.v(TAG, "refreshSdStatus() isMediaScanning false");
                showList();
                refreshMovieList();
            }
        } else {
            showSdcardLost();
        }
    }

    private void showSdcardLost() {
        mListView.setVisibility(View.GONE);
        mEmptyView.setVisibility(View.GONE);
        mNoSdView.setVisibility(View.VISIBLE);
    }
    
    private void showList() {
        mListView.setVisibility(View.VISIBLE);
        mEmptyView.setVisibility(View.GONE);
        mNoSdView.setVisibility(View.GONE);
    }
    
    private void showEmpty() {
        mListView.setVisibility(View.GONE);
        mEmptyView.setVisibility(View.VISIBLE);
        mNoSdView.setVisibility(View.GONE);
    }

    @Override
    public void onItemClick(final AdapterView<?> parent, final View view, final int position, final long id) {
        final Object o = view.getTag();
        ViewHolder holder = null;
        if (o instanceof ViewHolder) {
            holder = (ViewHolder) o;
            mAdapter.updateCheckBox(holder);
            invalidateOptionsMenu();
        }

        MtkLog.v(TAG, "onItemClick(" + position + ", " + id + ") holder=" + holder);
    }

    @Override
    protected void onResume() {
        MtkLog.d(TAG, "onResume()");
        super.onResume();
        if (mAdapter != null) { //update drm icon
            mAdapter.notifyDataSetChanged();
        }
        mCachedVideoInfo.setLocale(Locale.getDefault());
    }

    @Override
    protected void onStart() {
        MtkLog.d(TAG, "onStart()");
        super.onStart();
    }

    @Override
    protected void onPause() {
        MtkLog.d(TAG, "onPause()");
        super.onPause();
    }

    @Override
    protected void onStop() {
        MtkLog.d(TAG, "onStop()");
        super.onStop();
    }
    
    @Override
    protected void onDestroy() {
        MtkLog.d(TAG, "onDestroy()");
        if (mAdapter != null) {
            mAdapter.clearCachedHolder();
            mAdapter.changeCursor(null);
        }
        if (mThumbnailCache != null) {
            mThumbnailCache.removeListener(mAdapter);
            mThumbnailCache.clear();
        }
        mCachedVideoInfo.setLocale(null);
        unregisterReceiver(mStorageListener);
        MtkUtils.releaseDrmManagerIfNeeded();
        super.onDestroy();
    }

    class MovieListAdapter extends SimpleCursorAdapter implements ThumbnailCache.ThumbnailStateListener, OnScrollListener {
        private static final String TAG = "Videos/MovieListAdapter";
        private final QueryHandler mQueryHandler;
        private final ArrayList<ViewHolder> mCachedHolder = new ArrayList<ViewHolder>();
        private static final String VALUE_IS_DRM = "1";
        private ArrayList<String> mItemChecked = new ArrayList<String>();

        QueryHandler getQueryHandler() {
            return mQueryHandler;
        }
        
        public MovieListAdapter(final Context context, final int layout, final Cursor c,
                final String[] from, final int[] to) {
            super(context, layout, c, from, to);
            mQueryHandler = new QueryHandler(getContentResolver());

            mItemChecked.clear();
            for (String path : mInVideos) {
                mItemChecked.add(path);
            }
        }
        
        public boolean isSelectEmpty() {
            return mItemChecked.isEmpty();
        }

        public String[] getSelectedVideos() {
            if (!mItemChecked.isEmpty()) {
                int size = mItemChecked.size();
                String[] videos = new String[size];
                for (int i = 0; i < size; i++) {
                    videos[i] = mItemChecked.get(i);
                }
                MtkLog.d(TAG, "getSelectedVideos() mItemChecked: " + mItemChecked);
                return videos;
            }
            return new String[0];
        }

        @Override
        public View newView(final Context context, final Cursor cursor, final ViewGroup parent) {
            final View view = super.newView(context, cursor, parent);
            final ViewHolder holder = new ViewHolder();
            holder.mIcon = (ImageView) view.findViewById(R.id.item_icon);
            holder.mTitleView = (TextView) view.findViewById(R.id.item_title);
            holder.mFileSizeView = (TextView) view.findViewById(R.id.item_date);
            holder.mDurationView = (TextView) view.findViewById(R.id.item_duration);
            int width = mThumbnailCache.getDefaultThumbnailWidth();
            int height = mThumbnailCache.getDefaultThumbnailHeight();
            holder.mFastDrawable = new FastBitmapDrawable(width, height);
            holder.mCheckBox = (CheckBox) view.findViewById(R.id.item_cb);
            view.setTag(holder);
            mCachedHolder.add(holder);
            MtkLog.v(TAG, "newView() mCachedHolder.size()=" + mCachedHolder.size());
            return view;
        }

        public void updateCheckBox(ViewHolder holder) {
            CheckBox cb = holder.mCheckBox;
            if (cb != null) {
                if (cb.isChecked()) {
                    cb.setChecked(false);
                    mItemChecked.remove(holder.mData);
                } else {
                    if (mItemChecked.size() < mMaxVideo) {
                        cb.setChecked(true);
                        mItemChecked.remove(holder.mData);
                        mItemChecked.add(holder.mData);
                    } else {
                        MtkLog.d(TAG, "updateCheckBox show maximum");
                        if (mMaxToast == null) {
                            mMaxToast = Toast.makeText(getApplicationContext(), 
                                    getResources().getString(R.string.max_video) + ": " +
                                    mMaxVideo, Toast.LENGTH_SHORT);
                        }
                        mMaxToast.show();
                    }
                }
            }
            MtkLog.d(TAG, "updateCheckBox: mItemChecked" + mItemChecked);
        }

        public void onChanged(final long rowId, final int type, final Bitmap drawable) {
            MtkLog.v(TAG, "onChanged(" + rowId + ", " + type + ", " + drawable + ")");
            for (final ViewHolder holder : mCachedHolder) {
                if (holder.mId == rowId) {
                    refreshThumbnail(holder);
                    break;
                }
            }
        }

        public void clearCachedHolder() {
            mCachedHolder.clear();
        }

        @Override
        public void bindView(final View view, final Context context, final Cursor cursor) {
            final ViewHolder holder = (ViewHolder) view.getTag();
            holder.mId = cursor.getLong(INDEX_ID);
            holder.mTitle = cursor.getString(INDEX_DISPLAY_NAME);
            holder.mDateTaken = cursor.getLong(INDEX_TAKEN_DATE);
            holder.mMimetype = cursor.getString(INDEX_MIME_TYPE);
            holder.mData = cursor.getString(INDEX_DATA);
            holder.mFileSize = cursor.getLong(INDEX_FILE_SIZE);
            holder.mDuration = cursor.getLong(INDEX_DRUATION);
            holder.mIsDrm = VALUE_IS_DRM.equals(cursor.getString(INDEX_IS_DRM));
            holder.mDateModified = cursor.getLong(INDEX_DATE_MODIFIED);
            holder.mSupport3D = false;

            holder.mTitleView.setText(holder.mTitle);
            holder.mFileSizeView.setText(mCachedVideoInfo.getFileSize(MovieChooserActivity.this, holder.mFileSize));
            holder.mDurationView.setText(mCachedVideoInfo.getDuration(holder.mDuration));

            refreshThumbnail(holder);

            if (mItemChecked != null && cursor != null) {
                String path =  cursor.getString(INDEX_DATA);
                if (mItemChecked.contains(path)) {
                    MtkLog.d(TAG, "bindView set checked true path = " + path);
                    holder.mCheckBox.setChecked(true);
                } else {
                    holder.mCheckBox.setChecked(false);
                }
            }

            MtkLog.v(TAG, "bindView() " + holder);
        }

        private void refreshThumbnail(final ViewHolder holder) {
            Bitmap bitmap = mThumbnailCache.getCachedThumbnail(holder.mId, holder.mDateModified,
                    holder.mSupport3D, !mFling);
            if (MtkUtils.isSupportDrm() && holder.mIsDrm) {
                bitmap = MtkUtils.overlayDrmIcon(MovieChooserActivity.this, holder.mData, OmaDrmStore.Action.PLAY, bitmap);
            }
            holder.mFastDrawable.setBitmap(bitmap);
            holder.mIcon.setImageDrawable(holder.mFastDrawable);
            holder.mIcon.invalidate();
        }

        @Override
        public void changeCursor(final Cursor c) {
            super.changeCursor(c);
        }

        @Override
        protected void onContentChanged() {
            super.onContentChanged();
            mQueryHandler.onQueryComplete(0, null, getCursor());
        }
        
        private void refreshCheckedItem(Cursor c) {
            if (c == null) {
                MtkLog.d(TAG, "refreshCheckedItem cursor is null");
                mItemChecked.clear();
            } else if (c.getCount() == 0) {
                MtkLog.d(TAG, "refreshCheckedItem cursor count is 0");
                mItemChecked.clear();
            } else {
                MtkLog.d(TAG, "refreshCheckedItem start refresh");
                // Check the video in mItemChecked if also in cursor,
                // if not in cursor, means delete it, then remove.
                ArrayList<String> allVideos = new ArrayList<String>();
                for (c.moveToFirst(); !c.isAfterLast(); c.moveToNext()) {
                    String data = c.getString(INDEX_DATA);
                    allVideos.add(data);
                }
                Iterator<String> integer = mItemChecked.iterator();
                while (integer.hasNext()) {
                    String video = integer.next();
                    if (!allVideos.contains(video)) {
                        MtkLog.d(TAG, "refreshCheckedItem remove:" + video);
                        integer.remove();
                    }
                }
            }
           
        }

        class QueryHandler extends AsyncQueryHandler {

            QueryHandler(final ContentResolver cr) {
                super(cr);
            }
            
            @Override
            protected void onQueryComplete(final int token, final Object cookie,
                    final Cursor cursor) {
                MtkLog.v(TAG, "onQueryComplete(" + token + "," + cookie + "," + cursor + ")");
                //MtkUtils.disableSpinnerState(MovieChooserActivity.this);
                if (cursor == null || cursor.getCount() == 0) {
                    showEmpty();
                    if (cursor != null) { //to observe database change
                        changeCursor(cursor);
                    }
                } else {
                    showList();
                    changeCursor(cursor);
                }
                if (cursor != null) {
                    MtkLog.v(TAG, "onQueryComplete() end");
                }
                refreshCheckedItem(getCursor());
                invalidateOptionsMenu();
            }
        }

        @Override
        public void onScroll(final AbsListView view, final int firstVisibleItem,
                final int visibleItemCount, final int totalItemCount) {
            
        }

        private boolean mFling = false;
        @Override
        public void onScrollStateChanged(final AbsListView view, final int scrollState) {
            switch (scrollState) {
            case OnScrollListener.SCROLL_STATE_IDLE:
                mFling = false;
                //notify data changed to load bitmap from mediastore.
                notifyDataSetChanged();
                break;
            case OnScrollListener.SCROLL_STATE_TOUCH_SCROLL:
                mFling = false;
                break;
            case OnScrollListener.SCROLL_STATE_FLING:
                mFling = true;
                break;
            default:
                break;
            }
            MtkLog.v(TAG, "onScrollStateChanged(" + scrollState + ") mFling=" + mFling);
        }
    }
    
    public class ViewHolder {
        long mId;
        String mTitle;
        String mMimetype;
        String mData;
        long mDuration;
        long mDateTaken;
        long mFileSize;
        boolean mIsDrm;
        long mDateModified;
        boolean mSupport3D;
        ImageView mIcon;
        TextView mTitleView;
        TextView mFileSizeView;
        TextView mDurationView;
        FastBitmapDrawable mFastDrawable;
        CheckBox mCheckBox;

        @Override
        public String toString() {
            return new StringBuilder()
                    .append("ViewHolder(mId=")
                    .append(mId)
                    .append(", mTitle=")
                    .append(mTitle)
                    .append(", mDuration=")
                    .append(mDuration)
                    .append(", mIsDrm=")
                    .append(mIsDrm)
                    .append(", mData=")
                    .append(mData)
                    .append(", mFileSize=")
                    .append(mFileSize)
                    .append(", mSupport3D=")
                    .append(mSupport3D)
                    .append(")")
                    .toString();
        }

        /**
         * just clone info
         */
        @Override
        protected ViewHolder clone() {
            final ViewHolder holder = new ViewHolder();
            holder.mId = mId;
            holder.mTitle = mTitle;
            holder.mMimetype = mMimetype;
            holder.mData = mData;
            holder.mDuration = mDuration;
            holder.mDateTaken = mDateTaken;
            holder.mFileSize = mFileSize;
            holder.mIsDrm = mIsDrm;
            holder.mDateModified = mDateModified;
            holder.mSupport3D = mSupport3D;
            return holder;
        }
    }
}
