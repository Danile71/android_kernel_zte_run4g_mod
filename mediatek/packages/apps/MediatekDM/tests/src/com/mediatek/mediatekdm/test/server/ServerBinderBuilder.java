package com.mediatek.mediatekdm.test.server;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.mediatek.mediatekdm.test.server.MockServerService.CMCCTestRequest;

public class ServerBinderBuilder {
    private static final String TAG = "MDMTest/ServerBinderBuilder";
    
    public static IMockServer.Stub buildServerBinder(Intent intent, Context context) {
        String action = intent.getAction();
        Log.d(TAG, "buildServerBinder: " + action);
        IMockServer.Stub result = null;
        if (action.equals(CMCCTestRequest.PAIRING_INFO_RETRIEVAL)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.DM_ACCOUNT_RETRIEVAL)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.PARAMETERS_RETRIEVAL)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.PARAMETERS_RETRIEVAL_RETRY)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.PARAMETERS_PROVISION)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.PARAMETERS_PROVISION_RETRY)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.FIRMWARE_UPDATE)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.FIRMWARE_QUERY)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.SOFTWARE_INSTALL)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.SOFTWARE_LIST_RETRIEVAL)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.LOCK_PHONE)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.UNLOCK_PHONE)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.WIPE_PHONE)) {
            result = buildTemplateDriverBinder(intent, context);
        } else if (action.equals(CMCCTestRequest.PROBE_PHONE)) {
            result = buildTemplateDriverBinder(intent, context);
        }
        return result;
    }
    
    private static IMockServer.Stub buildTemplateDriverBinder(Intent intent, Context context) {
        int port = intent.getIntExtra(MockServerService.KEY_PORT, MockServerService.DEFAULT_PORT);
        TemplateDrivenBinder result = new TemplateDrivenBinder(port, context);
        result.put(TemplateDrivenBinder.KEY_TYPE, intent.getAction());
        return result;
    }
}
