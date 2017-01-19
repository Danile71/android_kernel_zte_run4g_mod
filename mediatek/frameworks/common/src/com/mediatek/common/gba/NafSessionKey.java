package com.mediatek.common.gba;

import android.os.Parcel;
import android.os.Parcelable;

/** 
 * @hide
 */
public class NafSessionKey implements Parcelable {
    private String mBtid;
    private byte[] mKey;
    private String mKeylifetime;
    private String mNafKeyName;

    public NafSessionKey() {
        super();
        
    }

    public NafSessionKey(final String btid, final byte[] key, final String keylifetime) {
        mBtid = btid;
        mKey = key;
        mKeylifetime = keylifetime;
    }

    public String getBtid() {
        return mBtid;
    }

    public void setBtid(final String btid) {
       mBtid = btid;
    }

    public byte[] getKey() {
        return mKey;
    }    
    
    public void setKey(final byte[] key) {
        mKey = key;
    }
    
    public String getKeylifetime() {
        return mKeylifetime;
    }

    public void setKeylifetime(final String keylifetime) {
        mKeylifetime = keylifetime;
    }

    public String getNafKeyName() {
        return mNafKeyName;
    }

    public void setNafKeyName(String nafKeyName) {
        mNafKeyName = nafKeyName;
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
        dest.writeString(mBtid);
        dest.writeByteArray(mKey);
        dest.writeString(mKeylifetime);
        dest.writeString(mNafKeyName);
    }

    /**
     * Implement the Parcelable interface.
     * @hide
     */
    public static final Creator<NafSessionKey> CREATOR =
        new Creator<NafSessionKey>() {
            public NafSessionKey createFromParcel(Parcel in) {
                NafSessionKey nafSessionKey = new NafSessionKey();
                String btid = in.readString();
                if(btid != null) {
                    try {
                        nafSessionKey.setBtid(btid);
                    } catch (Exception e) {
                        return null;
                    }
                }
                byte[] key = in.createByteArray();
                if(key != null) {
                    try {
                        nafSessionKey.setKey(key);
                    } catch (Exception e) {
                        return null;
                    }
                }
                String keylifetime = in.readString();
                if(keylifetime != null) {
                    try {
                        nafSessionKey.setKeylifetime(keylifetime);
                    } catch (Exception e) {
                        return null;
                    }
                }
                String nafKeyName = in.readString();
                if(nafKeyName != null) {
                    try {
                        nafSessionKey.setNafKeyName(nafKeyName);
                    } catch (Exception e) {
                        return null;
                    }                    
                }
                return nafSessionKey;
            }

            public NafSessionKey[] newArray(int size) {
                return new NafSessionKey[size];
            }            
        };

    @Override
    public String toString(){
        synchronized (this) {
            StringBuilder builder = new StringBuilder("NafSessionKey: btid:");
            builder.append(mBtid).append(":").append(mKeylifetime).append(":").append(mNafKeyName);
            return builder.toString();
        }        
    }
}