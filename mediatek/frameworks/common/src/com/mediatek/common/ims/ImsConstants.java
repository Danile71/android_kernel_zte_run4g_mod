package com.mediatek.common.ims;


/**
 * @hide
 */
public class ImsConstants {
    public static final int IMS_MO_PCSCF     = 0;
    public static final int IMS_MO_TIMER_1   = IMS_MO_PCSCF + 1;
    public static final int IMS_MO_TIMER_2   = IMS_MO_PCSCF + 2;
    public static final int IMS_MO_TIMER_4   = IMS_MO_PCSCF + 3;
    public static final int IMS_MO_IMPI      = IMS_MO_PCSCF + 4;   // Private_user_identity
    public static final int IMS_MO_IMPU      = IMS_MO_PCSCF + 5;   // Public_user_identity
    public static final int IMS_MO_DOMAIN    = IMS_MO_PCSCF + 6;   // Home_network_domain_name
    public static final int IMS_MO_RESOURCE  = IMS_MO_PCSCF + 7;   // Resource_Allocation_Mode
    public static final int IMS_MO_VOICE_E   = IMS_MO_PCSCF + 8;   // Voice_Domain_Preference_E_UTRAN
    public static final int IMS_MO_SMS       = IMS_MO_PCSCF + 9;   // SMS_Over_IP_Networks_Indication
    public static final int IMS_MO_KEEPALIVE = IMS_MO_PCSCF + 10;   // Keep_Alive_Enabled
    public static final int IMS_MO_VOICE_U   = IMS_MO_PCSCF + 11;   // Voice_Domain_Preference_UTRAN
    public static final int IMS_MO_MOBILITY  = IMS_MO_PCSCF + 12;  // Mobility_Management_IMS_Voice_Termination
    public static final int IMS_MO_REG_BASE  = IMS_MO_PCSCF + 13;  // RegRetryBaseTime
    public static final int IMS_MO_REG_MAX   = IMS_MO_PCSCF + 14;  // RegRetryMaxTime
    
    public static final int IMS_GBA_NONE     = 0;
    public static final int IMS_GBA_ME       = 1;
    public static final int IMS_GBA_U        = 2;
    
    public static final String IMS_GBA_KS_NAF       = "Ks_NAF";
    public static final String IMS_GBA_KS_EXT_NAF   = "Ks_ext_NAF";

    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID0 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00};
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID1 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x01}; //MBMS
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID2 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x02};
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID3 = new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x03}; //MBMS
    
}

