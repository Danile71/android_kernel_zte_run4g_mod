package com.mediatek.mediatekdm.test.server;

import com.mediatek.mediatekdm.DmConst.NotificationInteractionType;
import com.mediatek.mediatekdm.test.MockDmNotification;
import com.mediatek.mediatekdm.test.server.MockServerService.CMCCTestRequest;

public class PairingInfoRetrievalTestCase extends MockServerTestCase {

    public PairingInfoRetrievalTestCase() {
        super(CMCCTestRequest.PAIRING_INFO_RETRIEVAL, 10);
    }

    @Override
    protected String getTag() {
        return "MDMTest/PairingInfoRetrievalTestCase";
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
