
package com.mediatek.mediatekdm.test;

import android.app.Service;
import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;
import com.mediatek.mediatekdm.PlatformManager;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

public class MockPlatformManager extends PlatformManager {
    public static final String TAG = "MDMTest/MockPlatformManager";

    private DmAgent mAgent = null;

    public MockPlatformManager() {
        super();
    }

    public static void setUp() {
        PlatformManager pm = new MockPlatformManager();
        try {
            Field field = PlatformManager.class.getDeclaredField("sInstance");
            field.setAccessible(true);
            field.set(PlatformManager.class, pm);
            field.setAccessible(false);
        } catch (Exception e) {
            throw new Error(e);
        }
    }

    public static void tearDown() {
        try {
            Field field = PlatformManager.class.getDeclaredField("sInstance");
            field.setAccessible(true);
            field.set(PlatformManager.class, null);
            field.setAccessible(false);
        } catch (Exception e) {
            throw new Error(e);
        }
    }

    @Override
    public void stayForeground(Service service) {
        Log.i(TAG, "Bring service to foreground");
    }

    @Override
    public void leaveForeground(Service service) {
        Log.d(TAG, "Exec stopForeground with para true.");
    }

    @Override
    public String getSimOperator(int simId) {
        return TestEnvironment.TEST_MCCMNC;
    }

    @Override
    public List<SimInfoRecord> getInsertedSimInfoList(Context context) {
        Log.e("MDMTest/SimInfoRecord", "getInsertedSimInfoList");
        for (SimInfoRecord record : super.getInsertedSimInfoList(context)) {
            Log.e("MDMTest/SimInfoRecord", "mSimInfoId: " + record.mSimInfoId);
            Log.e("MDMTest/SimInfoRecord", "mSimSlotId: " + record.mSimSlotId);
        }
        List<SimInfoRecord> result = new ArrayList<SimInfoRecord>();
        try {
            Constructor<?> ctor = SimInfoManager.SimInfoRecord.class.getDeclaredConstructor();
            ctor.setAccessible(true);
            SimInfoRecord sir = (SimInfoRecord) ctor.newInstance();
            sir.mSimSlotId = 0;
            result.add(sir);
            ctor.setAccessible(false);
        } catch (NoSuchMethodException e) {
            throw new Error(e);
        } catch (IllegalArgumentException e) {
            throw new Error(e);
        } catch (InstantiationException e) {
            throw new Error(e);
        } catch (IllegalAccessException e) {
            throw new Error(e);
        } catch (InvocationTargetException e) {
            throw new Error(e);
        }
        return result;
    }

    @Override
    public String getSubscriberId(int simId) {
        if (simId == 0) {
            return TestEnvironment.TEST_IMSI;
        } else {
            return null;
        }
    }

    @Override
    public synchronized DmAgent getDmAgent() {
        if (mAgent == null) {
            mAgent = new MockDmAgent(super.getDmAgent());
        }
        return mAgent;
    }

    public static class MockDmAgent implements DmAgent {
        private final DmAgent mAgent;
        private byte[] mImsi = TestEnvironment.TEST_IMSI.getBytes();

        public MockDmAgent(DmAgent agent) {
            mAgent = agent;
        }

        @Override
        public IBinder asBinder() {
            return mAgent.asBinder();
        }

        @Override
        public boolean clearLockFlag() throws RemoteException {
            return mAgent.clearLockFlag();
        }

        @Override
        public int clearOtaResult() throws RemoteException {
            return mAgent.clearOtaResult();
        }

        @Override
        public boolean clearRebootFlag() throws RemoteException {
            return mAgent.clearRebootFlag();
        }

        @Override
        public boolean clearWipeFlag() throws RemoteException {
            return mAgent.clearWipeFlag();
        }

        @Override
        public byte[] getDmSwitchValue() throws RemoteException {
            return mAgent.getDmSwitchValue();
        }

        @Override
        public int getLockType() throws RemoteException {
            return mAgent.getLockType();
        }

        @SuppressWarnings("deprecation")
        @Override
        public int getOperatorId() throws RemoteException {
            return mAgent.getOperatorId();
        }

        @SuppressWarnings("deprecation")
        @Override
        public byte[] getOperatorName() throws RemoteException {
            return mAgent.getOperatorName();
        }

        @Override
        public byte[] getRegisterSwitch() throws RemoteException {
            return mAgent.getRegisterSwitch();
        }

        @Override
        public byte[] getSmsRegSwitchValue() throws RemoteException {
            return mAgent.getSmsRegSwitchValue();
        }

        @Override
        public byte[] getSwitchValue() throws RemoteException {
            return mAgent.getSwitchValue();
        }

        @SuppressWarnings("deprecation")
        @Override
        public int getUpgradeStatus() throws RemoteException {
            return mAgent.getUpgradeStatus();
        }

        @SuppressWarnings("deprecation")
        @Override
        public boolean isBootRecoveryFlag() throws RemoteException {
            return mAgent.isBootRecoveryFlag();
        }

        @Override
        public boolean isHangMoCallLocking() throws RemoteException {
            return mAgent.isHangMoCallLocking();
        }

        @Override
        public boolean isHangMtCallLocking() throws RemoteException {
            return mAgent.isHangMtCallLocking();
        }

        @Override
        public boolean isLockFlagSet() throws RemoteException {
            return mAgent.isLockFlagSet();
        }

        @Override
        public boolean isWipeSet() throws RemoteException {
            return mAgent.isWipeSet();
        }

        @SuppressWarnings("deprecation")
        @Override
        public byte[] readDmTree() throws RemoteException {
            return mAgent.readDmTree();
        }

        @Override
        public byte[] readImsi() throws RemoteException {
            return mImsi;
        }

        @Override
        public byte[] readImsi1() throws RemoteException {
            return mAgent.readImsi1();
        }

        @Override
        public byte[] readImsi2() throws RemoteException {
            return mAgent.readImsi2();
        }

        @SuppressWarnings("deprecation")
        @Override
        public byte[] readOperatorName() throws RemoteException {
            return mAgent.readOperatorName();
        }

        @Override
        public int readOtaResult() throws RemoteException {
            return mAgent.readOtaResult();
        }

        @Override
        public int restartAndroid() throws RemoteException {
            return mAgent.restartAndroid();
        }

        @Override
        public boolean setDmSwitchValue(byte[] arg0) throws RemoteException {
            return mAgent.setDmSwitchValue(arg0);
        }

        @Override
        public boolean setLockFlag(byte[] arg0) throws RemoteException {
            return mAgent.setLockFlag(arg0);
        }

        @Override
        public boolean setRebootFlag() throws RemoteException {
            return mAgent.setRebootFlag();
        }

        @Override
        public boolean setRegisterSwitch(byte[] arg0) throws RemoteException {
            return mAgent.setRegisterSwitch(arg0);
        }

        @Override
        public boolean setSmsRegSwitchValue(byte[] arg0) throws RemoteException {
            return mAgent.setSmsRegSwitchValue(arg0);
        }

        @Override
        public boolean setSwitchValue(byte[] arg0) throws RemoteException {
            return mAgent.setSwitchValue(arg0);
        }

        @Override
        public boolean setWipeFlag() throws RemoteException {
            return mAgent.setWipeFlag();
        }

        @SuppressWarnings("deprecation")
        @Override
        public boolean writeDmTree(byte[] arg0) throws RemoteException {
            return mAgent.writeDmTree(arg0);
        }

        @Override
        public boolean writeImsi(byte[] imsi) throws RemoteException {
            mImsi = imsi;
            return true;
        }

        @Override
        public boolean writeImsi1(byte[] arg0, int arg1) throws RemoteException {
            mAgent.writeImsi1(arg0, arg1);
            return true;
        }

        @Override
        public boolean writeImsi2(byte[] arg0, int arg1) throws RemoteException {
            mAgent.writeImsi2(arg0, arg1);
            return true;
        }

        @Override
        public byte[] readRegisterFlag() throws RemoteException {
            return mAgent.readRegisterFlag();
        }

        @Override
        public boolean setRegisterFlag(byte[] arg0, int arg1) throws RemoteException {
            return mAgent.setRegisterFlag(arg0, arg1);
        }
    }
}
