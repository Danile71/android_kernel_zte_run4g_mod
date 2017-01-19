package com.mediatek.op.telephony;

import android.util.Log;
import java.util.Arrays;

public class PhoneNumberExtOP09 extends PhoneNumberExt {
    // CT min match is 11 digits
    public int getMinMatch() {
        return 11;
    }
}
