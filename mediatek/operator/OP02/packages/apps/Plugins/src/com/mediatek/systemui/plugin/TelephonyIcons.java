package com.mediatek.systemui.plugin;

import com.mediatek.op02.plugin.R;

/**
 * M: This class define the OP02 constants of telephony icons.
 */
final class TelephonyIcons {

    /** Data connection type icons for roaming. @{ */

    static final int[] DATA_1X_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_1x_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_green_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_1x_white_roam
    };
    
    static final int[] DATA_3G_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_3g_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_green_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_3g_white_roam
    };
    
    static final int[] DATA_4G_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_4g_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_green_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_4g_white_roam
    };
    
    static final int[] DATA_E_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_e_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_e_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_e_green_roam,
        R.drawable.stat_sys_gemini_data_connected_e_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_e_white_roam
    };
    
    static final int[] DATA_G_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_g_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_g_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_g_green_roam,
        R.drawable.stat_sys_gemini_data_connected_g_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_g_white_roam
    };
    
    static final int[] DATA_H_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_h_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_h_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_h_green_roam,
        R.drawable.stat_sys_gemini_data_connected_h_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_h_white_roam
    };

    static final int[] DATA_H_PLUS_ROAM = {
        R.drawable.stat_sys_gemini_data_connected_h_plus_blue_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_orange_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_green_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_purple_roam,
        R.drawable.stat_sys_gemini_data_connected_h_plus_white_roam
    };
    
    static final int[][] DATA_ROAM = {
        DATA_1X_ROAM,
        DATA_3G_ROAM,
        DATA_4G_ROAM,
        DATA_E_ROAM,
        DATA_G_ROAM,
        DATA_H_ROAM,
        DATA_H_PLUS_ROAM
    };

    /** Data connection type icons for roaming. @} */

    /** Data connection type icons for normal. @{ */

    //GSM/UMTS
    static final int[] DATA_G = {
        R.drawable.stat_sys_gemini_data_connected_g_blue,
        R.drawable.stat_sys_gemini_data_connected_g_orange,
        R.drawable.stat_sys_gemini_data_connected_g_green,
        R.drawable.stat_sys_gemini_data_connected_g_purple,
        R.drawable.stat_sys_gemini_data_connected_g_white
    };

    static final int[] DATA_3G = {
        R.drawable.stat_sys_gemini_data_connected_3g_blue,
        R.drawable.stat_sys_gemini_data_connected_3g_orange,
        R.drawable.stat_sys_gemini_data_connected_3g_green,
        R.drawable.stat_sys_gemini_data_connected_3g_purple,
        R.drawable.stat_sys_gemini_data_connected_3g_white
    };

    static final int[] DATA_E = {
        R.drawable.stat_sys_gemini_data_connected_e_blue,
        R.drawable.stat_sys_gemini_data_connected_e_orange,
        R.drawable.stat_sys_gemini_data_connected_e_green,
        R.drawable.stat_sys_gemini_data_connected_e_purple,
        R.drawable.stat_sys_gemini_data_connected_e_white
    };

    //3.5G
    static final int[] DATA_H = {
        R.drawable.stat_sys_gemini_data_connected_h_blue,
        R.drawable.stat_sys_gemini_data_connected_h_orange,
        R.drawable.stat_sys_gemini_data_connected_h_green,
        R.drawable.stat_sys_gemini_data_connected_h_purple,
        R.drawable.stat_sys_gemini_data_connected_h_white
    };

    //3.5G
    static final int[] DATA_H_PLUS = {
        R.drawable.stat_sys_gemini_data_connected_h_plus_blue,
        R.drawable.stat_sys_gemini_data_connected_h_plus_orange,
        R.drawable.stat_sys_gemini_data_connected_h_plus_green,
        R.drawable.stat_sys_gemini_data_connected_h_plus_purple,
        R.drawable.stat_sys_gemini_data_connected_h_plus_white
    };

    static final int[] DATA_1X = {
        R.drawable.stat_sys_gemini_data_connected_1x_blue,
        R.drawable.stat_sys_gemini_data_connected_1x_orange,
        R.drawable.stat_sys_gemini_data_connected_1x_green,
        R.drawable.stat_sys_gemini_data_connected_1x_purple,
        R.drawable.stat_sys_gemini_data_connected_1x_white
    };

    // LTE and eHRPD
    static final int[] DATA_4G = {
        R.drawable.stat_sys_gemini_data_connected_4g_blue,
        R.drawable.stat_sys_gemini_data_connected_4g_orange,
        R.drawable.stat_sys_gemini_data_connected_4g_green,
        R.drawable.stat_sys_gemini_data_connected_4g_purple,
        R.drawable.stat_sys_gemini_data_connected_4g_white
    };

    static final int[][] DATA = {
        DATA_1X,
        DATA_3G,
        DATA_4G,
        DATA_E,
        DATA_G,
        DATA_H,
        DATA_H_PLUS
    };

    /** Data connection type icons for normal. @} */

    /** Data activity type icons. @{ */

    static final int[][] DATA_ACTIVITY = {
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_blue,
          R.drawable.stat_sys_signal_out_blue,
          R.drawable.stat_sys_signal_inout_blue},
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_orange,
          R.drawable.stat_sys_signal_out_orange,
          R.drawable.stat_sys_signal_inout_orange},
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_green,
          R.drawable.stat_sys_signal_out_green,
          R.drawable.stat_sys_signal_inout_green},
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in_purple,
          R.drawable.stat_sys_signal_out_purple,
          R.drawable.stat_sys_signal_inout_purple},
        { R.drawable.stat_sys_signal_not_inout,
          R.drawable.stat_sys_signal_in,
          R.drawable.stat_sys_signal_out,
          R.drawable.stat_sys_signal_inout}
    };

    /** Data activity type icons. @} */

    /** Network type icons. @{ */

    static final int[] NETWORK_TYPE_G = {
        R.drawable.stat_sys_gemini_signal_g_blue,
        R.drawable.stat_sys_gemini_signal_g_orange,
        R.drawable.stat_sys_gemini_signal_g_green,
        R.drawable.stat_sys_gemini_signal_g_purple,
        R.drawable.stat_sys_gemini_signal_g_white
    };
    
    static final int[] NETWORK_TYPE_3G = {
        R.drawable.stat_sys_gemini_signal_3g_blue,
        R.drawable.stat_sys_gemini_signal_3g_orange,
        R.drawable.stat_sys_gemini_signal_3g_green,
        R.drawable.stat_sys_gemini_signal_3g_purple,
        R.drawable.stat_sys_gemini_signal_3g_white
    };

    static final int[] NETWORK_TYPE_4G = {
        R.drawable.stat_sys_gemini_signal_4g_blue,
        R.drawable.stat_sys_gemini_signal_4g_orange,
        R.drawable.stat_sys_gemini_signal_4g_green,
        R.drawable.stat_sys_gemini_signal_4g_purple,
        R.drawable.stat_sys_gemini_signal_4g_white
    };

    /** Network type icons. @} */

}

