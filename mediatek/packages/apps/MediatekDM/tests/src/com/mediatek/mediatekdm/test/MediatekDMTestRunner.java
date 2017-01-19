
package com.mediatek.mediatekdm.test;

import android.test.InstrumentationTestRunner;
import android.test.InstrumentationTestSuite;

import com.mediatek.mediatekdm.test.server.DmAccountRetrievalTestCase;
import com.mediatek.mediatekdm.test.server.PairingInfoRetrievalTestCase;
import com.mediatek.mediatekdm.test.server.ParameterProvisionRetryTestCase;
import com.mediatek.mediatekdm.test.server.ParameterProvisionTestCase;
import com.mediatek.mediatekdm.test.server.ParameterRetrievalRetryTestCase;
import com.mediatek.mediatekdm.test.server.ParameterRetrievalTestCase;

import junit.framework.TestSuite;

public class MediatekDMTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        InstrumentationTestSuite suite = new InstrumentationTestSuite(this);
        // suite.addTestSuite(BareTestCase.class);

        suite.addTestSuite(ReceiverTest.class);
        suite.addTestSuite(OperationTest.class);
        suite.addTestSuite(ParameterRetrievalTestCase.class);
        suite.addTestSuite(ParameterRetrievalRetryTestCase.class);
        suite.addTestSuite(ParameterProvisionTestCase.class);
        suite.addTestSuite(ParameterProvisionRetryTestCase.class);

        suite.addTestSuite(PairingInfoRetrievalTestCase.class);
        suite.addTestSuite(DmAccountRetrievalTestCase.class);

        suite.addTestSuite(PermDialogTest.class);
        return suite;
    }
}
