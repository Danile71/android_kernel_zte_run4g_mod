package com.mediatek.systemui.plugin;

import com.mediatek.op09.plugin.R;

/**
 * M: This class define the OP09 constants of telephony icons.
 */
final class TelephonyIcons {

    /** Data connection type icons for normal. @{ */

    static final int[] DATA_1X = {
        R.drawable.stat_sys_gemini_data_connected_1x_blue,
        R.drawable.stat_sys_gemini_data_connected_1x_orange,
        R.drawable.stat_sys_gemini_data_connected_1x_green,
        R.drawable.stat_sys_gemini_data_connected_1x_purple,
        R.drawable.stat_sys_gemini_data_connected_1x_white
    };

    static final int[] DATA_2G = {
        R.drawable.stat_sys_gemini_data_connected_2g_blue,
        R.drawable.stat_sys_gemini_data_connected_2g_orange,
        R.drawable.stat_sys_gemini_data_connected_2g_green,
        R.drawable.stat_sys_gemini_data_connected_2g_purple,
        R.drawable.stat_sys_gemini_data_connected_2g_white
    };

    static final int[] DATA_3G = {
        R.drawable.stat_sys_gemini_data_connected_3g_blue,
        R.drawable.stat_sys_gemini_data_connected_3g_orange,
        R.drawable.stat_sys_gemini_data_connected_3g_green,
        R.drawable.stat_sys_gemini_data_connected_3g_purple,
        R.drawable.stat_sys_gemini_data_connected_3g_white
    };

    static final int[] DATA_4G = {
        R.drawable.stat_sys_gemini_data_connected_4g_blue,
        R.drawable.stat_sys_gemini_data_connected_4g_orange,
        R.drawable.stat_sys_gemini_data_connected_4g_green,
        R.drawable.stat_sys_gemini_data_connected_4g_purple,
        R.drawable.stat_sys_gemini_data_connected_4g_white
    };

    static final int[] DATA_E = DATA_2G;

    static final int[] DATA_G = DATA_2G;

    static final int[] DATA_H = DATA_3G;

    static final int[] DATA_H_PLUS = DATA_3G;

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

    /** Network type icons. @{ */

    static final int[] NETWORK_TYPE_2G = {
        R.drawable.stat_sys_gemini_signal_2g_blue,
        R.drawable.stat_sys_gemini_signal_2g_orange,
        R.drawable.stat_sys_gemini_signal_2g_green,
        R.drawable.stat_sys_gemini_signal_2g_purple,
        R.drawable.stat_sys_gemini_signal_2g_white
    };
    static final int[] NETWORK_TYPE_3G = {
        R.drawable.stat_sys_gemini_signal_3g_blue,
        R.drawable.stat_sys_gemini_signal_3g_orange,
        R.drawable.stat_sys_gemini_signal_3g_green,
        R.drawable.stat_sys_gemini_signal_3g_purple,
        R.drawable.stat_sys_gemini_signal_3g_white
    };
    static final int[] NETWORK_TYPE_1X = {
        R.drawable.stat_sys_gemini_signal_1x_blue,
        R.drawable.stat_sys_gemini_signal_1x_orange,
        R.drawable.stat_sys_gemini_signal_1x_green,
        R.drawable.stat_sys_gemini_signal_1x_purple,
        R.drawable.stat_sys_gemini_signal_1x_white
    };
    static final int[] NETWORK_TYPE_1X3G = {
        R.drawable.stat_sys_gemini_signal_1x_3g_blue,
        R.drawable.stat_sys_gemini_signal_1x_3g_orange,
        R.drawable.stat_sys_gemini_signal_1x_3g_green,
        R.drawable.stat_sys_gemini_signal_1x_3g_purple,
        R.drawable.stat_sys_gemini_signal_1x_3g_white
    };

    static final int[][] NETWORK_TYPE = {
        NETWORK_TYPE_2G,
        NETWORK_TYPE_3G,
        NETWORK_TYPE_1X,
        NETWORK_TYPE_1X3G
    };

    /** Network type icons. @} */

    /** Data icons. {@ */

    static final int[][] DATA_ACTIVITY = {
        {   0,
            R.drawable.stat_sys_signal_in_blue,
            R.drawable.stat_sys_signal_out_blue,
            R.drawable.stat_sys_signal_inout_blue },
        {   0,
            R.drawable.stat_sys_signal_in_orange,
            R.drawable.stat_sys_signal_out_orange,
            R.drawable.stat_sys_signal_inout_orange },
        {   0,
            R.drawable.stat_sys_signal_in_green,
            R.drawable.stat_sys_signal_out_green,
            R.drawable.stat_sys_signal_inout_green },
        {   0,
            R.drawable.stat_sys_signal_in_purple,
            R.drawable.stat_sys_signal_out_purple,
            R.drawable.stat_sys_signal_inout_purple }
    };

    /** Data icons. @} */

    /** WIFI icons. {@ */

    static final int[] WIFI_STRENGTH_ICON = {
        R.drawable.stat_sys_wifi_signal_0,
        R.drawable.stat_sys_wifi_signal_1,
        R.drawable.stat_sys_wifi_signal_2,
        R.drawable.stat_sys_wifi_signal_3,
        R.drawable.stat_sys_wifi_signal_4
    };

    static final int[] WIFI_INOUT_ICON = {
        0,
        R.drawable.stat_sys_wifi_in,
        R.drawable.stat_sys_wifi_out,
        R.drawable.stat_sys_wifi_inout
    };

    /** WIFI icons. @} */
    /// M: Add for SIM Conn.
    public static final int[] IC_SIM_CONNECT_INDICATOR = {
        R.drawable.ic_qs_mobile_sim1_enable_disable,
        R.drawable.ic_qs_mobile_sim1_enable_enable,
        R.drawable.ic_qs_mobile_sim1_disable_disable,
        R.drawable.ic_qs_mobile_sim1_disable_enable,
        R.drawable.ic_qs_mobile_sim2_enable_disable,
        R.drawable.ic_qs_mobile_sim2_enable_enable,
        R.drawable.ic_qs_mobile_sim2_disable_disable,
        R.drawable.ic_qs_mobile_sim2_disable_enable,
        R.drawable.ic_qs_mobile_all_disable_disable,
        R.drawable.ic_qs_mobile_sim1_enable_off,
        R.drawable.ic_qs_mobile_sim1_disable_off,
        R.drawable.ic_qs_mobile_sim2_enable_off,
        R.drawable.ic_qs_mobile_sim2_disable_off
    };
}

