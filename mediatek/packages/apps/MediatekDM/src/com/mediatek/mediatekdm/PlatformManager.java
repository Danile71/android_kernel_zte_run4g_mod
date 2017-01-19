
package com.mediatek.mediatekdm;

import android.app.KeyguardManager;
import android.app.Notification;
import android.app.Service;
import android.app.KeyguardManager.KeyguardLock;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageInstallObserver;
import android.content.pm.PackageManager;
import android.content.pm.PackageParser;
import android.content.pm.PackageParser.Package;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.ServiceManager;
import android.provider.Telephony.Carriers;
import android.util.DisplayMetrics;
import android.util.Log;

import com.android.internal.os.AtomicFile;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.dm.DmAgent;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.telephony.TelephonyManagerEx;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OptionalDataException;
import java.io.StreamCorruptedException;
import java.util.List;

@SuppressWarnings("deprecation")
public class PlatformManager {
    public interface InstallListener {
        void packageInstalled(final String name, final int status);
    }

    public static final class PackageInfo {
        public String name;
        public String label;
        public String version;
        public String description;
        public Drawable icon;
        public Package pkg;
    }

    public static final int APN_ALREADY_ACTIVE = PhoneConstants.APN_ALREADY_ACTIVE;
    public static final int APN_TYPE_NOT_AVAILABLE = PhoneConstants.APN_TYPE_NOT_AVAILABLE;
    public static final int APN_REQUEST_FAILED = PhoneConstants.APN_REQUEST_FAILED;
    public static final int APN_REQUEST_STARTED = PhoneConstants.APN_REQUEST_STARTED;
    public static final String FEATURE_ENABLE_DM = Phone.FEATURE_ENABLE_DM;

    private static PlatformManager sInstance;
    static final String TAG = "MDM/PlatformManager";
    public static final int TYPE_MOBILE_DM = ConnectivityManager.TYPE_MOBILE_DM;
    public static final String EXTRA_SIM_ID = ConnectivityManager.EXTRA_SIM_ID;
    public static final int INSTALL_SUCCEEDED = PackageManager.INSTALL_SUCCEEDED;
    public static final int INSTALL_ALLOW_DOWNGRADE = PackageManager.INSTALL_ALLOW_DOWNGRADE;
    public static final int INSTALL_REPLACE_EXISTING = PackageManager.INSTALL_REPLACE_EXISTING;
    public static final int INSTALL_FAILED_ALREADY_EXISTS = PackageManager.INSTALL_FAILED_ALREADY_EXISTS;
    public WakeLock mFullWakelock = null;
    public WakeLock mPartialWakelock = null;
    public KeyguardLock mKeyguardLock = null;

    protected PlatformManager() {
    }

    public static synchronized PlatformManager getInstance() {
        if (sInstance == null) {
            sInstance = new PlatformManager();
        }
        return sInstance;
    }

    public SimInfoRecord getSimInfoBySlot(Context context, int simId) {
        return SimInfoManager.getSimInfoBySlot(context, simId);
    }

    public List<SimInfoRecord> getInsertedSimInfoList(Context context) {
        return SimInfoManager.getInsertedSimInfoList(context);
    }

    public String getSimOperator(int simId) {
        return TelephonyManagerEx.getDefault().getSimOperator(simId);
    }

    public String getImei() {
        return TelephonyManagerEx.getDefault().getDeviceId(PhoneConstants.GEMINI_SIM_1);
    }

    public String getSubscriberId(int simId) {
        return TelephonyManagerEx.getDefault().getSubscriberId(simId);
    }

    public DmAgent getDmAgent() {
        IBinder binder = ServiceManager.getService("DmAgent");
        if (binder == null) {
            Log.e("MTKPhone", "ServiceManager.getService(DmAgent) failed.");
            return null;
        }
        DmAgent agent = DmAgent.Stub.asInterface(binder);
        return agent;
    }

    public Uri getDmContentUri(int simId) {
        if (simId == 0) {
            return Carriers.CONTENT_URI_DM;
        } else if (simId == 1) {
            return Carriers.GeminiCarriers.CONTENT_URI_DM;
        } else {
            throw new Error("Invalid SIM ID.");
        }
    }

    public Uri getContentUri(int simId) {
        if (simId == 0) {
            return DmFeatureSwitch.MTK_GEMINI_SUPPORT ? Carriers.SIM1Carriers.CONTENT_URI : Carriers.CONTENT_URI;
        } else if (simId == 1) {
            return Carriers.SIM2Carriers.CONTENT_URI;
        } else {
            throw new Error("Invalid SIM ID.");
        }
    }

    public void installPackage(PackageManager pm, Uri packUri, int flag, final InstallListener listener) {
        pm.installPackage(packUri, new IPackageInstallObserver.Stub() {
            public void packageInstalled(final String name, final int status) {
                listener.packageInstalled(name, status);
            }
        }, flag, null);
    }

    public PackageInfo getPackageInfo(PackageManager pm, Resources resources, String archiveFilePath) {
        PackageInfo ret = new PackageInfo();
    
        PackageParser packageParser = new PackageParser(archiveFilePath);
        File sourceFile = new File(archiveFilePath);
        DisplayMetrics metrics = new DisplayMetrics();
        metrics.setToDefaults();
        PackageParser.Package pkg = packageParser.parsePackage(sourceFile, archiveFilePath,
                metrics, 0);
        if (pkg == null) {
            Log.w(DmConst.TAG.SCOMO, "package Parser get package is null");
            return null;
        }
        packageParser = null;
    
        ret.name = pkg.packageName;
        ret.version = pkg.mVersionName;
        ret.pkg = pkg;
        // get icon and label from archive file
        ApplicationInfo appInfo = pkg.applicationInfo;
        AssetManager assmgr = new AssetManager();
        assmgr.addAssetPath(archiveFilePath);
        Resources res = new Resources(assmgr, resources.getDisplayMetrics(),
                resources.getConfiguration());
        CharSequence label = null;
        if (appInfo.labelRes != 0) {
            try {
                label = res.getText(appInfo.labelRes);
            } catch (Resources.NotFoundException e) {
                e.printStackTrace();
            }
        }
        if (label == null) {
            label = (appInfo.nonLocalizedLabel != null) ? appInfo.nonLocalizedLabel
                    : appInfo.packageName;
        }
        Drawable icon = null;
        if (appInfo.icon != 0) {
            try {
                icon = res.getDrawable(appInfo.icon);
            } catch (Resources.NotFoundException e) {
                e.printStackTrace();
            }
        }
        if (icon == null) {
            icon = pm.getDefaultActivityIcon();
        }
        ret.label = label.toString();
        ret.icon = icon;
    
        return ret;
    }

    public int startUsingNetworkFeature(ConnectivityManager connMgr, int type, String feature, int simId) {
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            return connMgr.startUsingNetworkFeatureGemini(type, feature, simId);
        } else {
            return connMgr.startUsingNetworkFeature(type, feature);
        }
    }

    public int stopUsingNetworkFeatureGemini(ConnectivityManager connMgr, int type, String feature, int simId) {
        if (DmFeatureSwitch.MTK_GEMINI_SUPPORT) {
            return connMgr.stopUsingNetworkFeatureGemini(type, feature, simId);
        } else {
            return connMgr.stopUsingNetworkFeature(type, feature);
        }
    }

    public int extractSimId(NetworkInfo info) {
        return info.getSimId();
    }

    public Object atomicRead(File file) {
        Object obj = null;
        try {
            AtomicFile atomicFile = new AtomicFile(file);
            ObjectInputStream in = new ObjectInputStream(atomicFile.openRead());
            obj = in.readObject();
            in.close();
        } catch (ClassNotFoundException e) {
            Log.e(PlatformManager.TAG, "atomicRead: exception", e);
        } catch (StreamCorruptedException e) {
            Log.e(PlatformManager.TAG, "atomicRead: exception", e);
        } catch (OptionalDataException e) {
            Log.e(PlatformManager.TAG, "atomicRead: exception", e);
        } catch (IOException e) {
            Log.e(PlatformManager.TAG, "atomicRead: exception", e);
        }
        return obj;
    }

    public void atomicWrite(File file, Object data) {
        AtomicFile atomicFile = new AtomicFile(file);
        FileOutputStream fos = null;
        try {
            fos = atomicFile.startWrite();
            ObjectOutputStream out = new ObjectOutputStream(fos);
            out.writeObject(data);
            atomicFile.finishWrite(fos);
            Log.i(TAG, "atomicWrite: state stored: " + data);
        } catch (IOException e) {
            atomicFile.failWrite(fos);
            Log.e(TAG, "atomicWrite: exception", e);
        }
    }
    
    private PowerManager getPowerManager(Context context) {
        return (PowerManager) context.getSystemService(Context.POWER_SERVICE);
    }
    
    private KeyguardManager getKeyguardManager(Context context) {
        return (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
    }

    public void releaseWakeLock(Context context) {
        releasePartialWakeLock(context);
        releaseFullWakeLock(context);
    }

    public void enableKeyguard(Context context) {
        if (mKeyguardLock != null) {
            mKeyguardLock.reenableKeyguard();
            mKeyguardLock = null;
            Log.d(TAG, "enableKeyguard reenableKeyguard");
        } else {
            Log.d(TAG, "enableKeyguard mKeyguardLock == null");
        }
    }

    public void releasePartialWakeLock(Context context) {
        if (mPartialWakelock != null) {
            if (mPartialWakelock.isHeld()) {
                mPartialWakelock.release();
                mPartialWakelock = null;
                Log.d(TAG, "releasePartialWakeLock release");
            } else {
                Log.d(TAG, "releasePartialWakeLock mWakelock.isHeld() == false");
            }
        } else {
            Log.d(TAG, "releasePartialWakeLock mWakelock == null");
        }
    }

    public void releaseFullWakeLock(Context context) {
        if (mFullWakelock != null) {
            if (mFullWakelock.isHeld()) {
                mFullWakelock.release();
                mFullWakelock = null;
                Log.d(TAG, "releaseFullWakeLock release");
            } else {
                Log.d(TAG, "releaseFullWakeLock mWakelock.isHeld() == false");
            }
        } else {
            Log.d(TAG, "releaseFullWakeLock mWakelock == null");
        }
    }

    public void disableKeyguard(Context context) {
        KeyguardManager km = getInstance().getKeyguardManager(context);
    
        if (mKeyguardLock == null) {
            // get KeyguardLock
            mKeyguardLock = km.newKeyguardLock("dm_KL");
            if (km.inKeyguardRestrictedInputMode()) {
                Log.d(TAG, "need to disableKeyguard");
                // release key guard lock
                mKeyguardLock.disableKeyguard();
            } else {
                mKeyguardLock = null;
                Log.d(TAG, "not need to disableKeyguard");
            }
        }
    }

    public void acquireFullWakelock(Context context) {
        PowerManager pm = getInstance().getPowerManager(context);
    
        if (mFullWakelock == null) {
            // get WakeLock
            mFullWakelock = pm.newWakeLock(
                    PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.ON_AFTER_RELEASE,
                    "dm_FullLock");
            if (!mFullWakelock.isHeld()) {
                Log.d(TAG, "need to aquire full wake up");
                // wake lock
                mFullWakelock.acquire();
            } else {
                mFullWakelock = null;
                Log.d(TAG, "not need to aquire full wake up");
            }
        }
    }

    public void acquirePartialWakelock(Context context) {
        PowerManager pm = getInstance().getPowerManager(context);
        if (mPartialWakelock == null) {
            // get WakeLock
            mPartialWakelock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "dm_PartialLock");
            if (!mPartialWakelock.isHeld()) {
                Log.d(TAG, "need to aquire partial wake up");
                // wake lock
                mPartialWakelock.acquire();
            } else {
                mPartialWakelock = null;
                Log.d(TAG, "not need to aquire partial wake up");
            }
        }
    }

    public void stayForeground(Service service) {
        Log.i(TAG, "Bring service to foreground");
        Notification notification = new Notification();
        notification.flags |= Notification.FLAG_HIDE_NOTIFICATION;
        service.startForeground(1, notification);
    }

    public void leaveForeground(Service service) {
        Log.d(TAG, "Exec stopForeground with para true.");
        service.stopForeground(true);
    }
}
