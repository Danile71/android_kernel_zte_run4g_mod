package com.mediatek.common.ims.mo;

import android.os.Parcelable;
import android.os.Parcel;

public class ImsLboPcscf implements Parcelable {
    private String mLboPcscfAddress;
    private String mLboPcscfAddressType;

    public ImsLboPcscf(){
        mLboPcscfAddress = "";
        mLboPcscfAddressType = "";
    }

    public ImsLboPcscf(String lboPcscfAddress, String lboPcscfAddressType){
        mLboPcscfAddress = lboPcscfAddress;
        mLboPcscfAddressType = lboPcscfAddressType;
    }

    public String getLboPcscfAddress(){
        return mLboPcscfAddress;
    }

    public String getLboPcscfAddressType(){
        return mLboPcscfAddressType;
    }

    public void setLboPcscfAddress(String lboPcscfAddress){
        mLboPcscfAddress = lboPcscfAddress;
    }

    public void setLboPcscfAddressType(String lboPcscfAddressType){
        mLboPcscfAddressType = lboPcscfAddressType;
    }


    @Override
    public String toString() {
        synchronized (this) {
            StringBuilder builder = new StringBuilder("ImsLboPcscf: ");
            builder.append("LBO PCSCF Address: ").append(mLboPcscfAddress).
            append(", Address Type: ").append(mLboPcscfAddressType);
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
            dest.writeString(mLboPcscfAddress);
            dest.writeString(mLboPcscfAddressType);            
        }
    }

    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public static final Creator<ImsLboPcscf> CREATOR =
        new Creator<ImsLboPcscf>() {
            public ImsLboPcscf createFromParcel(Parcel in) {
                String lboPcscfAddress = in.readString();
                String lboPcscfAddressType = in.readString();
                ImsLboPcscf imsLboPcscf = new ImsLboPcscf(lboPcscfAddress, lboPcscfAddressType);
                return imsLboPcscf;
            }

            public ImsLboPcscf[] newArray(int size) {
                return new ImsLboPcscf[size];
            }
        };
    
}

