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

package com.mediatek.weather3dwidget;

public class WeatherInfo {
    private int mCityIndex;
    private int mTotalCity;
    protected String mLocationName;
    protected String mTimezone;
    protected int mTempType;
    protected double mCurrentTemp;
    protected double mTempHigh;
    protected double mTempLow;
    protected long mLastUpdated = 0;
    protected int mCondition;
    protected ForecastData[] mForecastData;
    protected int mResult = -1;

    public int getCityIndex() {
        return mCityIndex;
    }

    public int getTotalCity() {
        return mTotalCity;
    }

    public String getCityName() {
        return mLocationName;
    }

    public double getCurrentTemp() {
        return mCurrentTemp;
    }

    public int getTempType() {
        return mTempType;
    }

    public double getTempHigh() {
        return mTempHigh;
    }

    public double getTempLow() {
        return mTempLow;
    }

    public long getLastUpdated() {
        return mLastUpdated;
    }

    public int getCondition() {
        return mCondition;
    }

    public String getTimeZone() {
        return mTimezone;
    }

    public ForecastData[] getForecastData() {
        return mForecastData.clone();
    }

    public int getResult() {
        return mResult;
    }

    public void setCityIndex(int index) {
        mCityIndex = index;
    }

    public void setTotalCity(int total) {
        mTotalCity = total;
    }

    public void setCityName(String name) {
        mLocationName = name;
    }

    public void setTempType(int type) {
        mTempType = type;
    }

    public void setCurrentTemp(double temp) {
        mCurrentTemp = temp;
    }

    public void setTempHigh(double high) {
        mTempHigh = high;
    }

    public void setTempLow(double low) {
        mTempLow = low;
    }

    public void setLastUpdated(long update) {
        mLastUpdated = update;
    }

    public void setCondition(int condition) {
        mCondition = condition;
    }

    public void setTimeZone(String timeZone) {
        mTimezone = timeZone;
    }

    public void setResult(int result) {
        mResult = result;
    }

    public void setForecastData(ForecastData[] data) {
        mForecastData = null;
        mForecastData = new ForecastData[data.length];
        System.arraycopy(data, 0, mForecastData, 0, data.length);
    }
}
