
package com.mediatek.mediatekdm.test.server;

import com.mediatek.mediatekdm.DmConst.NotificationInteractionType;
import com.mediatek.mediatekdm.test.MockDmNotification;
import com.mediatek.mediatekdm.test.server.MockServerService.CMCCTestRequest;

public class ParameterProvisionTestCase extends MockServerTestCase {

    public ParameterProvisionTestCase() {
        super(CMCCTestRequest.PARAMETERS_PROVISION, 10);
    }

    @Override
    protected String getTag() {
        return "MDMTest/ParameterProvisionTestCase";
    }

    public void testcase01() {
        testTemplate(
                "testcase01",
                0,
                NotificationInteractionType.TYPE_INVALID,
                MockDmNotification.RESPONSE_INVALID,
                NotificationInteractionType.TYPE_INVALID,
                MockDmNotification.RESPONSE_INVALID);
    }

    public void testcase02() {
        testTemplate(
                "testcase02",
                1,
                NotificationInteractionType.TYPE_INVALID,
                MockDmNotification.RESPONSE_INVALID,
                NotificationInteractionType.TYPE_INVALID,
                MockDmNotification.RESPONSE_INVALID);
    }

    public void testcase03() {
        testTemplate(
                "testcase03",
                2,
                NotificationInteractionType.TYPE_NOTIFICATION_VISIBLE,
                MockDmNotification.RESPONSE_INVALID,
                NotificationInteractionType.TYPE_INVALID,
                MockDmNotification.RESPONSE_INVALID);
    }

    public void testcase04() {
        testTemplate(
                "testcase04",
                3,
                NotificationInteractionType.TYPE_NOTIFICATION_INTERACT,
                MockDmNotification.RESPONSE_TRUE,
                NotificationInteractionType.TYPE_INVALID,
                MockDmNotification.RESPONSE_INVALID);
    }
}
