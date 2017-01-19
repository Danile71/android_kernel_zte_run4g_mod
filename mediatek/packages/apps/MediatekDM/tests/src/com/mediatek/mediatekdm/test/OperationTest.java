
package com.mediatek.mediatekdm.test;

import android.test.ApplicationTestCase;
import android.util.Log;

import com.mediatek.mediatekdm.DmApplication;
import com.mediatek.mediatekdm.DmOperation;
import com.mediatek.mediatekdm.DmOperationManager;

public class OperationTest extends ApplicationTestCase<DmApplication> {
    public static final String TAG = "MDMTest/OperationTest";

    public OperationTest() {
        super(DmApplication.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.d(TAG, "OperationTest.setUp()");
    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "OperationTest.tearDown()");
        super.tearDown();
    }

    public void testcase01() {
        DmOperationManager om = DmOperationManager.getInstance();
        assertNotNull(om);
        assertSame(om, DmOperationManager.getInstance());
    }
    
    public void testcase02() {
        assertTrue(DmOperation.Type.isCIOperation(DmOperation.Type.TYPE_CI + "xxx"));
        assertTrue(DmOperation.Type.isCIOperation(DmOperation.Type.TYPE_CI_FUMO));
        assertTrue(DmOperation.Type.isCIOperation(DmOperation.Type.TYPE_REPORT_SCOMO));
        assertTrue(DmOperation.Type.isCIOperation(DmOperation.Type.TYPE_REPORT_FUMO));
        assertTrue(DmOperation.Type.isCIOperation(DmOperation.Type.TYPE_REPORT_LAWMO));
        assertFalse(DmOperation.Type.isCIOperation(DmOperation.Type.TYPE_SI));

        assertTrue(DmOperation.Type.isSIOperation(DmOperation.Type.TYPE_SI));
        assertTrue(DmOperation.Type.isSIOperation(DmOperation.Type.TYPE_SI + "xxx"));
        assertFalse(DmOperation.Type.isSIOperation(DmOperation.Type.TYPE_CI_FUMO));
        assertFalse(DmOperation.Type.isSIOperation(DmOperation.Type.TYPE_REPORT_SCOMO));
        assertFalse(DmOperation.Type.isSIOperation(DmOperation.Type.TYPE_REPORT_FUMO));
        assertFalse(DmOperation.Type.isSIOperation(DmOperation.Type.TYPE_REPORT_LAWMO));

        assertFalse(DmOperation.Type.isReportOperation(DmOperation.Type.TYPE_SI));
        assertFalse(DmOperation.Type.isReportOperation(DmOperation.Type.TYPE_CI_FUMO));
        assertTrue(DmOperation.Type.isReportOperation(DmOperation.Type.TYPE_REPORT_SCOMO));
        assertTrue(DmOperation.Type.isReportOperation(DmOperation.Type.TYPE_REPORT_FUMO));
        assertTrue(DmOperation.Type.isReportOperation(DmOperation.Type.TYPE_REPORT_LAWMO));

        assertTrue(DmOperation.Type.isDLOperation(DmOperation.Type.TYPE_DL));
        assertTrue(DmOperation.Type.isDLOperation(DmOperation.Type.TYPE_DL + "xxx"));
        assertFalse(DmOperation.Type.isDLOperation("xxx" + DmOperation.Type.TYPE_DL));
    }
    
    public void testcase03() {
        
    }
}
