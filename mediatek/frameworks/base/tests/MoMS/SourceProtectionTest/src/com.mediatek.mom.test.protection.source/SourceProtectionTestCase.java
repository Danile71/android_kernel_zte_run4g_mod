package com.mediatek.mom.test.protection.source;

import java.lang.reflect.Method;

import android.test.AndroidTestCase;
import android.util.Log;
import com.mediatek.common.jpe.a;
//Target Package
import com.mediatek.mom.MobileManagerService;

public class SourceProtectionTestCase extends AndroidTestCase {
    private static final String TAG = "SourceProtectionTestCase";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testProGuard() throws Throwable {
        Log.i(TAG, "testProGuard()");
        boolean ObjNameObfuscated = true;
        boolean ObjAMethodName = false;

        try {
            MobileManagerService mobileManagerService = new MobileManagerService(mContext);
            Method[] methods = mobileManagerService.getClass().getDeclaredMethods();  

            // Find method enforceCheckLicense(String message) in MobileManagerService
            for (Method checkMethodName : methods) {
                String methodName = checkMethodName.getName();
                Log.i(TAG, "checkMethodName = " + methodName);
                Class[] parameters = checkMethodName.getParameterTypes();

                for (Class checkParaName : parameters) {
                    Log.i(TAG, "MethodName: " +  methodName + " checkMethodParameter: " + checkParaName.getName());
                }

                if (methodName.equals("enforceCheckLicense") && (parameters.length == 1)
                        && (parameters[0].getName().equals("java.lang.String"))) {
                    ObjNameObfuscated = false;
                    Log.i(TAG, "checkMethodName hit = " + methodName + "ObjNameObfuscated = " + ObjNameObfuscated);
                }
                // check method name "a" ~ "z"
                String charAtoZ = "abcdefghijklmnopqrstuvwxyz";
                for (int i=0; i<charAtoZ.length(); i++) {
                    String testMethodName = "" + charAtoZ.charAt(i);
                    if (!ObjAMethodName && methodName.equals(testMethodName)){
                        ObjAMethodName = true;
                        break;
                    }
                }
                // check method name "aa" ~ "zz"
                for (int i=0; i<charAtoZ.length(); i++) {
                    for (int j=0; j<charAtoZ.length(); j++) {
                        String testMethodName = "" + charAtoZ.charAt(i) + charAtoZ.charAt(j);
                        if (!ObjAMethodName && methodName.equals(testMethodName)) {
                            ObjAMethodName = true;
                            break;
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Get MobileManagerService instance failed!", e);
        }
        Log.i(TAG, "MoMS checkMethodName = enforceCheckLicense is obfusted : " + ObjNameObfuscated);
        Log.i(TAG, "MoMS checkMethodName = a is                      : " + ObjAMethodName);

        assertTrue("MoMS object name is not obfuscated", (ObjNameObfuscated & ObjAMethodName));
    }

    public void testJPE() {
        com.mediatek.common.jpe.a.b = false;
        Log.i(TAG, "testJPE()");
        try {
            MobileManagerService mobileManagerService = new MobileManagerService(mContext);
            Log.i(TAG, "MoMS NativeCheck = " + com.mediatek.common.jpe.a.b);

        } catch (Exception e) {
            Log.e(TAG, "Get MobileManagerService instance failed!", e);
        }

        assertTrue("MoMS is not JPE protected", (com.mediatek.common.jpe.a.b));
    }
}
