package com.mediatek.blemanager.ui;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;

import com.mediatek.blemanager.R;
import com.mediatek.blemanager.provider.BLEConstants;

public class ConnectProgressAlertDialog {

    private static final String TAG = BLEConstants.COMMON_TAG + "[ProgressAlertDialog]";
    
    private static AlertDialog sDialog;
    
    public static void show(Context context,/* String content,*/
            DialogInterface.OnClickListener cancelClickListener) {
        if (sDialog != null && sDialog.isShowing()) {
            sDialog.dismiss();
            sDialog = null;
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        View view = LayoutInflater.from(context).inflate(R.layout.progress_alert_dialog, null);
//        TextView contentView = (TextView)view.findViewById(R.id.dialog_content_text);
//        contentView.setText(content);
        builder.setView(view);
        builder.setCancelable(false);
        builder.setNegativeButton(R.string.cancel, cancelClickListener);
        sDialog = builder.create();
        sDialog.show();
        return ;
    }
    
    public static void dismiss() {
        if (sDialog != null && sDialog.isShowing()) {
            sDialog.dismiss();
        }
    }
    
}
