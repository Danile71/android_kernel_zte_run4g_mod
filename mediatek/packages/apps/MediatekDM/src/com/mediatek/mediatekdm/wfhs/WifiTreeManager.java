package com.mediatek.mediatekdm.wfhs;

import android.R.id;
import android.R.integer;
import android.R.string;
import android.content.Context;

import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import com.mediatek.common.passpoint.WifiTree;
import com.mediatek.mediatekdm.DmOperation;
import com.mediatek.mediatekdm.DmService;
import com.mediatek.mediatekdm.SessionHandler;
import com.mediatek.mediatekdm.mdm.MdmException;
import com.mediatek.mediatekdm.mdm.MdmTree;
import com.mediatek.mediatekdm.mdm.NodeIoHandler;
import com.mediatek.mediatekdm.mdm.MdmException.MdmError;
import com.mediatek.mediatekdm.wfhs.io.NodeIOHandlerInfo;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.security.PublicKey;
import java.util.List;

public class WifiTreeManager extends SessionHandler{

    private static final String TAG = "DM/Wifi.WifiTreeManager";
    public static final String WIFI_NODE_START = "./Wi-Fi";
    
    private MdmWifi mMdmWifi;
    
    public WifiTreeManager(DmService service, MdmWifi wifi) {
        
        super(service);
        mContext = service;
        mTree = new MdmTree();
        mMdmWifi = wifi;
    }

    @Override
    protected void dmComplete() {
        mOperationManager.finishCurrent();
        mMdmWifi.onSessionEnd(0);
    }

    @Override
    protected void dmAbort(int lastError) {
        mOperationManager.finishCurrent();
        mMdmWifi.onSessionEnd(-1);
        
    }
    public void replaceStringValue(String uri, String val) {
        Log.i(TAG, "change (" + uri + ") value to : " + val);
        try {
            mTree.replaceStringValue(uri, val);
            mTree.writeToPersistentStorage();
        } catch (MdmException e) {
            e.printStackTrace();
        } 
    }

    public String getValue(String uri, int format) {
        try {
            switch (format) {
                case FORMAT_INT:
                    return String.valueOf(mTree.getIntValue(uri));
                case FORMAT_CHR:
                    return mTree.getStringValue(uri);
                case FORMAT_BOL:
                    return String.valueOf(mTree.getBoolValue(uri));
                case FORMAT_BIN:
                    byte[] data = new byte[64];
                    int l = mTree.getBinValue(uri, data);
                    data [l] = '\0';
                    Log.i(TAG, " Get bin data length is " + l);
                    return new String(data);
            }
        } catch (MdmException e) {
            e.printStackTrace();
            return null;
        }
        return null;
    }

    public void registerOnAddHandler() {
        Log.i(TAG, "registerOnAddHandler to : " + URI_WIFI);
        try {
            mTree.registerSubtreeOnAddHandler(URI_WIFI, new SubscriptionOnAddHandler(mMdmWifi));
        } catch (MdmException e) {
            Log.e(TAG, "exception when registerOnAddHandler");
            e.printStackTrace();
        }
    }

    public void registerExecuteHandler() {
        try {
            mTree.registerExecute(URI_EXECUTE + "/launchBrowserToURI",
                    new LaunchBrowserExecuteHandler(mContext, mMdmWifi));
            mTree.registerExecute(URI_EXECUTE + "/negotiateClientCertTLS",
                    new NegotiateClientCertExecuteHandler(mContext, mMdmWifi));
            mTree.registerExecute(URI_EXECUTE + "/getCertificate",
                    new CertificateEnrollExecuteHandler(mContext, mMdmWifi));
        } catch (MdmException e) {
            Log.e(TAG, "exception when registerExecuteHandler");
            e.printStackTrace();
        }
    }

    public void registerNodeIOHandler() {
        Log.i(TAG, "registerNodeIOHandler");
        try {
            String[] sps = mTree.listChildren(URI_WIFI);
            for (String sp : sps) {
                registerNodeIOHandlerForServiceProvider(URI_WIFI + "/" + sp);
            }
        } catch (MdmException e) {
            e.printStackTrace();
        }
    }
    
    public String registerNodeIOHandler(String uri) {
        Log.i(TAG, "registerNodeIOHandler for : " + uri);
        String hotspot = null;
        if (uri.endsWith("PerProviderSubscription")) {
            try {
                String[] hss = mTree.listChildren(uri);
                hotspot = uri + "/" + hss[0];
                registerNodeIOHandlerForHotspot(hotspot);
            } catch (MdmException e) {
                e.printStackTrace();
            }
        } else if (uri.contains("PerProviderSubscription")) {
            hotspot = uri.substring(0, uri.lastIndexOf("/"));
            registerNodeIOHandlerForNode(uri);
        } else {
            try {
                String[] hss = mTree.listChildren(String.format(FORMAT_PPS, uri));
                hotspot = String.format(FORMAT_HOTSPOT, uri, hss[0]);
                registerNodeIOHandlerForHotspot(hotspot);
            } catch (MdmException e) {
                e.printStackTrace();
            }
        }
        Log.i(TAG, "the hotspot uri : " + hotspot);
        return hotspot;
    }
    
    private void registerNodeIOHandlerForServiceProvider(String sp) {
        Log.i(TAG, "registerNodeIOHandlerFor service provider : " + sp);
        try {
            String[] pps = mTree.listChildren(sp);
            if (pps.length == 0 || TextUtils.isEmpty(pps[0])) {
                Log.i(TAG, "no mo under " + sp + ", exit register");
                return;
            }
                
            String[] hss = mTree.listChildren(String.format(FORMAT_PPS, sp));
            for (String hs :hss) {
                registerNodeIOHandlerForHotspot(String.format(FORMAT_HOTSPOT, sp, hs));
            }
        } catch (MdmException e) {
            e.printStackTrace();
        }
    }

    public void registerNodeIOHandlerForHotspot(String hotspot) {
        Log.i(TAG, "registerNodeIOHandlerFor Hot Spot : " + hotspot);
        for (NodeIOHandlerInfo info : NodeIOHandlerInfo.ioHandlerInfos) {
            registerNodeIOHandlerViaInfo(hotspot, info);
        }
    }
    
    public void registerNodeIOHandlerForNode(String uri) {
        Log.i(TAG, "registerNodeIOHandlerForNode : " + uri);
        String hotspot = uri.substring(0, uri.lastIndexOf("/"));
        String nodeName = uri.substring(uri.lastIndexOf("/") + 1);
        Log.i(TAG, "registerNodeIOHandlerForNode : " + hotspot);
        Log.i(TAG, "registerNodeIOHandlerForNode : " + nodeName);
        for (NodeIOHandlerInfo info : NodeIOHandlerInfo.ioHandlerInfos) {
            if (info.localUri.contains(nodeName))
                registerNodeIOHandlerViaInfo(hotspot, info);
        }
    }

    private void registerNodeIOHandlerViaInfo(String hotspot, NodeIOHandlerInfo info) {
        List<String> uris = info.getRegisterUris(hotspot);
            for (String uri : uris) {
                Log.i(TAG, "register NodeIOHandler to " + uri);
                try {
                    Class<?> clazz = Class.forName(PACKAGE + info.handlerName);
                    Constructor<?> constructor = clazz.getConstructor(
                            Context.class, String.class, String.class,
                            Integer[].class);
                    NodeIoHandler handler = (NodeIoHandler) constructor
                            .newInstance(mContext, uri, info.nodeToKey(uri),
                                    info.preferSessionIds);
                    mTree.registerNodeIoHandler(uri, handler);
                } catch (ClassNotFoundException e) {
                    e.printStackTrace();
                } catch (NoSuchMethodException e) {
                    e.printStackTrace();
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (InstantiationException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                } catch (MdmException e) {
                    e.printStackTrace();
                }
            }
        }

    public String[] getChildren(String uri) {
        String[] uris = null;
        try {
            uris = mTree.listChildren(uri);
        } catch (MdmException e) {
            e.printStackTrace();
        }
        return uris;
    }
    
    public WifiTree getDMTreeToSOAPTree() {
        WifiTree tree = new WifiTree();
        //Firstly,create spFqdn obj from tree.xml
        try {
            String[] spFqdnNodeNames = getChildrenNodeName(URI_WIFI);
            for (String spFqdnNodeName : spFqdnNodeNames) {
                String currentSpfqdnNodeUri = String.format("%s/%s", URI_WIFI, spFqdnNodeName);
                //currentSpfqdnNodeUri = ./Wi-Fi/X
                WifiTree.SpFqdn currentSpfqdn = tree.createSpFqdn(spFqdnNodeName);
                
                String[] ppsNodeNames = getChildrenNodeName(currentSpfqdnNodeUri);
                for (String ppsNodeName : ppsNodeNames) {
                    String currentPpsNodeUri = String.format("%s/%s", currentSpfqdnNodeUri, ppsNodeName);
                    //currentPpsNodeUri = ./Wi-Fi/X/PerProviderSubscription
                    String[] credentialInfoNodeNames = getChildrenNodeName(currentPpsNodeUri);
                    for (String credentialInfoNodeName : credentialInfoNodeNames) {
                        String currentCredentialInfoNodeUri = String.format("%s/%s", currentPpsNodeUri, credentialInfoNodeName);
                        //currentCredentialInfoNodeUri = ./Wi-Fi/ X /PerProviderSubscription/UpdateIdentifier
                        //currentCredentialInfoNodeUri = ./Wi-Fi/ X /PerProviderSubscription/X
                        if (credentialInfoNodeName.equals("UpdateIdentifier")) {
                            currentSpfqdn.perProviderSubscription.UpdateIdentifier = mTree.getStringValue(currentCredentialInfoNodeUri);
                        } else {
                            //./Wi-Fi/ X /PerProviderSubscription/<X+>
                            WifiTree.CredentialInfo info = currentSpfqdn.perProviderSubscription.createCredentialInfo(credentialInfoNodeName);
                            String[] credentialChildrenNodeNames = getChildrenNodeName(currentCredentialInfoNodeUri);
                            for (String credentialInfoChildrenNodeName : credentialChildrenNodeNames) {
                                String credentialInfoChilNodeUri = String.format("%s/%s", currentCredentialInfoNodeUri, credentialInfoChildrenNodeName);
                                //credentialInfoChilNodeUri = ./Wi-Fi/ X /PerProviderSubscription/<X+>/node
                                if(credentialInfoChildrenNodeName.equals("Policy")) {
                                    setPolicyNode(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("CredentialPriority")) {
                                    setCredentialPriority(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("AAAServerTrustRoot")) {
                                    setAAAServerTrustRoot(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("SubscriptionUpdate")) {
                                    setSubscriptionUpdate(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("HomeSP")) {
                                    setHomeSP(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("SubscriptionParameters")) {
                                    setSubscriptionParameters(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("Credential")) {
                                    setCredential(info, credentialInfoChilNodeUri);
                                } else if(credentialInfoChildrenNodeName.equals("Extension")) {
                                    setExtension(info, credentialInfoChilNodeUri);
                                } else {
                                    Log.i(TAG, "Unknown credential info  hotspot children node name : " + credentialInfoChildrenNodeName);
                                }
                            }
                        }
                    }
                }
            }
        } catch (MdmException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return tree;
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/Policy
     */
    public void setPolicyNode(WifiTree.CredentialInfo info,String policyUri) throws MdmException{
        
        String[] policyChilNodeNames = getChildrenNodeName(policyUri);
        for (String policyChlNodeName : policyChilNodeNames) { 
            String policyChilNodeUri = String.format("%s/%s", policyUri, policyChlNodeName);
            //policyChilNodeUri = ./Wi-Fi/ X /PerProviderSubscription/X/Policy/X
            if(policyChlNodeName.equals("PreferredRoamingPartnerList")) {
                
                String[] PRPLChilNodeNames = getChildrenNodeName(policyChilNodeUri);
                for (String PRPLChilNodeName : PRPLChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", policyChilNodeUri, PRPLChilNodeName);
                    //./Wi-Fi/ X /PerProviderSubscription/X/Policy/PreferredRoamingPartnerList/X
                    String fqdn = null;
                    String priority = null;
                    String country = null;   
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equalsIgnoreCase("FQDN_Match")) {
                            fqdn = mTree.getStringValue(XChlUri);
                        } else if(XChlNodeName.equals("Priority")) {
                            priority = mTree.getStringValue(XChlUri);
                        } else if(XChlNodeName.equals("Country")) {
                            country = mTree.getStringValue(XChlUri);
                        } else {
                            Log.i(TAG, "Unknown MBT X children node name : " + XChlNodeName);
                        }
                    }
                    info.policy.createPreferredRoamingPartnerList(PRPLChilNodeName, fqdn, priority, country);
                }
            } else if(policyChlNodeName.equals("MinBackhaulThreshold")) {
                String[] MBTChilNodeNames = getChildrenNodeName(policyChilNodeUri);
                for (String MBTChilNodeName : MBTChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", policyChilNodeUri, MBTChilNodeName);
                    //./Wi-Fi/ X /PerProviderSubscription/X/Policy/MinBackhaulThreshold/X
                    String networkType = null;
                    String dLBandwidth = null;
                    String uLBandwidth = null;
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equals("NetworkType")) {
                            networkType = mTree.getStringValue(XChlUri);
                        } else if(XChlNodeName.equals("DLBandwidth")) {
                            dLBandwidth = mTree.getStringValue(XChlUri);
                        } else if(XChlNodeName.equals("ULBandwidth")) {
                            uLBandwidth = mTree.getStringValue(XChlUri);
                        } else {
                            Log.i(TAG, "Unknown MBT X children node name : " + XChlNodeName);
                        }
                    }
                    info.policy.createMinBackhaulThreshold(MBTChilNodeName, networkType, dLBandwidth, uLBandwidth);
                }
            } else if(policyChlNodeName.equals("PolicyUpdate")) {
                String upI = null;
                String upm = null;
                String res = null;
                String uri = null;
                String other = null;
                String[] PUChilNodeNames = getChildrenNodeName(policyChilNodeUri);
                for (String PUChilNodeName : PUChilNodeNames) {
                    String PUChiNodeUri = String.format("%s/%s", policyChilNodeUri, PUChilNodeName);
                    //./Wi-Fi/ X /PerProviderSubscription/X/Policy/PolicyUpdate/X
                    if (PUChilNodeName.equals("UpdateInterval")) {
                        upI = mTree.getStringValue(PUChiNodeUri);
                        info.policy.policyUpdate.UpdateInterval = upI;
                    } else if (PUChilNodeName.equals("UpdateMethod")) {
                        upm = mTree.getStringValue(PUChiNodeUri);
                        info.policy.policyUpdate.UpdateMethod = upm;
                    } else if (PUChilNodeName.equals("Restriction")) {
                        res = mTree.getStringValue(PUChiNodeUri);
                        info.policy.policyUpdate.Restriction = res;
                    } else if (PUChilNodeName.equals("URI")) {
                        uri = mTree.getStringValue(PUChiNodeUri);
                        info.policy.policyUpdate.URI = uri;
                    } else if (PUChilNodeName.equals("Other")) {
                        other = mTree.getStringValue(PUChiNodeUri);
                        info.policy.policyUpdate.Other = other;
                    } else if (PUChilNodeName.equals("UsernamePassword")) {
                        //String USPWNodeUri = String.format("%s/%s", PUChiNodeUri, PUChilNodeName);
                        String un = null;
                        String pw = null;
                        String[] USPWChilNodeNames = getChildrenNodeName(PUChiNodeUri);
                        for (String USPWChilNodeName : USPWChilNodeNames) {
                            String USPWChlNodeUri = String.format("%s/%s", PUChiNodeUri, USPWChilNodeName);
                            if (USPWChilNodeName.equals("Username")) {
                                un = mTree.getStringValue(USPWChlNodeUri);
                                info.policy.policyUpdate.usernamePassword.Username = un;
                            } else if (USPWChilNodeName.equals("Password")) {
                                String base64Password = null;
                                String format = null;
                                format = getProperty(USPWChlNodeUri,"Format");
                                Log.i(TAG, "password format = " + format);
                                if (format != null) {
                                    if (format.equalsIgnoreCase("bin")) {
                                        pw = mTree.getStringValue(USPWChlNodeUri);
                                        Log.i(TAG, "Password = " + pw);
                                    } 
                                } else {
                                    base64Password = mTree.getStringValue(USPWChlNodeUri);
                                    Log.i(TAG, "get string base64Password = " + base64Password);
                                    pw = decodeBase64(base64Password);
                                    Log.i(TAG, "Password = " + pw);
                                }
                                
                                info.policy.policyUpdate.usernamePassword.Password = pw;
                            } else {
                                Log.i(TAG, "Unknown policyupdate children  USPW node name : " + USPWChilNodeName);
                            }
                        }
                    } else if (PUChilNodeName.equals("TrustRoot")) {
                        String curl = null;
                        String csha = null;
                        String[] USPWChilNodeNames = getChildrenNodeName(PUChiNodeUri);
                        for (String TRChilNodeName : USPWChilNodeNames) {
                            String TRChlNodeUri = String.format("%s/%s", PUChiNodeUri, TRChilNodeName);
                            if (TRChilNodeName.equals("CertURL")) {
                                curl = mTree.getStringValue(TRChlNodeUri);
                                info.policy.policyUpdate.trustRoot.CertURL = curl;
                            } else if (TRChilNodeName.equals("CertSHA256Fingerprint")) {
                                csha = mTree.getStringValue(TRChlNodeUri);
                                info.policy.policyUpdate.trustRoot.CertSHA256Fingerprint = csha;
                            } else {
                                Log.i(TAG, "Unknown policyupdate/TrustRoot children node name : " + TRChilNodeName);
                            }
                        }
                    } else {
                        Log.i(TAG, "Unknown policyupdate children node name : " + PUChilNodeName);
                    }
                }
            } else if(policyChlNodeName.equals("SPExclusionList")) {
                String[] PRPLChilNodeNames = getChildrenNodeName(policyChilNodeUri);
                for (String SPELChilNodeName : PRPLChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", policyChilNodeUri, SPELChilNodeName);
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    String ssid = null;
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equals("SSID")) {
                            ssid = mTree.getStringValue(XChlUri);
                        } else {
                            Log.i(TAG, "Unknown SPExclusionList X children node name : " + XChlNodeName);
                        }
                    }
                    info.policy.createSPExclusionList(SPELChilNodeName, ssid);
                }
            } else if(policyChlNodeName.equals("RequiredProtoPortTuple")) {
                String[] RPPTChilNodeNames = getChildrenNodeName(policyChilNodeUri);
                for (String SPELChilNodeName : RPPTChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", policyChilNodeUri, SPELChilNodeName);
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    String proto = null;
                    String port = null;
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equals("IPProtocol")) {
                            proto = mTree.getStringValue(XChlUri);
                        } if (XChlNodeName.equals("PortNumber")) {
                            port = mTree.getStringValue(XChlUri);
                        } else {
                            Log.i(TAG, "Unknown RequiredProtoPortTuple X children node name : " + XChlNodeName);
                        }
                    }
                    info.policy.createRequiredProtoPortTuple(SPELChilNodeName, proto, port);
                }
            } else if(policyChlNodeName.equals("MaximumBSSLoadValue")) {
                info.policy.maximumBSSLoadValue = mTree.getStringValue(policyChilNodeUri);
            } else {
                Log.i(TAG, "Unknown Policy children node name : " + policyChlNodeName);
            }
        }
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/CredentialPriority
     */
    public void setCredentialPriority(WifiTree.CredentialInfo info, String credentialPriorityUri) throws MdmException {
        String credentialPriority = mTree.getStringValue(credentialPriorityUri);
        info.credentialPriority = credentialPriority;
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/AAAServerTrustRoot
     */
    public void setAAAServerTrustRoot(WifiTree.CredentialInfo info, String AAAServerTrustRootUri) throws MdmException {
        String[] AAASTRChilNodeNames = getChildrenNodeName(AAAServerTrustRootUri);
        for (String AAASTRChilNodeName : AAASTRChilNodeNames) {
            String XNodeUri = String.format("%s/%s", AAAServerTrustRootUri, AAASTRChilNodeName);
            String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
            String curl = null;
            String csha = null;  
            for (String XChlNodeName : XChilNodeNames) {
                String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                if(XChlNodeName.equals("CertURL")) {
                    curl = mTree.getStringValue(XChlUri);
                } else if(XChlNodeName.equals("CertSHA256Fingerprint")) {
                    csha = mTree.getStringValue(XChlUri);
                } else {
                    Log.i(TAG, "Unknown MBT X children node name : " + XChlNodeName);
                }
            }
            info.createAAAServerTrustRoot(AAASTRChilNodeName, curl, csha);
        }
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/SubscriptionUpdate
     */
    public void setSubscriptionUpdate(WifiTree.CredentialInfo info, String subscriptionUpdateUri) throws MdmException {
        
        String[] SUChilNodeNames = getChildrenNodeName(subscriptionUpdateUri);
        String upI = null;
        String upm = null;
        String res = null;
        String uri = null;
        String other = null;
        for (String SUChilNodeName : SUChilNodeNames) {
            String SubUpChiNodeUri = String.format("%s/%s", subscriptionUpdateUri, SUChilNodeName);
            
            if (SUChilNodeName.equals("UpdateInterval")) {
                upI = mTree.getStringValue(SubUpChiNodeUri);
                info.subscriptionUpdate.UpdateInterval = upI;
            } else if (SUChilNodeName.equals("UpdateMethod")) {
                upm = mTree.getStringValue(SubUpChiNodeUri);
                info.subscriptionUpdate.UpdateMethod = upm;
            } else if (SUChilNodeName.equals("Restriction")) {
                res = mTree.getStringValue(SubUpChiNodeUri);
                info.subscriptionUpdate.Restriction = res;
            } else if (SUChilNodeName.equals("URI")) {
                uri = mTree.getStringValue(SubUpChiNodeUri);
                info.subscriptionUpdate.URI = uri;
            } else if (SUChilNodeName.equals("Other")) {
                other = mTree.getStringValue(SubUpChiNodeUri);
                info.subscriptionUpdate.Other = other;
            } else if (SUChilNodeName.equals("UsernamePassword")) {
                //String USPWNodeUri = String.format("%s/%s", subscriptionUpdateUri, SUChilNodeName);
                String[] USPWChilNodeNames = getChildrenNodeName(SubUpChiNodeUri);
                String un = null;
                String pw = null;
                for (String USPWChilNodeName : USPWChilNodeNames) {
                    String USPWChlNodeUri = String.format("%s/%s", SubUpChiNodeUri, USPWChilNodeName);
                    if (USPWChilNodeName.equals("Username")) {
                        un = mTree.getStringValue(USPWChlNodeUri);
                        info.subscriptionUpdate.usernamePassword.Username = un;
                    } else if (USPWChilNodeName.equals("Password")) {
                        String base64Password = null;
                        String format = null;
                        format = getProperty(USPWChlNodeUri,"Format");
                        Log.i(TAG, "password format = " + format);
                        if (format != null) {
                            if (format.equalsIgnoreCase("bin")) {
                                pw = mTree.getStringValue(USPWChlNodeUri);
                                Log.i(TAG, "Password = " + pw);
                            } 
                        } else {
                            base64Password = mTree.getStringValue(USPWChlNodeUri);
                            Log.i(TAG, "get string base64Password = " + base64Password);
                            pw = decodeBase64(base64Password);
                            Log.i(TAG, "Password = " + pw);
                        }
                        
                        info.subscriptionUpdate.usernamePassword.Password = pw;
                    } else {
                        Log.i(TAG, "Unknown subscriptionUpdate children  USPW node name : " + USPWChilNodeName);
                    }
                }
            } else if (SUChilNodeName.equals("TrustRoot")) {
                //String TRNodeUri = String.format("%s/%s", subscriptionUpdateUri, SUChilNodeName);
                String[] USPWChilNodeNames = getChildrenNodeName(SubUpChiNodeUri);
                String curl = null;
                String csha = null;
                for (String TRChilNodeName : USPWChilNodeNames) {
                    String TRChlNodeUri = String.format("%s/%s", SubUpChiNodeUri, TRChilNodeName);
                    if (TRChilNodeName.equals("CertURL")) {
                        curl = mTree.getStringValue(TRChlNodeUri);
                        info.subscriptionUpdate.trustRoot.CertURL = curl;
                    } else if (TRChilNodeName.equals("CertSHA256Fingerprint")) {
                        csha = mTree.getStringValue(TRChlNodeUri);
                        info.subscriptionUpdate.trustRoot.CertSHA256Fingerprint = csha;
                    } else {
                        Log.i(TAG, "Unknown subscriptionUpdate children  TrustRoot node name : " + TRChilNodeName);
                    }
                }
            } else {
                Log.i(TAG, "Unknown subscriptionUpdate children node name : " + SUChilNodeName);
            }
        }
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/HomeSP
     */
    public void setHomeSP(WifiTree.CredentialInfo info, String homeSPUri) throws MdmException {
        String[] hSPChilNodeNames = getChildrenNodeName(homeSPUri);
        for (String hSPChlNodeName : hSPChilNodeNames) { 
            String hSPChlNodeUri = String.format("%s/%s", homeSPUri, hSPChlNodeName);
            if(hSPChlNodeName.equals("NetworkID")) {
                String[] nwIDChilNodeNames = getChildrenNodeName(hSPChlNodeUri);
                for (String nwIDChilNodeName : nwIDChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", hSPChlNodeUri, nwIDChilNodeName);
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    String ssid = null;
                    String hessid = null;
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equals("SSID")) {
                            ssid = mTree.getStringValue(XChlUri);
                        } else if(XChlNodeName.equals("HESSID")) {
                            hessid = mTree.getStringValue(XChlUri);
                        } else {
                            Log.i(TAG, "Unknown HomeSP NetworkID X children node name : " + XChlNodeName);
                        }
                    }
                    info.homeSP.createNetworkID(nwIDChilNodeName, ssid, hessid);
                }
            } else if(hSPChlNodeName.equals("FriendlyName")) {
                String friendlyName = mTree.getStringValue(hSPChlNodeUri);
                info.homeSP.FriendlyName = friendlyName;
            } else if(hSPChlNodeName.equals("IconURL")) {
                String iconURL = mTree.getStringValue(hSPChlNodeUri);
                info.homeSP.IconURL = iconURL;
            } else if(hSPChlNodeName.equals("FQDN")) {
                String fqdn = mTree.getStringValue(hSPChlNodeUri);
                info.homeSP.FQDN = fqdn;
            } else if(hSPChlNodeName.equals("HomeOIList")) {
                String[] homeOILChilNodeNames = getChildrenNodeName(hSPChlNodeUri);
                for (String homeOILChilNodeName : homeOILChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", hSPChlNodeUri, homeOILChilNodeName);
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    String homeOI = null;
                    boolean homeOIRe = false;
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equals("HomeOI")) {
                            homeOI = mTree.getStringValue(XChlUri);
                        } else if(XChlNodeName.equals("HomeOIRequired")) {
                            String homeOIReqStr = mTree.getStringValue(XChlUri);
                            if (homeOIReqStr != null) {
                                if(homeOIReqStr.equalsIgnoreCase("true")) {
                                    homeOIRe = true;
                                } else if(homeOIReqStr.equalsIgnoreCase("false")) {
                                    homeOIRe = false;
                                } else {
                                    Log.i(TAG, "error HomeSP/HomeOIList/HomeOIRequired value is : " + homeOIReqStr);
                                }
                            } else {
                                Log.i(TAG, "HomeOIRequired node value is null ");
                            }
                        } else {
                            Log.i(TAG, "Unknown HomeSP/HomeOIList/X children node name : " + XChlNodeName);
                        }
                    }
                    info.homeSP.createHomeOIList(homeOILChilNodeName, homeOI, homeOIRe);
                }
            } else if(hSPChlNodeName.equals("OtherHomePartners")) {
                String[] otherHPChilNodeNames = getChildrenNodeName(hSPChlNodeUri);
                for (String otherHPChilNodeName : otherHPChilNodeNames) {
                    String XNodeUri = String.format("%s/%s", hSPChlNodeUri, otherHPChilNodeName);
                    String[] XChilNodeNames = getChildrenNodeName(XNodeUri);
                    String fqdn = null;
                    for (String XChlNodeName : XChilNodeNames) {
                        String XChlUri = String.format("%s/%s", XNodeUri, XChlNodeName);
                        if(XChlNodeName.equals("FQDN")) {
                            fqdn = mTree.getStringValue(XChlUri);
                        } else {
                            Log.i(TAG, "Unknown OtherHomePartners X children node name : " + XChlNodeName);
                        }
                    }
                    info.homeSP.createOtherHomePartners(otherHPChilNodeName, fqdn);
                }
            } else if(hSPChlNodeName.equals("RoamingConsortiumOI")) {
                String RCOIUri = String.format("%s/%s", hSPChlNodeUri, hSPChlNodeName);
                info.homeSP.RoamingConsortiumOI = mTree.getStringValue(RCOIUri);
            } else {
                Log.i(TAG, "Unknown OtherHomePartners children node name : " + hSPChlNodeName);
            }
        }
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/SubscriptionParameters
     */
    public void setSubscriptionParameters(WifiTree.CredentialInfo info, String subscriptionParametersUri) throws MdmException {

        String credate = null;
        String expdate = null;
        String typofsub = null;
        String[] subParChilNodeNames = getChildrenNodeName(subscriptionParametersUri);
        for (String subParChilNodeName : subParChilNodeNames) {
            String subParChiNodeUri = String.format("%s/%s", subscriptionParametersUri, subParChilNodeName);
            
            if (subParChilNodeName.equals("CreationDate")) {
                credate = mTree.getStringValue(subParChiNodeUri);
                info.subscriptionParameters.CreationDate = credate;
            } else if (subParChilNodeName.equals("ExpirationDate")) {
                expdate = mTree.getStringValue(subParChiNodeUri);
                info.subscriptionParameters.ExpirationDate = expdate;
            } else if (subParChilNodeName.equals("TypeOfSubscription")) {
                typofsub = mTree.getStringValue(subParChiNodeUri);
                info.subscriptionParameters.TypeOfSubscription = typofsub;
            } else if (subParChilNodeName.equals("UsageLimits")) {
                String[] UsaLimChilNodeNames = getChildrenNodeName(subParChiNodeUri);
                String datalim = null;
                String startdate = null;
                String timelim = null;
                String usagetim = null;
                for (String UsaLimChilNodeName : UsaLimChilNodeNames) {
                    String UsaLimChlNodeUri = String.format("%s/%s", subParChiNodeUri, UsaLimChilNodeName);
                    if (UsaLimChilNodeName.equals("DataLimit")) {
                        datalim = mTree.getStringValue(UsaLimChlNodeUri);
                        info.subscriptionParameters.usageLimits.DataLimit = datalim;
                    } else if (UsaLimChilNodeName.equals("StartDate")) {
                        startdate = mTree.getStringValue(UsaLimChlNodeUri);
                        info.subscriptionParameters.usageLimits.StartDate = startdate;
                    } else if (UsaLimChilNodeName.equals("TimeLimit")) {
                        timelim = mTree.getStringValue(UsaLimChlNodeUri);
                        info.subscriptionParameters.usageLimits.TimeLimit = timelim;
                    } else if (UsaLimChilNodeName.equals("UsageTimePeriod")) {
                        usagetim = mTree.getStringValue(UsaLimChlNodeUri);
                        info.subscriptionParameters.usageLimits.UsageTimePeriod = usagetim;
                    } else {
                        Log.i(TAG, "Unknown SubscriptionParameters/UsageLimits children  node name : " + UsaLimChilNodeName);
                    }
                }
            } else {
                Log.i(TAG, "Unknown SubscriptionParameters children node name : " + subParChilNodeName);
            }
        }
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/Credential
     */
    public void setCredential(WifiTree.CredentialInfo info, String credentialUri) throws MdmException {  
        String credate = null;
        String expdate = null;
        String realm = null;
        boolean CheckAAAServerCertStatus = false;
        String[] credChilNodeNames = getChildrenNodeName(credentialUri);
        for (String credChilNodeName : credChilNodeNames) {
            String credParChiNodeUri = String.format("%s/%s", credentialUri, credChilNodeName);
            
            if (credChilNodeName.equals("CreationDate")) {
                credate = mTree.getStringValue(credParChiNodeUri);
                info.credential.CreationDate = credate;
            } else if (credChilNodeName.equals("ExpirationDate")) {
                expdate = mTree.getStringValue(credParChiNodeUri);
                info.credential.ExpirationDate = expdate;
            } else if (credChilNodeName.equals("Realm")) {
                realm = mTree.getStringValue(credParChiNodeUri);
                info.credential.Realm = realm;
            } else if (credChilNodeName.equals("CheckAAAServerCertStatus")) {
                String checkAAAServerCertStatus = mTree.getStringValue(credParChiNodeUri);
                if (checkAAAServerCertStatus != null) {
                    if(checkAAAServerCertStatus.equalsIgnoreCase("true")) {
                        CheckAAAServerCertStatus = true;
                    } else if(checkAAAServerCertStatus.equalsIgnoreCase("false")) {
                        CheckAAAServerCertStatus = false;
                    } else {
                        Log.i(TAG, "error Credential/CheckAAAServerCertStatus value is : " + checkAAAServerCertStatus);
                    }
                    info.credential.CheckAAAServerCertStatus = CheckAAAServerCertStatus;
                } else {
                    Log.i(TAG, "MachineManaged node value is null ");
                }
            } else if (credChilNodeName.equals("UsernamePassword")) {
                String[] USPWChilNodeNames = getChildrenNodeName(credParChiNodeUri);
                String username = null;
                String password = null;
                boolean machmanag = false;
                String softtoken = null;
                String abletoshare = null;

                for (String USPWChilNodeName : USPWChilNodeNames) {
                    String USPWChlNodeUri = String.format("%s/%s", credParChiNodeUri, USPWChilNodeName);
                    if (USPWChilNodeName.equals("Username")) {
                        username = mTree.getStringValue(USPWChlNodeUri);
                        info.credential.usernamePassword.Username = username;
                    } else if (USPWChilNodeName.equals("Password")) {
                        String base64Password = null;
                        String format = null;
                        format = getProperty(USPWChlNodeUri,"Format");
                        Log.i(TAG, "password format = " + format);
                        if (format != null) {
                            if (format.equalsIgnoreCase("bin")) {
                                password = mTree.getStringValue(USPWChlNodeUri);
                                Log.i(TAG, "Password = " + password);
                            } 
                        } else {
                            base64Password = mTree.getStringValue(USPWChlNodeUri);
                            Log.i(TAG, "get string base64Password = " + base64Password);
                            password = decodeBase64(base64Password);
                            Log.i(TAG, "Password = " + password);
                        }
                        
                        info.credential.usernamePassword.Password = password;
                    } else if (USPWChilNodeName.equals("MachineManaged")) {
                        String machmanagStr = mTree.getStringValue(USPWChlNodeUri);
                        if (machmanagStr != null) {
                            if(machmanagStr.equalsIgnoreCase("true")) {
                                machmanag = true;
                            } else if(machmanagStr.equalsIgnoreCase("false")) {
                                machmanag = false;
                            } else {
                                Log.i(TAG, "error Credential/UsernamePassword/MachineManaged value is : " + machmanagStr);
                            }
                            info.credential.usernamePassword.MachineManaged = machmanag;
                        } else {
                            Log.i(TAG, "MachineManaged node value is null ");
                        }
                    } else if (USPWChilNodeName.equals("SoftTokenApp")) {
                        softtoken = mTree.getStringValue(USPWChlNodeUri);
                        info.credential.usernamePassword.SoftTokenApp = softtoken;
                    } else if (USPWChilNodeName.equals("AbleToShare")) {
                        abletoshare = mTree.getStringValue(USPWChlNodeUri);
                        info.credential.usernamePassword.AbleToShare = abletoshare;
                    } else if (USPWChilNodeName.equals("EAPMethod")) {
                        
                        String eaptype = null;
                        String vendorld = null;
                        String vendortype = null;
                        String innereaptype = null;
                        String innervid = null;
                        String innervtype = null;
                        String innermet = null;
                        String[] EAPMethodChilNodeNames = getChildrenNodeName(USPWChlNodeUri);
                        for (String EAPMethodChilNodeName : EAPMethodChilNodeNames) {
                            String EAPMethodChlNodeUri = String.format("%s/%s", USPWChlNodeUri, EAPMethodChilNodeName);
                            if (EAPMethodChilNodeName.equals("EAPType")) {
                                eaptype = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.EAPType = eaptype;
                            } else if (EAPMethodChilNodeName.equals("VendorID")) {
                                vendorld = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.VendorId = vendorld;
                            } else if (EAPMethodChilNodeName.equals("VendorType")) {
                                vendortype = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.VendorType = vendortype;
                            } else if (EAPMethodChilNodeName.equals("InnerEAPType")) {
                                innereaptype = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.InnerEAPType = innereaptype;
                            } else if (EAPMethodChilNodeName.equals("InnerVendorID")) {
                                innervid = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.InnerVendorId = innervid;
                            } else if (EAPMethodChilNodeName.equals("InnerVendorType")) {
                                innervtype = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.InnerVendorType = innervtype;
                            } else if (EAPMethodChilNodeName.equals("InnerMethod")) {
                                innermet = mTree.getStringValue(EAPMethodChlNodeUri);
                                info.credential.usernamePassword.eAPMethod.InnerMethod = innermet;
                            } else {
                                Log.i(TAG, "unknown Credential/UsernamePassword/EAPMethod child node: " + EAPMethodChilNodeName);
                            }
                        }
                    } else {
                        Log.i(TAG, "Unknown UsernamePassword children  node name : " + USPWChilNodeName);
                    }
                }
            } else if (credChilNodeName.equals("DigitalCertificate")) {

                String[] digitCerChilNodeNames = getChildrenNodeName(credParChiNodeUri);
                String certtype = null;
                String cerfprint = null;
                for (String digitCerChilNodeName : digitCerChilNodeNames) {
                    String digitCerChlNodeUri = String.format("%s/%s", credParChiNodeUri, digitCerChilNodeName);
                    if (digitCerChilNodeName.equals("CertificateType")) {
                        certtype = mTree.getStringValue(digitCerChlNodeUri);
                        info.credential.digitalCertificate.CertificateType = certtype;
                    } else if (digitCerChilNodeName.equals("CertSHA256Fingerprint")) {
                        cerfprint = mTree.getStringValue(digitCerChlNodeUri);
                        info.credential.digitalCertificate.CertSHA256Fingerprint = cerfprint;
                    } else {
                        Log.i(TAG, "Unknown DigitalCertificate children  node name : " + digitCerChilNodeName);
                    }
                }
            } else if (credChilNodeName.equals("SIM")) {
                String[] SIMChilNodeNames = getChildrenNodeName(credParChiNodeUri);
                String imsi = null;
                String eaptype = null;
                for (String SIMChilNodeName : SIMChilNodeNames) {
                    String SIMChlNodeUri = String.format("%s/%s", credParChiNodeUri, SIMChilNodeName);
                    if (SIMChilNodeName.equals("IMSI")) {
                        imsi = mTree.getStringValue(SIMChlNodeUri);
                        info.credential.sim.IMSI = imsi;
                    } else if (SIMChilNodeName.equals("EAPType")) {
                        eaptype = mTree.getStringValue(SIMChlNodeUri);
                        info.credential.sim.EAPType = eaptype;
                    } else {
                        Log.i(TAG, "Unknown SIM children  node name : " + SIMChilNodeName);
                    }
                }
            } else {
                Log.i(TAG, "Unknown Credential children node name : " + credChilNodeName);
            }
        }
    }
    /*
     * parse dm tree wifi node:./Wi-Fi/ X /PerProviderSubscription/X/Extension
     */
    public void setExtension(WifiTree.CredentialInfo info, String extensionUri) throws MdmException {
        String extension = mTree.getStringValue(extensionUri);
        info.extension.empty = extension;
    }

    public void parseWifiMO (String nodeUri) throws MdmException {
        String[] childrenNodeNames = null;
        boolean isLeaf ;
        //mTree.w();
        //isLeaf = isLeaf("./Wi-Fi/wi-fi.org/PerproviderSubscription/Cred01");
        //Log.i(TAG, "default node uri is Cred01:");
        //Log.i(TAG, "isLeaf :" + isLeaf);
        
        isLeaf = isLeaf(nodeUri);
        Log.i(TAG, "node uri is :" + nodeUri);
        Log.i(TAG, "isLeaf :" + isLeaf);
        if (isLeaf) {
            String value = null;
            value = getValue(nodeUri,FORMAT_CHR);
            Log.i(TAG, "<leaf> : " + nodeUri + " : " + value);
            return;
        } else {
            Log.i(TAG, "<node> : " + nodeUri);
            childrenNodeNames = getChildrenNodeName(nodeUri);
            if (0 == childrenNodeNames.length) {
                return;
            } else {
                for (String nodeName : childrenNodeNames) {
                    parseWifiMO(String.format("%s/%s", nodeUri, nodeName));
                }
            }
        }
    } 
    
    /**
     * get list of children node name 
     **/
    public String[] getChildrenNodeName(String parentUri) {
        String[] childrenNodeNames = null;
        Log.i(TAG, "ParentUri = " + parentUri);
        try {
            childrenNodeNames = mTree.listChildren(parentUri);
            for (String nodeName : childrenNodeNames) {
                Log.i(TAG, "nodeName = " + nodeName);
            }
        } catch (MdmException e) {
            e.printStackTrace();
        }
        return childrenNodeNames;
    }
    /**
     *  Judge the node which is leaf node or not
     *  true :leaf node
     **/
    public boolean isLeaf(String nodeUri) throws MdmException {
        return mTree.isLeaf(nodeUri);
    }
    
    /**
     * For SOAP API tree management:Add command 
     **/
    public boolean addSubTree(String uri, byte[] data) {
        boolean result =false;
        dumpSubtree(data);
        try {
            result = mTree.addSubTree(uri, data);
            if (result) {
                mTree.writeToPersistentStorage();
            } else {
                Log.i(TAG, "addSubTree fail: sub tree format error or exist in tree ");
            }
            
        } catch (MdmException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return result;
    }
    
    /**
     * For SOAP API tree management:Add command 
     **/
    public boolean replaceSubTree(String uri, byte[] data) {
        boolean result = false;
        dumpSubtree(data);
        try {
            result = mTree.replaceSubTree(uri, data);
            if (result) {
                mTree.writeToPersistentStorage();
            } else {
                Log.i(TAG, "Replace SubTree fail: sub tree format error or exist in tree ");
            }
            
        } catch (MdmException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return result;
    }
    
    public boolean deleteNode(String nodeUri) {
        boolean result = false;
        if (nodeUri == null)
            return result;
        try {
            result = mTree.deleteNode(nodeUri);
            if (result) {
                mTree.writeToPersistentStorage();
            } else {
                Log.i(TAG, "delete node fail: node format error or exist in tree ");
            }
            
        } catch (MdmException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return result;
    }
    
    public String getHotspotUri(String uri) {
        Log.i(TAG, "getHotspotUri from : " + uri);
        if (uri == null) {
            try {
                String[] sps = mTree.listChildren(URI_WIFI);
                if (TextUtils.isEmpty(sps[0])) {
                    Log.i(TAG, URI_WIFI + "has no Children node, sps[0] is : " + sps[0]);
                    return URI_WIFI;
                }
                uri = URI_WIFI + "/" + sps[0];
                Log.i(TAG, "URI null after is : " + uri);
                sps = mTree.listChildren(uri);
                //if (TextUtils.isEmpty(sps[0])) {
                if (sps.length == 0) {
                    Log.i(TAG, "service provider is not exist,uri is : " + uri);
                    return uri;
                }
                
            } catch (MdmException e) {
                e.printStackTrace();
            }
        } else if (uri.contains("PerProviderSubscription")) {
            uri = uri.substring(0, uri.indexOf("/PerProviderSubscription"));
            Log.i(TAG, "service provider : " + uri);
        }
        try {
            
            String[] hss = mTree.listChildren(uri + "/PerProviderSubscription");
            if (hss.length == 0) {
                Log.i(TAG, "Tree node just is : " + uri);
                return uri;
            }
            Log.i(TAG, "service provider's hotspot node :>>" + uri);
            for(String hs :hss) {
                Log.i(TAG, "hotspot node is: " + hs);
                if (hs.equals(WIFI_TREE_UPDATEIDENTIFIER_NODE_NAME)) {
                    continue ;
                } else {
                    uri = uri + "/PerProviderSubscription/" + hs;
                }
            }
            Log.i(TAG, "service provider's hotspot node :<<" + uri);
        } catch (MdmException e) {
            e.printStackTrace();
        }
        Log.i(TAG, "hot spot : " + uri);
        return uri;
    }
    
    private String decodeBase64(String s) {
        try {
            byte[] decoded = Base64.decode(s,Base64.DEFAULT);
            return new String(decoded);
        } catch (Exception e) {
            Log.d(TAG, "decode err:" + e);
        }
        return null;
    }
    
    public String getProperty(String nodeUri,String proName) {
        String property = null;
        try {
            property = mTree.getProperty(nodeUri,proName);
        } catch (MdmException e) {
            // TODO: handle exception
            property = null;
            Log.i(TAG, "getProperty error:" + e.getMessage());
        }
        return property;
    }
    
    public void dumpSubtree( byte []subtree) {
        final byte tab =      0x20;
        final byte newline =  0x0A;
        final int MAX_CLOUMN = 100 ; 
        int length = subtree.length;
        int lineNum = length / MAX_CLOUMN ;
        int lastDataNum = length % MAX_CLOUMN; 
        Log.i(TAG, "subtree data length is : " + length);
        Log.i(TAG, "subtree data dump++++++++++++ ");
        for(int i = 0; i < lineNum ; i ++) {
            Log.i(TAG, new String(subtree,i*MAX_CLOUMN,MAX_CLOUMN));
        }
        Log.i(TAG, new String(subtree,lineNum*MAX_CLOUMN,lastDataNum));
        Log.i(TAG, "subtree data dump------------ ");
    }
    private Context mContext;
    private MdmTree mTree;

    public static final String PACKAGE = "com.mediatek.mediatekdm.mdm.wifi.io";
    public static final String SP_FQDN = "wi-fi.org";
    public static final String URI_WIFI = "./Wi-Fi";
    public static final String URI_SERVICE_PROVIDER = "./Wi-Fi/wi-fi.org/PerProviderSubscription";
    public static final String WIFI_TREE_UPDATEIDENTIFIER_NODE_NAME = "UpdateIdentifier";
    public static final String FORMAT_PPS = "%s/PerProviderSubscription";
    public static final String FORMAT_HOTSPOT = "%s/PerProviderSubscription/%s";
    public static final String URI_EXECUTE = "./DevDetail/Ext/org.wi-fi/Wi-Fi/Ops";
    
    public static final int FORMAT_INT = 0x0001;
    public static final int FORMAT_CHR = 0x0002;
    public static final int FORMAT_BOL = 0x0004;
    public static final int FORMAT_BIN = 0x0008;
}
