package com.mediatek.passpoint;

public class IANAEapMethod {   
    public static enum eIANA_EAPmethod {
        Reserved,
        Identity,
        Notification,
        Legacy_Nak,
        MD5_Challenge,
        OTP,
        GTC,
        Allocated_7,
        Allocated_8,
        RSA_Public_Key_Authentication,
        DSS_Unilateral,
        KEA,
        KEA_VALIDATE,
        EAP_TLS,
        AXENT,
        RSA_Security_SecurID_EAP,
        Arcot_Systems_EAP,
        EAP_Cisco_Wireless,
        EAP_SIM,
        SRP_SHA1,
        Unassigned_20,
        EAP_TTLS,
        Remote_Access_Service,
        EAP_AKA,
        EAP_3Com_Wireless,
        PEAP,
        MS_EAP_Authentication,
        MAKE,
        CRYPTOCard,
        EAP_MSCHAP_V2,
        DynamID,
        Rob_EAP,
        Protected_One_Time_Password,
        MS_Authentication_TLV,
        SentriNET,
        EAP_Actiontec_Wireless,
        Cogent_Systems_Biometrics_Authentication_EAP,
        AirFortress_EAP,
        EAP_HTTP_Digest,
        SecureSuite_EAP,
        DeviceConnect_EAP,
        EAP_SPEKE,
        EAP_MOBAC,
        EAP_FAST,
        ZLXEAP,
        EAP_Link,
        EAP_PAX,
        EAP_PSK,
        EAP_SAKE,
        EAP_IKEv2,
        EAP_AKA2,
        EAP_GPSK,
        EAP_pwd,
        EAP_EKE_Version_1
    };

    public final static String[] sIANA_EAPmethod = {
        "Reserved",//0
        "Identity",// 1
        "Notification",// 2
        "Legacy_Nak",// 3
        "MD5-Challenge",// 4
        "OTP",// 5
        "GTC",// 6
        "Allocated",// 7
        "Allocated",// 8
        "RSA_Public_Key_Authentication",// 9
        "DSS_Unilateral",// 10
        "KEA",// 11
        "KEA-VALIDATE",// 12
        "TLS",// 13
        "AXENT",// 14
        "RSA_Security_SecurID_EAP",// 15
        "Arcot_Systems_EAP",// 16
        "EAP-Cisco_Wireless",// 17
        "SIM",// 18
        "SRP-SHA1",// 19
        "Unassigned",// 20
        "TTLS",// 21
        "Remote_Access_Service",//22
        "AKA",//23
        "3Com_Wireless",//24
        "PEAP",//25
        "MS-EAP-Authentication",//26
        "MAKE",//27
        "CRYPTOCard",//28
        "MSCHAPv2",//29
        "DynamID",//30
        "Rob_EAP",//31
        "Protected_One-Time_Password",//32
        "MS-Authentication-TLV",//33
        "SentriNET",//34
        "EAP-Actiontec_Wireless",//35
        "Cogent_Systems_Biometrics_Authentication_EAP",//36
        "AirFortress_EAP",//37
        "EAP-HTTP_Digest",//38
        "SecureSuite_EAP",//39
        "DeviceConnect_EAP",//40
        "SPEKE",//41
        "MOBAC",//42
        "EAP-FAST",//43
        "ZLXEAP",//44
        "Link",//45
        "PAX",//46
        "PSK",//47
        "SAKE",//48
        "IKEv2",//49
        "AKA2",//50
        "GPSK",//51
        "pwd",//52
        "EKE_Version_1",//53
        "Unassigned"//54
    };

}

