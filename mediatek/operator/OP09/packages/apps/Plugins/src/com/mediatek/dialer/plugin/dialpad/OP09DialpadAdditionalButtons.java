package com.mediatek.dialer.plugin.dialpad;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewConfiguration;
import android.widget.FrameLayout;
import android.widget.ImageButton;

import com.mediatek.op09.plugin.R;

import android.util.Log;

public class OP09DialpadAdditionalButtons extends FrameLayout {

    private static final String TAG = "OP09DialpadAdditionalButtons";

    private Context mHostContext;

    private int mEdgeButtonWidth;
    private int mDialButtonWidth;
    private int mButtonHeight;

    private boolean mLayouted = false;

    public OP09DialpadAdditionalButtons(Context pluginContext, Context hostContext) {
        //super(pluginContext, attrs);
        super(pluginContext);

        mHostContext = hostContext;

        Resources hostResource = hostContext.getResources();
        String hostPackageName = hostContext.getPackageName();

        mEdgeButtonWidth = pluginContext.getResources().getDimensionPixelSize(R.dimen.dialpad_additional_edge_button_width);
        mDialButtonWidth = pluginContext.getResources().getDimensionPixelSize(R.dimen.dialpad_additional_dial_button_width);
        mButtonHeight = hostResource.getDimensionPixelSize(hostResource.getIdentifier("dialpad_additional_button_height",
                                                           "dimen", hostPackageName));
        init();
    }

    @Override
    protected void onFinishInflate() {
        // TODO Auto-generated method stub
        super.onFinishInflate();
    }

    protected void init() {
        Resources resource = mHostContext.getResources();
        String packageName = mHostContext.getPackageName();

        ImageButton button = new ImageButton(getContext());
        button.setImageDrawable(resource.getDrawable(resource.getIdentifier("ic_menu_history_lt", "drawable", packageName)));
        button.setBackgroundResource(R.drawable.btn_call);
        button.setId(resource.getIdentifier("call_history_on_dialpad_button", "id", packageName));
        addView(button);

        button = new ImageButton(getContext());
        button.setImageResource(R.drawable.ic_dial_action_call);
        button.setBackgroundResource(R.drawable.btn_call);
        button.setId(R.id.dialButtonLeft);
        addView(button);

        button = new ImageButton(getContext());
        button.setImageResource(R.drawable.ic_dial_action_call);
        button.setBackgroundResource(R.drawable.btn_call);
        button.setId(R.id.dialButtonRight);
        addView(button);

        button = new ImageButton(getContext());
        button.setBackgroundResource(R.drawable.btn_call);
        if (ViewConfiguration.get(getContext()).hasPermanentMenuKey()) {
            button.setId(R.id.sendSMSButton);
            button.setImageResource(R.drawable.badge_action_sms);
        } else {
            button.setId(resource.getIdentifier("overflow_menu_on_dialpad", "id", packageName));
            button.setImageDrawable(resource.getDrawable(resource.getIdentifier("ic_menu_overflow_lt", "drawable", packageName)));
        }
        addView(button);

        setBackgroundColor(resource.getColor(resource.getIdentifier("dialpad_primary_text_color", "color", packageName)));
    }

    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        if (mLayouted) {
            return;
        }

        mLayouted = true;

        // call history button
        View child = getChildAt(0);
        child.layout(0, 0, mEdgeButtonWidth, mButtonHeight);

        // left dial button
        View leftDialButton = getChildAt(1);
        View rightDialButton = getChildAt(2);
        if (View.VISIBLE == rightDialButton.getVisibility()) {
            leftDialButton.layout(mEdgeButtonWidth, 0,
                    mEdgeButtonWidth + mDialButtonWidth, mButtonHeight);
        } else {
            leftDialButton.layout(mEdgeButtonWidth, 0,
                    mEdgeButtonWidth + (mDialButtonWidth << 1), mButtonHeight);
        }

        // right dial button
        if (View.VISIBLE == leftDialButton.getVisibility()) {
            rightDialButton.layout(mEdgeButtonWidth + mDialButtonWidth, 0,
                         mEdgeButtonWidth + (mDialButtonWidth << 1), mButtonHeight);
        } else {
            rightDialButton.layout(mEdgeButtonWidth, 0,
                    mEdgeButtonWidth + (mDialButtonWidth << 1), mButtonHeight);
        }

        // sms or flow menu button
        child = getChildAt(3);
        child.layout(mEdgeButtonWidth + (mDialButtonWidth << 1), 0,
                     (mEdgeButtonWidth << 1) + (mDialButtonWidth << 1), mButtonHeight);
    }

    public void hideLeftShowRightDialButton() {
        View leftDialButton = getChildAt(1);
        leftDialButton.setVisibility(View.GONE);

        View rightDialButton = getChildAt(2);
        rightDialButton.setVisibility(View.VISIBLE);
    }

    public void hideRightShowLeftDialButton() {
        View rightDialButton = getChildAt(2);
        rightDialButton.setVisibility(View.GONE);

        View leftDialButton = getChildAt(1);
        leftDialButton.setVisibility(View.VISIBLE);
    }

    public void showLeftRightDialButton() {

        View leftDialButton = getChildAt(1);
        leftDialButton.setVisibility(View.VISIBLE);

        View rightDialButton = getChildAt(2);
        rightDialButton.setVisibility(View.VISIBLE);
    }
}
