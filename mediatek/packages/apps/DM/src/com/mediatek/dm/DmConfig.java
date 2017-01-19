/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.dm;

import android.content.Context;
import android.net.Proxy;
import android.util.Log;

import com.mediatek.dm.DmConst.TAG;
import com.mediatek.dm.conn.DmDatabase;
import com.mediatek.dm.option.Options;
import com.redbend.vdm.VdmConfig;
import com.redbend.vdm.VdmConfig.DmAccConfiguration;
import com.redbend.vdm.VdmConfig.HttpAuthLevel;
import com.redbend.vdm.VdmException;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class DmConfig {
    private Context mContext;
    private VdmConfig mConfig;
    private Properties mParamTable;
    private boolean mIsDdlExtSet;

    private static final String VERSION_DM_1_1_2 = "1.1.2";
    private static final String VERSION_DM_112 = "112";
    private static final String LEVEL_NONE = "none";
    private static final String LEVEL_DIGEST = "digest";
    private static final String LEVEL_BASIC = "basic";

    private static final String DL_EXT = "dl_ext";
    private static final String DDL = "ddl";
    private static final String ENCODE = "encode";
    private static final String XML = "xml";
    private static final String DL_PROXY = "dlproxy";
    private static final String DM_PROXY = "dmproxy";
    private static final String DM_VERSION = "dmversion";
    private static final String ACC_ROOT = "12accountroot";
    private static final String EXCLUSIVE = "exclusive";
    private static final String UPDATE_INACTIVE_ACC = "updateinactiveaccount";
    private static final String DM_SERVER = "dmserver";
    private static final String INSTALL_NOTIFY_SUCCESS_ONLY = "installnotifysuccessonly";
    private static final String SERVER_202_UNSUPPORTED = "server_202_unsupported";
    private static final String DM_HEXSID = "hexsid";

    // HTTP/Proxy Authentication
    private static final int HTTP_AUTH_LEVEL = 0;
    private static final int HTTP_AUTH_UNAME = 1;
    private static final int HTTP_AUTH_PWD = 2;
    private static final String[] DM_HTTP_AUTH = { "dm_http_auth_level", "dm_http_auth_username", "dm_http_auth_password" };
    private static final String[] DL_HTTP_AUTH = { "dl_http_auth_level", "dl_http_auth_username", "dl_http_auth_password" };
    private static final String[] DM_PROXY_AUTH = { "dm_proxy_auth_level", "dm_proxy_auth_username",
            "dm_proxy_auth_password" };
    private static final String[] DL_PROXY_AUTH = { "dl_proxy_auth_level", "dl_proxy_auth_username",
            "dl_proxy_auth_password" };

    // config the dm message buffer size, if happened buffer overflow, change it bigger.
    private static final int MAX_DM_MESSAGE_SIZE = 6000;

    public DmConfig(Context ctx) {
        mContext = ctx;
        mConfig = new VdmConfig();
        try {
            InputStream cfgStream = new FileInputStream(DmConst.PathName.CONFIG_FILE_IN_SYSTEM);

            if (cfgStream != null) {
                mParamTable = new Properties();
                mParamTable.loadFromXML(cfgStream);
                cfgStream.close();
            }
        } catch (IOException e) {
            Log.w(TAG.COMMON, "VdmcConfig:Caught exception " + e.getMessage());
        }
    }

    public void configure() {
        try {
            // String proxyAddr = mDmDatabase.getApnProxyFromSettings();
            // int proxyPort = mDmDatabase.getApnProxyPortFromSettings();

            if (Options.USE_DIRECT_INTERNET) {
                // for ZTE DM server(via wifi/net), MUST not be set!
                Log.i(TAG.COMMON, "[DMConfig] skip setting proxy for direct internet.");
            } else {
                // for cmcc/cu DM server, proxy MUST be set!
                Log.i(TAG.COMMON, "[DMConfig] setting proxy for WAP.");

                if (Proxy.getDefaultHost() != null) {
                    String defaultProxcy = new StringBuilder("http://").append(Proxy.getDefaultHost())
                            .append(File.pathSeparator).append(Proxy.getDefaultPort()).toString();
                    mConfig.setDmProxy(defaultProxcy);
                    mConfig.setDlProxy(defaultProxcy);

                } else {
                    DmDatabase dmDB = new DmDatabase(mContext);
                    String proxyAddr = dmDB.getApnProxyFromSettings();
                    int proxyPort = dmDB.getApnProxyPortFromSettings();
                    Log.i(TAG.COMMON,
                            new StringBuilder("Proxy addr = ").append(proxyAddr).append(", port = ").append(proxyPort)
                                    .toString());

                    if (proxyAddr != null && proxyPort > 0) {
                        String proxcy = new StringBuilder("http://").append(proxyAddr).append(File.pathSeparator)
                                .append(proxyPort).toString();
                        mConfig.setDmProxy(proxcy);
                        mConfig.setDlProxy(proxcy);
                    } else {
                        Log.w(TAG.COMMON, "DM_PROXY not configed");
                    }
                }
            }

            mConfig.setEncodeWBXMLMsg(false);
            mConfig.setSwapCpPeers(true);
            mConfig.setMaxNetRetries(3);
            mConfig.setMaxMsgSize(MAX_DM_MESSAGE_SIZE);
            mConfig.setDDVersionCheck(false);
            mConfig.setIgnoreMissingETag();
            mConfig.setNotificationVerificationMode(VdmConfig.NotifVerificationMode.DISABLED);
            if (mParamTable != null) {

                configureDMAcc();

                if (mParamTable.containsKey(DM_SERVER)) {
                    String dmServer = mParamTable.getProperty(DM_SERVER);
                    if (dmServer != null) {
                        if (dmServer.equals("wedm")) {
                            mConfig.setIsUsingWedmServer(true, "IBM");
                            mConfig.setIsDMAccNameOptional(true);
                        }
                    }
                } else {
                    Log.w(TAG.COMMON, "DM_SERVER not configed");
                }

                if (mParamTable.containsKey(DM_HEXSID)) {
                    String dmServer = mParamTable.getProperty(DM_HEXSID);
                    if (dmServer != null) {
                        if (dmServer.equals("true")) {
                            mConfig.setSessionIDAsDec(false);
                        } else {
                            mConfig.setSessionIDAsDec(true);
                        }
                    }
                } else {
                    mConfig.setSessionIDAsDec(true);
                    Log.w(TAG.COMMON, "DM_HEXSID not configed");
                }

                if (mParamTable.containsKey(DM_PROXY)) {
                    String dmProxy = mParamTable.getProperty(DM_PROXY);
                    mConfig.setDmProxy(dmProxy);
                } else {
                    Log.w(TAG.COMMON, "DM_PROXY not configed");
                }

                if (mParamTable.containsKey(DL_PROXY)) {
                    String dlProxy = mParamTable.getProperty(DL_PROXY);
                    mConfig.setDlProxy(dlProxy);
                } else {
                    Log.w(TAG.COMMON, "DL_PROXY not configed");
                }

                if (mParamTable.containsKey(DL_EXT)) {
                    String dlExt = mParamTable.getProperty(DL_EXT);
                    if (dlExt != null) {
                        if (dlExt.equals(DDL)) {
                            mIsDdlExtSet = true;
                        }
                    }
                } else {
                    Log.w(TAG.COMMON, "DL_EXT not configed");
                }

                if (mParamTable.containsKey(ENCODE)) {
                    String encode = mParamTable.getProperty(ENCODE);
                    if (encode != null) {
                        if (encode.compareTo(XML) == 0) {
                            Log.i(TAG.COMMON, "Call mConfig.setEncodeWBXMLMsg(false)");
                            mConfig.setEncodeWBXMLMsg(false);
                        }
                    }
                } else {
                    Log.w(TAG.COMMON, "ENCODE not configed");
                }

                if (mParamTable.containsKey(SERVER_202_UNSUPPORTED)) {
                    String server202NotSupported = mParamTable.getProperty(SERVER_202_UNSUPPORTED);
                    if (server202NotSupported != null) {
                        if (server202NotSupported.charAt(0) == 'T' || server202NotSupported.charAt(0) == 't') {
                            mConfig.set202statusCodeNotSupportedByServer(true);
                        }
                    }
                } else {
                    Log.w(TAG.COMMON, "SERVER_202_UNSUPPORTED not configed");
                }

                if (mParamTable.containsKey(INSTALL_NOTIFY_SUCCESS_ONLY)) {
                    String installNotify = mParamTable.getProperty(INSTALL_NOTIFY_SUCCESS_ONLY);
                    if (installNotify != null) {
                        if (installNotify.charAt(0) == 'T' || installNotify.charAt(0) == 't') {
                            mConfig.setInstallNotifySuccessOnly(true);
                        }
                    }
                } else {
                    Log.w(TAG.COMMON, "INSTALL_NOTIFY_SUCCESS_ONLY not configed");
                }

                configureHttpAuth(true); // DM HTTP Authentication
                configureHttpAuth(false); // DL HTTP Authentication
                configureProxyAuth(true); // DM Proxy Authentication
                configureProxyAuth(false); // DL Proxy Authentication
            } else {
                mConfig.setSessionIDAsDec(true);
            }
        } catch (VdmException e) {
            Log.w(TAG.COMMON, "VdmcConfig:Caught exception " + e.getMessage());
        }
    }

    public boolean isDDLExtSet() {
        return mIsDdlExtSet;
    }

    private void configureDMAcc() throws VdmException {
        DmAccConfiguration dmacc = mConfig.new DmAccConfiguration();

        if (mParamTable.containsKey(DM_VERSION)) {
            String dmVersion = mParamTable.getProperty(DM_VERSION);
            if (VERSION_DM_112.equals(dmVersion) || VERSION_DM_1_1_2.equals(dmVersion)) {
                dmacc.activeAccountDMVersion = VdmConfig.DmVersion.DM_1_1_2;
            }
        } else {
            Log.w(TAG.COMMON, "DM_VERSION not configed");
        }

        if (mParamTable.containsKey(ACC_ROOT)) {
            dmacc.dm12root = mParamTable.getProperty(ACC_ROOT);
        } else {
            Log.w(TAG.COMMON, "ACC_ROOT not configed");
        }

        if (mParamTable.containsKey(EXCLUSIVE)) {
            String exclusive = mParamTable.getProperty(EXCLUSIVE);
            if (exclusive != null) {
                if (exclusive.charAt(0) == 'T' || exclusive.charAt(0) == 't') {
                    // dmacc.isExclusive = true;
                    Log.e(TAG.COMMON, "[config]exclusive=" + exclusive);
                }
            }
        } else {
            Log.w(TAG.COMMON, "EXCLUSIVE not configed");
        }

        if (mParamTable.containsKey(UPDATE_INACTIVE_ACC)) {
            String updateInactiveAcc = mParamTable.getProperty(UPDATE_INACTIVE_ACC);
            if (updateInactiveAcc != null) {
                if (updateInactiveAcc.charAt(0) == 'T' || updateInactiveAcc.charAt(0) == 't') {
                    dmacc.updateInactiveDMAccount = true;
                }
            }
        } else {
            Log.w(TAG.COMMON, "UPDATE_INACTIVE_ACC not configed");
        }

        mConfig.setDmAccConfiguration(dmacc);
    }

    private void configureHttpAuth(boolean isDM) throws VdmException {

        String[] auth = isDM ? DM_HTTP_AUTH : DL_HTTP_AUTH;
        String dmDL = isDM ? "DM" : "DL";

        if (mParamTable.containsKey(auth[HTTP_AUTH_LEVEL])) {

            String level = mParamTable.getProperty(auth[HTTP_AUTH_LEVEL]);
            if (LEVEL_NONE.equalsIgnoreCase(level)) {
                if (isDM) {
                    mConfig.setDmHttpAuthentication(HttpAuthLevel.NONE, null, null);
                } else {
                    mConfig.setDlHttpAuthentication(HttpAuthLevel.NONE, null, null);
                }

            } else {
                String username = mParamTable.getProperty(auth[HTTP_AUTH_UNAME]);
                String password = mParamTable.getProperty(auth[HTTP_AUTH_PWD]);

                if (username == null || password == null) {
                    Log.w(TAG.COMMON, "Missing credentials for " + dmDL + " HTTP Authentication");
                    throw new VdmException(VdmException.VdmError.BAD_INPUT);
                }
                if (LEVEL_DIGEST.equalsIgnoreCase(level)) {
                    Log.w(TAG.COMMON, dmDL + " HTTP Authentication 'Digest' is not supported");
                    throw new VdmException(VdmException.VdmError.NOT_IMPLEMENTED);
                }
                if (!LEVEL_BASIC.equalsIgnoreCase(level)) {
                    Log.w(TAG.COMMON,
                            new StringBuilder("Invalid ").append(dmDL).append(" HTTP Authentication ").append(level)
                                    .toString());
                    throw new VdmException(VdmException.VdmError.BAD_INPUT);
                }

                if (isDM) {
                    mConfig.setDmHttpAuthentication(HttpAuthLevel.BASIC, username, password);
                } else {
                    mConfig.setDlHttpAuthentication(HttpAuthLevel.BASIC, username, password);
                }

            }
        }
    }

    private void configureProxyAuth(boolean isDM) throws VdmException {

        String[] auth = isDM ? DM_PROXY_AUTH : DL_PROXY_AUTH;
        String dmDL = isDM ? "DM" : "DL";

        if (mParamTable.containsKey(auth[HTTP_AUTH_LEVEL])) {

            Log.w(TAG.COMMON, "auth[HTTP_AUTH_LEVEL] was configed");

            String level = mParamTable.getProperty(auth[HTTP_AUTH_LEVEL]);

            if (LEVEL_NONE.equalsIgnoreCase(level)) {
                Log.w(TAG.COMMON, "level was configed");
                if (isDM) {
                    mConfig.setDmProxyAuthentication(HttpAuthLevel.NONE, null, null);
                } else {
                    mConfig.setDlProxyAuthentication(HttpAuthLevel.NONE, null, null);
                }

            } else {
                Log.w(TAG.COMMON, "level was NOT configed");
                String username = mParamTable.getProperty(auth[HTTP_AUTH_UNAME]);
                String password = mParamTable.getProperty(auth[HTTP_AUTH_PWD]);

                if (username == null || password == null) {
                    Log.w(TAG.COMMON, "Missing credentials for " + dmDL + " HTTP Authentication");
                    throw new VdmException(VdmException.VdmError.BAD_INPUT);
                }
                if (LEVEL_DIGEST.equalsIgnoreCase(level)) {
                    Log.w(TAG.COMMON, dmDL + " Proxy Authentication 'Digest' is not supported");
                    throw new VdmException(VdmException.VdmError.NOT_IMPLEMENTED);
                }
                if (!LEVEL_BASIC.equalsIgnoreCase(level)) {
                    Log.w(TAG.COMMON,
                            new StringBuilder("Invalid ").append(dmDL).append(" Proxy Authentication ").append(level)
                                    .toString());
                    throw new VdmException(VdmException.VdmError.BAD_INPUT);
                }

                if (isDM) {
                    mConfig.setDmProxyAuthentication(HttpAuthLevel.BASIC, username, password);
                } else {
                    mConfig.setDlProxyAuthentication(HttpAuthLevel.BASIC, username, password);
                }

            }
        } else {
            Log.w(TAG.COMMON, "auth not configed");
        }
    }
}
