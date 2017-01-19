
package com.mediatek.mediatekdm.test;

import android.content.Intent;
import android.util.Log;

import com.mediatek.mediatekdm.DmConst;
import com.mediatek.mediatekdm.DmConst.NotificationInteractionType;
import com.mediatek.mediatekdm.DmNotification;
import com.mediatek.mediatekdm.DmService;
import com.mediatek.mediatekdm.test.server.IServiceTest;

import junit.framework.TestCase;

public class MockDmNotification extends DmNotification {
    private DmService mService;
    private IServiceTest mTestCase;
    private final int mNotificationResponse;
    private final int mAlertResponse;

    public static final int RESPONSE_INVALID = -1;
    public static final int RESPONSE_FALSE = 0;
    public static final int RESPONSE_TRUE = 1;
    public static final int RESPONSE_TIMEOUT = 2;

    private int mNotificationCount = 0;
    private int mAlertCount = 0;

    public MockDmNotification(
            DmService service,
            IServiceTest testCase,
            int notificationResponse,
            int alertResponse) {
        super(service);
        Log.d("MDMTest/MockDmNotification", "MockDmNotification");
        mService = service;
        mTestCase = testCase;
        mNotificationResponse = notificationResponse;
        mAlertResponse = alertResponse;
    }

    @Override
    public void showNotification(int type) {
        Intent serviceIntent = null;
        switch (type) {
            case NotificationInteractionType.TYPE_NOTIFICATION_VISIBLE:
                mTestCase.getChecklist().fillCheckItem("notification_type_" + mNotificationCount, type);
                mNotificationCount += 1;
                clear();
                break;
            case NotificationInteractionType.TYPE_NOTIFICATION_INTERACT:
                mTestCase.getChecklist().fillCheckItem("notification_type_" + mNotificationCount, type);
                mNotificationCount += 1;
                serviceIntent = new Intent(mService, DmService.class);
                if (mNotificationResponse == RESPONSE_FALSE) {
                    serviceIntent.setAction(DmConst.IntentAction.DM_NOTIFICATION_RESPONSE);
                    serviceIntent.putExtra("response", false);
                } else if (mNotificationResponse == RESPONSE_TRUE) {
                    serviceIntent.setAction(DmConst.IntentAction.DM_NOTIFICATION_RESPONSE);
                    serviceIntent.putExtra("response", true);
                } else if (mNotificationResponse == RESPONSE_TIMEOUT) {
                    serviceIntent.setAction(DmConst.IntentAction.DM_NOTIFICATION_TIMEOUT);
                    serviceIntent.putExtra("type", type);
                } else {
                    TestCase.assertEquals(RESPONSE_INVALID, mNotificationResponse);
                }
                mTestCase.startServiceEmulation(serviceIntent);
                break;
            case NotificationInteractionType.TYPE_ALERT_1101:
                mTestCase.getChecklist().fillCheckItem("alert_type_" + mAlertCount, type);
                mAlertCount += 1;
                serviceIntent = new Intent(mService, DmService.class);
                if (mAlertResponse == RESPONSE_FALSE) {
                    serviceIntent.setAction(DmConst.IntentAction.DM_ALERT_RESPONSE);
                    serviceIntent.putExtra("response", false);
                } else if (mAlertResponse == RESPONSE_TRUE) {
                    serviceIntent.setAction(DmConst.IntentAction.DM_ALERT_RESPONSE);
                    serviceIntent.putExtra("response", true);
                } else if (mAlertResponse == RESPONSE_TIMEOUT) {
                    serviceIntent.setAction(DmConst.IntentAction.DM_ALERT_TIMEOUT);
                    serviceIntent.putExtra("type", type);
                } else {
                    TestCase.assertEquals(RESPONSE_INVALID, mAlertResponse);
                }
                mTestCase.startServiceEmulation(serviceIntent);
                break;
            default:
                if (type != NotificationInteractionType.TYPE_INVALID) {
                    TestCase.fail("Invalid NotificationInteractionType: " + type);
                }
        }
    }

    @Override
    public void clear() {
        Log.d("MDMTest/MockDmNotification", "clear");
        mService.cancelNiaAlertTimeout();
    }
}
