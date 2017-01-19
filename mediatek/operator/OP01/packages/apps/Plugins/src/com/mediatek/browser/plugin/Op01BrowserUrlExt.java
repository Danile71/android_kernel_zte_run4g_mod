package com.mediatek.browser.plugin;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.InputFilter;
import android.text.Spanned;

import com.mediatek.browser.ext.DefaultBrowserUrlExt;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

public class Op01BrowserUrlExt extends DefaultBrowserUrlExt {

    private static final String TAG = "Op01BrowserUrlExt";

    private Context mContext;

    public Op01BrowserUrlExt(Context context) {
        super();
        mContext = context;
    }

    public InputFilter[] checkUrlLengthLimit(final Context context) {
        Xlog.i(TAG, "Enter: " + "checkUrlLengthLimit" + " --OP01 implement");
        //Use to constraint the max word number that user can input.
        InputFilter[] contentFilters = new InputFilter[1];
        final int nLimit = mContext.getResources().getInteger(
            com.mediatek.internal.R.integer.max_input_browser_search_limit);
        contentFilters[0] = new InputFilter.LengthFilter(nLimit) {
            public CharSequence filter(CharSequence source, int start, int end,
                    Spanned dest, int dstart, int dend) {
                int keep = nLimit - (dest.length() - (dend - dstart));
                if (keep <= 0) {
                    showWarningDialog(context);
                    return "";
                } else if (keep >= end - start) {
                    return null;
                } else {
                    if (keep < source.length()) {
                        showWarningDialog(context);
                    }
                    return source.subSequence(start, start + keep);
                }
            }
        };
        return contentFilters;
    }

    public String checkAndTrimUrl(String url) {
        Xlog.i(TAG, "Enter: " + "checkAndTrimUrl" + " --OP01 implement");
        final int nLimit = mContext.getResources().getInteger(
        		com.mediatek.internal.R.integer.max_input_browser_search_limit);
        if (url != null && url.length() >= nLimit) {
            return url.substring(0, nLimit);
        }
        return url;
    }

    public String getNavigationBarTitle(String title, String url) {
        Xlog.i(TAG, "Enter: " + "getNavigationBarTitle" + " --OP01 implement");
        if (title == null || title.isEmpty()) {
            return url;
        }
        if (title.equals(mContext.getResources().getString(R.string.site_navigation))) {
            return url;
        } else {
            return title;
        }
    }

    public String getOverrideFocusContent(boolean hasFocus, String newContent, String oldContent, String url) {
        Xlog.i(TAG, "Enter: " + "getOverrideFocusContent" + " --OP01 implement");
        if (hasFocus) {
            if (url.startsWith("about:blank")){
                return "about:blank";
            } else {
                return url;
            }
        } else {
            return null;
        }
    }

    public String getOverrideFocusTitle(String title, String content) {
        Xlog.i(TAG, "Enter: " + "getOverrideFocusTitle" + " --OP01 implement");
        return title;
    }

    private AlertDialog mWarningDialog = null;
    private void showWarningDialog(Context context) {
        if (mWarningDialog == null) {
            mWarningDialog = new AlertDialog.Builder(context).create();
        }
        if (mWarningDialog != null && !mWarningDialog.isShowing()) {
            mWarningDialog.setTitle(mContext.getResources().getString(R.string.max_input_browser_search_title));
            mWarningDialog.setMessage(mContext.getResources().getString(R.string.max_input_browser_search));
            mWarningDialog.setButton(mContext.getResources().getString(R.string.max_input_browser_search_button),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        mWarningDialog.dismiss();
                        mWarningDialog = null;
                    }
                });
            mWarningDialog.show();
        }
    }

}
