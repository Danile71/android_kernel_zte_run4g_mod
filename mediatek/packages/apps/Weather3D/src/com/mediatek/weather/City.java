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

package com.mediatek.weather;

import android.os.Parcel;
import android.os.Parcelable;

public class City implements Parcelable {
    String mCityName = null;
    int mCityId = -1;
    String mCountry = null;
    String mState = null;
    double mLongitude = -1;
    double mLatitude = -1;
    long mLastUpdated = -1;
    int mPosition = -1;

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mCityName);
        out.writeInt(mCityId);
        out.writeString(mCountry);
        out.writeString(mState);
        out.writeDouble(mLongitude);
        out.writeDouble(mLatitude);
        out.writeLong(mLastUpdated);
        out.writeInt(mPosition);
    }

    /**
     * read data from Parcel package.
     * @param in input a city parcel
     */
    public void readFromParcel(Parcel in) {
        mCityName = in.readString();
        mCityId = in.readInt();
        mCountry = in.readString();
        mState = in.readString();
        mLongitude = in.readDouble();
        mLatitude = in.readDouble();
        mLastUpdated = in.readLong();
        mPosition = in.readInt();
    }

    public static final Parcelable.Creator<City> CREATOR = new Parcelable.Creator<City>() {
        public City createFromParcel(Parcel in) {
            City r = new City();
            r.readFromParcel(in);
            return r;
        }

        public City[] newArray(int size) {
            return new City[size];
        }
    };

    @Override
    public int hashCode() {
        return this.mCityId;
    }

    @Override
    public boolean equals(Object o) {
        if (super.equals(o)) {
            return true;
        } else {
            City city = (City) o;
            if (mCityId == city.mCityId) {
                return true;
            }
            return false;
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public String getCityName() {
        return mCityName;
    }

    public void setCityName(String name) {
        mCityName = name;
    }

    public int getCityId() {
        return mCityId;
    }

    public void setCityId(String id) {
        Integer integer= Integer.valueOf(id);
        if(integer != null){
            mCityId = integer.intValue();
        }
    }

    public void setCityId(int id) {
        mCityId = id;
    }

    public String getCountry() {
        return mCountry;
    }

    public void setCountry(String country) {
        mCountry = country;
    }

    public String getState() {
        return mState;
    }

    public void setState(String state) {
        mState = state;
    }

    public double getLongitude() {
        return mLongitude;
    }

    public void setLongitude(double longitude) {
        mLongitude = longitude;
    }

    public void setLongitude(String longitude) {
        Double db= Double.valueOf(longitude);
        if(db != null){
            mLongitude = db.doubleValue();
        }
    }

    public double getLatitude() {
        return mLatitude;
    }

    public void setLatitude(double latitude) {
        mLatitude = latitude;
    }

    public void setLatitude(String latitude) {
        Double db = Double.valueOf(latitude);
        if(db != null){
            mLatitude = db.doubleValue();
        }
    }

    public long getLastUpdated() {
        return mLastUpdated;
    }

    public void setLastUpdated(long lastUpdated) {
        mLastUpdated = lastUpdated;
    }

    public void setLastUpdated(String lastUpdated) {
        Long lon = Long.valueOf(lastUpdated);
        if(lon != null){
            mLastUpdated = lon.longValue();
        }
    }

    public int getPosition() {
        return mPosition;
    }

    public void setPosition(int position) {
            mPosition = position;
    }

    public void setPosition(String position) {
        Integer integer= Integer.valueOf(position);
        if(integer != null){
            mPosition = integer.intValue();
        }
    }

    @Override
    public String toString() {
        return "City [mCityName=" + mCityName + ", mCityId=" + mCityId + ", mCountry=" + mCountry
                + ", mState=" + mState + ", mLongitude=" + mLongitude + ", mLatitude=" + mLatitude
                + ", mLastUpdated=" + mLastUpdated + ", mPosition=" + mPosition + "]";
    }

}
