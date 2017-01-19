package com.mediatek.calloption.plugin;

import android.app.Activity;
import android.content.Intent;
import android.content.Context;
import android.location.CountryDetector;
import android.provider.Settings;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.view.LayoutInflater;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ImageView;

import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.phone.Constants;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.op09.plugin.R;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Locale;

public class ESurfingDialingGuideActivity extends Activity
        implements View.OnClickListener {

    private static final String TAG = "ESurfingDialingGuideActivity";

    private static final String CHINA_MAINLAND_EXAMPLE_NO = "01058501234";
    private static final String MACAU_EXAMPLE_NO = "28831622";
    private static final String HONGKONG_EXAMPLE_NO = "39437000";
    private static final String TAIWAN_EXAMPLE_NO = "0226598088";
    private static final String OTHER_EXAMPLE_NO = "26806802";

    private static final String CHINA_MAINLAND_MCC = "460";
    private static final String MACAU_MCC = "455";
    private static final String HONGKONG_MCC = "454";
    private static final String TAIWAN_MCC = "466";

    private Context mContext;
    private ViewPager mGuidePager;
    private ArrayList<View> mStepViews;
    private ViewGroup mDialingGuide;
    private Intent mIntent;
    private String mPhoneNumber;
    private String mCurrentCountryIso;
    private String mDialToHome;
    private String mStep2Discript;
    private String mExampleNumber;
    private String mMcc;
    private int mSlotId;
    private int mCurrentPosition;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mContext = this;

        LayoutInflater inflater = getLayoutInflater();
        mStepViews = new ArrayList<View>();
        View step1View = (View)inflater.inflate(R.layout.esurfing_guide_page_first, null);
        View step2View = (View)inflater.inflate(R.layout.esurfing_guide_page_second, null);
        View step3View = (View)inflater.inflate(R.layout.esurfing_guide_page_third, null);
        View step4View = (View)inflater.inflate(R.layout.esurfing_guide_page_fourth, null);
        View step5View = (View)inflater.inflate(R.layout.esurfing_guide_page_fifth, null);
        View step6View = (View)inflater.inflate(R.layout.esurfing_guide_page_sixth, null);

        mStepViews.add(step1View);
        mStepViews.add(step2View);
        mStepViews.add(step3View);
        mStepViews.add(step4View);
        mStepViews.add(step5View);
        mStepViews.add(step6View);

        mDialingGuide = (ViewGroup) inflater.inflate(R.layout.esurfing_dialing_guide, null);
        mGuidePager = (ViewPager) mDialingGuide.findViewById(R.id.eSurfingDialingGuidePages);
        setContentView(mDialingGuide);

        mGuidePager.setAdapter(new ViewPagerAdapter());
        mGuidePager.setOnPageChangeListener(new ViewPagerChangeListener());
    }

    @Override
    public void onResume() {
        super.onResume();
        log("onResume");

        mSlotId = this.getIntent().getIntExtra(Constants.EXTRA_SLOT_ID, -1);
        mPhoneNumber = this.getIntent().getStringExtra(Constants.EXTRA_ACTUAL_NUMBER_TO_DIAL);
        initEsurfingGuide();
        int firstLoad = 0;
        String firstLoadEsurfing = "first_load_esurfing_";
        firstLoadEsurfing += mMcc;
        firstLoad = Settings.System.getInt(mContext.getContentResolver(), firstLoadEsurfing, 0);
        //if (null != mPhoneNumber) {
            log("onResume firstLoadEsurfing : " + firstLoadEsurfing);
            Settings.System.putInt(mContext.getContentResolver(), firstLoadEsurfing, 1);
        //}

        mGuidePager.setCurrentItem(0);
        CountryDetector detector =
                    (CountryDetector) mContext.getSystemService(Context.COUNTRY_DETECTOR);
        mCurrentCountryIso = detector.detectCountry().getCountryIso();

        for(Iterator<View> it = mStepViews.iterator(); it.hasNext();){
            View stepView = it.next();
            TextView dialPadNumEdit = (TextView)stepView.findViewById(R.id.numberDialpad);
            TextView countrySelect = (TextView)stepView.findViewById(R.id.country_select_button);
            TextView eSurfingNumEdit = (TextView)stepView.findViewById(R.id.numberEdit);
            TextView dialToHomeCountry = (TextView)stepView.findViewById(R.id.home_country_name);
            Button skipButton = (Button)stepView.findViewById(R.id.skip_button);

            if (null != dialPadNumEdit) {
                dialPadNumEdit.setText(mExampleNumber);
            }
            if (null != eSurfingNumEdit) {
                eSurfingNumEdit.setText(getExampleFormatePhoneNo(mExampleNumber));
            }
            if (null != countrySelect) {
                countrySelect.setText(getCountryButtonText());
            }
            if (null != dialToHomeCountry) {
                dialToHomeCountry.setText(mDialToHome);
            }
            if (null != skipButton) {
                skipButton.setOnClickListener(this);
                if (null == mPhoneNumber || 0 != firstLoad) {
                    skipButton.setText(mContext.getString(R.string.country_exit));
                } else if (stepView == mStepViews.get(5)) {
                    skipButton.setText(mContext.getString(R.string.country_start));
                }
            }
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        log("onKey: " + keyCode + "event: " + event);

        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mGuidePager.getCurrentItem() > 0) {
                mGuidePager.setCurrentItem(mGuidePager.getCurrentItem() -1, true);
            } else {
                moveTaskToBack(true);
            }
            return true;
        }
        return false;
    }

    @Override
    public void onClick(View view) {
        log("onClick: " + view);
        if (R.id.skip_button == view.getId()) {
            moveTaskToBack(true);
            return ;
        }
        return ;
    }

    @Override
    public void onStop() {
        super.onStop();
        log("onStop");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        log("onDestroy");
    }

    private class ViewPagerAdapter extends PagerAdapter{

        @Override
        public void destroyItem(View v, int position, Object arg2) {
            // TODO Auto-generated method stub
            ((ViewPager)v).removeView(mStepViews.get(position));
        }

        @Override
        public void finishUpdate(View arg0) {
            // TODO Auto-generated method stub
        }

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return mStepViews.size();
        }

        @Override
        public Object instantiateItem(View v, int position) {
            // TODO Auto-generated method stub
            ((ViewPager) v).addView(mStepViews.get(position));
            return mStepViews.get(position);
        }

        @Override
        public boolean isViewFromObject(View v, Object arg1) {
            // TODO Auto-generated method stub
            return v == arg1;
        }



        @Override
        public void startUpdate(View arg0) {
            // TODO Auto-generated method stub
        }

        @Override
        public int getItemPosition(Object object) {
            // TODO Auto-generated method stub
            return super.getItemPosition(object);
        }

        @Override
        public void restoreState(Parcelable arg0, ClassLoader arg1) {
            // TODO Auto-generated method stub
        }

        @Override
        public Parcelable saveState() {
            // TODO Auto-generated method stub
            return null;
        }
    }

    private class ViewPagerChangeListener implements OnPageChangeListener{

        @Override
        public void onPageScrollStateChanged(int arg0) {
            // TODO Auto-generated method stub
        }

        @Override
        public void onPageScrolled(int arg0, float arg1, int arg2) {
            // TODO Auto-generated method stub
        }

        @Override
        public void onPageSelected(int position) {
            // TODO Auto-generated method stub
            View stepView = mStepViews.get(position);
            mCurrentPosition = position;
            ImageView homeCountryIcon= (ImageView) stepView.findViewById(R.id.home_country_icon);
            ImageView internatonalIcon= (ImageView) stepView.findViewById(R.id.international_call_icon);
            ImageView callBackIcon= (ImageView) stepView.findViewById(R.id.ct_callback_icon);
            ImageView otherCountryIcon= (ImageView) stepView.findViewById(R.id.dial_to_another_country_icon);
            ImageView locallCallIcon= (ImageView) stepView.findViewById(R.id.local_call_icon);
            TextView inCallScreenPhoneNum= (TextView) stepView.findViewById(R.id.in_call_screen_phone_number);

            switch(position) {
                case 0 :
                case 1 : {
                    TextView step2Discrpit = (TextView)stepView.findViewById(R.id.stepText);
                    if (1 == position && null != step2Discrpit) {
                        inCallScreenPhoneNum.setText(mStep2Discript);
                    }
                    if (null != inCallScreenPhoneNum) {
                        inCallScreenPhoneNum.setText(getExampleFormatePhoneNo(mExampleNumber));
                    }

                    homeCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    internatonalIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    callBackIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    otherCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    locallCallIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    break;
                }
                case 2 : {
                    TextView eSurfingNumEdit = (TextView)stepView.findViewById(R.id.numberEdit);
                    String formatNumber = getExampleFormatePhoneNo(mExampleNumber);

                    if (formatNumber.startsWith("+")) {
                        formatNumber = formatNumber.substring(1);
                    }
                    formatNumber = "**133*" + formatNumber + "#";
                    if (null != inCallScreenPhoneNum) {
                        inCallScreenPhoneNum.setText(formatNumber);
                    }
                    if (null != eSurfingNumEdit) {
                        eSurfingNumEdit.setText(formatNumber);
                    }

                    homeCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    internatonalIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    callBackIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    otherCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    locallCallIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    break;
                }
                case 3 : {
                    TextView eSurfingNumEdit = (TextView)stepView.findViewById(R.id.numberEdit);
                    View eSurfingDialog = (View)stepView.findViewById(R.id.esurfing_dialing_dialog_guide);
                    String formatNumber = getExampleFormatePhoneNo(OTHER_EXAMPLE_NO);

                    if (null != eSurfingNumEdit) {
                        eSurfingNumEdit.setText(formatNumber);
                    }
                    homeCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    internatonalIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    callBackIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    otherCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    locallCallIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    eSurfingDialog.setBackgroundResource(R.drawable.esurfing_dialing_dialog_enable);
                    break;
                }
                case 4 : {
                    TextView countrySelect = (TextView)stepView.findViewById(R.id.country_select_button);
                    TextView eSurfingNumEdit = (TextView)stepView.findViewById(R.id.numberEdit);
                    View eSurfingDialog = (View)stepView.findViewById(R.id.esurfing_dialing_dialog_guide);
                    String egyptCountry = mContext.getString(R.string.country_select_egypt);
                    String egyptCountryCode = mContext.getString(R.string.country_select_egypt_code);

                    if (null != eSurfingNumEdit) {
                        eSurfingNumEdit.setText(egyptCountryCode + OTHER_EXAMPLE_NO);
                    }
                    if (null != countrySelect) {
                        countrySelect.setText(egyptCountry + "(" + egyptCountryCode + ")");
                    }
                    if (null != inCallScreenPhoneNum) {
                        inCallScreenPhoneNum.setText(egyptCountryCode + OTHER_EXAMPLE_NO);
                    }

                    homeCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    internatonalIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    callBackIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    otherCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    locallCallIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    eSurfingDialog.setBackgroundResource(R.drawable.esurfing_dialing_dialog_enable);
                    break;
                }
                case 5 : {
                    TextView eSurfingNumEdit = (TextView)stepView.findViewById(R.id.numberEdit);
                    if (null != eSurfingNumEdit) {
                        eSurfingNumEdit.setText(mExampleNumber);
                    }
                    if (null != inCallScreenPhoneNum) {
                        inCallScreenPhoneNum.setText(mExampleNumber);
                    }
                    homeCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    internatonalIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    callBackIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    otherCountryIcon.setBackgroundResource(R.drawable.esurfing_radio_unchecked);
                    locallCallIcon.setBackgroundResource(R.drawable.esurfing_radio_checked);
                    break;
                }
                default:
                    break;
            }
        }
    }

    /**
     * @param null
     * @return String
     */
    private String getCountryButtonText() {
        Locale locale = new Locale(Locale.getDefault().getLanguage(), mCurrentCountryIso);
        int countryCode = PhoneNumberUtil.getInstance().getCountryCodeForRegion(mCurrentCountryIso);
        return locale.getDisplayCountry(Locale.getDefault()) + "(+" + Integer.toString(countryCode) + ")";
    }

    /**
     * @param number
     * @return String
     */
    private String getExampleFormatePhoneNo(String number) {
        String number2 = PhoneNumberUtils.stripSeparators(number);
        String formatNumber = PhoneNumberUtils.formatNumberToE164(number2, mCurrentCountryIso);
        log("getPhoneNumText(), currentCountryIso = " + mCurrentCountryIso);
        if (null == formatNumber || number2.equals(formatNumber)) {
            int countryCode = PhoneNumberUtil.getInstance().getCountryCodeForRegion(mCurrentCountryIso);
            formatNumber = "+" + Integer.toString(countryCode) + number2;
        }
        log("getPhoneNumText(), formatNumber = " + formatNumber);

        return formatNumber;
    }

    /**
     * @param null
     * @return null
     */
    private void initEsurfingGuide() {
        log("setExamplePhoneNo() mSlotId : " + mSlotId);
        mMcc = OP09CallOptionUtils.getCountryBySlotId(mSlotId);

        log("setExamplePhoneNo() mMcc : " + mMcc);
        if (null != mMcc && mMcc.equals(CHINA_MAINLAND_MCC)) {
            mExampleNumber = CHINA_MAINLAND_EXAMPLE_NO;
            mDialToHome = mContext.getString(R.string.dial_to_china_mainland);
            mStep2Discript = mContext.getString(R.string.esurfing_dialing_step2_china_mainland);
        } else if (null != mMcc && mMcc.equals(MACAU_MCC)) {
            mExampleNumber = MACAU_EXAMPLE_NO;
            mDialToHome = mContext.getString(R.string.dial_to_macau);
            mStep2Discript = mContext.getString(R.string.esurfing_dialing_step2_china_macau);
        } else if (null != mMcc && mMcc.equals(HONGKONG_MCC)) {
            mExampleNumber = HONGKONG_EXAMPLE_NO;
            mDialToHome = mContext.getString(R.string.dial_to_hongkong);
            mStep2Discript = mContext.getString(R.string.esurfing_dialing_step2_china_hongkong);
        } else if (null != mMcc && mMcc.equals(TAIWAN_MCC)) {
            mExampleNumber = TAIWAN_EXAMPLE_NO;
            mDialToHome = mContext.getString(R.string.dial_to_taiwan);
            mStep2Discript = mContext.getString(R.string.esurfing_dialing_step2_china_taiwan);
        } else {
            mExampleNumber = OTHER_EXAMPLE_NO;
            mDialToHome = mContext.getString(R.string.dial_to_home_country);
            mStep2Discript = mContext.getString(R.string.esurfing_dialing_step2_china_other);
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
 }
