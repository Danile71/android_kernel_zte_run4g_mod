/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.passpoint;

import com.mediatek.common.passpoint.*;
import android.os.SystemProperties;
import android.util.Log;

import java.util.*;

public class WifiTreeHelper {
    final String TAG = "WifiTreeHelper";
    static private Object mGlobalSync = new Object();

    public WifiTreeHelper() {

    }

    public void configToPpsMoNumber(int id, WifiTree tree) {
        WifiTree.SpFqdn currentSpfqdn = tree.createSpFqdn("wi-fi.org");
        tree.PpsMoId = id;
        WifiTree.AAAServerTrustRoot aaa = null;
        WifiTree.SPExclusionList spexclusion = null;
        WifiTree.CredentialInfo info = currentSpfqdn.perProviderSubscription.createCredentialInfo("Cred01");

        switch(id) {
        case 1:
        case 2:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test01";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            if (id == 2) {
                currentSpfqdn.perProviderSubscription.UpdateIdentifier = "2";
                info.credential.usernamePassword.Username = "test02";
                info.credential.usernamePassword.MachineManaged = false;
            }
            break;

        case 3:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "3";
            info.policy.createPreferredRoamingPartnerList("PRP01","ruckuswireless.com,exactMatch","1","US");
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test03";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 4:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "4";
            aaa = info.createAAAServerTrustRoot("STR01","xxx","xxx");
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test04";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 5:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "5";
            info.policy.policyUpdate.UpdateInterval= "0xFFFFFFFF";
            info.policy.policyUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            spexclusion = info.policy.createSPExclusionList("SPE01","Hotspot 2.0");
            info.credentialPriority = "1";
            aaa = info.createAAAServerTrustRoot("STR01","xxx","xxx");
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test05";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 6:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "6";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Example";
            info.homeSP.FQDN = "example.com";
            info.homeSP.createHomeOIList("HOI01", "001BC504BE", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test06";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "example.com";
            break;

        case 7:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "7";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.homeSP.createHomeOIList("HOI01", "001BC504BE", true);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test07";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 8:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "8";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc08";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test08";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 9:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "9";
            info.policy.createPreferredRoamingPartnerList("PRP01","ruckuswireless.com,exactMatch","1","US");
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.usernamePassword.Username = "testdmacc08";
            info.policy.policyUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test09";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 10:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "10";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.IconURL = "TBD";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.digitalCertificate.CertificateType = "x509v3";
            info.credential.digitalCertificate.CertSHA256Fingerprint = SystemProperties.get("persist.service.credsha256");
            info.credential.Realm = "wi-fi.org";
            break;

        case 11:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "11";
            info.policy.createPreferredRoamingPartnerList("PRP01","ruckuswireless.com,includeSubdomains","1","US");
            info.policy.policyUpdate.UpdateInterval= "0xFFFFFFFF";
            info.policy.policyUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Example";
            info.homeSP.FQDN = "example.com";
            info.homeSP.createHomeOIList("HOI01", "001BC504BE", false);
            info.homeSP.createOtherHomePartners("OHP01","bt.com");
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test11";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "mail.example.com";
            break;

        case 12:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "12";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "TBD";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.digitalCertificate.CertificateType = "x509v3";
            info.credential.Realm = "wi-fi.org";
            break;

        case 13:
        case 14:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "13";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test13";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            if (id == 14) {
                currentSpfqdn.perProviderSubscription.UpdateIdentifier = "14";
                info.credential.usernamePassword.Username = "test14";
                info.credential.usernamePassword.MachineManaged = false;
            }
            break;

        case 15:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "15";
            info.policy.createPreferredRoamingPartnerList("PRP01","ruckuswireless.com,exactMatch","1","US");
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("HOI01", "506F9A", false);
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test15";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 16:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "16";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test16";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 17:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "17";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.digitalCertificate.CertificateType = "x509v3";
            info.credential.Realm = "wi-fi.org";
            break;

        case 18:
            //Note: this PPS MO is no longer needed, but the ID is reserved so that the testplan does not need renumbering.
            break;

        case 19:
            //Note: this PPS MO is no longer needed, but the ID is reserved so that the testplan does not need renumbering.
            break;

        case 20:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "20";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test20";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 21:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "21";
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.usernamePassword.Username = "testdmacc21";
            info.policy.policyUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test21";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 22:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "22";
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.usernamePassword.Username = "testdmacc22";
            info.policy.policyUpdate.usernamePassword.Password = "P@ssw0rd";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Example dot com";
            info.homeSP.FQDN = "example.com";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test22";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "example.com";
            break;

        case 23:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "23";
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.usernamePassword.Username = "testdmacc23";
            info.policy.policyUpdate.usernamePassword.Password = "P@ssw0rd";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test23";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 24:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "24";
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.usernamePassword.Username = "testdmacc24";
            info.policy.policyUpdate.usernamePassword.Password = "P@ssw0rd";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test24";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 25:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "25";
            info.policy.policyUpdate.UpdateInterval= "0xA";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "Unrestricted";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.usernamePassword.Username = "testdmacc25";
            info.policy.policyUpdate.usernamePassword.Password = "P@ssw0rd";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.FQDN = "wi-fi.org";
            info.credentialPriority = "1";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test25";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 26:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "26";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test26";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";

            WifiTree.CredentialInfo info2 = currentSpfqdn.perProviderSubscription.createCredentialInfo("Cred02");
            info2.credentialPriority = "2";
            info2.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info2.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info2.subscriptionUpdate.Restriction = "HomeSp";
            info2.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info2.homeSP.FriendlyName = "Wi-Fi Alliance";
            info2.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info2.homeSP.FQDN = "wi-fi.org";
            info2.credential.CreationDate = "20121201T12:00:00Z";
            info2.credential.sim.IMSI = "234564085551515";
            info2.credential.sim.EAPType = "18";
            info2.credential.Realm = "wlan.mnc56.mcc234.3gppnetwork.org";
            break;

        case 27:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "27";
            info.policy.createPreferredRoamingPartnerList("Prp01", "ericsson.com,exactMatch", "1", "US");
            info.policy.createPreferredRoamingPartnerList("Prp02", "example.com,exactMatch", "2", "US");
            info.policy.createPreferredRoamingPartnerList("Prp03", "example2.com,exactMatch", "3", "US");
            info.policy.createMinBackhaulThreshold("Mbt01", "Home", "3000", "500");
            info.policy.createMinBackhaulThreshold("Mbt02", "Roaming", "3000", "500");
            info.policy.policyUpdate.UpdateInterval = "0xFFFFFFFF";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "HomeSP";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.trustRoot.CertURL = "TBD";
            info.policy.policyUpdate.trustRoot.CertSHA256Fingerprint = "TBD";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test27";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 28:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "28";
            info.policy.createPreferredRoamingPartnerList("Prp01", "ericsson.com,exactMatch", "1", "US");
            info.policy.createPreferredRoamingPartnerList("Prp02", "example.com,exactMatch", "2", "US");
            info.policy.createPreferredRoamingPartnerList("Prp03", "example2.com,exactMatch", "3", "US");
            info.policy.createRequiredProtoPortTuple("Ppt01", "6", "5060");
            info.policy.createRequiredProtoPortTuple("Ppt02", "17", "5060");
            info.policy.policyUpdate.UpdateInterval = "0xFFFFFFFF";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "HomeSP";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.policy.policyUpdate.trustRoot.CertURL = "TBD";
            info.policy.policyUpdate.trustRoot.CertSHA256Fingerprint = "TBD";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc28";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test28";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 29:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "29";
            info.policy.maximumBSSLoadValue = "100";
            info.policy.policyUpdate.UpdateInterval = "0xFFFFFFFF";
            info.policy.policyUpdate.UpdateMethod = "OMA-DM-ClientInitiated";
            info.policy.policyUpdate.Restriction = "HomeSP";
            info.policy.policyUpdate.URI = "https://policy-server.r2-testbed.wi-fi.org";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc29";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("Home01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test29";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 30:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "30";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc30";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("Home01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test30";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 31:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "31";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc31";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("Home01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test31";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 32:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "32";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc32";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("Home01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test32";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 33:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "33";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc33";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("Home01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test33";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 34:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "34";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc34";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.homeSP.createHomeOIList("Home01", "506F9A", false);
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test34";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        case 35:
            currentSpfqdn.perProviderSubscription.UpdateIdentifier = "35";
            info.credentialPriority = "1";
            info.subscriptionUpdate.UpdateInterval = "0xFFFFFFFF";
            info.subscriptionUpdate.UpdateMethod = "SPP-ClientInitiated";
            info.subscriptionUpdate.Restriction = "HomeSp";
            info.subscriptionUpdate.URI = "https://subscription-server.r2-testbed.wi-fi.org";
            info.subscriptionUpdate.usernamePassword.Username = "testdmacc35";
            info.subscriptionUpdate.usernamePassword.Password = "P@ssw0rd";
            info.homeSP.FriendlyName = "Wi-Fi Alliance";
            info.homeSP.IconURL = "http://www.wi-fi.org/sites/default/files/uploads/wfa_certified_3d_web_lr.png";
            info.homeSP.FQDN = "wi-fi.org";
            info.credential.CreationDate = "20121201T12:00:00Z";
            info.credential.usernamePassword.Username = "test35";
            info.credential.usernamePassword.Password = "ChangeMe";
            info.credential.usernamePassword.MachineManaged = true;
            info.credential.usernamePassword.eAPMethod.EAPType = "21";
            info.credential.usernamePassword.eAPMethod.InnerMethod = "29";
            info.credential.Realm = "wi-fi.org";
            break;

        default:
            Log.d(TAG, "configToPpsMoNumber unknow id:" + id);
            break;
        }

        //for fast change parameters in PF
        info.subscriptionUpdate.URI = "http" + SystemProperties.get("persist.service.remurl");
        if(!SystemProperties.get("persist.service.remurl2").isEmpty()) {
            info.subscriptionUpdate.URI += ("&" + SystemProperties.get("persist.service.remurl2"));
        }
        info.policy.policyUpdate.URI = "http" + SystemProperties.get("persist.service.polupdurl");
        if(!SystemProperties.get("persist.service.polupdurl2").isEmpty()) {
            info.policy.policyUpdate.URI += ("&" + SystemProperties.get("persist.service.polupdurl2"));
        }

        if (!SystemProperties.get("persist.service.creduser").isEmpty()) {
            info.credential.usernamePassword.Username = SystemProperties.get("persist.service.creduser");
        }

        if (!SystemProperties.get("persist.service.credpass", null).isEmpty()) {
            info.credential.usernamePassword.Password = SystemProperties.get("persist.service.credpass");
        }

        if (!SystemProperties.get("persist.service.subupduser", null).isEmpty()) {
            info.subscriptionUpdate.usernamePassword.Username = SystemProperties.get("persist.service.subupduser");
        }

        if (!SystemProperties.get("persist.service.subupdpass", null).isEmpty()) {
            info.subscriptionUpdate.usernamePassword.Password = SystemProperties.get("persist.service.subupdpass");
        }

        if (!SystemProperties.get("persist.service.polupduser", null).isEmpty()) {
            info.policy.policyUpdate.usernamePassword.Username = SystemProperties.get("persist.service.polupduser");
        }

        if (!SystemProperties.get("persist.service.polupdpass", null).isEmpty()) {
            info.policy.policyUpdate.usernamePassword.Password = SystemProperties.get("persist.service.polupdpass");
        }

        if (aaa != null && !SystemProperties.get("persist.service.aaasha256").isEmpty()) {
            aaa.CertSHA256Fingerprint = SystemProperties.get("persist.service.aaasha256");
        }

        if (spexclusion != null && !SystemProperties.get("persist.service.spelist").isEmpty()) {
            spexclusion.SSID = SystemProperties.get("persist.service.spelist");
        }
    }

    public void dumpTreeInfo(WifiTree tree) {

        synchronized (mGlobalSync) {

            Log.d(TAG, "===== dumpTreeInfo =====");
            Set spfqdnSet = tree.spFqdn.entrySet();
            Iterator spfqdnItr = spfqdnSet.iterator();

            for (WifiTree.SpFqdn sp : getSp(tree)) {
                Log.d(TAG, "<X>+service provider <" + sp.nodeName + ">");
                Log.d(TAG, "    sp.perProviderSubscription.UpdateIdentifier <" + sp.perProviderSubscription.UpdateIdentifier + ">");

                for (WifiTree.CredentialInfo info : getCredentialInfo(sp)) {
                    Log.d(TAG, "    <X>+credential info <" + info.nodeName + ">");
                    Log.d(TAG, "        credentialPriority <" + info.credentialPriority + ">");
                    Log.d(TAG, "        ---- ----");
                    //homeSp
                    Log.d(TAG, "        homeSP.FQDN <" + info.homeSP.FQDN + ">");
                    Log.d(TAG, "        homeSP.FriendlyName <" + info.homeSP.FriendlyName + ">");

                    for (WifiTree.HomeOIList homeoi : getHomeOIList(info)) {
                        Log.d(TAG, "        <X>+homeSP.HomeOIList <" + homeoi.nodeName + ">");
                        Log.d(TAG, "            HomeOI <" + homeoi.HomeOI + ">");
                        Log.d(TAG, "            HomeOIRequired <" + homeoi.HomeOIRequired + ">");
                    }

                    for (WifiTree.OtherHomePartners otherHP: getOtherHomePartner(info)) {
                        Log.d(TAG, "        <X>+homeSP.OtherHomePartners <" + otherHP.nodeName + ">");
                        Log.d(TAG, "            FQDN <" + otherHP.FQDN + ">");
                    }
                    Log.d(TAG, "        ---- ----");
                    //credential
                    Log.d(TAG, "        credential.Realm <" + info.credential.Realm + ">");
                    Log.d(TAG, "        credential.CheckAAAServerCertStatus <" + info.credential.CheckAAAServerCertStatus + ">");
                    Log.d(TAG, "        credential.usernamePassword.eAPMethod.EAPType <" +
                          info.credential.usernamePassword.eAPMethod.EAPType + ">");
                    Log.d(TAG, "        credential.usernamePassword.eAPMethod.InnerMethod <" +
                          info.credential.usernamePassword.eAPMethod.InnerMethod + ">");
                    Log.d(TAG, "        credential.usernamePassword.Username <" + info.credential.usernamePassword.Username + ">");
                    Log.d(TAG, "        credential.usernamePassword.Password <" + info.credential.usernamePassword.Password + ">");
                    Log.d(TAG, "        credential.usernamePassword.MachineManaged <" + info.credential.usernamePassword.MachineManaged + ">");
                    Log.d(TAG, "        credential.digitalCertificate.CertificateType <" + info.credential.digitalCertificate.CertificateType + ">");
                    Log.d(TAG, "        credential.digitalCertificate.CertSHA256Fingerprint <" + info.credential.digitalCertificate.CertSHA256Fingerprint + ">");
                    Log.d(TAG, "        credential.sim.IMSI <" + info.credential.sim.IMSI + ">");
                    Log.d(TAG, "        credential.sim.EAPType <" + info.credential.sim.EAPType + ">");
                    Log.d(TAG, "        ---- ----");
                    //subscriptionUpdate
                    Log.d(TAG, "        subscriptionUpdate.UpdateInterval <" + info.subscriptionUpdate.UpdateInterval + ">");
                    Log.d(TAG, "        subscriptionUpdate.UpdateMethod <" + info.subscriptionUpdate.UpdateMethod + ">");
                    Log.d(TAG, "        subscriptionUpdate.Restriction <" + info.subscriptionUpdate.Restriction + ">");
                    Log.d(TAG, "        subscriptionUpdate.URI <" + info.subscriptionUpdate.URI + ">");
                    Log.d(TAG, "        subscriptionUpdate.usernamePassword.Username <" + info.subscriptionUpdate.usernamePassword.Username + ">");
                    Log.d(TAG, "        subscriptionUpdate.usernamePassword.Password <" + info.subscriptionUpdate.usernamePassword.Password + ">");
                    Log.d(TAG, "        ---- ----");
                    //policy
                    Log.d(TAG, "        policy.policyUpdate.UpdateInterval <" + info.policy.policyUpdate.UpdateInterval + ">");
                    Log.d(TAG, "        policy.policyUpdate.UpdateMethod <" + info.policy.policyUpdate.UpdateMethod + ">");
                    Log.d(TAG, "        policy.policyUpdate.Restriction <" + info.policy.policyUpdate.Restriction + ">");
                    Log.d(TAG, "        policy.policyUpdate.URI <" + info.policy.policyUpdate.URI + ">");
                    Log.d(TAG, "        policy.policyUpdate.usernamePassword.Username <" + info.policy.policyUpdate.usernamePassword.Username + ">");
                    Log.d(TAG, "        policy.policyUpdate.usernamePassword.Password <" + info.policy.policyUpdate.usernamePassword.Password+ ">");
                    Log.d(TAG, "        policy.maximumBSSLoadValue <" + info.policy.maximumBSSLoadValue + ">");

                    for (WifiTree.PreferredRoamingPartnerList prp : getPreferredRoamingPartnerList(info)) {
                        Log.d(TAG, "        ---- ----");
                        Log.d(TAG, "        <X>+policy.PreferredRoamingPartnerList <" + prp.nodeName + ">");
                        Log.d(TAG, "            FQDN_Match <" + prp.FQDN_Match + ">");
                        Log.d(TAG, "            Priority <" + prp.Priority + ">");
                        Log.d(TAG, "            Country <" + prp.Country + ">");
                    }
                    Set minBackSet = info.policy.minBackhaulThreshold.entrySet();
                    Iterator minBackItr = minBackSet.iterator();
                    for (WifiTree.MinBackhaulThresholdNetwork minBT : getMinBackhaulThreshold(info)) {
                        Log.d(TAG, "        ---- ----");
                        Log.d(TAG, "        <X>+policy.MinBackhaulThresholdNetwork <" + minBT.nodeName + ">");
                        Log.d(TAG, "            NetworkType <" + minBT.NetworkType + ">");
                        Log.d(TAG, "            DLBandwidth <" + minBT.DLBandwidth + ">");
                        Log.d(TAG, "            ULBandwidth <" + minBT.ULBandwidth + ">");
                    }

                    for (WifiTree.RequiredProtoPortTuple protoPort : getRequiredProtoPortTuple(info)) {
                        Log.d(TAG, "        ---- ----");
                        Log.d(TAG, "        <X>+policy.RequiredProtoPortTuple <" + protoPort.nodeName + ">");
                        Log.d(TAG, "            IPProtocol <" + protoPort.IPProtocol + ">");
                        Log.d(TAG, "            PortNumber <" + protoPort.PortNumber + ">");
                    }

                    for (WifiTree.SPExclusionList spExc : getSPExclusionList(info)) {
                        Log.d(TAG, "        ---- ----");
                        Log.d(TAG, "        <X>+policy.SPExclusionList <" + spExc.nodeName + ">");
                        Log.d(TAG, "            SSID <" + spExc.SSID + ">");
                    }

                    for (WifiTree.AAAServerTrustRoot aaaTrustRoot : getAaaServerTrustRoot(info)) {
                        Log.d(TAG, "        ---- ----");
                        Log.d(TAG, "        <X>+AAAServerTrustRoot <" + aaaTrustRoot.nodeName + ">");
                        Log.d(TAG, "            CertURL <" + aaaTrustRoot.CertURL+ ">");
                        Log.d(TAG, "            CertSHA256Fingerprint <" + aaaTrustRoot.CertSHA256Fingerprint+ ">");
                    }
                }
            }
            Log.d(TAG, "===== End =====");
        }
    }

    public Collection<WifiTree.CredentialInfo> getCredentialInfo(WifiTree.SpFqdn sp) {
        try {
            return sp.perProviderSubscription.credentialInfo.values();
        } catch (Exception e) {
            Log.d(TAG, "getCredentialInfo err:" + e);
        }
        return null;
    }

    public WifiTree.CredentialInfo getCredentialInfo(WifiTree tree, String spnodename, String crednetialnodename) {
        try {
            for (String skey : tree.spFqdn.keySet()) {
                if (spnodename.equals(skey)) {
                    WifiTree.SpFqdn sp = tree.spFqdn.get(skey);
                    for(String ckey : sp.perProviderSubscription.credentialInfo.keySet()) {
                        if (crednetialnodename.equals(ckey)) {
                            return sp.perProviderSubscription.credentialInfo.get(ckey);
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "getCredentialInfo err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.SpFqdn> getSp(WifiTree tree) {
        try {
            return tree.spFqdn.values();
        } catch (Exception e) {
            Log.d(TAG, "getSp err:" + e);
        }
        return null;
    }

    public WifiTree.SpFqdn getSp(WifiTree tree, String spnodename) {
        try {
            for (String key : tree.spFqdn.keySet()) {
                if (spnodename.equals(key)) {
                    return tree.spFqdn.get(key);
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "getSp err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.HomeOIList> getHomeOIList(WifiTree.CredentialInfo info) {
        try {
            return info.homeSP.homeOIList.values();
        } catch (Exception e) {
            Log.d(TAG, "getHomeOiList err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.OtherHomePartners> getOtherHomePartner(WifiTree.CredentialInfo info) {
        try {
            return info.homeSP.otherHomePartners.values();
        } catch (Exception e) {
            Log.d(TAG, "getOtherHomePartner err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.AAAServerTrustRoot> getAaaServerTrustRoot(WifiTree.CredentialInfo info) {
        try {
            return info.aAAServerTrustRoot.values();
        } catch (Exception e) {
            Log.d(TAG, "getAaaServerTrustRoot err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.PreferredRoamingPartnerList> getPreferredRoamingPartnerList(WifiTree.CredentialInfo info) {
        try {
            return info.policy.preferredRoamingPartnerList.values();
        } catch (Exception e) {
            Log.d(TAG, "getPreferredRoamingPartnerList err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.SPExclusionList> getSPExclusionList(WifiTree.CredentialInfo info) {
        try {
            return info.policy.sPExclusionList.values();
        } catch (Exception e) {
            Log.d(TAG, "getSPExclusionList err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.MinBackhaulThresholdNetwork> getMinBackhaulThreshold(WifiTree.CredentialInfo info) {
        try {
            return info.policy.minBackhaulThreshold.values();
        } catch (Exception e) {
            Log.d(TAG, "getMinBackhaulThreshold err:" + e);
        }
        return null;
    }

    public Collection<WifiTree.RequiredProtoPortTuple> getRequiredProtoPortTuple(WifiTree.CredentialInfo info) {
        try {
            return info.policy.requiredProtoPortTuple.values();
        } catch (Exception e) {
            Log.d(TAG, "getRequiredProtoPortTuple err:" + e);
        }
        return null;
    }

    public String getMaximumBSSLoadValue(WifiTree.CredentialInfo info) {
        try {
            return info.policy.maximumBSSLoadValue;
        } catch (Exception e) {
            Log.d(TAG, "getMaximumBSSLoadValue err:" + e);
        }
        return null;
    }


}
