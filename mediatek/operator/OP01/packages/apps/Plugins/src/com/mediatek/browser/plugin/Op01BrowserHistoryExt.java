package com.mediatek.browser.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.provider.Browser;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.mediatek.browser.ext.DefaultBrowserHistoryExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

public class Op01BrowserHistoryExt extends DefaultBrowserHistoryExt {

    private static final String TAG = "Op01BrowserHistoryExt";

    private int mClearMenuId;
    private Context mContext;
    
    public Op01BrowserHistoryExt(Context context) {
    	super();
    	mContext = context;
    }

    public void createHistoryPageOptionsMenu(Menu menu, MenuInflater inflater) {
        Xlog.i(TAG, "Enter: " + "createHistoryPageOptionsMenu" + " --OP01 implement");
        MenuItem clearMenu = menu.add(mContext.getResources().getString(R.string.clear_history));
        mClearMenuId = clearMenu.getItemId();
    }

    public void prepareHistoryPageOptionsMenuItem(Menu menu, boolean isNull, boolean isEmpty) {
        Xlog.i(TAG, "Enter: " + "prepareHistoryPageOptionsMenuItem" + " --OP01 implement");
        MenuItem clearMenu = menu.findItem(mClearMenuId);
        if (!isNull && !isEmpty) {
        	clearMenu.setEnabled(true);
        } else {
        	clearMenu.setEnabled(false);
        }
    }

    public boolean historyPageOptionsMenuItemSelected(MenuItem item, Activity activity) {
        Xlog.i(TAG, "Enter: " + "historyPageOptionsMenuItemSelected" + " --OP01 implement");
        if (item.getItemId() == mClearMenuId) {
            promptToClearHistory(activity);
            return true;
        } else {
            return false;
        }
    }

    static class ClearHistoryTask extends Thread {
        ContentResolver mResolver;

        public ClearHistoryTask(ContentResolver resolver) {
            mResolver = resolver;
        }

        @Override
        public void run() {
            Browser.clearHistory(mResolver);
            Browser.clearSearches(mResolver);
        }
    }

    private void promptToClearHistory(Activity activity) {
        final ContentResolver resolver = activity.getContentResolver();
        final ClearHistoryTask clear = new ClearHistoryTask(resolver);
        AlertDialog.Builder builder = new AlertDialog.Builder(activity)
                .setMessage(mContext.getResources().getString(R.string.clear_history_dlg))
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                     @Override
                     public void onClick(DialogInterface dialog, int which) {
                         if (which == DialogInterface.BUTTON_POSITIVE) {
                             if (!clear.isAlive()) {
                                 clear.start();
                             }
                         }
                     }
                });
        final Dialog dialog = builder.create();
        dialog.show();
    }

}
