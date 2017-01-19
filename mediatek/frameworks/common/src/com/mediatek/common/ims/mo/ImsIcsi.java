package com.mediatek.common.ims.mo;

import android.os.Parcelable;
import android.os.Parcel;

public class ImsIcsi implements Parcelable {
    private String mIcsi;
    private boolean mIsAllocated;

    public ImsIcsi(){
        mIcsi = "";
        mIsAllocated = false;
    }

    public ImsIcsi(String icsi, boolean isAllocated){
        mIcsi = icsi;
        mIsAllocated = isAllocated;
    }

    public String getIcsi(){
        return mIcsi;
    }

    public boolean getIsAllocated(){
        return mIsAllocated; 
    }

    public void setIcsi(String icsi){
        mIcsi = icsi;
    }

    public void setIsAllocated(boolean isAllocated){
        mIsAllocated = isAllocated;
    }

    @Override
    public String toString() {
        synchronized (this) {
            StringBuilder builder = new StringBuilder("ImsIsci: ");
            builder.append("ICSI: ").append(mIcsi).append(", isAllocated: ").append(mIsAllocated);
            return builder.toString();
        }
    }

    /**
     * Implement the Parcelable interface
     * @hide
     */
    public int describeContents() {
        return 0;
    }
    
    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public void writeToParcel(Parcel dest, int flags) {
         synchronized (this) {
            dest.writeString(mIcsi);
            dest.writeInt(mIsAllocated ? 1 : 0);
        }
    }

    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public static final Creator<ImsIcsi> CREATOR =
        new Creator<ImsIcsi>() {
            public ImsIcsi createFromParcel(Parcel in) {
                String icsi = in.readString();
                boolean isAllocated = in.readInt() != 0;                
                ImsIcsi ImsIcsi = new ImsIcsi(icsi, isAllocated);
                return ImsIcsi;
            }

            public ImsIcsi[] newArray(int size) {
                return new ImsIcsi[size];
            }
        };
    
}

