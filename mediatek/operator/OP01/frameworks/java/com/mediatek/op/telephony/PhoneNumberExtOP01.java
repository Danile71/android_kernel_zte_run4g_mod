package com.mediatek.op.telephony;

public class PhoneNumberExtOP01 extends PhoneNumberExt {
    public boolean isPauseOrWait(char c) {
        return (c == 'p' || c == 'P' || c == 'w' || c == 'W');
    }
}
