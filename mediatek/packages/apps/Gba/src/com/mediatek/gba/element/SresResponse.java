package com.mediatek.gba.element;

import android.util.Base64;
import android.util.Log;

import com.mediatek.gba.auth.KeyCalculator;
import com.mediatek.gba.telephony.TelephonyUtils;

import java.io.IOException;
import java.util.Arrays;

public class SresResponse {
    private static final String TAG = "SresResponse";
    private byte[] mKc;
    private byte[] mKsInput;
    private byte[] mSres;    
    private byte[] mRand;    

    private static final int RAND_LEN       = 16;
    private static final int KS_INPUT_LEN   = 16;
    private static final int SRES_LEN       = 4;
    private static final int KC_LEN         = 8;

    private SresResponse() {        
        mKc = null;
        mSres = null;
        mKsInput = null;        
        mRand = null;
    }

    private SresResponse(byte[] sres, byte[] kc, byte[] ksInput, byte[] rand){
        mKc = kc;
        mSres = sres;
        mKsInput = ksInput;        
        mRand = rand;
    }

    public static SresResponse getSresResponse(String nonce){
        byte[] inputBytes = null;
        try {
            inputBytes = Base64.decode(nonce, Base64.DEFAULT);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "SresResponse: Bad base64 sim result: " + nonce);
            return null;
        }

        if (inputBytes.length < RAND_LEN * 2) {
            Log.e(TAG, "decode2GResult: len = " + inputBytes.length);
            return null;
        }

        byte[] rand = new byte[RAND_LEN];
        rand = Arrays.copyOfRange(inputBytes, 0, RAND_LEN);
        byte[] ksInput = new byte[KS_INPUT_LEN];
        ksInput = Arrays.copyOfRange(inputBytes, RAND_LEN, RAND_LEN + KS_INPUT_LEN);

        Log.i(TAG, "[2G] rand:" + getHexByteArray(rand));
        Log.i(TAG, "[2G] ksinput:" + getHexByteArray(ksInput));
        byte[] result  = TelephonyUtils.calculate2GSres(rand);
        if(result != null){
            byte[] sres = new byte[SRES_LEN];
            sres = Arrays.copyOfRange(result, 1, 1 + SRES_LEN);
            byte[] kc = new byte[KC_LEN];            
            kc = Arrays.copyOfRange(result, 6, 6 + KC_LEN);
            
            Log.i(TAG, "[2G] sres:" + getHexByteArray(sres));
            Log.i(TAG, "[2G] kc:" + getHexByteArray(kc));
            
            return new SresResponse(sres, kc, ksInput, rand);            
        }
        return null;
    }
    
    public byte[] getSres() {
        return mSres;
    }

    public byte[] getKc() {
        return mKc;
    }

    public byte[] getKsInput() {
        return mKsInput;
    }

    public byte[] getRand() {
        return mRand;
    }

    private static String getHexByteArray(byte[] data){
        return TelephonyUtils.bytesToHex(data);        
    }

    @Override
    public String toString() {
        return "SresResponse [kc=" + getHexByteArray(mKc) + ", sRes=" + getHexByteArray(mSres) + "]";
    }

}