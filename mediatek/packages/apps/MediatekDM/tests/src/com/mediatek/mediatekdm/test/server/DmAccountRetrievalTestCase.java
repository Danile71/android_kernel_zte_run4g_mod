package com.mediatek.mediatekdm.test.server;

import com.mediatek.mediatekdm.DmConst.NotificationInteractionType;
import com.mediatek.mediatekdm.test.MockDmNotification;
import com.mediatek.mediatekdm.test.server.MockServerService.CMCCTestRequest;

public class DmAccountRetrievalTestCase extends MockServerTestCase {

    public DmAccountRetrievalTestCase() {
        super(CMCCTestRequest.DM_ACCOUNT_RETRIEVAL, 60);
    }

    @Override
    protected String getTag() {
        return "MDMTest/DmAccountRetrievalTestCase";
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
}
