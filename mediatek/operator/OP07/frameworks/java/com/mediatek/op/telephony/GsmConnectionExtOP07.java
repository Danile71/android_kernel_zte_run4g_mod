package com.mediatek.op.telephony;

public class GsmConnectionExtOP07 extends GsmConnectionExt {

    public int getFirstPauseDelayMSeconds(int defaultValue) {
       return 3000;  //Return 3 seconds.
    }
}
