package com.mediatek.gba.cache;


import com.mediatek.common.gba.NafSessionKey;
import com.mediatek.gba.element.NafId;

import java.util.Arrays;

class GbaKeysCacheEntryKey {

    private NafId mNafId;
    private int mGbaType;

    public GbaKeysCacheEntryKey(NafId nafId, int gbaType) {        
        mNafId = nafId;
        mGbaType = gbaType;
    }

    public int getGbaType() {
        return mGbaType;
    }

    public void setGbaType(int gbaType) {
        mGbaType = gbaType;
    }

    public NafId getNafId() {
        return mNafId;
    }

    public void setNafId(NafId nafId) {
        mNafId = nafId;
    }

    @Override
    public int hashCode() {        
        return Arrays.hashCode(mNafId.getNafIdBin());
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        GbaKeysCacheEntryKey other = (GbaKeysCacheEntryKey) obj;
        if (mGbaType != other.mGbaType)
            return false;
        if (mNafId == null) {
            if (other.mNafId != null)
                return false;
        } else if (!mNafId.equals(other.mNafId))
            return false;
        return true;
    }

    @Override
    public String toString() {
        return "GbaKeysCacheEntryKey [nafId=" + mNafId + ", gbaType=" + mGbaType + "]";
    }

}
