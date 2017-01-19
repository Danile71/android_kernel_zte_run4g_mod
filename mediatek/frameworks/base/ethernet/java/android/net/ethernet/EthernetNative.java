/*
 * Copyright (C) 2010 The Android-X86 Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Yi Sun <beyounn@gmail.com>
 */

package android.net.ethernet;

import android.util.Slog;


/**
 * Native calls for sending requests to the kernel,
 * {@hide}
 */
public class EthernetNative {

    public static final String TAG = "EthernetNative";

    static {		
        try { 
            System.loadLibrary("ethernet_jni");
        } catch (UnsatisfiedLinkError e) {
            Slog.e(TAG, "error load ethernet_jni so");
        }
    }
    public native static String getInterfaceName(int i);
    public native static int getInterfaceCnt();
    public native static int initEthernetNative();
    public native static String waitForEvent();
    public native static String getMacAddressCommand();
    public native static int getLinkStatus();
}
