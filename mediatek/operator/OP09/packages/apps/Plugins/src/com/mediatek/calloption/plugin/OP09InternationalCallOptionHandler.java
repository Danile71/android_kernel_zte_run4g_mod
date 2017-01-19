/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.calloption.plugin;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.phone.Constants;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.calloption.CallOptionBaseHandler;
import com.mediatek.calloption.CallOptionUtils;
import com.mediatek.calloption.Request;
import com.mediatek.calloption.InternationalDialogHandler;
import com.mediatek.calloption.InternationalDialogHandler.PrefixInfo;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;
import com.mediatek.phone.provider.CallHistoryAsync;
import com.mediatek.telephony.PhoneNumberUtilsEx;

import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class OP09InternationalCallOptionHandler extends CallOptionBaseHandler
                                    implements DialogInterface.OnClickListener,
                                               DialogInterface.OnDismissListener,
                                               DialogInterface.OnCancelListener,
                                               RadioGroup.OnCheckedChangeListener,
                                               View.OnClickListener,
                                               OP09InternationalCountrySelectDialogHandler.OnCountrySelectListener {

    private static final String TAG = "OP09InternationalCallOptionHandler";

    private static final String COUNTRY_ISO_CHINA = "CN";
    private static final String PLUGIN_PACKAGE = "com.mediatek.op09.plugin";
    private static final String DIALING_GUIDE_CLASS =
        "com.mediatek.calloption.plugin.ESurfingDialingGuideActivity";

    private static final String CHINA_MAINLAND_MCC = "460";
    private static final String MACAU_MCC = "455";
    private static final String HONGKONG_MCC = "454";
    private static final String TAIWAN_MCC = "466";

    private final String[] VALID_133_COUNTRY_ISO = {
            "CN", "AL", "DZ", "AD", "AR", "AM", "AU", "AT", "AZ", "BH", "BD", "BY", "BE", "BJ", "BO", "BW", "BR", "BN",
            "BF", "BI", "CA", "CF", "CL", "CG", "CY", "CZ", "DK", "EG", "FI", "FR", "GF", "GM", "DE", "GH", "GI", "GR",
            "GU", "GN", "IR", "IQ", "IE", "IL", "IT", "JM", "JP", "JO", "KW", "LA", "LV", "LB", "LY", "LT", "MG", "MW",
            "MY", "MV", "MU", "MD", "MN", "MZ", "NA", "NP", "NL", "NZ", "NE", "NG", "NO", "OM", "PK", "PY", "PE", "PH",
            "PL", "PT", "RO", "RU", "SA", "SN", "SC", "SL", "SG", "ZA", "ES", "LK", "SD", "SR", "SZ", "SE", "CH", "TZ",
            "TH", "TT", "TN", "TR", "UG", "UA", "AE", "GB", "US", "UZ", "VN", "ZW", "ZM", "BA", "HR", "FO", "GP", "RE",
            "MR", "MK", "ER", "CV", "GQ", "AF"};

    private Request mRequest;

    private Button mCountrySelectButton;
    private RadioGroup mHomeCountryGroup;
    private RadioButton mInternationalCallRadioButton;
    private RadioButton mCTCallBackRadioButton;
    private RadioButton mHomeCountryRadioButton;
    private RadioGroup mWholeGroup;
    private EditText mPhoneNumEdit;
    private ImageButton mESurfingDialingHelp;

    private Context mPluginContext;

    private String mSelectCountryISO;
    private String mHomeMcc;
    private String mDialToHome;

    protected OP09InternationalCountrySelectDialogHandler mCountrySelectDialogHandler;

    public OP09InternationalCallOptionHandler(Context pluginContext) {
        mPluginContext = pluginContext;
    }

    @Override
    public void handleRequest(final Request request) {
        log("handleRequest()");

        mRequest = request;

        int internationalDialOption = request.getIntent().getIntExtra(Constants.EXTRA_INTERNATIONAL_DIAL_OPTION,
                                                                      Constants.INTERNATIONAL_DIAL_OPTION_NORMAL);
        if (Constants.INTERNATIONAL_DIAL_OPTION_WITH_COUNTRY_CODE == internationalDialOption) {
            log("international dial option is dial with country code, show yi dialing dialog directly");
            showYiDialingDialog();
            return;
        }

        int slot = request.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);

        boolean supporteSufing = (Settings.System.getInt(mPluginContext.getContentResolver(),
                Settings.System.ESURFING_DIALING, 1) == 1);

        log("supportEsufing " + supporteSufing);

        if (null != mSuccessor
                && (!supporteSufing || ((-1 == slot || !OP09CallOptionUtils.isNetworkRoaming(slot))))) {
                mSuccessor.handleRequest(request);
            return;
        }

        // number start with "+" or (start with "**133" and end with "#", just continue
        String number = CallOptionUtils.getInitialNumber(request.getApplicationContext(), request.getIntent());

        // CT 0729  01-035: in China and Macao, remove "**133*86" "#"
        String currentCountryISO = CallOptionUtils.getCurrentCountryISO(mRequest.getApplicationContext());
        if ((currentCountryISO.equalsIgnoreCase(COUNTRY_ISO_CHINA) || currentCountryISO.equalsIgnoreCase("MO"))
                 && (number.startsWith("**133*86") && number.endsWith("#"))) {
            number = number.replace("**133*86", "");
            number = number.replace("#", "");
            Log.d(TAG, "number replace = " + number);
            request.getIntent().putExtra(Constants.EXTRA_ACTUAL_NUMBER_TO_DIAL, number);

            if (null != mSuccessor) {
                mSuccessor.handleRequest(request);
            }
            return;
        }

        // number start with "+" or (start with "**133" and end with "#", just continue
        if (number.startsWith("+") || (number.startsWith("**133") && number.endsWith("#"))) {
            if (null != mSuccessor) {
                mSuccessor.handleRequest(request);
            }
            return;
        }

        // number start with international prefix of current country, just continue
        String internationlPrefix = PhoneNumberUtilsEx.getInternationalPrefix(getNetworkCountryISO(slot));
        log("internationlPrefix = " + internationlPrefix);
        if (null == internationlPrefix) {
            log("internationlPrefix is null");
            if (null != mSuccessor) {
                mSuccessor.handleRequest(request);
            }
            return;
        }
        Pattern pattern = Pattern.compile(internationlPrefix);
        Matcher matcher = pattern.matcher(number);
        if (!matcher.matches() && matcher.lookingAt()) {
            if (null != mSuccessor) {
                mSuccessor.handleRequest(request);
            }
            return;
        }

        //show eSurfing dialing guide activity
        String firstLoadEsurfing = "first_load_esurfing_";
        initHomeCountry();
        firstLoadEsurfing += mHomeMcc;
        log("getSIMOperator(), firstLoadEsurfing = " + firstLoadEsurfing);
        int firstLoad = Settings.System.getInt(mPluginContext.getContentResolver(), firstLoadEsurfing, 0);
        if (0 == firstLoad) {
            Intent intent = new Intent(Intent.ACTION_MAIN, null);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                         | Intent.FLAG_ACTIVITY_NO_ANIMATION);
            intent.setClassName(PLUGIN_PACKAGE, DIALING_GUIDE_CLASS);
            intent.putExtra(Constants.EXTRA_SLOT_ID, slot);
            intent.putExtra(Constants.EXTRA_ACTUAL_NUMBER_TO_DIAL, number);
            mPluginContext.startActivity(intent);
        }

        // pop up dialog
        showYiDialingDialog();
    }

    private void showYiDialingDialog() {
        log("showYiDialingDialog()");
        LayoutInflater inflater
                = (LayoutInflater) mPluginContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View dialogTitleView = inflater.inflate(R.layout.international_yi_dialing_title, null);
        mESurfingDialingHelp = (ImageButton)dialogTitleView.findViewById(R.id.yi_dialing_title_help);
        mESurfingDialingHelp.setOnClickListener(this);

        if (null == mDialog || !mDialog.isShowing()) {
            AlertDialog.Builder builder = new AlertDialog.Builder(mRequest.getActivityContext(), AlertDialog.THEME_DEVICE_DEFAULT_LIGHT);
            builder.setPositiveButton(mPluginContext.getString(R.string.yi_dialing_dialing_call_button), this)
                   .setView(createYiDialingDialogView());
            builder.setCustomTitle(dialogTitleView);
            mDialog = builder.create();
            mDialog.setOnDismissListener(this);
            mDialog.setOnCancelListener(this);
            mDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
            mDialog.show();
        }
    }

    protected View createYiDialingDialogView() {
        LayoutInflater inflater
                = (LayoutInflater) mPluginContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View dialogView = inflater.inflate(R.layout.international_yi_dialing_dialog, null);

        mCountrySelectButton = (Button) dialogView.findViewById(R.id.country_select_button);
        mCountrySelectButton.setText(getDefaultButtonText());
        mCountrySelectButton.setOnClickListener(this);
        mCountrySelectButton.setEnabled(false);

        mWholeGroup = (RadioGroup) dialogView.findViewById(R.id.whole_group);
        mWholeGroup.check(R.id.home_country);
        mWholeGroup.setOnCheckedChangeListener(this);

        mHomeCountryRadioButton = (RadioButton) dialogView.findViewById(R.id.home_country);
        mHomeCountryRadioButton.setText(mDialToHome);

        mHomeCountryGroup = (RadioGroup) dialogView.findViewById(R.id.home_country_group);
        mHomeCountryGroup.check(R.id.international_call);

        mInternationalCallRadioButton = (RadioButton) dialogView.findViewById(R.id.international_call);
        mInternationalCallRadioButton.setOnClickListener(this);
        mCTCallBackRadioButton = (RadioButton) dialogView.findViewById(R.id.ct_callback);
        mCTCallBackRadioButton.setOnClickListener(this);

        if (can133CallBackFunctionWork()) {
            mCTCallBackRadioButton.setEnabled(true);
        } else {
            mCTCallBackRadioButton.setEnabled(false);
        }

        mSelectCountryISO = COUNTRY_ISO_CHINA;

        mPhoneNumEdit = (EditText) dialogView.findViewById(R.id.numberEdit);
        mPhoneNumEdit.setText(getPhoneNumText());
        mPhoneNumEdit.setOnClickListener(this);
        mPhoneNumEdit.setEnabled(true);

        final int currentPosition = mPhoneNumEdit.length();
        if (currentPosition > 0) {
            mPhoneNumEdit.setSelection(currentPosition);
        }
        return dialogView;
    }

    public void onDismiss(DialogInterface dialog) {
        log("onDismiss()");
    }

    public void onCancel(DialogInterface dialog) {
        log("onCancel()");
        mRequest.getResultHandler().onHandlingFinish();
    }

    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            mRequest.getIntent().putExtra(Constants.EXTRA_ACTUAL_NUMBER_TO_DIAL, mPhoneNumEdit.getText().toString());
            if (null != mSuccessor) {
                mSuccessor.handleRequest(mRequest);
            }
        }
    }

    public void onCheckedChanged(RadioGroup group, int checkedId) {
        if (group == mWholeGroup) {
            if (R.id.home_country == checkedId) {
                mHomeCountryGroup.setEnabled(true);
                mInternationalCallRadioButton.setEnabled(true);
                if (can133CallBackFunctionWork()) {
                    mCTCallBackRadioButton.setEnabled(true);
                } else {
                    mCTCallBackRadioButton.setEnabled(false);
                }
                mCountrySelectButton.setEnabled(false);
                mCountrySelectButton.setText(getDefaultButtonText());
                mSelectCountryISO = COUNTRY_ISO_CHINA;

                if (R.id.international_call == mHomeCountryGroup.getCheckedRadioButtonId()) {
                    mPhoneNumEdit.setText(getPhoneNumText());
                } else if (R.id.ct_callback == mHomeCountryGroup.getCheckedRadioButtonId()) {
                    String formatNumber = getPhoneNumText();
                    if (formatNumber.startsWith("+")) {
                        formatNumber = formatNumber.substring(1);
                    }
                    formatNumber = "**133*" + formatNumber + "#";
                    mPhoneNumEdit.setText(formatNumber);
                }
            } else if (R.id.dial_to_another_country == checkedId) {
                mInternationalCallRadioButton.setEnabled(false);
                mCTCallBackRadioButton.setEnabled(false);
                mHomeCountryGroup.setEnabled(false);
                mCountrySelectButton.setEnabled(true);
                mPhoneNumEdit.setText(getPhoneNumText());
            } else if (R.id.local_call == checkedId) {
                mInternationalCallRadioButton.setEnabled(false);
                mCTCallBackRadioButton.setEnabled(false);
                mHomeCountryGroup.setEnabled(false);
                mCountrySelectButton.setEnabled(false);
                mCountrySelectButton.setText(getDefaultButtonText());
                mSelectCountryISO = COUNTRY_ISO_CHINA;
                String number = CallOptionUtils.getInitialNumber(mRequest.getApplicationContext(), mRequest.getIntent());
                mPhoneNumEdit.setText(PhoneNumberUtils.stripSeparators(number));
            }
            final int currentPosition = mPhoneNumEdit.length();
            if (currentPosition > 0) {
                mPhoneNumEdit.setSelection(currentPosition);
            }
        }
    }

    @Override
    public void onClick(View view) {
        if (view == mCountrySelectButton) {
            if (null == mCountrySelectDialogHandler) {
                mCountrySelectDialogHandler =
                    new OP09InternationalCountrySelectDialogHandler(mRequest.getActivityContext(),
                                                                    mPluginContext, this);
            }
            mCountrySelectDialogHandler.showCountrySelectDialog();
        } else if (view == mInternationalCallRadioButton) {
            mPhoneNumEdit.setText(getPhoneNumText());
            final int currentPosition = mPhoneNumEdit.length();
            if (currentPosition > 0) {
                mPhoneNumEdit.setSelection(currentPosition);
            }
        } else if (view == mCTCallBackRadioButton) {
            String formatNumber = getPhoneNumText();
            if (formatNumber.startsWith("+")) {
                formatNumber = formatNumber.substring(1);
            }
            formatNumber = "**133*" + formatNumber + "#";
            mPhoneNumEdit.setText(formatNumber);
            final int currentPosition = mPhoneNumEdit.length();
            if (currentPosition > 0) {
                mPhoneNumEdit.setSelection(currentPosition);
            }
        } else if (view == mESurfingDialingHelp) {
            //show eSurfing dialing guide activity
            int slot = mRequest.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);
            Intent intent = new Intent(Intent.ACTION_MAIN, null);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                         | Intent.FLAG_ACTIVITY_NO_ANIMATION);
            intent.putExtra(Constants.EXTRA_SLOT_ID, slot);
            intent.setClassName(PLUGIN_PACKAGE, DIALING_GUIDE_CLASS);
            mPluginContext.startActivity(intent);
        }
    }

    private String getDefaultButtonText() {
        Locale locale = new Locale(Locale.getDefault().getLanguage(), COUNTRY_ISO_CHINA);
        int countryCode = PhoneNumberUtil.getInstance().getCountryCodeForRegion(COUNTRY_ISO_CHINA);
        return locale.getDisplayCountry(Locale.getDefault()) + "(+" + Integer.toString(countryCode) + ")";
    }

    private String getNetworkCountryISO(int slot) {
        log("getNetworkCountryISO(), slot = " + slot);
        TelephonyManager telephony = TelephonyManager.getDefault();
        if (mRequest.isMultipleSim()) {
            log("getNetworkCountryISO(), country ISO = " + telephony.getNetworkCountryIsoGemini(slot).toUpperCase());
            return telephony.getNetworkCountryIsoGemini(slot).toUpperCase();
        } else {
            log("getNetworkCountryISO(), country ISO = " + telephony.getNetworkCountryIso().toUpperCase());
            return telephony.getNetworkCountryIso().toUpperCase();
        }
    }

    @Override
    public void onCountrySelected(String countryISO, String countryCode, String countryName) {
        mSelectCountryISO = countryISO;
        mCountrySelectButton.setText(countryName + "(+" + countryCode + ")");
        mPhoneNumEdit.setText(getPhoneNumText());
    }

    public void dismissDialog() {
        log("dismissDialog()");
        if (null != mCountrySelectDialogHandler) {
            mCountrySelectDialogHandler.onHandledDialogDismiss();
        }
        //super.dismissDialog();
    }

    private boolean can133CallBackFunctionWork() {
        int slot = mRequest.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);
        return isCurrentCountryFromValid133() && !OP09CallOptionUtils.isCDMAPhoneTypeBySlot(slot);
    }

    private boolean isCurrentCountryFromValid133() {
        String currentCountryISO = CallOptionUtils.getCurrentCountryISO(mRequest.getApplicationContext());
        for (String countryISO : VALID_133_COUNTRY_ISO) {
            if (currentCountryISO.equalsIgnoreCase(countryISO)) {
                return true;
            }
        }
        return false;
    }

    private String getPhoneNumText() {
        int slot = mRequest.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);
        String number = CallOptionUtils.getInitialNumber(mRequest.getApplicationContext(), mRequest.getIntent());
        String number2 = PhoneNumberUtils.stripSeparators(number);
        String formatNumber = PhoneNumberUtils.formatNumberToE164(number2, mSelectCountryISO);
        log("getInternationalPhoneNumber(), mSelectCountryISO = " + mSelectCountryISO);
        if (null == formatNumber || number2.equals(formatNumber)) {
            int countryCode = PhoneNumberUtil.getInstance().getCountryCodeForRegion(mSelectCountryISO);
            formatNumber = "+" + Integer.toString(countryCode) + number2;
        }
        log("getInternationalPhoneNumber(), formatNumber = " + formatNumber);

        return formatNumber;
    }

    private void initHomeCountry() {
        int slot = mRequest.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);
        log("getSIMOperator(), slot = " + slot);
        mHomeMcc = OP09CallOptionUtils.getCountryBySlotId(slot);
        if (null != mHomeMcc && mHomeMcc.equals(CHINA_MAINLAND_MCC)) {
            mDialToHome = mPluginContext.getString(R.string.dial_to_china_mainland);
        } else if (null != mHomeMcc && mHomeMcc.equals(MACAU_MCC)) {
            mDialToHome = mPluginContext.getString(R.string.dial_to_macau);
        } else if (null != mHomeMcc && mHomeMcc.equals(HONGKONG_MCC)) {
            mDialToHome = mPluginContext.getString(R.string.dial_to_hongkong);
        } else if (null != mHomeMcc && mHomeMcc.equals(TAIWAN_MCC)) {
            mDialToHome = mPluginContext.getString(R.string.dial_to_taiwan);
        } else {
            mDialToHome = mPluginContext.getString(R.string.dial_to_home_country);
        }
        log("getSIMOperator(), mcc = " + mHomeMcc);
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
