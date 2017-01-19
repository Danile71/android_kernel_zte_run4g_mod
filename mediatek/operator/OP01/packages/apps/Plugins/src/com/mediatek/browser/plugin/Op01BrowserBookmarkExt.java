package com.mediatek.browser.plugin;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.database.sqlite.SQLiteDatabase;
import android.provider.Browser;
import android.provider.BrowserContract;
import android.provider.BrowserContract.Bookmarks;
import android.provider.BrowserContract.Images;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.mediatek.browser.ext.DefaultBrowserBookmarkExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class Op01BrowserBookmarkExt extends DefaultBrowserBookmarkExt {

    private static final String TAG = "Op01BrowserBookmarkExt";

    private static final String ACTION_Add_BOOKMARK_FOLDER = "com.android.browser.ADD_BOOKMARK_FOLDER";
    private static final String TABLE_IMAGES = "images";
    private static final long FIXED_ID_ROOT = 1;

    private Context mContext;

    public Op01BrowserBookmarkExt(Context context) {
        super();
        mContext = context;
    }

    public int addDefaultBookmarksForCustomer(SQLiteDatabase db) {
        TypedArray preloads = null;
        Xlog.i(TAG, "Enter: " + "addDefaultBookmarksForCustomer" + " --OP01 implement" + db);
        Resources res = mContext.getResources();
        final CharSequence[] bookmarks = res.getTextArray(
                R.array.bookmarks_for_op01);
        preloads = mContext.getResources().obtainTypedArray(R.array.bookmark_preloads_for_op01);
        return addDefaultBookmarks(db, bookmarks, preloads);
    }

    public void createBookmarksPageOptionsMenu(Menu menu, MenuInflater inflater) {
        Xlog.i(TAG, "Enter: " + "createBookmarksPageOptionsMenu" + " --OP01 implement");
        menu.add(mContext.getResources().getString(R.string.add_new_bookmark));
        menu.add(mContext.getResources().getString(R.string.new_folder));
    }

    public boolean bookmarksPageOptionsMenuItemSelected(MenuItem item, Activity activity, long folderId) {
        Xlog.i(TAG, "Enter: " + "bookmarksPageOptionsMenuItemSelected" + " --OP01 implement");
        if (item.getTitle().equals(mContext.getResources().getString(R.string.add_new_bookmark))) {
            String url = "http://";
            String title = "";
            Intent intent = new Intent(Intent.ACTION_INSERT, Browser.BOOKMARKS_URI);
            intent.putExtra(BrowserContract.Bookmarks.TITLE, title);
            intent.putExtra(BrowserContract.Bookmarks.PARENT, folderId);
            intent.putExtra(BrowserContract.Bookmarks.URL, url);
            Xlog.i(TAG, "Add bookmark, CurrentFolderId: " + folderId);
            activity.startActivity(intent);
            return true;
        } else if (item.getTitle().equals(mContext.getResources().getString(R.string.new_folder))) {
            Intent intent = new Intent(ACTION_Add_BOOKMARK_FOLDER, Browser.BOOKMARKS_URI);
            intent.putExtra(BrowserContract.Bookmarks.PARENT, folderId);
            Xlog.i(TAG, "Add bookmark folder, CurrentFolderId: " + folderId);
            activity.startActivity(intent);
            return true;
        } else {
            return false;
        }
    }

    private int addDefaultBookmarks(SQLiteDatabase db,CharSequence[] bookmarks,TypedArray preloads) {
        Resources res = mContext.getResources();
        int size = bookmarks.length;
        Xlog.i(TAG, "bookmarks length = " + size);
        try {
            String parent = Long.toString(FIXED_ID_ROOT);
            String now = Long.toString(System.currentTimeMillis());
            for (int i = 0; i < size; i = i + 2) {
                CharSequence bookmarkDestination = bookmarks[i + 1];
                db.execSQL("INSERT INTO bookmarks (" +
                    Bookmarks.TITLE + ", " +
                    Bookmarks.URL + ", " +
                    Bookmarks.IS_FOLDER + "," +
                    Bookmarks.PARENT + "," +
                    Bookmarks.POSITION + "," +
                    Bookmarks.DATE_CREATED +
                    ") VALUES (" +
                    "'" + bookmarks[i] + "', " +
                    "'" + bookmarkDestination + "', " +
                    "0," +
                    "1," +
                    (Integer.toString(i)) + "," +
                    now +
                    ");");

                int faviconId = preloads.getResourceId(i, 0);
                int thumbId = preloads.getResourceId(i + 1, 0);
                byte[] thumb = null, favicon = null;
                try {
                    thumb = readRaw(res, thumbId);
                } catch (IOException e) {
                }
                try {
                    favicon = readRaw(res, faviconId);
                } catch (IOException e) {
                }
                if (thumb != null || favicon != null) {
                    ContentValues imageValues = new ContentValues();
                    imageValues.put(Images.URL, bookmarkDestination.toString());
                    if (favicon != null) {
                        imageValues.put(Images.FAVICON, favicon);
                    }
                    if (thumb != null) {
                        imageValues.put(Images.THUMBNAIL, thumb);
                    }
                    db.insert(TABLE_IMAGES, Images.FAVICON, imageValues);
                }
            }
        } catch (ArrayIndexOutOfBoundsException e) {
        } finally {
            preloads.recycle();
        }
        return size;
    }

    private byte[] readRaw(Resources res, int id) throws IOException {
        if (id == 0) {
            return null;
        }
        InputStream is = res.openRawResource(id);
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int read;
            while ((read = is.read(buf)) > 0) {
                bos.write(buf, 0, read);
            }
            bos.flush();
            return bos.toByteArray();
        } finally {
            is.close();
        }
    }

}
