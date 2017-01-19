package com.mediatek.common.ims.mo;

import android.os.Parcelable;
import android.os.Parcel;

public class ImsXcapInfo implements Parcelable {
    private String mXcapRootUri;    
    private String mXcapAuth;
    private String mXcapAuthSecret;
    private String mXcapAuthType;
        
    public ImsXcapInfo(){
        mXcapRootUri = "";
        mXcapAuth = "";
        mXcapAuthSecret = "";
        mXcapAuthType = "";
    }

    public ImsXcapInfo(String xcapRootUri, String xcapAuth, String xcapAuthSecret, String xcapAuthType){
        mXcapRootUri = xcapRootUri;
        mXcapAuth = xcapAuth;
        mXcapAuthSecret = xcapAuthSecret;
        mXcapAuthType = xcapAuthType;
    }

    public String getXcapRootUri(){
        return mXcapRootUri;
    }
    
    public String getXcapAuth(){
        return mXcapAuth;
    }

    public String getXcapAuthSecret(){
        return mXcapAuthSecret;
    }
    
    public String getXcapAuthType(){
        return mXcapAuthType;
    }
    
    public void setXcapRootUri(String xcapRootUri){
        mXcapRootUri = xcapRootUri;
    }
    
    public void setXcapAuth(String xcapAuth){
        mXcapAuth = xcapAuth;
    }

    public void setXcapAuthSecret(String xcapAuthSecret){
        mXcapAuthSecret = xcapAuthSecret;
    }
    
    public void setXcapAuthType(String xcapAuthType){
        mXcapAuthType = xcapAuthType;
    }    

    @Override
    public String toString() {
        synchronized (this) {
            StringBuilder builder = new StringBuilder("ImsXcapInfo: ");
            builder.append("XCAP Root Uri: ").append(mXcapRootUri).
            append(", Auth info:").append(mXcapAuth).append(":").append(mXcapAuthSecret).append(":").append(mXcapAuthType);
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
            dest.writeString(mXcapRootUri);
            dest.writeString(mXcapAuth);
            dest.writeString(mXcapAuthSecret);
            dest.writeString(mXcapAuthType);
            
        }
    }

    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public static final Creator<ImsXcapInfo> CREATOR =
        new Creator<ImsXcapInfo>() {
            public ImsXcapInfo createFromParcel(Parcel in) {
                String xcapRootUri = in.readString();
                String xcapAuth = in.readString();
                String xcapAuthSecret = in.readString();
                String xcapAuthType = in.readString();
                ImsXcapInfo imsXcapInfo = new ImsXcapInfo(xcapRootUri, xcapAuth, xcapAuthSecret, xcapAuthType);
                return imsXcapInfo;
            }

            public ImsXcapInfo[] newArray(int size) {
                return new ImsXcapInfo[size];
            }
        };
    
}

