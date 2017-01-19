package com.mediatek.contacts.plugin.vcard;


import android.accounts.Account;

import com.android.vcard.VCardConfig;
import com.android.vcard.VCardEntry;
import com.android.vcard.VCardEntryConstructor;

public class OP09VCardEntryConstructor extends VCardEntryConstructor{

    private static String LOG_TAG = "OP09VCardEntryConstructor";
    
    private final int mVCardType;
    private final Account mAccount;
    
    public OP09VCardEntryConstructor() {
        this(VCardConfig.VCARD_TYPE_V21_GENERIC, null, null);
    }

    public OP09VCardEntryConstructor(final int vcardType) {
        this(vcardType, null, null);
    }

    public OP09VCardEntryConstructor(final int vcardType, final Account account) {
        this(vcardType, account, null);
    }

    /**
     * @deprecated targetCharset is not used anymore.
     * Use {@link #VCardEntryConstructor(int, Account)}
     */
    @Deprecated
    public OP09VCardEntryConstructor(final int vcardType, final Account account,
            String targetCharset) {
        super(vcardType, account, targetCharset);
        mVCardType = vcardType;
        mAccount = account;
    }
    
    @Override
    public VCardEntry getVCardEntry() {
        return new OP09VCardEntry(mVCardType, mAccount);
    }
    
}
