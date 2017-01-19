package com.mediatek.dialer.plugin.calllog;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CallLog.Calls;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.util.Log;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.dialer.PhoneCallDetailsEx;
import com.mediatek.dialer.calllogex.ContactInfoEx;
import com.mediatek.dialer.calllogex.CallLogQueryEx;
import com.mediatek.dialer.ext.CallDetailExtension;
import com.mediatek.dialer.ext.IPhoneNumberHelper;
import com.mediatek.calloption.plugin.OP09CallOptionUtils;
import com.mediatek.op09.plugin.R;

import java.lang.ref.WeakReference;
import java.util.Iterator;
import java.util.Map;

public class CallDetailOP09Extension extends CallDetailExtension
                                     implements CallLogQueryHandler.Listener {
 
    private static final String TAG = "CallDetailOP09Extension";

    private static final String EXTRA_CALL_LOG_IDS = "EXTRA_CALL_LOG_IDS";

    private Activity mActivity;
    private Context mPluginContext;
    private IPhoneNumberHelper mPhoneNumberHelper;
    private ContactInfoEx mFirstCallContactInfo;
    private CallLogQueryHandler mCallLogQueryHandler;

    // sim id of sip call in the call log database
    public static final int CALL_TYPE_SIP = -2;

    public CallDetailOP09Extension(Context pluginContext) {
        mPluginContext = pluginContext;
    }

    public void onCreate(Activity activity, IPhoneNumberHelper phoneNumberHelper) {
        mActivity = activity;
        mPhoneNumberHelper = phoneNumberHelper;
        mCallLogQueryHandler = new CallLogQueryHandler(mActivity.getContentResolver(), this);
    }

    public void onDestroy() {
        mActivity = null;
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(ContextMenu.NONE, R.id.menu_delete_all_calls_with_number, 15,
                mPluginContext.getString(R.string.delete_all_calls_with_number));
        menu.add(ContextMenu.NONE, R.id.menu_delete_all_calls_with_contact, 15,
                mPluginContext.getString(R.string.delete_all_calls_with_contact));
        return false;
    }

    public boolean onPrepareOptionsMenu(Menu menu) {
        if (null == mFirstCallContactInfo) {
            menu.findItem(R.id.menu_delete_all_calls_with_number).setVisible(false);
            menu.findItem(R.id.menu_delete_all_calls_with_contact).setVisible(false);
            return false;
        }
        if (0 == mFirstCallContactInfo.contactId) {
            menu.findItem(R.id.menu_delete_all_calls_with_number).setVisible(true);
            menu.findItem(R.id.menu_delete_all_calls_with_contact).setVisible(false);
        } else {
            menu.findItem(R.id.menu_delete_all_calls_with_number).setVisible(false);
            menu.findItem(R.id.menu_delete_all_calls_with_contact).setVisible(true);
        }
        return false;
    }

    public boolean onMenuItemSelected(int featureId, MenuItem item) {

        switch (item.getItemId()) {

            case R.id.menu_delete_all_calls_with_number:
                showDeleteAllCallsWithNumberDialog(mFirstCallContactInfo.number);
                // !!! needs to run in anther thread, 
                //mActivity.getContentResolver().delete(Calls.CONTENT_URI,
                //        Calls.NUMBER + " = '" + mFirstCallContactInfo.number + "'", null);
                return true;

            case R.id.menu_delete_all_calls_with_contact:
                showDeleteAllCallsWithContactDialog(mFirstCallContactInfo.rawContactId);
                // !!! needs to run in anther thread,
                //mActivity.getContentResolver().delete(Calls.CONTENT_URI,
                //        "calls." + Calls.RAW_CONTACT_ID + " = " + Integer.toString(),
                //        null);
                return true;

            default:
                break;
        }
        return false;
    }

    private void showDeleteAllCallsWithNumberDialog(final String number) {
        AlertDialog dialog;
        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity)
                   .setTitle(mPluginContext.getString(R.string.delete_all_calls_with_number_dialog_title))
                   .setIconAttribute(android.R.attr.alertDialogIcon)
                   .setMessage(mPluginContext.getString(R.string.delete_all_calls_with_number_dialog_message))
                   .setNegativeButton(android.R.string.cancel, null)
                   .setPositiveButton(android.R.string.ok,
                       new DialogInterface.OnClickListener() {
                           public void onClick(DialogInterface dialog, int which) {
                               mCallLogQueryHandler.deleteSpecifiedCalls(Calls.NUMBER + " = '" + number + "'");
                           }
                       });
        dialog = builder.create();
        dialog.show();
    }

    private void showDeleteAllCallsWithContactDialog(final int rawContactId) {
        AlertDialog dialog;
        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity)
                   .setTitle(mPluginContext.getString(R.string.delete_all_calls_with_contact_dialog_title))
                   .setIconAttribute(android.R.attr.alertDialogIcon)
                   .setMessage(mPluginContext.getString(R.string.delete_all_calls_with_contact_dialog_message))
                   .setNegativeButton(android.R.string.cancel, null)
                   .setPositiveButton(android.R.string.ok,
                       new DialogInterface.OnClickListener() {
                           public void onClick(DialogInterface dialog, int which) {
                               mCallLogQueryHandler.deleteSpecifiedCalls(
                                       "calls." + Calls.RAW_CONTACT_ID + " = " + Integer.toString(rawContactId));
                           }
                       });
        dialog = builder.create();
        dialog.show();
    }

    public PhoneCallDetailsEx[] doInBackgroundForUpdateData(final Uri[] callUris, PhoneCallDetailsEx[] phoneCallDetails) {
        /** M: Bug Fix for ALPS01448622 @{ */
        if (null == mActivity) {
            log("The mActivity is null");
            return null;
        }
        /** @} */

        Cursor firstCallCursor = mActivity.getContentResolver().query(callUris[0],
                CallLogQueryEx.PROJECTION_CALLS_JOIN_DATAVIEW, null, null, null);
        try {
            firstCallCursor.moveToFirst();
            mFirstCallContactInfo = ContactInfoEx.fromCursor(firstCallCursor);
        } finally {
            if (null != firstCallCursor) {
                firstCallCursor.close();
            }
        }
        PhoneCallDetailsEx firstDetails = getPhoneCallDetailsContactInfo(mFirstCallContactInfo,
                OP09PhoneCallDetails.DISPLAY_TYPE_DEFAULT_CALL);

        //log("raw contact id of first call log = " + Integer.toString(mFirstCallContactInfo.rawContactId));
        String where = (mFirstCallContactInfo.contactId > 0) ? 
                "calls." + Calls.RAW_CONTACT_ID + " = " + Integer.toString(mFirstCallContactInfo.rawContactId) :
                Calls.NUMBER + " = '" + mFirstCallContactInfo.number + "' OR "
                    + Calls.NUMBER + " = '" + mFirstCallContactInfo.formattedNumber + "'";
        Cursor allCallsCursor = mActivity.getContentResolver().query(
                Uri.parse("content://call_log/callsjoindataview"),
                CallLogQueryEx.PROJECTION_CALLS_JOIN_DATAVIEW, 
                where, null, Calls.DATE + " DESC");

        final int numCalls = callUris.length;
        log("numCall = " + numCalls + ", allCallsCursor.getCount = " + allCallsCursor.getCount());
        PhoneCallDetailsEx[] resultPhoneCallDetails = new PhoneCallDetailsEx[numCalls + allCallsCursor.getCount() + 1];
        //PhoneCallDetails[] resultPhoneCallDetails = new PhoneCallDetails[numCalls + 1 + 1];

        resultPhoneCallDetails[0] = firstDetails;
        int i = 1;
        while (i < numCalls) {
            resultPhoneCallDetails[i] = phoneCallDetails[i];
            ++i;
        }
        // "all calls" text list item 
        resultPhoneCallDetails[i++] = new OP09PhoneCallDetails(firstDetails, OP09PhoneCallDetails.DISPLAY_TYPE_TEXT);
        // "all calls" list items
        try {
            if (allCallsCursor.getCount() > 0) {
                allCallsCursor.moveToFirst();
                resultPhoneCallDetails[i++] = getPhoneCallDetailsContactInfo(ContactInfoEx.fromCursor(allCallsCursor),
                        OP09PhoneCallDetails.DISPLAY_TYPE_NEW_CALL);
                while (allCallsCursor.moveToNext()) {
                    resultPhoneCallDetails[i++] = getPhoneCallDetailsContactInfo(ContactInfoEx.fromCursor(allCallsCursor),
                            OP09PhoneCallDetails.DISPLAY_TYPE_NEW_CALL);
                }
            }
        } finally {
            if (null != allCallsCursor) {
                allCallsCursor.close();
            }
        }
        return resultPhoneCallDetails;
    }

    public PhoneCallDetailsEx getPhoneCallDetailsContactInfo(ContactInfoEx contactInfo, int displayType) {
        log("number = " + contactInfo.number);
        if (!mPhoneNumberHelper.canPlaceCallsTo(contactInfo.number)
                || mPhoneNumberHelper.isVoiceMailNumberForMtk(contactInfo.number, contactInfo.simId)
                || mPhoneNumberHelper.isEmergencyNumber(contactInfo.number, contactInfo.simId)) {
            contactInfo.formattedNumber = mPhoneNumberHelper.getDisplayNumber(contactInfo.number, null).toString();
            contactInfo.name = "";
            contactInfo.nNumberTypeId = 0;
            contactInfo.label = "";
            contactInfo.lookupUri = null;
        }
        return new OP09PhoneCallDetails(contactInfo.number, contactInfo.formattedNumber, 
                                        contactInfo.countryIso, contactInfo.geocode,
                                        contactInfo.type, contactInfo.date,
                                        contactInfo.duration, contactInfo.name,
                                        contactInfo.nNumberTypeId, contactInfo.label,
                                        contactInfo.lookupUri, contactInfo.photoUri, contactInfo.simId,
                                        contactInfo.vtCall, 0, contactInfo.ipPrefix,
                                        displayType);
    }

    public void onCallsDeleted() {
        if (null != mActivity) {
            mActivity.finish();
        }
    }

    public boolean setSimInfo(int simId, TextView simIndicator) {
        if (CALL_TYPE_SIP == simId) {
            // The request is sip color
            if (null != simIndicator && simIndicator.getVisibility() == View.VISIBLE) {
                simIndicator.setBackgroundDrawable(
                            mPluginContext.getResources().getDrawable(R.drawable.dark_small_internet_call));
            }
        }
        if (null != simIndicator) {
            simIndicator.setText(" ");
        }
	return false;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
