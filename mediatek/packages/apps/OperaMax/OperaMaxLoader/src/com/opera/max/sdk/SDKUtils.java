package com.opera.max.sdk;

import java.util.HashSet;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Binder;

import com.opera.max.loader.BuildConfig;

public class SDKUtils {
    private final static String PREF_LOAD_PRODUCT = "load.product";
    private final static String PREFS_FILE = "com.opera.max.loader.prfs";

    private static final String[] AUTHORIZED_PACKAGES = {
            "com.opera.max.loadertest",
            "com.miui.networkassistent",
            "com.opera.max",
            "com.oupeng.max" };
    private static final HashSet<Integer> AUTHORIZED_PACKAGES_UIDS = new HashSet<Integer>();
    private static boolean initialized;

    public static void Initialize(Context context) {
        if (initialized) {
            return;
        }
        AUTHORIZED_PACKAGES_UIDS.clear();
        PackageManager pm = context.getPackageManager();
        for (String pkg : AUTHORIZED_PACKAGES) {
            try {
                ApplicationInfo info = pm.getApplicationInfo(pkg, 0);
                if (info != null) {
                    AUTHORIZED_PACKAGES_UIDS.add(info.uid);
                }
            } catch (NameNotFoundException e) {
            }
        }
        initialized = true;
    }

    public static boolean isAuthorized() {
        return BuildConfig.DEBUG ? true : AUTHORIZED_PACKAGES_UIDS.contains(Binder.getCallingUid());
    }

    public static int getLoadProduct(Context context) {
        final SharedPreferences prefs = context.getSharedPreferences(
                PREFS_FILE, Context.MODE_PRIVATE);
        return prefs.getInt(PREF_LOAD_PRODUCT, 0);
    }

    public static void setLoadProduct(Context context, int loadProduct) {
        final SharedPreferences prefs = context.getSharedPreferences(
                PREFS_FILE, Context.MODE_PRIVATE);
        final Editor editor = prefs.edit();
        editor.putInt(PREF_LOAD_PRODUCT, loadProduct);
        editor.apply();
    }
}
