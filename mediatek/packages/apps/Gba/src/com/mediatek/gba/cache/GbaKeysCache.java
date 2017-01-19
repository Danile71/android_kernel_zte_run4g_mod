package com.mediatek.gba.cache;

import android.util.Log;

import com.mediatek.common.gba.NafSessionKey;

import com.mediatek.gba.element.NafId;

import java.text.SimpleDateFormat;

import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.TimeZone;

public class GbaKeysCache{
    private static final String TAG = "GbaKeysCache";

    private static final String UTC_PATTERN_TIMEZONE = "yyyy-MM-dd'T'HH:mm:ss'Z'";
    private static final String UTC_PATTERN = "yyyy-MM-dd'T'HH:mm:ss";
    
    private Map<GbaKeysCacheEntryKey, NafSessionKey> map = new HashMap<GbaKeysCacheEntryKey, NafSessionKey>();

    public boolean isExpiredKey(NafId nafId, int gbaType) {
        boolean isExpired = true;
        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, gbaType);
        
        boolean res = map.containsKey(key);
        Log.i(TAG, "   containsKey=" + res);
        
        if (res) {
            NafSessionKey nafSessionKey = map.get(key);
            //Check time is expired or not

            //Get current UTC time
            Calendar calenar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));            
            long cTime = calenar.getTimeInMillis();
            Date expiredDate = getExpiredDate(nafSessionKey.getKeylifetime());
            if(expiredDate == null){
                Log.e(TAG, "Can't get expired date");
                return true;
            }
            calenar.setTime(expiredDate);
            long eTime = calenar.getTimeInMillis();
            return (eTime < cTime);
        }
        return isExpired;
    }

    public boolean hasKey(NafId nafId, int gbaType) {        
        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, gbaType);
        
        return map.containsKey(key);
    }

    public NafSessionKey getKeys(NafId nafId, int gbaType) {        
        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, gbaType);

        return map.get(key);
    }

    public void putKeys(NafId nafId, int gbaType, NafSessionKey sessionKey) {        
        if (sessionKey == null) {
            throw new IllegalArgumentException("The sessionKey mus be not null.");
        }

        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, gbaType);
        map.put(key, sessionKey);        
    }
    
    private Date getExpiredDate(String utcDate){
        SimpleDateFormat sDateFormat = null;
        Date expiredDate = null;
        
        try{
            Log.i(TAG, "Expired date:" + utcDate);

            if(utcDate.indexOf("Z") != -1){
                sDateFormat = new SimpleDateFormat(UTC_PATTERN_TIMEZONE);
            }else{
                sDateFormat = new SimpleDateFormat(UTC_PATTERN);
            }
            expiredDate = sDateFormat.parse(utcDate);
        }catch(Exception e){
            e.printStackTrace();
        }

        return expiredDate;
    }
}