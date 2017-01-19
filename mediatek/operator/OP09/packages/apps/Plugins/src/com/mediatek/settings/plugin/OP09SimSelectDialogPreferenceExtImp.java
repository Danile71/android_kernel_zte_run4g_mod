package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.Preference;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.mediatek.gemini.simui.ext.DefaultSimSelectDialogPreferenceExt;
import com.mediatek.op09.plugin.R;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.util.List;
/**
 * CT new indicator.
 */
public class OP09SimSelectDialogPreferenceExtImp extends DefaultSimSelectDialogPreferenceExt {
    /// M: CT dual sim feature defines that only two color supported: Blue and orange
    private static final int COLORNUM = 2;
    private Context mContext;

    public OP09SimSelectDialogPreferenceExtImp(Context baseContext) {
        super(baseContext);
        mContext = baseContext;
    }

    public void updateDefaultSimPreferenceLayout(Preference pref) {
        pref.setLayoutResource(R.layout.default_sim_preference);
    }

    @Override
    public View onCreateView(Preference pref, ViewGroup parent) {
        final LayoutInflater layoutInflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        final View layout = layoutInflater.inflate(pref.getLayoutResource(), parent, false);
        final ViewGroup widgetFrame = (ViewGroup) layout
                .findViewById(com.android.internal.R.id.widget_frame);
        if (widgetFrame != null) {
            if (pref.getWidgetLayoutResource() != 0) {
                layoutInflater.inflate(pref.getWidgetLayoutResource(), widgetFrame);
            } else {
                widgetFrame.setVisibility(View.GONE);
            }
        }
        return layout;
    }

    public void updateDefaultSimPreferenceSimIndicator(View viewContainer,
            List<SimInfoRecord> simInfos, int index) {
        int color = -1;
        if (simInfos != null && simInfos.size() > 0 && index >= 0
                && index < simInfos.size()) {
            SimInfoRecord simInfo = simInfos.get(index);
            if (simInfo != null) {
                color = simInfo.mColor;
            }
        }

        ImageView simIndicatorView = (ImageView) viewContainer.findViewById(R.id.sim_indicator);
        if (simIndicatorView != null && color != -1) {
            simIndicatorView.setImageResource(getSimColorResource(color));
            simIndicatorView.setVisibility(View.VISIBLE);
        } else {
            simIndicatorView.setVisibility(View.GONE);
        }
    }

    static int getSimColorResource(int color) {
        if ((color >= 0) && (color <= COLORNUM)) {
            return SimInfoManager.SimBackgroundDarkSmallRes[color];
        } else {
            return -1;
        }
    }
}
