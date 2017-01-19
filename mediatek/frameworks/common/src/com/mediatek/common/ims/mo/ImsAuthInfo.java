package com.mediatek.common.ims.mo;

import android.os.Parcelable;
import android.os.Parcel;

public class ImsAuthInfo implements Parcelable {
    private String mAuthType;    
    private String mRealm;
    private String mUserName;
    private String mUserPwd;
    
    
    public ImsAuthInfo(){
        mAuthType = "";
        mRealm = "";
        mUserName = "";
        mUserPwd = "";
    }

    public ImsAuthInfo(String authType, String realm, String userName, String userPwd){
        mAuthType = authType;
        mRealm = realm;
        mUserName = userName;
        mUserPwd = userPwd;
    }

    public String getAuthType(){
        return mAuthType;
    }
    
    public String getRelam(){
        return mRealm;
    }
    
    public String getUserName(){
        return mUserName;
    }
    
    public String getUserPwd(){
        return mUserPwd;
    }

    public void setAuthType(String authType){
        mAuthType = authType;
    }
    
    public void setRelam(String relam){
        mRealm = relam;
    }
    
    public void setUserName(String userName){
        mUserName = userName;
    }
    
    public void setUserPwd(String userPwd){
        mUserPwd = userPwd;
    }

    @Override
    public String toString() {
        synchronized (this) {
            StringBuilder builder = new StringBuilder("Ims Auth Info: ");            
            builder.append(mAuthType).append(":").append(mRealm).append(":").append(mUserName).append(":").append(mUserPwd);
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
            dest.writeString(mAuthType);
            dest.writeString(mRealm);
            dest.writeString(mUserName);
            dest.writeString(mUserPwd);
            
        }
    }

    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public static final Creator<ImsAuthInfo> CREATOR =
        new Creator<ImsAuthInfo>() {
            public ImsAuthInfo createFromParcel(Parcel in) {
                String authType = in.readString();
                String realm = in.readString();
                String userName = in.readString();
                String userPwd = in.readString();
                ImsAuthInfo imsAuthInfo = new ImsAuthInfo(authType, realm, userName, userPwd);
                return imsAuthInfo;
            }

            public ImsAuthInfo[] newArray(int size) {
                return new ImsAuthInfo[size];
            }
        };
    
}

