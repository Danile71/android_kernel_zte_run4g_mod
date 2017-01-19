package com.mediatek.contacts.plugin.vcard;

import java.util.List;
import java.util.Map;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Event;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Im;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Note;
import android.provider.ContactsContract.CommonDataKinds.Organization;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.CommonDataKinds.Relation;
import android.provider.ContactsContract.CommonDataKinds.SipAddress;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.provider.ContactsContract.CommonDataKinds.Website;
import android.util.Log;

import com.android.vcard.VCardComposer;
import com.android.vcard.VCardConfig;
import com.android.vcard.VCardPhoneNumberTranslationCallback;

public class OP09VCardComposer extends VCardComposer{
    public static final String LOG_TAG = "OP09VCardComposer";
    
    private final int mVCardType;
    private final ContentResolver mContentResolver;
    private VCardPhoneNumberTranslationCallback mPhoneTranslationCallback;

    public OP09VCardComposer(final Context context, final int vcardType,
            final boolean careHandlerErrors) {
        this(context, vcardType, null, careHandlerErrors);
    }
    
    public OP09VCardComposer(final Context context, final int vcardType, String charset,
            final boolean careHandlerErrors) {
        this(context, context.getContentResolver(), vcardType, charset, careHandlerErrors);
    }
    
    public OP09VCardComposer(final Context context, ContentResolver resolver,
            final int vcardType, String charset, final boolean careHandlerErrors) {
        super(context,resolver,vcardType,charset,careHandlerErrors);
        mVCardType = vcardType;
        mContentResolver = resolver;
    }
    
    @Override
    public void setPhoneNumberTranslationCallback(VCardPhoneNumberTranslationCallback callback) {
        super.setPhoneNumberTranslationCallback(callback);
        mPhoneTranslationCallback = callback;
    }

    @Override
    public String buildVCard(Map<String, List<ContentValues>> contentValuesListMap) {
        if (contentValuesListMap == null) {
            Log.e(LOG_TAG, "The given map is null. Ignore and return empty String");
            return "";
        } else {
            OP09VCardBuilder builder = new OP09VCardBuilder(mVCardType, mCharset);
            builder.appendNameProperties(contentValuesListMap.get(StructuredName.CONTENT_ITEM_TYPE))
                    .appendNickNames(contentValuesListMap.get(Nickname.CONTENT_ITEM_TYPE))
                    .appendPhones(contentValuesListMap.get(Phone.CONTENT_ITEM_TYPE),
                            mPhoneTranslationCallback)
                    .appendEmails(contentValuesListMap.get(Email.CONTENT_ITEM_TYPE))
                    .appendPostals(contentValuesListMap.get(StructuredPostal.CONTENT_ITEM_TYPE))
                    .appendOrganizations(contentValuesListMap.get(Organization.CONTENT_ITEM_TYPE))
                    .appendWebsites(contentValuesListMap.get(Website.CONTENT_ITEM_TYPE));
            if ((mVCardType & VCardConfig.FLAG_REFRAIN_IMAGE_EXPORT) == 0) {
                builder.appendPhotos(contentValuesListMap.get(Photo.CONTENT_ITEM_TYPE));
            }
            
            builder = (OP09VCardBuilder) builder.appendNotes(
                    contentValuesListMap.get(Note.CONTENT_ITEM_TYPE))
                    .appendEvents(contentValuesListMap.get(Event.CONTENT_ITEM_TYPE))
                    .appendIms(contentValuesListMap.get(Im.CONTENT_ITEM_TYPE))
                    .appendSipAddresses(contentValuesListMap.get(SipAddress.CONTENT_ITEM_TYPE))
                    .appendRelation(contentValuesListMap.get(Relation.CONTENT_ITEM_TYPE));
                    
                    
            builder.appendGroups(contentValuesListMap.get(GroupMembership.CONTENT_ITEM_TYPE), mContentResolver);
            
            return builder.toString();
        }
    }
    
    
}
