package com.mediatek.contacts.plugin.vcard;

import android.accounts.Account;
import android.content.Context;
import android.util.Log;

import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.ImportExportEnhancementExtension;
import com.android.vcard.VCardComposer;
import com.android.vcard.VCardEntryConstructor;

public class OP09ImportExportEnhancementExtension extends ImportExportEnhancementExtension {
    private static final String TAG = "OP09ImportExportEnhancementExtension";

    private Context mPluginContext;

    public OP09ImportExportEnhancementExtension(Context context) {
        mPluginContext = context;
    }

    @Override
    public String getCommond() {
        return ContactPluginDefault.COMMD_FOR_OP09;
    }

    @Override
    public VCardEntryConstructor getVCardEntryConstructorExt(int estimatedVCardType,
            Account account,
            String estimatedCharset, String commond) {
        Log.i(TAG, "OP09ImportExportEnhancementExtension importVCardExtension " + commond);
        if (!ContactPluginDefault.COMMD_FOR_OP09.equals(commond)) {
            return super.getVCardEntryConstructorExt(estimatedVCardType, account, estimatedCharset,
                    commond);
        }

        return new OP09VCardEntryConstructor(estimatedVCardType, account, estimatedCharset);
    }

    @Override
    public VCardComposer getVCardComposerExt(Context context, int vcardType,
            boolean careHandlerErrors, String commond) {
        Log.i(TAG, "OP09ImportExportEnhancementExtension exportVCardExtension " + commond);
        if (!ContactPluginDefault.COMMD_FOR_OP09.equals(commond)) {
            return super.getVCardComposerExt(context, vcardType, careHandlerErrors, commond);
        }

        return new OP09VCardComposer(context, vcardType, careHandlerErrors);
    }

}
