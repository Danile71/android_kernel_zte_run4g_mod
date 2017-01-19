package com.mediatek.dialer.plugin.dialpad;

import android.app.Activity;
import android.app.Fragment;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;

import com.mediatek.dialer.ext.DialPadExtension;
import com.mediatek.dialer.ext.IDialpadFragment;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.phone.Constants;
import com.mediatek.calloption.plugin.OP09CallOptionUtils;
import com.mediatek.op09.plugin.R;
import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.telephony.TelephonyManagerEx;

public class DialPadOP09Extension extends DialPadExtension implements View.OnClickListener {
 
    private static final String TAG = "DialPadOP09Extension";

    private static final String ID = "id";
    private static final String ID_NAME_IP_DIAL_OPTION_MENU = "menu_ip_dial";
    private static final String ID_NAME_TOP = "top";
    private static final String ID_NAME_DIALPAD_ADDITIONAL_BUTTONS = "dialButtonContainer";
    private static final String CDMA_INFO_SPECIFICATION_ACTION = "com.mediatek.phone.plugin.CdmaInfoSpecification";
    private static final String PRL_VERSION_DISPLAY = "*#0000#";

    private Fragment mFragment;
    private IDialpadFragment mDialpadFragment;
    private Context mPluginContext;

    private ImageButton mDialButtonLeft;
    private ImageButton mDialButtonRight;
    private ImageButton mSendSMSButton;
    private EditText mDigits;
    private ListView mDialpadChooser;
    private OP09DialpadAdditionalButtons mDialpadAdditionalButtons;

    private BroadcastReceiver mReceiver;

    public DialPadOP09Extension(Context pluginContext) {
        mPluginContext = pluginContext;
    }

    public void onCreate(Fragment fragment, IDialpadFragment dialpadFragment) {
        mFragment = fragment;
        mDialpadFragment = dialpadFragment;

        mReceiver = new DialpadBroadcastReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_VOICE_CALL_DEFAULT_SIM_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        mFragment.getActivity().registerReceiver(mReceiver, intentFilter);
    }

    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedState, View fragmentView) {
        log("onCreateView()");

        Resources resource = mFragment.getActivity().getResources();
        String packageName = mFragment.getActivity().getPackageName();
        LinearLayout dialerLayout =
                (LinearLayout) fragmentView.findViewById(resource.getIdentifier(ID_NAME_TOP, ID, packageName));

        if (null != dialerLayout) {
            View dialpadAdditionalButtons =
                        dialerLayout.findViewById(
                                resource.getIdentifier(ID_NAME_DIALPAD_ADDITIONAL_BUTTONS, ID, packageName));
            log("dialpadAdditionalButtons = " + dialpadAdditionalButtons);
            if (null != dialpadAdditionalButtons) {
            int indexOfDialpadAdditionalButtons = ((ViewGroup)fragmentView).indexOfChild(dialpadAdditionalButtons);
            ViewGroup.LayoutParams dialpadAdditionalButtonsLayoutParams =
                    (ViewGroup.LayoutParams) dialpadAdditionalButtons.getLayoutParams();
            dialerLayout.removeView(dialpadAdditionalButtons);

                mDialpadAdditionalButtons = new OP09DialpadAdditionalButtons(mPluginContext, mFragment.getActivity());
                mDialpadAdditionalButtons.setId(
                        resource.getIdentifier(ID_NAME_DIALPAD_ADDITIONAL_BUTTONS, ID, packageName));
                mDialButtonLeft = (ImageButton)mDialpadAdditionalButtons.findViewById(R.id.dialButtonLeft);
                if (null != mDialButtonLeft) {
                    mDialButtonLeft.setOnClickListener(this);
                }
                mDialButtonRight = (ImageButton)mDialpadAdditionalButtons.findViewById(R.id.dialButtonRight);
                if (null != mDialButtonRight) {
                    mDialButtonRight.setOnClickListener(this);
                }
                mSendSMSButton = (ImageButton)mDialpadAdditionalButtons.findViewById(R.id.sendSMSButton);
                if (null != mSendSMSButton) {
                    mSendSMSButton.setOnClickListener(this);
                }
                if (null != mDialpadAdditionalButtons) {
                    ((ViewGroup)fragmentView).addView(mDialpadAdditionalButtons, indexOfDialpadAdditionalButtons,
                                                      dialpadAdditionalButtonsLayoutParams);
                }
            }
        }
        mDigits = (EditText) fragmentView.findViewById(resource.getIdentifier("digits", ID, packageName));
        mDialpadChooser = (ListView) fragmentView.findViewById(
                resource.getIdentifier("dialpadChooser", ID, packageName));
        updateNumberButtons(fragmentView);
        updateDialButtons();
        return fragmentView;
    }

    public void onDestroy() {
        if (null != mReceiver && null != mFragment) {
            mFragment.getActivity().unregisterReceiver(mReceiver);
        }
        mFragment = null;
    }

    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        /// !!!! add same condition as that in DialpadFragement
        if (ViewConfiguration.get(mFragment.getActivity()).hasPermanentMenuKey()) {
            createMenu(menu);
        }
    }

    public void onPrepareOptionsMenu(Menu menu) {
        if (ViewConfiguration.get(mFragment.getActivity()).hasPermanentMenuKey()) {
            setupMenu(menu);
        }
    }

    public void constructPopupMenu(PopupMenu popupMenu, View anchorView, Menu menu) {
        createMenu(menu);
        setupMenu(menu);
    }

    private void createMenu(Menu menu) {
        menu.add(0, R.id.menu_ip_call_slot_1, 0, mPluginContext.getString(R.string.call_ip_call_slot_1));
        menu.add(0, R.id.menu_ip_call_slot_2, 0, mPluginContext.getString(R.string.call_ip_call_slot_2));
        menu.add(0, R.id.menu_eSufing_dialing, 1, mPluginContext.getString(R.string.menu_eSurfing_dialing)).setCheckable(true);
    }

    private void setupMenu(Menu menu) {
        Resources resource = mFragment.getActivity().getResources();
        String packageName = mFragment.getActivity().getPackageName();

        MenuItem ipDialMenuItem = menu.findItem(resource.getIdentifier(ID_NAME_IP_DIAL_OPTION_MENU, ID, packageName));
        if (null != ipDialMenuItem) {
            ipDialMenuItem.setVisible(false);
        }

        menu.findItem(R.id.menu_eSufing_dialing).setChecked((Settings.System.getInt(mPluginContext.getContentResolver(),
                Settings.System.ESURFING_DIALING, 1) == 1));

        MenuItem slot1IpDialMenuItem = menu.findItem(R.id.menu_ip_call_slot_1);
        MenuItem slot2IpDialMenuItem = menu.findItem(R.id.menu_ip_call_slot_2);

        if (dialpadChooserVisible() || isDigitsEmpty()) {
            if (null != slot1IpDialMenuItem) {
                slot1IpDialMenuItem.setVisible(false);
            }
            if (null != slot2IpDialMenuItem) {
                slot2IpDialMenuItem.setVisible(false);
            }
            return;
        }

        if (0 == SIMInfoWrapper.getDefault().getInsertedSimCount()) {
            if (null != slot1IpDialMenuItem) {
                slot1IpDialMenuItem.setVisible(false);
            }
            if (null != slot2IpDialMenuItem) {
                slot2IpDialMenuItem.setVisible(false);
            }
        } else if (1 == SIMInfoWrapper.getDefault().getInsertedSimCount()) {
            if (OP09CallOptionUtils.isSimInsert(PhoneConstants.GEMINI_SIM_1)) {
                if (null != slot1IpDialMenuItem) {
                    slot1IpDialMenuItem.setVisible(OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1) ?
                                                   false : true);
                }
                if (null != slot2IpDialMenuItem) {
                    slot2IpDialMenuItem.setVisible(false);
                }
            } else if (OP09CallOptionUtils.isSimInsert(PhoneConstants.GEMINI_SIM_2)) {
                //insertSimSlot = PhoneConstants.GEMINI_SIM_2;
                if (null != slot1IpDialMenuItem) {
                    slot1IpDialMenuItem.setVisible(false);
                }
                if (null != slot2IpDialMenuItem) {
                    slot2IpDialMenuItem.setVisible(OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_2) ?
                                                   false : true);
                }
            } else {
                log("both slot 1 and slot 2 do not insert sim");
                return;
            }
        } else {
            if (OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1)
                    && OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_2)) {
                if (null != slot1IpDialMenuItem) {
                    slot1IpDialMenuItem.setVisible(false);
                }
                if (null != slot2IpDialMenuItem) {
                    slot2IpDialMenuItem.setVisible(false);
                }
            } else if (OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1)
                    && !OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_2)) {
                if (OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_1)) {
                    if (null != slot1IpDialMenuItem) {
                        slot1IpDialMenuItem.setVisible(false);
                    }
                    if (null != slot2IpDialMenuItem) {
                        slot2IpDialMenuItem.setVisible(true);
                    }
                } else {
                    if (null != slot1IpDialMenuItem) {
                        slot1IpDialMenuItem.setVisible(true);
                    }
                    if (null != slot2IpDialMenuItem) {
                        slot2IpDialMenuItem.setVisible(false);
                    }
                }
            } else if (!OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_1)
                    && OP09CallOptionUtils.isNetworkRoaming(PhoneConstants.GEMINI_SIM_2)) {
                if (OP09CallOptionUtils.isCDMAPhoneTypeBySlot(PhoneConstants.GEMINI_SIM_2)) {
                    if (null != slot1IpDialMenuItem) {
                        slot1IpDialMenuItem.setVisible(false);
                    }
                    if (null != slot2IpDialMenuItem) {
                        slot2IpDialMenuItem.setVisible(true);
                    }
                } else {
                    if (null != slot1IpDialMenuItem) {
                        slot1IpDialMenuItem.setVisible(true);
                    }
                    if (null != slot2IpDialMenuItem) {
                        slot2IpDialMenuItem.setVisible(false);
                    }
                }
            } else {
                if (null != slot1IpDialMenuItem) {
                    slot1IpDialMenuItem.setVisible(true);
                }
                if (null != slot2IpDialMenuItem) {
                    slot2IpDialMenuItem.setVisible(true);
                }
            }
        }
    }

    public boolean onMenuItemClick(MenuItem item) {
        return handleMenuItemClick(item);
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        return handleMenuItemClick(item);
    }

    private boolean handleMenuItemClick(MenuItem item) {
        switch (item.getItemId()) {

            case R.id.menu_ip_call_slot_1:
                Intent intentSlot1 = OP09CallOptionUtils.getIPCallIntent(mDigits.getText().toString());
                intentSlot1.putExtra(Constants.EXTRA_SLOT_ID, PhoneConstants.GEMINI_SIM_1);
                mFragment.getActivity().startActivity(intentSlot1);
                return true;

            case R.id.menu_ip_call_slot_2:
                Intent intentSlot2 = OP09CallOptionUtils.getIPCallIntent(mDigits.getText().toString());
                intentSlot2.putExtra(Constants.EXTRA_SLOT_ID, PhoneConstants.GEMINI_SIM_2);
                mFragment.getActivity().startActivity(intentSlot2);
                return true;

            case R.id.menu_eSufing_dialing:
                if (item.isChecked()) {
                    item.setChecked(false);
                    Settings.System.putInt(mPluginContext.getContentResolver(),
                            Settings.System.ESURFING_DIALING, 0);
                    //close
                } else {
                    item.setChecked(true);
                    Settings.System.putInt(mPluginContext.getContentResolver(),
                            Settings.System.ESURFING_DIALING, 1);
                    //open
                }
                return true;
            default:
                break;
        }
        return false;
    }

    public void onClick(View view) {
        switch (view.getId()) {

            case R.id.dialButtonLeft:
                log("click dialButtonLeft button");
                if (OP09CallOptionUtils.isSimInsert(PhoneConstants.GEMINI_SIM_1)) {
                    onDialButtonPressed(OP09CallOptionUtils.getCallIntentWithSlot(mDigits.getText().toString(),
                                                                                  PhoneConstants.GEMINI_SIM_1));
                } else {
                    onDialButtonPressed(OP09CallOptionUtils.getCallIntent(mDigits.getText().toString()));
                }
                break;

            case R.id.dialButtonRight:
                log("click dialButtonRight button");
                onDialButtonPressed(OP09CallOptionUtils.getCallIntentWithSlot(mDigits.getText().toString(),
                                                                              PhoneConstants.GEMINI_SIM_2));
                break;

            case R.id.sendSMSButton:
                log("click send sms button");
                if (null != mDigits) {
                    try {
                        mFragment.getActivity().startActivity(
                                OP09CallOptionUtils.getSMSIntent(mDigits.getText().toString()));
                    } catch (ActivityNotFoundException e) {
                        Log.e(TAG, "No activity found ");
                    }
                }
                break;

            default:
                break;
        }
    }

    public boolean updateDialAndDeleteButtonEnabledState(final String lastNumberDialed) {
        if (null != mDialButtonLeft && null != mDialButtonRight) {
            if (phoneIsCdma() && phoneIsOffhook()) {
                mDialButtonLeft.setEnabled(true);
                mDialButtonRight.setEnabled(true);
            } else {
                mDialButtonLeft.setEnabled(!isDigitsEmpty() || !TextUtils.isEmpty(lastNumberDialed));
                mDialButtonRight.setEnabled(!isDigitsEmpty() || !TextUtils.isEmpty(lastNumberDialed));
            }
        }
        return false;
    }

    /**
     * @return true if the phone is a CDMA phone type
     */
    private boolean phoneIsCdma() {
        boolean isCdma = false;
        try {
            ITelephony phone = ITelephony.Stub.asInterface(ServiceManager.checkService("phone"));
            if (phone != null) {
                isCdma = (phone.getActivePhoneType() == TelephonyManager.PHONE_TYPE_CDMA);
            }
        } catch (RemoteException e) {
            log("phone.getActivePhoneType() failed");
        }
        return isCdma;
    }

    /**
     * @return true if the phone state is OFFHOOK
     */
    private boolean phoneIsOffhook() {
        boolean phoneOffhook = false;
        try {
            ITelephony phone = ITelephony.Stub.asInterface(ServiceManager.checkService("phone"));
            if (phone != null) {
                phoneOffhook = phone.isOffhook();
            }
        } catch (RemoteException e) {
            log("phone.isOffhook() failed");
        }
        return phoneOffhook;
    }

    /**
     * @return true if the widget with the phone number digits is empty.
     */
    private boolean isDigitsEmpty() {
        return (null == mDigits) ? false : mDigits.length() == 0;
    }

    private boolean dialpadChooserVisible() {
        return (null == mDialpadChooser) ? false : (mDialpadChooser.getVisibility() == View.VISIBLE);
    }

    private void onDialButtonPressed(Intent intent) {
        log("onDialButtonPressed intent = " + intent);
        if (TextUtils.isEmpty(mDigits.getText().toString())) {
            if (null != mDialpadFragment) {
                mDialpadFragment.handleDialButtonClickWithEmptyDigits();
            }
        } else {
            if (null != mDialpadFragment) {
                mDialpadFragment.doCallOptionHandle(intent);
                if (null != mDigits) {
                    if (mDigits.getText().length() > 0) {
                        log("mDigits.getText() " + mDigits.getText().toString());
                        mDigits.setText("");
                    }
                }
            }
        }
    }

    private void updateDialButtons() {

        if (0 == SIMInfoWrapper.getDefault().getInsertedSimCount()) {
            mDialButtonLeft.setImageDrawable(mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call));
            mDialpadAdditionalButtons.hideRightShowLeftDialButton();
            return;
        } else if (1 == SIMInfoWrapper.getDefault().getInsertedSimCount()) {
            if (OP09CallOptionUtils.isSimInsert(PhoneConstants.GEMINI_SIM_1)) {
                mDialButtonLeft.setImageDrawable(
                        mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_1_orange));
                mDialpadAdditionalButtons.hideRightShowLeftDialButton();
            } else if (OP09CallOptionUtils.isSimInsert(PhoneConstants.GEMINI_SIM_2)) {
                mDialButtonRight.setImageDrawable(
                        mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_2_blue));
                mDialpadAdditionalButtons.hideLeftShowRightDialButton();
            }
            return;
        }

        mDialpadAdditionalButtons.showLeftRightDialButton();

        long defaultSim = Settings.System.getLong(mFragment.getActivity().getContentResolver(),
                                                        Settings.System.VOICE_CALL_SIM_SETTING,
                                                        Settings.System.DEFAULT_SIM_NOT_SET);
        if (defaultSim > 0 
                || defaultSim == Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK) {
            int slot = SIMInfoWrapper.getDefault().getSlotIdBySimId((int)defaultSim);
            if (defaultSim > 0 && slot < 0) {
                log("slot < 0");
                return;
            } else if (defaultSim == Settings.System.DEFAULT_SIM_SETTING_ALWAYS_ASK) {
                //This is op09 project, but the layer tell us the voice call is always_ask
                //something is wrong, so we change it to the cdma card.
                if (PhoneConstants.PHONE_TYPE_CDMA == TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_1)) {
                    slot = PhoneConstants.GEMINI_SIM_1;
                    defaultSim = SIMInfoWrapper.getDefault().getSimIdBySlotId(PhoneConstants.GEMINI_SIM_1);
                } else if (PhoneConstants.PHONE_TYPE_CDMA == TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_2)) {
                    slot = PhoneConstants.GEMINI_SIM_2;
                    defaultSim = SIMInfoWrapper.getDefault().getSimIdBySlotId(PhoneConstants.GEMINI_SIM_2);
                }
                Settings.System.putLong(mFragment.getActivity().getContentResolver(),
                        Settings.System.VOICE_CALL_SIM_SETTING, defaultSim);
                log("change the voice call setting to slot = " + slot + " simId = " + defaultSim);
            }
            
            if (PhoneConstants.GEMINI_SIM_1 == slot) {
                mDialButtonLeft.setImageDrawable(
                        mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_1_orange));
                mDialButtonRight.setImageDrawable(
                        mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_2_blue_small));
            } else if (PhoneConstants.GEMINI_SIM_2 == slot) {
                mDialButtonLeft.setImageDrawable(
                        mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_1_orange_small));
                mDialButtonRight.setImageDrawable(
                        mPluginContext.getResources().getDrawable(R.drawable.ic_dial_action_call_2_blue));
            } else {
                log("slot is not 0 or 1, maybe it's triple sim version or above, please check it");
            }
        } else {
            log("default sim <= 0, is it always ask? CT version does not have this setting!");
        }
    }

    private void updateNumberButtons(View fragmentView) {
        Resources resource = mFragment.getActivity().getResources();
        String packageName = mFragment.getActivity().getPackageName();

        FrameLayout buttonOne = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("one", ID, packageName));
        setDialNumberTextSize(buttonOne);

        FrameLayout buttonTwo = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("two", ID, packageName));
        setDialNumberTextSize(buttonTwo);

        FrameLayout buttonThree = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("three", ID, packageName));
        setDialNumberTextSize(buttonThree);

        FrameLayout buttonFour = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("four", ID, packageName));
        setDialNumberTextSize(buttonFour);

        FrameLayout buttonFive = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("five", ID, packageName));
        setDialNumberTextSize(buttonFive);

        FrameLayout buttonSix = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("six", ID, packageName));
        setDialNumberTextSize(buttonSix);

        FrameLayout buttonSeven = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("seven", ID, packageName));
        setDialNumberTextSize(buttonSeven);

        FrameLayout buttonEight = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("eight", ID, packageName));
        setDialNumberTextSize(buttonEight);

        FrameLayout buttonNine = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("nine", ID, packageName));
        setDialNumberTextSize(buttonNine);

        FrameLayout buttonZero = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("zero", ID, packageName));
        setDialNumberTextSize(buttonZero);

        FrameLayout buttonPound = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("pound", ID, packageName));
        if (null != buttonPound) {
            TextView textNumber = (TextView) buttonPound.findViewById(resource.getIdentifier("dialpad_key_number", ID, packageName));
            textNumber.setTextSize(TypedValue.COMPLEX_UNIT_PX, mPluginContext.getResources().getDimension(
                            R.dimen.dialpad_key_special_characters_size));
        }

        FrameLayout buttonStar = (FrameLayout) fragmentView.findViewById(resource.getIdentifier("star", ID, packageName));
        if (null != buttonStar) {
            TextView textNumber = (TextView) buttonStar.findViewById(resource.getIdentifier("dialpad_key_number", ID, packageName));
            textNumber.setTextSize(TypedValue.COMPLEX_UNIT_PX, mPluginContext.getResources().getDimension(
                            R.dimen.dialpad_key_special_characters_size));
        }
    }

    private class DialpadBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log("DialtactsBroadcastReceiver, onReceive action = " + action);

            if (Intent.ACTION_VOICE_CALL_DEFAULT_SIM_CHANGED.equals(action) 
                    || TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)) {
                updateDialButtons();
            }
        }
    }

    public boolean handleChars(Context context, String input, String commd) {
        if (input.equals(PRL_VERSION_DISPLAY)) {
            if (PhoneConstants.PHONE_TYPE_CDMA == TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_1)) {
                showPRLVersionSetting(context, PhoneConstants.GEMINI_SIM_1);
                return true;
            } else if (PhoneConstants.PHONE_TYPE_CDMA == TelephonyManagerEx.getDefault().getPhoneType(PhoneConstants.GEMINI_SIM_2)) {
                showPRLVersionSetting(context, PhoneConstants.GEMINI_SIM_2);
                return true;
            } else {
                showPRLVersionSetting(context, -1);
                return true;
            }
        }
        return false;
    }

    private void showPRLVersionSetting(Context context, int slot) {
        Intent intent = new Intent(CDMA_INFO_SPECIFICATION_ACTION);
        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, slot);
        context.sendBroadcast(intent);
    }

    private void setDialNumberTextSize(FrameLayout buttonNumber) {
        Resources resource = mFragment.getActivity().getResources();
        String packageName = mFragment.getActivity().getPackageName();
        if (null != buttonNumber) {
            TextView textNumber = (TextView) buttonNumber.findViewById(resource.getIdentifier("dialpad_key_number", ID, packageName));
            textNumber.setTextSize(TypedValue.COMPLEX_UNIT_PX, mPluginContext.getResources().getDimension(
                            R.dimen.dialpad_key_numbers_size));
            textNumber.setGravity(Gravity.CENTER);
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
