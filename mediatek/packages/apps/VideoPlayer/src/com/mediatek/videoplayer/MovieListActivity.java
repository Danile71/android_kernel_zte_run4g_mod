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
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
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
import android.util.Log;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.AbsListView;
import android.widget.ActivityChooserView;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.ShareActionProvider;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.ShareActionProvider.OnShareTargetSelectedListener;

import com.mediatek.drm.OmaDrmStore;
import com.mediatek.videoplayer.SelectionManager.VideoItem;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Locale;

public class MovieListActivity extends Activity implements OnItemClickListener, OnItemLongClickListener {
    private static final String TAG = "Videos/MovieListActivity";
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
    };
    public static final int INDEX_ID = 0;
    public static final int INDEX_DISPLAY_NAME = 1;
    public static final int INDEX_TAKEN_DATE = 2;
    public static final int INDEX_DRUATION = 3;
    public static final int INDEX_MIME_TYPE = 4;
    public static final int INDEX_DATA = 5;
    public static final int INDEX_FILE_SIZE = 6;
    public static final int INDEX_IS_DRM = 7;
    public static final int INDEX_DATE_MODIFIED = 8;
    public static final int INDEX_SUPPORT_3D = 9;
    
    private static final String ORDER_COLUMN =
        VideoColumns.DATE_TAKEN + " DESC, " + 
        BaseColumns._ID + " DESC ";
    
    private ListView mListView;
    private TextView mEmptyView;
    private ViewGroup mNoSdView;
    private MovieListAdapter mAdapter;

    private static final String KEY_LOGO_BITMAP = "logo-bitmap";
    private static final String KEY_TREAT_UP_AS_BACK = "treat-up-as-back";
    private static final String EXTRA_ALL_VIDEO_FOLDER = "mediatek.intent.extra.ALL_VIDEO_FOLDER";
    private static final String EXTRA_ENABLE_VIDEO_LIST = "mediatek.intent.extra.ENABLE_VIDEO_LIST";
    private ProgressDialog mProgressDialog;
    private static String[] sExternalStoragePaths;
    
    private ThumbnailCache mThumbnailCache;
    private CachedVideoInfo mCachedVideoInfo;

    private Activity mActivity = null;
    private ActionMode mActionMode = null;
    public final ActionModeCallBack mActionModeCallBack = new ActionModeCallBack();
    private SelectionManager mSelectionManager = new SelectionManager();
    private ShareActionProvider mShareActionProvider = null;
    //the max count of files can be share,if too lager,the Binder will failed.
    private static final int MAX_SHARE_FILES_COUNT = 2000;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        MtkLog.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_PROGRESS);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);

        setContentView(R.layout.movielist);
        final StorageManager storageManager =
            (StorageManager)getSystemService(Context.STORAGE_SERVICE);
        sExternalStoragePaths = storageManager.getVolumePaths();

        mListView = (ListView) findViewById(android.R.id.list);
        mEmptyView = (TextView) findViewById(android.R.id.empty);
        mNoSdView = (ViewGroup) findViewById(R.id.no_sdcard);
        mAdapter = new MovieListAdapter(this, R.layout.movielist_item, null, new String[]{}, new int[]{});
        mListView.setAdapter(mAdapter);
        mListView.setOnScrollListener(mAdapter);
        
        mListView.setOnItemClickListener(this);
        //registerForContextMenu(mListView);
        registerStorageListener();
        refreshSdStatus(MtkUtils.isMediaMounted(MovieListActivity.this));
        
        mThumbnailCache = new ThumbnailCache(this);
        mThumbnailCache.addListener(mAdapter);
        mCachedVideoInfo = new CachedVideoInfo();

        // ActionMode
        mActivity = this;
        mListView.setOnItemLongClickListener(this);
        mSelectionManager.addListener(new SelectionManager.OnChangedListener() {
            @Override
            public void onChanged() {
                if (mActionMode != null) {
                    if (mShareActionProvider != null) {
                        mShareActionProvider.setShareIntent(getSelectIntent());
                    }
                    mActionModeCallBack.updateActionMode();
                }
            }
        });

        MtkLog.v(TAG, "onCreate(" + savedInstanceState + ")");
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.movielist_titlebar, menu);
        return true;
    }
    
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        boolean enableSelect = (mListView != null && View.VISIBLE == mListView.getVisibility());
        menu.findItem(R.id.menu_select_item).setEnabled(enableSelect);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_select_item:
                MtkLog.d(TAG, "click menu -> enter multi select mode");
                if (mActionMode != null) {
                    mActionMode.finish();
                }
                mActionMode = startActionMode(mActionModeCallBack);
                mSelectionManager.clearSelected();
                mActionModeCallBack.updateActionMode();
                break;
            default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }
    
    private void refreshMovieList() {
        mAdapter.getQueryHandler().removeCallbacks(null);
        mAdapter.getQueryHandler().startQuery(0, null,
                VIDEO_URI,
                PROJECTION,
                null,
                null,
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
                refreshSdStatus(MtkUtils.isMediaMounted(MovieListActivity.this));
            } else if (Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                refreshSdStatus(MtkUtils.isMediaMounted(MovieListActivity.this));
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
    
    private void showScanningProgress() {
        showProgress(getString(R.string.scanning), new OnCancelListener() {
            @Override
            public void onCancel(final DialogInterface dialog) {
                MtkLog.v(TAG, "mProgressDialog.onCancel()");
                hideScanningProgress();
                finish();
            }

        });
    }
    
    private void hideScanningProgress() {
        hideProgress();
    }

    private void showProgress(final String message, final OnCancelListener cancelListener) {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(this);
            mProgressDialog.setIndeterminate(true);
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setCancelable(cancelListener != null);
            mProgressDialog.setOnCancelListener(cancelListener);
            mProgressDialog.setMessage(message);
        }
        mProgressDialog.show();
    }

    private void hideProgress() {
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
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
        final Object obj = view.getTag();
        if (obj == null || !(obj instanceof ViewHolder)) {
            MtkLog.d(TAG, "onItemClick error. obj is:" + obj);
            return;
        }
        ViewHolder holder = (ViewHolder) obj;
        VideoItem item = new VideoItem();
        item.mId = holder.mId;
        item.mTitle = holder.mTitle;
        item.mIsDrm = holder.mIsDrm;
        item.mData = holder.mData;

        if (mActionMode != null) {
            MtkLog.d(TAG, "onItemClick select item id:" + holder.mId + ", path:" + holder.mData);
            mSelectionManager.onItemClick(item);
            mAdapter.notifyDataSetChanged();
            mActionModeCallBack.updateActionMode();
            mShareActionProvider.setShareIntent(getSelectIntent());
            //ViewHolder holder = mAdapter.getCachedHolderById(id);
            //MtkLog.d(TAG, "onPrepareActionMode holder:" + holder);
            // if select count is 0, finish ActionMode
            /*
            if (mSelectionManager.getCheckedItemsCount() == 0) {
                mActionMode.finish();
            }
            */
        } else {
            MtkLog.d(TAG, "onItemClick play a video id:" + holder.mId + ", path:" + holder.mData);
            final Intent intent = new Intent(Intent.ACTION_VIEW);
            String mime = "video/*";
            if (!(holder.mMimetype == null || "".equals(holder.mMimetype.trim()))) {
                mime = holder.mMimetype;
            }
            intent.setDataAndType(ContentUris.withAppendedId(VIDEO_URI, holder.mId), mime);
            intent.putExtra(EXTRA_ALL_VIDEO_FOLDER, true);
            intent.putExtra(KEY_TREAT_UP_AS_BACK, true);
            intent.putExtra(EXTRA_ENABLE_VIDEO_LIST, true);
            intent.putExtra(KEY_LOGO_BITMAP, BitmapFactory.decodeResource(getResources(), R.drawable.ic_video_app));
            try {
                startActivity(intent);
            } catch (final ActivityNotFoundException e) {
                e.printStackTrace();
            }
        }

        MtkLog.v(TAG, "onItemClick(" + position + ", " + id + ") holder=" + holder);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> arg0, View view,
            int arg2, long arg3) {
        final Object obj = view.getTag();
        if (obj == null || !(obj instanceof ViewHolder)) {
            MtkLog.d(TAG, "onItemLongClick error. obj is:" + obj);
            return false;
        }
        ViewHolder holder = (ViewHolder) obj;
        VideoItem item = new VideoItem();
        item.mId = holder.mId;
        item.mTitle = holder.mTitle;
        item.mIsDrm = holder.mIsDrm;
        item.mData = holder.mData;

        if (mActionMode == null) {
            MtkLog.d(TAG, "onItemLongClick select first item id:" + holder.mId + ", path:" + holder.mData);
            mSelectionManager.clearSelected();
            mSelectionManager.addSelectItem(item);
            mActionMode = startActionMode(mActionModeCallBack);
        } else {
            MtkLog.d(TAG, "onItemLongClick select other item id:" + holder.mId + ", path:" + holder.mData);
            mSelectionManager.onItemClick(item);
            // if select count is 0, finish ActionMode
            /*
            if (mSelectionManager.getCheckedItemsCount() == 0) {
                mActionMode.finish();
            }
            */
        }
        mActionModeCallBack.updateActionMode();
        mAdapter.notifyDataSetChanged();
        return true;
    }

    private void showDetail(final ViewHolder holder) {
        final DetailDialog detailDialog = new DetailDialog(this, holder);
        detailDialog.setTitle(R.string.media_detail);
        detailDialog.show();
    }

    public class DeleteTask extends AsyncTask<Void, Integer, Void> {
        private ViewHolder mHolder;
        private final ArrayList<Long> mDeleteVideos;
        private ProgressDialog mProgressDialog = null;
        private boolean mCanceled = false;

        public DeleteTask(final ArrayList<Long> videos) {
            int n = videos.size();
            mDeleteVideos = new ArrayList<Long>(videos);
            mProgressDialog = new ProgressDialog(mActivity);
            mProgressDialog.setMax(n);
            mProgressDialog.setTitle(mActivity.getResources().getString(R.string.delete));
            mProgressDialog.setCancelable(false);
            if (n > 1) {
                mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            } else if (n == 1) {
                long id = videos.get(0);
                ViewHolder holder = mAdapter.getCachedHolderById(id);
                if (holder != null) {
                    mProgressDialog.setMessage(holder.mTitle);
                }
                mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            } else {
                //error
            }
            mProgressDialog.setIndeterminate(false);
            mProgressDialog.setButton(mActivity.getResources().getString(android.R.string.cancel),
                    new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    MtkLog.d(TAG, "onClick cancel delete");
                    mCanceled = true;
                }
            });
            mProgressDialog.show();
        }

        @Override
        protected void onPreExecute() {
            //showDeleteProgress(getString(R.string.delete_progress, mHolder.mTitle));
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);
            if (mProgressDialog != null) {
                if (values.length > 0) {
                    mProgressDialog.setProgress(values[0]);
                }
            }
        }

        @Override
        protected void onPostExecute(final Void result) {
            //hideDeleteProgress();
            if (mProgressDialog != null) {
                mProgressDialog.dismiss();
                mProgressDialog = null;
            }
            if (mActionMode != null) {
                mActionMode.finish();
            }
        }

        // sleep to avoid high IO and make progress slow and can be canceled.
        private void useTime() {
            try {
                Thread.sleep(50);
            } catch (Exception e) {
                
            }
        }

        @Override
        protected Void doInBackground(final Void... params) {
            int n = mDeleteVideos.size();
            Iterator<Long> it = mDeleteVideos.iterator();
            int currentIndex = 0;
            while (it.hasNext()) {
                if (!mCanceled) {
                    useTime();
                    currentIndex++;
                    long id = it.next();
                    try {
                        // Delete the file first
                        deleteVideoFile(id);

                        getContentResolver().delete(ContentUris.withAppendedId(VIDEO_URI, id), null, null);
                        
                    } catch (final SQLiteException e) {
                        e.printStackTrace();
                    }
                    MtkLog.v(TAG, "DeleteTask.doInBackground delete id=" + id + ", currentIndex:" + currentIndex);
                    publishProgress(currentIndex);
                } else {
                    break;
                }
            }
            return null; 
        }

        private void deleteVideoFile(long id) {
            Uri uri = ContentUris.withAppendedId(VIDEO_URI, id);
            String path = getFilePathFromUri(uri);
            MtkLog.d(TAG, "doInBackground delete file path:" + path);
            if (path != null) {
                File file = new File(path);
                if (file.exists()) {
                    boolean ret = file.delete();
                    MtkLog.d(TAG, "doInBackground delete ret:" + ret);
                } else {
                    MtkLog.d(TAG, "doInBackground file not exist");
                }
            }
        }

        private String getFilePathFromUri(Uri uri) {
            String path = null;
            Cursor cursor = null;
            try {
                String[] projection = { MediaStore.Video.Media.DATA };
                cursor = getApplicationContext().getContentResolver().query(uri, projection,
                        null, null, null);
                if (cursor != null ) {
                    int index = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
                    cursor.moveToFirst();
                    path = cursor.getString(index);
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
            return path;
        }
    }

    @Override
    protected void onResume() {
        MtkLog.d(TAG, "onResume()");
        super.onResume();
        if (mAdapter != null) { //update drm icon
            mAdapter.notifyDataSetChanged();
        }
        mCachedVideoInfo.setLocale(Locale.getDefault());
        DrmManager.getInstance().init(mActivity);
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
        DrmManager.getInstance().release();
        MtkUtils.releaseDrmManagerIfNeeded();
        super.onDestroy();
    }

    public class MovieListAdapter extends SimpleCursorAdapter implements ThumbnailCache.ThumbnailStateListener, OnScrollListener {
        private static final String TAG = "MovieListAdapter";
        private final QueryHandler mQueryHandler;
        private final ArrayList<ViewHolder> mCachedHolder = new ArrayList<ViewHolder>();
        private static final String VALUE_IS_DRM = "1";
        private static final int THEME_COLOR_DEFAULT = 0x7F33b5e5;

        QueryHandler getQueryHandler() {
            return mQueryHandler;
        }

        public ViewHolder getCachedHolderById(long id) {
            for (final ViewHolder holder : mCachedHolder) {
                if (holder.mId == id) {
                    return holder;
                }
            }
            return null;
        }
        
        public MovieListAdapter(final Context context, final int layout, final Cursor c,
                final String[] from, final int[] to) {
            super(context, layout, c, from, to);
            mQueryHandler = new QueryHandler(getContentResolver());
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
            view.setTag(holder);
            mCachedHolder.add(holder);
            MtkLog.v(TAG, "newView() mCachedHolder.size()=" + mCachedHolder.size());
            return view;
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
            holder.mFileSizeView.setText(mCachedVideoInfo.getFileSize(MovieListActivity.this, holder.mFileSize));
            holder.mDurationView.setText(mCachedVideoInfo.getDuration(holder.mDuration));

            int color = Color.TRANSPARENT;
            if (mActionMode != null) {
                if (mSelectionManager.isSelected(holder.mId)) {
                    color = THEME_COLOR_DEFAULT;
                }
            }
            view.setBackgroundColor(color);
            
            refreshThumbnail(holder);
            MtkLog.v(TAG, "bindView() " + holder);
        }
        
        private void refreshThumbnail(final ViewHolder holder) {
            Bitmap bitmap = mThumbnailCache.getCachedThumbnail(holder.mId, holder.mDateModified,
                    holder.mSupport3D, !mFling);
            if (MtkUtils.isSupportDrm() && holder.mIsDrm) {
                bitmap = MtkUtils.overlayDrmIcon(MovieListActivity.this, holder.mData, OmaDrmStore.Action.PLAY, bitmap);
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
        
        
        
        class QueryHandler extends AsyncQueryHandler {

            QueryHandler(final ContentResolver cr) {
                super(cr);
            }
            
            @Override
            protected void onQueryComplete(final int token, final Object cookie,
                    final Cursor cursor) {
                MtkLog.v(TAG, "onQueryComplete(" + token + "," + cookie + "," + cursor + ")");
                //MtkUtils.disableSpinnerState(MovieListActivity.this);
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
                mSelectionManager.refreshCheckedItem(getCursor());

                //checkDrmSdType();
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

    protected class ActionModeCallBack implements ActionMode.Callback, OnMenuItemClickListener {
        private PopupMenu mSelectPopupMenu = null;
        private boolean mSelectedAll = true;
        private Button mTextSelect = null;

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            MtkLog.d(TAG, "onActionItemClicked: id " + item.getItemId());
            ArrayList<VideoItem> items = null;

            switch (item.getItemId()) {
            case R.id.select:
                mSelectionManager.setAllItemChecked(mSelectedAll);
                updateActionMode();
                invalidateOptionsMenu();
                break;
            case R.id.action_share:
                //Intent shareIntent = getSelectIntent();
                //mShareActionProvider.setShareIntent(shareIntent);
                break;
            case R.id.action_hotknot:
                hotknotShare();
                
                break;
            case R.id.action_delete:
                int count = mSelectionManager.getCheckedItemsCount();
                int resId = (count == 1) ? R.string.delete_selection_one : R.string.delete_selection_more;
                String confirmMsg = mActivity.getResources().getString(resId);

                MtkLog.d(TAG, "delete confirmMsg:" + confirmMsg + ", size:" + mSelectionManager.getCheckedItemsCount());

                new AlertDialog.Builder(mActivity)
                    //.setTitle(R.string.delete)
                    .setMessage(confirmMsg)
                    //.setCancelable(true)
                    .setPositiveButton(android.R.string.ok, new OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface dialog, final int which) {
                            ArrayList<VideoItem> items = mSelectionManager.getSelectItemList();
                            ArrayList<Long> videos = new ArrayList<Long>();
                            for (VideoItem item : items) {
                                videos.add(item.mId);
                            }
                            new DeleteTask(videos).execute();
                        }
                    })
                    .setNegativeButton(android.R.string.cancel, null)
                    .create()
                    .show();
                break;
            case R.id.action_detail:
                items = mSelectionManager.getSelectItemList();
                if (items.size() == 1) {
                    VideoItem firstItem = items.get(0);
                    if (firstItem != null) {
                        ViewHolder holder = mAdapter.getCachedHolderById(firstItem.mId);
                        if (holder != null) {
                            showDetail(holder.clone());
                        }
                    }
                }
                break;
            case R.id.action_drm_detail:
                items = mSelectionManager.getSelectItemList();
                if (items.size() == 1) {
                    VideoItem firstItem = items.get(0);
                    if (firstItem != null) {
                        MtkUtils.showDrmDetails(mActivity, firstItem.mData);
                    }
                }
                break;
            default:
                return false;
            }
            return true;
        }

        private void hotknotShare() {
            if (mActionMode == null) {
                MtkLog.w(TAG, "Maybe dispatch events twice, view mode error.");
                return;
            }

            ArrayList<VideoItem> items = mSelectionManager.getSelectItemList();
            int selectCount = items.size();
            if (selectCount >= 1) {

                ArrayList<Uri> uriList = new ArrayList<Uri>();
                for (int i = 0; i < selectCount; i++) {
                    VideoItem item = items.get(i);
                    if (item != null && item.mData != null) {
                        Uri uri = Uri.fromFile(new File(item.mData));
                        // the first one should add "?isMineType=no" to avoid play video JE
                        if (i== 0 && item.mIsDrm) {
                            if (item.mData != null) {
                                uri = Uri.parse("file://" + item.mData + "?isMineType=no");
                            }
                        }
                        uriList.add(uri);
                    }
                }

                int availableCount = uriList.size();
                Uri[] uris = new Uri[availableCount];
                for (int i = 0; i < availableCount; i++) {
                    uris[i] = uriList.get(i);
                    MtkLog.w(TAG, "hotknotShare uri:" + uriList.get(i));
                }

                Intent sIntent= new Intent();
                sIntent.setAction("com.mediatek.hotknot.action.SHARE");
                sIntent.putExtra("com.mediatek.hotknot.extra.SHARE_URIS", uris);
                try {
                    mActivity.startActivity(sIntent);
                    mActionMode.finish();
                } catch (ActivityNotFoundException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MtkLog.d(TAG, "onCreateActionMode start");
            LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View customView = layoutInflater.inflate(R.layout.action_mode, null);
            mode.setCustomView(customView);
            mTextSelect = (Button) customView.findViewById(R.id.selection_menu);
            MenuInflater menuInflater = mode.getMenuInflater();
            menuInflater.inflate(R.menu.movielist_menu, menu);

            MenuItem shareMenuItem = menu.findItem(R.id.action_share);
            MenuItem detailMenuItem = menu.findItem(R.id.action_detail);
            MenuItem hotKnotMenuItem = menu.findItem(R.id.action_hotknot);
            MenuItem drmDetailMenuItem = menu.findItem(R.id.action_drm_detail);
            drmDetailMenuItem.setTitle(com.mediatek.R.string.drm_protectioninfo_title);

            mShareActionProvider = (ShareActionProvider) shareMenuItem
                .getActionProvider();
            mShareActionProvider.setOnShareTargetSelectedListener(
                    mShareTargetSelectedListener);
            //mShareActionProvider.setShareHistoryFileName("share_history.xml");
            mShareActionProvider.setShareIntent(getSelectIntent());
            ((ActivityChooserView) shareMenuItem.getActionView()).setRecentButtonEnabled(false);

            mTextSelect.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mSelectPopupMenu == null) {
                        MtkLog.d(TAG, "onCreateActionMode.onClick mSelectPopupMenu is null");
                        mSelectPopupMenu = createSelectPopupMenu(mTextSelect);
                    } else {
                        updateSelectPopupMenu();
                        if (mActionMode != null) {
                            mActionMode.invalidate();
                        }
                    }
                    mSelectPopupMenu.show();
                }
            });
            mAdapter.notifyDataSetChanged();
            MtkLog.d(TAG, "onCreateActionMode end");
            return true;
        }

        private PopupMenu createSelectPopupMenu(View anchorView) {
            final PopupMenu popupMenu = new PopupMenu(getApplicationContext(), anchorView);
            popupMenu.inflate(R.menu.select_popup_menu);
            popupMenu.setOnMenuItemClickListener(this);
            return popupMenu;
        }
        
        private void updateSelectPopupMenu() {
            if (mSelectPopupMenu == null) {
                mSelectPopupMenu = createSelectPopupMenu(mTextSelect);
                return;
            }
            final Menu menu = mSelectPopupMenu.getMenu();
            int selectedCount = mSelectionManager.getCheckedItemsCount();
            if (mAdapter.getCount() == 0) {
                menu.findItem(R.id.select).setEnabled(false);
            } else {
                menu.findItem(R.id.select).setEnabled(true);
                if (mAdapter.getCount() != selectedCount) {
                    MtkLog.d(TAG, "updateSelectPopupMenu select_all");
                    menu.findItem(R.id.select).setTitle(R.string.select_all);
                    mSelectedAll = true;
                } else {
                    MtkLog.d(TAG, "updateSelectPopupMenu deselect_all");
                    menu.findItem(R.id.select).setTitle(R.string.deselect_all);
                    mSelectedAll = false;
                }
            }
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            if (mActionMode != null) {
                mActionMode = null;
            }
            if (mSelectPopupMenu != null) {
                mSelectPopupMenu.dismiss();
                mSelectPopupMenu = null;
            }
            mSelectionManager.clearSelected();
            mAdapter.notifyDataSetChanged();
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            int n = mSelectionManager.getCheckedItemsCount();
            menu.findItem(R.id.action_delete).setVisible(n != 0);
            menu.findItem(R.id.action_detail).setVisible(n == 1);
            menu.findItem(R.id.action_drm_detail).setVisible(MtkUtils.isSupportDrm() && selectOneIsDrm());

            // check drm sd type
            boolean drmCanShare = true;
            ArrayList<VideoItem> items = mSelectionManager.getSelectItemList();
            for (VideoItem item : items) {
                //MtkLog.d(TAG, "onPrepareActionMode item.mId:" + item.mId);
                if (item != null && item.mIsDrm && DrmManager.getInstance().isRightsStatus(item.mData)) {
                    drmCanShare = false;
                    break;
                }
            }
            menu.findItem(R.id.action_share).setVisible(n != 0 && drmCanShare);
            menu.findItem(R.id.action_hotknot).setVisible(n != 0 && drmCanShare && MtkUtils.isHotKnotSupport());
            MtkLog.d(TAG, "onPrepareActionMode drmCanShare:" + drmCanShare + ", n:" + n);
            return true;
        }

        private boolean selectOneIsDrm() {
            int count = mSelectionManager.getCheckedItemsCount();
            if (count == 1) {
                ArrayList<VideoItem> items = mSelectionManager.getSelectItemList();
                VideoItem item = items.get(0);
                if (item != null && item.mIsDrm) {
                    MtkLog.d(TAG, "selectOneIsDrm return true");
                    return true;
                }
            }
            MtkLog.d(TAG, "selectOneIsDrm return false");
            return false;
        }

        @Override
        public boolean onMenuItemClick(MenuItem item) {
            MtkLog.d(TAG, "onMenuItemClick: item:" + item + " id:" + item.getItemId());
            switch (item.getItemId()) {
                case R.id.select:
                    MtkLog.d(TAG, "updateSelectPopupMenu mSelectedAll:" + mSelectedAll);
                    if (mSelectedAll) {
                        mSelectionManager.setAllItemChecked(true);
                        if (mShareActionProvider != null) {
                            mShareActionProvider.setShareIntent(getSelectIntent());
                        }
                    } else {
                        mSelectionManager.setAllItemChecked(false);
                    }
                    mAdapter.notifyDataSetChanged();
                    updateActionMode();
                    invalidateOptionsMenu();
                    break;
                default:
                    return false;
            }
            return true;
        }

        public void updateActionMode() {
           updateSelectNumberText();

            mActionModeCallBack.updateSelectPopupMenu();
            if (mActionMode != null) {
                mActionMode.invalidate();
            }
        }
        
        private void updateSelectNumberText () {
            int selectedCount = mSelectionManager.getCheckedItemsCount();
            String selected = "";
            if(Locale.getDefault().getLanguage().equals("fr") && selectedCount > 1) {
                //try {
                //    selected = getResources().getString(R.string.mutil_selected);
                //} catch (Resources.NotFoundException e) {
                    selected = getResources().getString(R.string.selected);
                //}
            } else {
                selected = getResources().getString(R.string.selected);
            }
            selected = "" + selectedCount + " " + selected;
            mTextSelect.setText(selected);
        }
    }

    private final OnShareTargetSelectedListener mShareTargetSelectedListener =
        new OnShareTargetSelectedListener() {
            @Override
            public boolean onShareTargetSelected(
                    android.widget.ShareActionProvider source, Intent intent) {
                Log.e(TAG,"onShareTargetSelected:intent="+intent);
                return false;
            }
    };

    private Intent getSelectIntent() {
        ArrayList<VideoItem> items = mSelectionManager.getSelectItemList();
        int count = items.size();
        if (count == 0 ) {
            return null;
        }
        MtkLog.d(TAG, "getSelectIntent() count:" + count);

        Intent intent = null;
        if (count > 1) {
            intent = new Intent(Intent.ACTION_SEND_MULTIPLE);
            ArrayList<Uri> uris = new ArrayList<Uri>();
            for (VideoItem item : items) {
                Uri uri = ContentUris.withAppendedId(VIDEO_URI, item.mId);
                MtkLog.d(TAG, "getSelectIntent add uri:" + uri);
                uris.add(uri);
            }
            intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, uris);
        } else {
            VideoItem item = items.get(0);
            if (item != null) {
                Uri uri = ContentUris.withAppendedId(VIDEO_URI, items.get(0).mId);
                intent = new Intent(Intent.ACTION_SEND);
                intent.putExtra(Intent.EXTRA_STREAM, uri);
                MtkLog.d(TAG, "getSelectIntent uri:" + uri);
            }
        }
        intent.setType("video/*");
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        return intent;
    }
}
