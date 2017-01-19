package com.mediatek.common.ims.mo;

import android.os.Parcelable;
import android.os.Parcel;
import android.util.Log;

public class ImsPhoneCtx implements Parcelable {
    private String mPhoneCtx;    
    private String[] mPhoneCtxIpuis;

    public ImsPhoneCtx(){
        mPhoneCtx = "";        
    }

    public ImsPhoneCtx(String phoneCtx, String[] phoneCtxIpuis){
        mPhoneCtx = phoneCtx;
        mPhoneCtxIpuis = phoneCtxIpuis;
    }

    public String getPhoneCtx(){
        return mPhoneCtx;
    }

    public String[] getPhoneCtxIpuis(){
        return mPhoneCtxIpuis;
    }

    public void setPhoneCtx(String phoneCtx){
        mPhoneCtx = phoneCtx;
    }

    public void setPhoneCtxIpuis(String[] phoneCtxIpuis){
        mPhoneCtxIpuis = phoneCtxIpuis;
    }

    @Override
    public String toString() {
        synchronized (this) {
            StringBuilder builder = new StringBuilder("ImsPhoneCtx: ");
            builder.append("Phone Context: ").append(mPhoneCtx).append(", Address Type: ");
            if(mPhoneCtxIpuis != null){
                for(String ipuis:mPhoneCtxIpuis){
                    builder.append("-").append(ipuis);
                }
            }
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
            dest.writeString(mPhoneCtx);
            dest.writeStringArray(mPhoneCtxIpuis);
        }
    }

    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public static final Creator<ImsPhoneCtx> CREATOR =
        new Creator<ImsPhoneCtx>() {
            public ImsPhoneCtx createFromParcel(Parcel in) {
                String phoneCtx = in.readString();                
                String[] phoneCtxIpuis = in.createStringArray();                
                ImsPhoneCtx imsPhoneCtx = new ImsPhoneCtx(phoneCtx, phoneCtxIpuis);
                Log.i("ImsPhoneCtx", "imsPhoneCtx:" + imsPhoneCtx);
                return imsPhoneCtx;
            }

            public ImsPhoneCtx[] newArray(int size) {
                return new ImsPhoneCtx[size];
            }
        };
    
}

