package com.mediatek.contacts.plugin;

import android.content.Context;

import com.mediatek.contacts.ext.ContactAccountExtension;
import com.mediatek.contacts.ext.ContactDetailExtension;
import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.LabeledEditorExtension;
import com.mediatek.contacts.plugin.AASSNEOptions;

public class ContactsPlugin extends ContactPluginDefault {

    private Context mContext = null;
    public static final String COMMD_FOR_AAS;
    static
    {
      if (com.mediatek.contacts.plugin.AASSNEOptions.AAS)
      {
         COMMD_FOR_AAS = "ExtensionForAAS";
      }
      else
      {
         COMMD_FOR_AAS = "NotSupported";
      }
    }
    
    public ContactsPlugin(Context context) {
        mContext = context;
    }

    /**
     * Return a new OpContactAccountExtension
     */
    public ContactAccountExtension createContactAccountExtension() {
        return new OpContactAccountExtension(mContext);
    }

    /**
     * Return a new OpContactDetailExtension
     */
    public ContactDetailExtension createContactDetailExtension() {
        return new OpContactDetailExtension(mContext);
    }

    /**
     * Return a new OpContactListExtension
     */
    public ContactListExtension createContactListExtension() {
        return new OpContactListExtension();
    }
    
    /**
     * return a new OpLabeledEditorExtension
     */
    public LabeledEditorExtension createLabeledEditorExtension() {
        return new OpLabeledEditorExtension(mContext);
    }

}
