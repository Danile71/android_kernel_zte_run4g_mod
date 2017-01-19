package com.mediatek.mediatekdm.test.server;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;

import com.mediatek.mediatekdm.test.Checklist;

public interface IServiceTest {
    Checklist getChecklist();
    ComponentName startServiceEmulation(Intent intent);
    boolean bindServiceEmulation(Intent intent, ServiceConnection conn, int flags);
}
