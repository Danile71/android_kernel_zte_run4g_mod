package com.mediatek.launcher3.plugin;

import android.content.Context;
import android.content.res.Resources;
import android.util.TypedValue;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.launcher3.ext.DefaultOperatorChecker;
import com.mediatek.launcher3.ext.LauncherLog;
import com.mediatek.op09.plugin.R;

public class Op09OperatorCheckerForLauncher3 extends DefaultOperatorChecker {
    private static final String TAG = "Op09OperatorCheckerForLauncher3";
    
    public Op09OperatorCheckerForLauncher3(Context context) {
        super(context);
    }

    @Override
    public boolean supportEditAndHideApps() {
        return true;
    }

    @Override
    public boolean supportAppListCycleSliding() {
        return true;
    }

    @Override
    public void customizeWorkSpaceIconText(TextView tv, float orgTextSize) {
        final Resources res = mContext.getResources();
        final int workspaceTextSize = (int) res.getDimension(R.dimen.launcher3_workspace_text_size);
        LauncherLog.d(TAG, "customizeWorkSpaceIconText: workspaceTextSize = " + workspaceTextSize);
        tv.setSingleLine(false);
        tv.setMaxLines(res.getInteger(R.integer.config_workspaceTextLine));
        tv.setTextSize(TypedValue.COMPLEX_UNIT_SP, workspaceTextSize);
    }

    @Override
    public void customizeCompoundPaddingForBubbleText(TextView tv, int orgPadding) {
        tv.setCompoundDrawablePadding(0);
    }
    
    @Override
    public void customizeFolderPreviewLayoutParams(LinearLayout.LayoutParams lp) {
        final Resources res = mContext.getResources();
        lp.bottomMargin = (int)res.getDimension(R.dimen.launcher3_folder_icon_preview_margin_bottom);
    }

    @Override
    public int customizeFolderCellHeight(int orgHeight) {
        final Resources res = mContext.getResources();
        return (int)res.getDimension(R.dimen.launcher3_folder_cell_width);
    }
}
