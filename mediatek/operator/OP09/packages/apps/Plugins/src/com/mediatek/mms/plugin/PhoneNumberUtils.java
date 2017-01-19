/**
 * 
 */
package com.mediatek.mms.plugin;

import android.content.Context;
import android.location.CountryDetector;
import android.text.TextUtils;

import com.android.i18n.phonenumbers.NumberParseException;
import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.i18n.phonenumbers.Phonenumber.PhoneNumber;
import com.android.i18n.phonenumbers.geocoding.PhoneNumberOfflineGeocoder;
import com.mediatek.common.geocoding.IGeoCodingQuery;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.featureoption.FeatureOption;

import java.util.Locale;

/**
 *
 */
public class PhoneNumberUtils {

    private String mCurrentCountryIso;
    private Context mContext;
    private PhoneNumberUtil mPhoneNumberUtil;
    private PhoneNumberOfflineGeocoder mPhoneNumberOfflineGeocoder;
    private final Locale mLocale;

    public PhoneNumberUtils(Context context) {
        mContext = context;
        mLocale = context.getResources().getConfiguration().locale;
    }

    public String getNumberLocation(String number) {
        String countryIso = getCountryIso();
        return getGeocodedLocationFor(number, countryIso);
    }

    private String getGeocodedLocationFor(String number, String countryIso) {
        /// M: Comment out because it can not find GeoCodingQuery @{
        if (FeatureOption.MTK_PHONE_NUMBER_GEODESCRIPTION) {
            IGeoCodingQuery geoCodingQuery = MediatekClassFactory.createInstance(
                                                IGeoCodingQuery.class,
                                                IGeoCodingQuery.GET_INSTANCE,
                                                mContext);
            String geoInfo = geoCodingQuery.queryByNumber(number);
            if (!TextUtils.isEmpty(geoInfo)) {
                return geoInfo;
            }
        }
        // / @}
        PhoneNumber structuredPhoneNumber = parsePhoneNumber(number, countryIso);
        if (structuredPhoneNumber != null) {
            return getPhoneNumberOfflineGeocoder().getDescriptionForNumber(structuredPhoneNumber,
                mContext.getResources().getConfiguration().locale);
        } else {
            return null;
        }
    }


    private synchronized PhoneNumberUtil getPhoneNumberUtil() {
        if (mPhoneNumberUtil == null) {
            mPhoneNumberUtil = PhoneNumberUtil.getInstance();
        }
        return mPhoneNumberUtil;
    }

    private PhoneNumber parsePhoneNumber(String number, String countryIso) {
        try {
            return getPhoneNumberUtil().parse(number, countryIso);
        } catch (NumberParseException e) {
            return null;
        }
    }

    private synchronized PhoneNumberOfflineGeocoder getPhoneNumberOfflineGeocoder() {
        if (mPhoneNumberOfflineGeocoder == null) {
            mPhoneNumberOfflineGeocoder = PhoneNumberOfflineGeocoder.getInstance();
        }
        return mPhoneNumberOfflineGeocoder;
    }

    /**
     * Get the current country code
     * 
     * @return the ISO 3166-1 two letters country code of current country.
     */
    public synchronized String getCountryIso() {
        if (mCurrentCountryIso == null) {
            final CountryDetector countryDetector = (CountryDetector) mContext
                    .getSystemService(Context.COUNTRY_DETECTOR);
            mCurrentCountryIso = countryDetector.detectCountry().getCountryIso();
//            countryDetector.addCountryListener(new CountryListener() {
//                public void onCountryDetected(Country country) {
//                    mCurrentCountryIso = country.getCountryIso();
//                }
//            }, Looper.getMainLooper());
        }
        return mCurrentCountryIso;
    }
}
