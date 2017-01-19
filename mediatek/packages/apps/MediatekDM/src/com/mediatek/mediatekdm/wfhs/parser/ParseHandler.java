package com.mediatek.mediatekdm.wfhs.parser;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import android.text.TextUtils;
import android.util.Log;
import static com.mediatek.mediatekdm.wfhs.parser.ElementState.*;
import static com.mediatek.mediatekdm.wfhs.parser.ParseState.*;

public class ParseHandler extends DefaultHandler {

    private static final String TAG = "ParseHandler";

    public void startDocument() throws SAXException {
        Log.d(TAG, "startDocument");
        mElementState = ElementState.Start;
        mPerProviderSubscriptionMo = new PerProviderSubscriptionMo();
    }
    
    public void endDocument() throws SAXException {
        Log.d(TAG, "endDocument");
        mElementState = ElementState.End;
    }
    
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
        Log.d(TAG, "<startElement> uri : " + uri);
        Log.d(TAG, "<startElement> localName : " + localName);
        Log.d(TAG, "<startElement> qName : " + qName);
        Log.d(TAG, "<startElement> attributes : " + attributes);
        
        mParseState = parse_start;
        mElementName = localName;
        mElementState = parseState(localName);
        Log.i(TAG, "the parse state is " + mElementState);     
        handleParseStart();
    }
    
    public void endElement(String uri, String localName, String qName) throws SAXException {
        Log.d(TAG, "<endElement> uri : " + uri);
        Log.d(TAG, "<endElement> localName : " + localName);
        Log.d(TAG, "<endElement> qName : " + qName);
        
        mParseState = parse_end;
        mElementState = parseState(localName);
        handleParseEnd();
        mElementState = Idle;
    }

    public void characters(char[] ch, int start, int length) throws SAXException {
        Log.d(TAG, "characters");
        mParseState = parse_content;
        mElementValue = new String(ch, start, length);
        Log.i(TAG, "the element <" + mElementName + "> value is " + mElementValue);  
        handleParseContent();
    }
    
    private void handleParseStart() {
        Log.d(TAG, "handleParseStart");
        switch (mElementState) {
            case Policy:
                mBuildMap.put("Policy", new Policy());
                break;
            case PreferredRoamingPartnerList:
                mBuildMap.put("PreferredRoamingPartnerList", new ArrayList<PreferredRoamingPartner>());               
                break;
            case FQDN_Match:
                if (mCache.get("PreferredRoamingPartner") != null) {
                    //this is the previous PreferredRoamingPartner
                    ((List<PreferredRoamingPartner>) mBuildMap.get("PreferredRoamingPartnerList")).add((PreferredRoamingPartner) mCache.get("PreferredRoamingPartner"));
                    mCache.remove("PreferredRoamingPartner");
                }
                mCache.put("PreferredRoamingPartner", new PreferredRoamingPartner());
                break;
            case MinBackhaulThreshold:
                mBuildMap.put("MinBackhaulThresholds", new ArrayList<MinBackhaulThreshold>());
                break;
            case NetworkType:
                if (mCache.get("MinBackhaulThreshold") != null) {
                    //this is the previous MinBackhaulThreshold
                    ((List<MinBackhaulThreshold>) mBuildMap.get("MinBackhaulThresholds")).add((MinBackhaulThreshold) mCache.get("MinBackhaulThreshold"));
                    mCache.remove("MinBackhaulThreshold");
                }
                mCache.put("MinBackhaulThreshold", new MinBackhaulThreshold());
                break;
            case PolicyUpdate:
                mBuildMap.put("PolicyUpdate", new PolicyUpdate());
                break;
            case PolicyUpdate_UNPW:
                mBuildMap.put("PolicyUpdate_UNPW", new UNPW());
                break;
            case PolicyUpdate_TrustRoot:
                break;
            case SPExclusionList:
                mBuildMap.put("SPExclusionList", new SPExclusion());
                break;
            case RequiredProtoPortTuple:
                mBuildMap.put("RequiredProtoPortTuples", new ArrayList<RequiredProtoPortTuple>());
                break;
            case IPProtocol:
                if (mCache.get("RequiredProtoPortTuple") != null) {
                    //this is the previous RequiredProtoPortTuple
                    ((List<RequiredProtoPortTuple>) mBuildMap.get("RequiredProtoPortTuples")).add((RequiredProtoPortTuple) mCache.get("RequiredProtoPortTuple"));
                    mCache.remove("RequiredProtoPortTuple");
                }
                mCache.put("RequiredProtoPortTuple", new RequiredProtoPortTuple());
                break;
            case SubscriptionRemediation:
                mBuildMap.put("SubscriptionRemediation", new SubscriptionRemediation());
                break;
            case SubscriptionRemediation_TrustRoot:
                mBuildMap.put("SubscriptionRemediation_TrustRoot", new TrustRoot());
                break;
            case AAAServerTrustRoot:
                mBuildMap.put("AAAServerTrustRoots", new ArrayList<AAAServerTrustRoot>());
                break;
            case AAAServerTrustRoot_CertURL:
                if (mCache.get("AAAServerTrustRoot") != null) {
                    //this is the previous AAAServerTrustRoot
                    ((List<AAAServerTrustRoot>) mBuildMap.get("AAAServerTrustRoots")).add((AAAServerTrustRoot) mCache.get("AAAServerTrustRoot"));
                    mCache.remove("AAAServerTrustRoot");
                }
                mCache.put("AAAServerTrustRoot", new AAAServerTrustRoot());
                break;
            case SubscriptionUpdate:
                mBuildMap.put("SubscriptionUpdate", new SubscriptionUpdate());
                break;
            case SubscriptionUpdate_UNPW:
                mBuildMap.put("SubscriptionUpdate_UNPW", new UNPW());
                break;
            case HomeSP:
                mBuildMap.put("HomeSP", new HomeSP());
                break;
            case NetworkID:
                mBuildMap.put("NetworkIDs", new ArrayList<NetworkID>());
                break;
            case NetworkID_SSID:
                if (mCache.get("NetworkID") != null) {
                    //this is the previous NetworkID
                    ((List<NetworkID>) mBuildMap.get("NetworkIDs")).add((NetworkID) mCache.get("NetworkID"));
                    mCache.remove("NetworkID");
                }
                mCache.put("NetworkID", new NetworkID());
                break;
            case HomeOIList:
                mBuildMap.put("HomeOIList", new ArrayList<HomeOI>());
                break;
            case HomeOI:
                if (mCache.get("HomeOI") != null) {
                    //this is the previous HomeOI
                    ((List<HomeOI>) mBuildMap.get("HomeOIList")).add((HomeOI) mCache.get("HomeOI"));
                    mCache.remove("HomeOI");
                }
                mCache.put("HomeOI", new HomeOI());
                break;
            case SubscriptionParameters:
                mBuildMap.put("SubscriptionParameters", new SubscriptionParameters());
                break;
            case UsageLimits:
                mBuildMap.put("UsageLimits", new UsageLimits());
                break;
            case Credential:
                mBuildMap.put("Credential", new Credential());
                break;
            case Credential_UsernamePassword:
                mBuildMap.put("UsernamePassword", new UsernamePassword());
                break;
            case EAPMethod:
                mBuildMap.put("EAPMethod", new EAPMethod());
                break;
            case DigitalCertificate:
                mBuildMap.put("DigitalCertificate", new DigitalCertificate());
                break;
            case SIM:
                mBuildMap.put("SIM", new SIM());
                break;           
            default:
                break;
        }
    }
    
    private void handleParseContent() {
        Log.d(TAG, "handleParseContent");
        /*
        switch (mElementState) {
            case FQDN_Match:
                ((PreferredRoamingPartner) mCache.get("PreferredRoamingPartner")).setFQDN_Match(mElementValue);
                break;
            case Priority:
                ((PreferredRoamingPartner) mCache.get("PreferredRoamingPartner")).setPriority(Integer.parseInt(mElementValue));
                break;
            case Country:
                ((PreferredRoamingPartner) mCache.get("PreferredRoamingPartner")).addContry(mElementValue);
                break;
            case NetworkType:
                ((MinBackhaulThreshold) mCache.get("MinBackhaulThreshold")).setNetworkType(mElementValue);
                break;
            case DLBandwidth:
                ((MinBackhaulThreshold) mCache.get("MinBackhaulThreshold")).setDLBandwidth(Integer.parseInt(mElementValue));
                break;
            case ULBandwidth:
                ((MinBackhaulThreshold) mCache.get("MinBackhaulThreshold")).setULBandwidth(Integer.parseInt(mElementValue));
                break;
            case PolicyUpdate_UpdateInterval:
                ((PolicyUpdate) mBuildMap.get("PolicyUpdate")).setUpdateInterval(Integer.parseInt(mElementValue));
                break;
            case PolicyUpdate_UpdateMethod:
                ((PolicyUpdate) mBuildMap.get("PolicyUpdate")).setUpdateMethod(mElementValue);
                break;
            case PolicyUpdate_Restriction:
                ((PolicyUpdate) mBuildMap.get("PolicyUpdate")).setRestriction(mElementValue);
                break;
            case PolicyUpdate_URI:
                ((PolicyUpdate) mBuildMap.get("PolicyUpdate")).setURI(mElementValue);
                break;
            case PolicyUpdate_Username:
                ((UNPW) mBuildMap.get("PolicyUpdate_UNPW")).setUsername(mElementValue);
                break;
            case PolicyUpdate_Password:
                ((UNPW) mBuildMap.get("PolicyUpdate_UNPW")).setPassword(mElementValue);
                break; 
            case PolicyUpdate_Other:
                break;
            case PolicyUpdate_TrustRoot:
                break;
            case SPExclusionList_SSID:
                ((SPExclusion) mBuildMap.get("SPExclusionList")).addSSID(mElementValue);
                break;
            case IPProtocol:
                ((RequiredProtoPortTuple) mCache.get("RequiredProtoPortTuple")).setIPProtocol(Integer.parseInt(mElementValue));
                break;
            case PortNumber:
                ((RequiredProtoPortTuple) mCache.get("RequiredProtoPortTuple")).addPortNumber(Integer.parseInt(mElementValue));
                break;
            case MaximumBSSLoadValue:
                ((Policy) mBuildMap.get("Policy")).setMaximumBSSLoadValue(Integer.parseInt(mElementValue));
                break;
            case SubscriptionPriority:
                mPerProviderSubscriptionMo.setSubscriptionPriority(Integer.parseInt(mElementValue));
                break;
            case SubscriptionRemediation_URI:
                ((SubscriptionRemediation) mBuildMap.get("SubscriptionRemediation")).setURI(mElementValue);
                break;
            case SubscriptionRemediation_TrustRoot_CertURL:
                ((TrustRoot) mBuildMap.get("SubscriptionRemediation_TrustRoot")).setCertURL(mElementValue);
                break;
            case SubscriptionRemediation_TrustRoot_CertSHA256Fingerprint:
                ((TrustRoot) mBuildMap.get("SubscriptionRemediation_TrustRoot")).setCertSHA256Fingerprint(mElementValue);
                break;
            case AAAServerTrustRoot_CertURL:
                ((AAAServerTrustRoot) mCache.get("AAAServerTrustRoot")).setCertURL(mElementValue);
                break;
            case AAAServerTrustRoot_CertSHA256Fingerprint:
                ((AAAServerTrustRoot) mCache.get("AAAServerTrustRoot")).setCertSHA256Fingerprint(mElementValue);
                break;
            case SubscriptionUpdate_UpdateInterval:
                ((SubscriptionUpdate) mBuildMap.get("SubscriptionUpdate")).setUpdateInterval(Integer.parseInt(mElementValue));
                break;
            case SubscriptionUpdate_UpdateMethod:
                ((SubscriptionUpdate) mBuildMap.get("SubscriptionUpdate")).setUpdateMethod(mElementValue);
                break;
            case SubscriptionUpdate_Restriction:
                ((SubscriptionUpdate) mBuildMap.get("SubscriptionUpdate")).setRestriction(mElementValue);
                break;
            case SubscriptionUpdate_URI:
                ((SubscriptionUpdate) mBuildMap.get("SubscriptionUpdate")).setURI(mElementValue);
                break;
            case SubscriptionUpdate_Username:
                ((UNPW) mBuildMap.get("SubscriptionUpdate_UNPW")).setUsername(mElementValue);
                break;
            case SubscriptionUpdate_Password:
                ((UNPW) mBuildMap.get("SubscriptionUpdate_UNPW")).setPassword(mElementValue);
                break;
            case SubscriptionUpdate_Other:
                break;
            case NetworkID_SSID:
                ((NetworkID) mCache.get("NetworkID")).setSSID(mElementValue);
                break;
            case HESSID:
                ((NetworkID) mCache.get("NetworkID")).setHESSID(mElementValue);
                break;
            case FriendlyName:
                ((HomeSP) mBuildMap.get("HomeSP")).setFriendlyName(mElementValue);
                break;
            case IconURL:
                ((HomeSP) mBuildMap.get("HomeSP")).setIconURL(mElementValue);
                break;
            case HomeSP_FQDN:
                ((HomeSP) mBuildMap.get("HomeSP")).setFQDN(mElementValue);
                break;
            case HomeOI:
                ((HomeOI) mCache.get("HomeOI")).setHomeOI(mElementValue);
                break;
            case HomeOIRequired:
                ((HomeOI) mCache.get("HomeOI")).setHomeOIRequired(mElementValue);
                break;
            case OtherHomePartners_FQDN:
                ((HomeSP) mBuildMap.get("HomeSP")).addOtherHomePartnerFQDN(mElementValue);
                break;
            case RoamingConsortiumOI:
                ((HomeSP) mBuildMap.get("HomeSP")).addRoamingConsortiumOI(mElementValue);
                break;
            case SubscriptionParameters_CreationDate:
                ((SubscriptionParameters) mBuildMap.get("SubscriptionParameters")).setCreationDate(mElementValue);
                break;
            case SubscriptionParameters_ExpirationDate:
                ((SubscriptionParameters) mBuildMap.get("SubscriptionParameters")).setExpirationDate(mElementValue);
                break;
            case TypeOfSubscription:
                ((SubscriptionParameters) mBuildMap.get("SubscriptionParameters")).setTypeOfSubscription(mElementValue);
                break;
            case DataLimit:
                ((UsageLimits) mBuildMap.get("UsageLimits")).setDataLimit(Integer.parseInt(mElementValue));
                break;
            case StartDate:
                ((UsageLimits) mBuildMap.get("UsageLimits")).setStartDate(mElementValue);
                break;
            case TimeLimit:
                ((UsageLimits) mBuildMap.get("UsageLimits")).setTimeLimit(Integer.parseInt(mElementValue));
                break;
            case UsageInterval:
                ((UsageLimits) mBuildMap.get("UsageLimits")).setUsageInterval(Integer.parseInt(mElementValue));
                break;
            case Credential_CreationDate:
                ((Credential) mBuildMap.get("Credential")).setCreationDate(mElementValue);
                break;
            case Credential_ExpirationDate:
                ((Credential) mBuildMap.get("Credential")).setExpirationDate(mElementValue);
                break;
            case Credential_Username:
                ((UsernamePassword) mBuildMap.get("UsernamePassword")).setUsername(mElementValue);
                break;
            case Credential_Password:
                ((UsernamePassword) mBuildMap.get("UsernamePassword")).setPassword(mElementValue);
                break;
            case MachineManaged:
                ((UsernamePassword) mBuildMap.get("UsernamePassword")).setMachineManaged("true".equalsIgnoreCase(mElementValue) ? true : false);
                break;
            case SoftTokenApp:
                ((UsernamePassword) mBuildMap.get("UsernamePassword")).setSoftTokenApp(mElementValue);
                break;
            case AbleToShare:
                ((UsernamePassword) mBuildMap.get("UsernamePassword")).setAbleToShare("true".equalsIgnoreCase(mElementValue) ? true : false);
                break;
            case EAPMethod_EAPType:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setEAPType(Integer.parseInt(mElementValue));
                break;
            case VendorId:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setVendorId(Integer.parseInt(mElementValue));
                break;
            case VendorType:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setVendorType(Integer.parseInt(mElementValue));
                break;
            case InnerEAPType:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setInnerEAPType(mElementValue);
                break;
            case InnerVendorID:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setInnerVendorID(Integer.parseInt(mElementValue));
                break;
            case InnerVendorType:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setInnerVendorType(Integer.parseInt(mElementValue));
                break;
            case InnerEAPMethod:
                ((EAPMethod) mBuildMap.get("EAPMethod")).setInnerEAPMethod(mElementValue);
                break;
            case CertificateType:
                ((DigitalCertificate) mBuildMap.get("DigitalCertificate")).setCertificateType(mElementValue);
                break;
            case DigitalCertificate_CertSHA256Fingerprint:
                ((DigitalCertificate) mBuildMap.get("DigitalCertificate")).setCertSHA256Fingerprint(mElementValue);
                break;
            case Realm:
                ((Credential) mBuildMap.get("Credential")).setRealm(mElementValue);
                break;
            case IMSI:
                ((SIM) mBuildMap.get("SIM")).setImsi(mElementValue);
                break;
            case SIM_EAPType:
                ((SIM) mBuildMap.get("SIM")).setEAPType(Integer.parseInt(mElementValue));
                break;            
            case Extension:
                break;
            default:
                break;
        }*/
    }
    
    @SuppressWarnings("unchecked")
    private void handleParseEnd() {
        Log.d(TAG, "handleParseEnd");
        /*
        switch (mElementState) {
            case Policy:
                mPerProviderSubscriptionMo.setPolicy((Policy) mBuildMap.get("Policy"));
                break;
            case PreferredRoamingPartnerList:
                if (mCache.get("PreferredRoamingPartner") != null) {
                    //this is the last PreferredRoamingPartner
                    ((List<PreferredRoamingPartner>) mBuildMap.get("PreferredRoamingPartnerList")).add((PreferredRoamingPartner) mCache.get("PreferredRoamingPartner"));
                    mCache.remove("PreferredRoamingPartner");
                }
                ((Policy) mBuildMap.get("Policy")).setPreferredRoamingPartnerList((List<PreferredRoamingPartner>) mBuildMap.get("PreferredRoamingPartnerList"));              
                break;
            case MinBackhaulThreshold:
                if (mCache.get("MinBackhaulThreshold") != null) {
                    //this is the last MinBackhaulThreshold
                    ((List<MinBackhaulThreshold>) mBuildMap.get("MinBackhaulThresholds")).add((MinBackhaulThreshold) mCache.get("MinBackhaulThreshold"));
                    mCache.remove("MinBackhaulThreshold");
                }
                ((Policy) mBuildMap.get("Policy")).setMinBackhaulThresholds((List<MinBackhaulThreshold>) mBuildMap.get("MinBackhaulThresholds"));
                break;
            case PolicyUpdate:
                ((Policy) mBuildMap.get("Policy")).setPolicyUpdate((PolicyUpdate) mBuildMap.get("PolicyUpdate"));
                break;
            case PolicyUpdate_UNPW:
                ((PolicyUpdate) mBuildMap.get("PolicyUpdate")).setUsernamePassword((UNPW) mBuildMap.get("PolicyUpdate_UNPW"));
                break;
            case PolicyUpdate_TrustRoot:
                break;
            case SPExclusionList:
                ((Policy) mBuildMap.get("Policy")).setPolicyUpdate((PolicyUpdate) mBuildMap.get("SPExclusionList"));
                break;
            case RequiredProtoPortTuple:
                if (mCache.get("RequiredProtoPortTuple") != null) {
                    //this is the last RequiredProtoPortTuple
                    ((List<RequiredProtoPortTuple>) mBuildMap.get("RequiredProtoPortTuples")).add((RequiredProtoPortTuple) mCache.get("RequiredProtoPortTuple"));
                    mCache.remove("RequiredProtoPortTuple");
                }
                ((Policy) mBuildMap.get("Policy")).setRequiredProtoPortTuples((List<RequiredProtoPortTuple>) mBuildMap.get("RequiredProtoPortTuples"));
                break;
            case SubscriptionRemediation:
                mPerProviderSubscriptionMo.setSubscriptionRemediation((SubscriptionRemediation) mBuildMap.get("SubscriptionRemediation"));
                break;
            case SubscriptionRemediation_TrustRoot:
                ((SubscriptionRemediation) mBuildMap.get("SubscriptionRemediation")).setRrustRoot((TrustRoot)mBuildMap.get("SubscriptionRemediation_TrustRoot"));
                break;
            case AAAServerTrustRoot:
                if (mCache.get("AAAServerTrustRoot") != null) {
                    //this is the last AAAServerTrustRoot
                    ((List<AAAServerTrustRoot>) mBuildMap.get("AAAServerTrustRoots")).add((AAAServerTrustRoot) mCache.get("AAAServerTrustRoot"));
                    mCache.remove("AAAServerTrustRoot");
                }
                mPerProviderSubscriptionMo.setAaaServerTrustRoots((List<AAAServerTrustRoot>) mBuildMap.get("AAAServerTrustRoots"));
                break;
            case SubscriptionUpdate:
                mPerProviderSubscriptionMo.setSubscriptionUpdate((SubscriptionUpdate) mBuildMap.get("SubscriptionUpdate"));
                break;
            case SubscriptionUpdate_UNPW:
                ((SubscriptionUpdate) mBuildMap.get("SubscriptionUpdate")).setUsernamePassword((UNPW)mBuildMap.get("SubscriptionUpdate_UNPW"));
                break;
            case HomeSP:
                mPerProviderSubscriptionMo.setHomeSP((HomeSP) mBuildMap.get("HomeSP"));
                break;
            case NetworkID:
                if (mCache.get("NetworkID") != null) {
                    //this is the last NetworkID
                    ((List<NetworkID>) mBuildMap.get("NetworkIDs")).add((NetworkID) mCache.get("NetworkID"));
                    mCache.remove("NetworkID");
                }
                ((HomeSP) mBuildMap.get("HomeSP")).setNetworkIDs((List<NetworkID>) mBuildMap.get("NetworkIDs"));
                break;
            case HomeOIList:
                if (mCache.get("HomeOI") != null) {
                    //this is the last HomeOI
                    ((List<HomeOI>) mBuildMap.get("HomeOIList")).add((HomeOI) mCache.get("HomeOI"));
                    mCache.remove("HomeOI");
                }
                ((HomeSP) mBuildMap.get("HomeSP")).setHomeOIList((List<HomeOI>) mBuildMap.get("HomeOIList"));
                break;
            case SubscriptionParameters:
                mPerProviderSubscriptionMo.setSubscriptionParameters((SubscriptionParameters) mBuildMap.get("SubscriptionParameters"));
                break;
            case UsageLimits:
                ((SubscriptionParameters) mBuildMap.get("SubscriptionParameters")).setUsageLimits((UsageLimits) mBuildMap.get("UsageLimits"));
                break;
            case Credential:
                mPerProviderSubscriptionMo.setCredential((Credential) mBuildMap.get("Credential"));
                break;
            case Credential_UsernamePassword:
                ((Credential) mBuildMap.get("Credential")).setUsernamePassword((UsernamePassword) mBuildMap.get("UsernamePassword"));
                break;
            case EAPMethod:
                ((UsernamePassword) mBuildMap.get("UsernamePassword")).setEAPMethod((EAPMethod) mBuildMap.get("EAPMethod"));
                break;
            case DigitalCertificate:
                ((Credential) mBuildMap.get("Credential")).setDigitalCertificate((DigitalCertificate) mBuildMap.get("DigitalCertificate"));
                break;
            case SIM:
                ((Credential) mBuildMap.get("Credential")).setSim((SIM) mBuildMap.get("SIM"));
                break;
            default:
                break;
        }*/
    }
    
    private ElementState parseState(String tagName) {
        
        if (TextUtils.isEmpty(tagName)) {
            return Idle;
        } else if (tagName.equals("PolicyUpdate")) {
            mParentState = PolicyUpdate;
            return PolicyUpdate;
        } else if (tagName.equals("SubscriptionUpdate")) {
            mParentState = SubscriptionUpdate;
            return SubscriptionUpdate;
        } else if (tagName.equals("SPExclusionList")) {
            mParentState = SPExclusionList;
            return SPExclusionList;
        } else if (tagName.equals("SubscriptionRemediation")) {
            mParentState = SubscriptionRemediation;
            return SubscriptionRemediation;
        } else if (tagName.equals("AAAServerTrustRoot")) {
            mParentState = AAAServerTrustRoot;
            return AAAServerTrustRoot;
        } else if (tagName.equals("NetworkID")) {
            mParentState = NetworkID;
            return NetworkID;
        } else if (tagName.equals("DigitalCertificate")) {
            mParentState = DigitalCertificate;
            return DigitalCertificate;
        } else if (tagName.equals("HomeSP")) {
            mParentState = HomeSP;
            return HomeSP;
        } else if (tagName.equals("OtherHomePartners")) {
            mParentState = OtherHomePartners;
            return OtherHomePartners;
        } else if (tagName.equals("SubscriptionParameters")) {
            mParentState = SubscriptionParameters;
            return SubscriptionParameters;
        } else if (tagName.equals("Credential")) {
            mParentState = Credential;
            return Credential;
        } 
        
        else if (tagName.equals("UpdateInterval")){
            return (mParentState == PolicyUpdate) ? PolicyUpdate_UpdateInterval : SubscriptionUpdate_UpdateInterval;
        } else if (tagName.equals("UpdateMethod")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_UpdateMethod : SubscriptionUpdate_UpdateMethod;
        } else if (tagName.equals("Restriction")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_Restriction : SubscriptionUpdate_Restriction;
        } else if (tagName.equals("URI")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_URI : SubscriptionUpdate_URI;
        } else if (tagName.equals("UsernamePassword")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_UNPW : 
                (mParentState == SubscriptionUpdate) ? SubscriptionUpdate_UNPW : Credential_UsernamePassword;
        } else if (tagName.equals("Other")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_Other : SubscriptionUpdate_Other;
        } else if (tagName.equals("TrustRoot")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_TrustRoot : SubscriptionRemediation_TrustRoot;
        } else if (tagName.equals("SSID")) {
            return (mParentState == SPExclusionList) ? SPExclusionList_SSID : NetworkID_SSID;
        } else if (tagName.equals("URI")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_URI : SubscriptionRemediation_URI;
        } else if (tagName.equals("CertURL")) {
            return (mParentState == SubscriptionRemediation) ? SubscriptionRemediation_TrustRoot_CertURL : AAAServerTrustRoot_CertURL;
        } else if (tagName.equals("CertSHA256Fingerprint")) {
            return (mParentState == SubscriptionRemediation) ? SubscriptionRemediation_TrustRoot_CertSHA256Fingerprint : 
                (mParentState == AAAServerTrustRoot) ? AAAServerTrustRoot_CertSHA256Fingerprint : DigitalCertificate_CertSHA256Fingerprint;
        } else if (tagName.equals("FQDN")) {
            return (mParentState == HomeSP) ? HomeSP_FQDN : OtherHomePartners_FQDN;
        } else if (tagName.equals("CreationDate")) {
            return (mParentState == Credential) ? Credential_CreationDate : SubscriptionParameters_CreationDate;
        } else if (tagName.equals("ExpirationDate")) {
            return (mParentState == Credential) ? Credential_ExpirationDate : SubscriptionParameters_ExpirationDate;
        } else if (tagName.equals("Username")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_Username : 
                (mParentState == SubscriptionUpdate) ? SubscriptionUpdate_Username : Credential_Username;
        } else if (tagName.equals("Password")) {
            return (mParentState == PolicyUpdate) ? PolicyUpdate_Password : 
                (mParentState == SubscriptionUpdate) ? SubscriptionUpdate_Password : Credential_Password;
        } 
        
        else {
            return reflectState(tagName);
        }  
    }
    
    private ElementState reflectState(String tagName) {
        Log.d(TAG, "get " + tagName + " via reflect from ParseState.class");
        ElementState parseState = Idle;
        Class<ElementState> state = ElementState.class;
        try {
            Field f = state.getDeclaredField(tagName);
            f.setAccessible(true);
            parseState = (ElementState) f.get(null);
        } catch (NoSuchFieldException e) {
            Log.e(TAG, "NoSuchFieldException");
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException");
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            Log.e(TAG, "IllegalAccessException");
            e.printStackTrace();
        }
        Log.d(TAG, "the reflected state is : " + parseState);
        return parseState;
    }
    
    public PerProviderSubscriptionMo getPerProviderSubscriptionMo() {
        return mPerProviderSubscriptionMo;
    }
    
    private ElementState mElementState = ElementState.None;
    private ElementState mParentState = ElementState.None;
    private ParseState mParseState = parse_none;
    private String mElementName;
    private String mElementValue;
    private PerProviderSubscriptionMo mPerProviderSubscriptionMo;
    
    private Map<String, Object> mBuildMap = new HashMap<String, Object>();
    private Map<String, Object> mCache = new HashMap<String, Object>();

}

class PerProviderSubscriptionMo{}
class Policy{}
class AAAServerTrustRoot{}
class Credential{}
class DigitalCertificate{}
class EAPMethod{}
class HomeOI{}
class HomeSP{}
class MinBackhaulThreshold{}
class NetworkID{}
class PolicyUpdate{}
class PreferredRoamingPartner{}
class RequiredProtoPortTuple{}
class SIM{}
class SPExclusion{}
class SubscriptionParameters{}
class SubscriptionRemediation{}
class SubscriptionUpdate{}
class TrustRoot{}
class UNPW{}
class UsageLimits{}
class UsernamePassword{}