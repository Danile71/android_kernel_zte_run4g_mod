/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.weather;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.R.integer;
import android.os.Parcel;
import android.os.Parcelable;

public class Weather implements Parcelable {
    private static final double NUM4 = 1.8;
    private static final double HALF = 0.5;
    private static final int NUM3 = 9;
    private static final int NUM2 = 5;
    private static final int NUM1 = 32;
    int mCityId = -1;
    int mTempType = 1;
    CurrentWeather mCurWeather = new CurrentWeather();
    List<ForecastDayWeather> mForecastes = new ArrayList<ForecastDayWeather>();

    public Weather() {

    }

    public Weather(Weather weather) {
        mCityId = weather.getCityId();
        mTempType = weather.getTempType();
        mCurWeather = new CurrentWeather(weather.getCurWeather());
        for (ForecastDayWeather fWeather : weather.getForecastWeather()) {
            ForecastDayWeather fw = new ForecastDayWeather(fWeather);
            mForecastes.add(fw);
        }
    }

    public static boolean isWeatherEmpty(Weather weather) {
        return weather == null || weather.mCurWeather == null
            || weather.getForecastWeather().isEmpty();
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mCityId);
        out.writeInt(mTempType);
        mCurWeather.writeToParcel(out, flags);
        out.writeTypedList(mForecastes);
    }

    /**
     * read data from Parcel package.
     * @param in input a city parcel
     */
    public void readFromParcel(Parcel in) {
        mCityId = in.readInt();
        mTempType = in.readInt();
        mCurWeather.readFromParcel(in);
        in.readTypedList(mForecastes, ForecastDayWeather.CREATOR);
    }

    public static final Parcelable.Creator<Weather> CREATOR = new Parcelable.Creator<Weather>() {
        public Weather createFromParcel(Parcel in) {
            Weather r = new Weather();
            r.readFromParcel(in);
            return r;
        }

        public Weather[] newArray(int size) {
            return new Weather[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    public void setCityId(int id) {
        mCityId = id;
    }

    public int getCityId() {
        return mCityId;
    }

    public void setTempType(int type) {
        mTempType = type;
    }

    public int getTempType() {
        return mTempType;
    }

    public CurrentWeather getCurWeather() {
        return mCurWeather;
    }

    public void addForecastWeather(ForecastDayWeather forecast) {
        mForecastes.add(forecast);
    }

    public List<ForecastDayWeather> getForecastWeather() {
        return mForecastes;
    }

    public void convertTemperature(int tempType) {
        int fromTempType = mTempType;
        int toTemptype = tempType;
        mTempType = tempType;
        mCurWeather.convertTemperature(fromTempType, toTemptype);
        for (int i = 0; i < mForecastes.size(); i++) {
            mForecastes.get(i).convertTemperature(fromTempType, toTemptype);
        }
    }

    /**
     * convert format between Celsius and fahrenheit
     * @param temp current temperature
     * @param fromTempType a kind of temperature
     * @param toTemptype other kind of temperature
     * @return converted temperature
     */
    public static final double convertTemperature(double temp, int fromTempType, int toTemptype) {
        if (fromTempType == toTemptype) {
            return temp;
        }
        int result = 0;
        if (toTemptype == Utils.TEMPERATURE_CELSIUS) {
            double newTemp = (temp - NUM1) * NUM2 / NUM3;
            double sign = Math.signum(newTemp);
            result = (int) (sign * (Math.abs(newTemp) + HALF));
            return (double) result;
        } else if (toTemptype == Utils.TEMPERATURE_FAHRENHEIT) {
            double newTemp = temp * NUM4 + NUM1;
            double sign = Math.signum(newTemp);
            result = (int) (sign * (Math.abs(newTemp) + HALF));
            return (double) result;
        }
        throw new IllegalArgumentException();
    }

    public static class ForecastDayWeather implements Parcelable {
        int mCityId = -1;
        int mDayOfWeek = -1;
        int mConditionTypeId = -1;
        double mTempLow = -1;
        double mTempHigh = -1;
        static HashMap<String, String> sWeatherMap = new HashMap<String, String>();

        public ForecastDayWeather() {

        }

        public ForecastDayWeather(ForecastDayWeather fWeather) {
            mCityId = fWeather.getCityId();
            mDayOfWeek = fWeather.getDayOfWeek();
            mConditionTypeId = fWeather.getConditionTypeId();
            mTempLow = fWeather.getTempLow();
            mTempHigh = fWeather.getTempHigh();
        }

        @Override
        public void writeToParcel(Parcel out, int flags) {
            out.writeInt(mCityId);
            out.writeInt(mDayOfWeek);
            out.writeInt(mConditionTypeId);
            out.writeDouble(mTempLow);
            out.writeDouble(mTempHigh);
        }

        /**
         * read data from Parcel package.
         * @param in input a city parcel
         */
        public void readFromParcel(Parcel in) {
            mCityId = in.readInt();
            mDayOfWeek = in.readInt();
            mConditionTypeId = in.readInt();
            mTempLow = in.readDouble();
            mTempHigh = in.readDouble();
        }

        public static final Parcelable.Creator<ForecastDayWeather> CREATOR
                      = new Parcelable.Creator<ForecastDayWeather>() {
            public ForecastDayWeather createFromParcel(Parcel in) {
                ForecastDayWeather r = new ForecastDayWeather();
                r.readFromParcel(in);
                return r;
            }

            public ForecastDayWeather[] newArray(int size) {
                return new ForecastDayWeather[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        public void setCityId(String cityId) {
            Integer integer = Integer.valueOf(cityId);
            if (integer != null) {
                mCityId = integer.intValue();
            }
        }

        public void setCityId(int cityId) {
            mCityId = cityId;
        }

        public int getCityId() {
            return mCityId;
        }

        public void setDayOfWeek(String day) {
            Integer integer= Integer.valueOf(day);
            if(integer != null){
                mDayOfWeek = integer.intValue();
            }
        }

        public void setDayOfWeek(int day) {
                mDayOfWeek = day;
        }

        public int getDayOfWeek() {
            return mDayOfWeek;
        }

        public void setConditionTypeId(String typeId) {
            String weathMap = sWeatherMap.get(typeId);
            Integer integer= Integer.valueOf(weathMap);
            if(integer != null){
                int conditionTypeId = integer.intValue();
                if (conditionTypeId < 0) {
                    //Default is sunny.
                    mConditionTypeId = 0;
                }
                mConditionTypeId = conditionTypeId;
            }
        }

        public void setConditionTypeId(int typeId) {
                mConditionTypeId = typeId;
        }

        public int getConditionTypeId() {
            return mConditionTypeId;
        }

        public void setTempLow(String tempLow) {
            Double db = Double.valueOf(tempLow);
            if(db != null){
                mTempLow = db.doubleValue();
            }
        }

        public void setTempLow(double tempLow) {
                mTempLow = tempLow;
        }

        public double getTempLow() {
            return mTempLow;
        }

        public void setTempHigh(String tempHigh) {
            Double db = Double.valueOf(tempHigh);
            if(db != null){
                mTempHigh = db.doubleValue();
            }
        }

        public void setTempHigh(double tempHigh) {
                mTempHigh = tempHigh;
        }

        public double getTempHigh() {
            return mTempHigh;
        }

        public static void setWeatherMap(HashMap<String, String> weatherMap) {
            sWeatherMap = weatherMap;
        }

        public void convertTemperature(int fromTempType, int toTemptype) {
            mTempLow = Weather.convertTemperature(mTempLow, fromTempType, toTemptype);
            mTempHigh = Weather.convertTemperature(mTempHigh, fromTempType, toTemptype);
        }

        @Override
        public String toString() {
            return "ForecastDayWeather [mCityId=" + mCityId + ", mDayOfWeek=" + mDayOfWeek
                    + ", mConditionTypeId=" + mConditionTypeId + ", mTempLow=" + mTempLow
                    + ", mTempHigh=" + mTempHigh + "]";
        }

    }

    public static class CurrentWeather extends ForecastDayWeather implements Parcelable {
        double mTempCurrent = -1;
        String mWindDirection;
        double mWindSpeed = -1;
        int mPollutionIndex = -1;
        String mSunrise = null;
        String mSunset = null;
        String mMoreUrl = null;
        String mTimeZone = null;
        int mHumidity = -1;
        int mPmLungs = -1;
        int mPmResoprable = -1;
        long mLastUpdated = -1;

        public CurrentWeather() {

        }

        public CurrentWeather(CurrentWeather cWeather) {
            super(cWeather);
            mTempCurrent = cWeather.getTempCurrent();
            mWindDirection = cWeather.getWindDirection();
            mWindSpeed = cWeather.getWindSpeed();
            mPollutionIndex = cWeather.getPollutionIndex();
            mSunrise = cWeather.getSunrise();
            mSunset = cWeather.getSunset();
            mMoreUrl = cWeather.getMoreUrl();
            mTimeZone = cWeather.getTimeZone();
            mHumidity = cWeather.getHumidity();
            mPmLungs = cWeather.getPmLungs();
            mPmResoprable = cWeather.getPmResoprable();
            mLastUpdated = cWeather.getLastUpdated();
        }

        @Override
        public void writeToParcel(Parcel out, int flags) {
            super.writeToParcel(out, flags);
            out.writeDouble(mTempCurrent);
            out.writeString(mWindDirection);
            out.writeDouble(mWindSpeed);
            out.writeInt(mPollutionIndex);
            out.writeString(mSunrise);
            out.writeString(mSunset);
            out.writeString(mMoreUrl);
            out.writeString(mTimeZone);
            out.writeInt(mHumidity);
            out.writeInt(mPmLungs);
            out.writeInt(mPmResoprable);
            out.writeLong(mLastUpdated);
        }

        /**
         * read data from Parcel package.
         * @param in input a city parcel
         */
        public void readFromParcel(Parcel in) {
            super.readFromParcel(in);
            mTempCurrent = in.readDouble();
            mWindDirection = in.readString();
            mWindSpeed = in.readDouble();
            mPollutionIndex = in.readInt();
            mSunrise = in.readString();
            mSunset = in.readString();
            mMoreUrl = in.readString();
            mTimeZone = in.readString();
            mHumidity = in.readInt();
            mPmLungs = in.readInt();
            mPmResoprable = in.readInt();
            mLastUpdated = in.readLong();
        }

        public static final Parcelable.Creator<CurrentWeather> CREATOR
                      = new Parcelable.Creator<CurrentWeather>() {
            public CurrentWeather createFromParcel(Parcel in) {
                CurrentWeather r = new CurrentWeather();
                r.readFromParcel(in);
                return r;
            }

            public CurrentWeather[] newArray(int size) {
                return new CurrentWeather[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        public int getCityId() {
            return mCityId;
        }

        public void setCityId(String cityId) {
            Integer integer= Integer.valueOf(cityId);
            if(integer != null){
                mCityId = integer.intValue();
            }
        }

        public void setCityId(int cityId){
            mCityId = cityId;
        }

        public void setTempCurrent(String temp) {
            Double db = Double.valueOf(temp);
            if(db != null){
                mTempCurrent = db.doubleValue();
            }
        }

        public void setTempCurrent(double temp) {
                mTempCurrent = temp;
        }

        public double getTempCurrent() {
            return mTempCurrent;
        }

        public void setWindDirection(String wd) {
            mWindDirection = wd;
        }

        public String getWindDirection() {
            return mWindDirection;
        }

        public void setWindSpeed(String ws) {
            Double db= Double.valueOf(ws);
            if(db != null){
                mWindSpeed = db.doubleValue();
            }
        }

        public void setWindSpeed(double ws) {
                mWindSpeed = ws;
        }

        public double getWindSpeed() {
            return mWindSpeed;
        }

        public void setPollutionIndex(String pIdx) {
            Integer integer= Integer.valueOf(pIdx);
            if(integer != null){
                mPollutionIndex = integer.intValue();
            }
        }

        public void setPollutionIndex(int pIdx) {
            mPollutionIndex = pIdx;
        }

        public int getPollutionIndex() {
            return mPollutionIndex;
        }

        public void setSunrise(String rise) {
            mSunrise = rise;
        }

        public String getSunrise() {
            return mSunrise;
        }

        public void setSunset(String set) {
            mSunset = set;
        }

        public String getSunset() {
            return mSunset;
        }

        public void setMoreUrl(String url) {
            mMoreUrl = url;
        }

        public String getMoreUrl() {
            return mMoreUrl;
        }

        public void setTimeZone(String tz) {
            mTimeZone = tz;
        }

        public String getTimeZone() {
            return mTimeZone;
        }

        public int getHumidity() {
            return mHumidity;
        }

        public void setHumidity(String humidity) {
            Integer integer = Integer.valueOf(humidity);
            if (integer != null) {
                mHumidity = integer.intValue();
            }
        }

        public void setHumidity(int humidity) {
            mHumidity = humidity;
        }

        public int getPmLungs() {
            return mPmLungs;
        }

        public void setPmLungs(String pmLungs) {
            Integer integer = Integer.valueOf(pmLungs);
            if (integer != null) {
                mPmLungs = integer.intValue();
            }
        }

        public void setPmLungs(int pmLungs) {
            mPmLungs = pmLungs;
        }

        public int getPmResoprable() {
            return mPmResoprable;
        }

        public void setPmResoprable(String pmResoprable) {
            Integer integer = Integer.valueOf(pmResoprable);
            if (integer != null) {
                mPmResoprable = integer.intValue();
            }
        }

        public void setPmResoprable(int pmResoprable) {
            mPmResoprable = pmResoprable;
        }

        public long getLastUpdated() {
            return mLastUpdated;
        }

        public void setLastUpdated(String lastUpdated) {
            Long lon = Long.valueOf(lastUpdated);
            if(lon != null){
                mLastUpdated = lon.longValue();
            }
        }

        public void setLastUpdated(long lastUpdated) {
            mLastUpdated = lastUpdated;
        }

        @Override
        public String toString() {
            return "CurrentWeather [mTempCurrent=" + mTempCurrent + ", mWindDirection="
                    + mWindDirection + ", mWindSpeed=" + mWindSpeed + ", mPollutionIndex="
                    + mPollutionIndex + ", mSunrise=" + mSunrise + ", mSunset=" + mSunset
                    + ", mMoreUrl=" + mMoreUrl + ", mTimeZone=" + mTimeZone + ", mHumidity="
                    + mHumidity + ", mPmLungs=" + mPmLungs + ", mPmResoprable=" + mPmResoprable
                    + ", mCityId=" + mCityId + ", mDayOfWeek=" + mDayOfWeek + ", mConditionTypeId="
                    + mConditionTypeId + ", mTempLow=" + mTempLow + "]";
        }

        public void convertTemperature(int fromTempType, int toTemptype) {
            super.convertTemperature(fromTempType, toTemptype);
            mTempCurrent = Weather.convertTemperature(mTempCurrent, fromTempType, toTemptype);
        }

    }
}
