package com.mediatek.contacts.plugin;

import android.content.Context;
import android.util.Log;

import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.IccCardExtension;
import com.mediatek.contacts.ext.ImportExportEnhancementExtension;
import com.mediatek.contacts.ext.SimServiceExtension;
import com.mediatek.contacts.plugin.OP09ContactListExtension;
import com.mediatek.contacts.plugin.simcontacts.OP09SimServiceExtension;
import com.mediatek.contacts.plugin.vcard.OP09ImportExportEnhancementExtension;
import com.mediatek.phone.SIMInfoWrapper;
import com.mediatek.contacts.ext.ContactDetailEnhancementExtension;

import java.util.Date;
import java.util.TimeZone;

public class OP09ContactsPlugin extends ContactPluginDefault {

    private static final String TAG = "OP09ContactsPlugin";

    private Context mPluginContext;
    private int mTimezoneRawOffset = 0;

    public OP09ContactsPlugin(Context context) {
        mPluginContext = context;
        SIMInfoWrapper.getDefault().init(mPluginContext);
    }
    public ContactDetailEnhancementExtension createContactDetailEnhancementExtension() {
        return new OP09ContactDetailEnhancementExtension(mPluginContext);
    }

    public SimServiceExtension createSimServiceExtension() {
        return new OP09SimServiceExtension(mPluginContext);
    }

    public IccCardExtension createIccCardExtension() {
        return new OP09IccCardExtension(mPluginContext);
    }

    public ContactListExtension createContactListExtension() {
        return new OP09ContactListExtension();
    }

    public ImportExportEnhancementExtension createImportExportEnhancementExtension() {
        return new OP09ImportExportEnhancementExtension(mPluginContext);
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
