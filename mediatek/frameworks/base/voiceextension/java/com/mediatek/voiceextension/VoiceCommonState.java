package com.mediatek.voiceextension;

/**
 * @hide
 */
public class VoiceCommonState {

    // ==========================RetCode Common==========================//
    private static final int VIE_COMMON_INDEX = 0;

    public static final int SUCCESS = VIE_COMMON_INDEX + 1;

    public static final int WRITE_STORAGE_FAIL = VIE_COMMON_INDEX + 2;

    public static final int MIC_INIT_FAIL = VIE_COMMON_INDEX + 3;

    public static final int MIC_OCCUPIED = VIE_COMMON_INDEX + 4;

    public static final int LISTENER_ILLEGAL = VIE_COMMON_INDEX + 5;

    public static final int LISTENER_NEVER_SET = VIE_COMMON_INDEX + 6;

    public static final int LISTENER_ALREADY_SET = VIE_COMMON_INDEX + 7;

    public static final int RECOGNITION_NEVER_START = VIE_COMMON_INDEX + 8;

    public static final int RECOGNITION_NEVER_PAUSE = VIE_COMMON_INDEX + 9;

    public static final int RECOGNITION_ALREADY_STARTED = VIE_COMMON_INDEX + 10;

    public static final int RECOGNITION_ALREADY_PAUSED = VIE_COMMON_INDEX + 11;

    public static final int SERVICE_NOT_EXIST = VIE_COMMON_INDEX + 12;

    public static final int SERVICE_DISCONNECTTED = VIE_COMMON_INDEX + 13;

    public static final int PROCESS_ILLEGAL = VIE_COMMON_INDEX + 14;

    // ===========================RetCode Set==========================//
    private static final int VIE_SET_INDEX = 200;

    public static final int SET_ALREADY_EXIST = VIE_SET_INDEX + 1;

    public static final int SET_NOT_EXIST = VIE_SET_INDEX + 2;

    public static final int SET_ILLEGAL = VIE_SET_INDEX + 3;

    public static final int SET_EXCEED_LIMIT = VIE_SET_INDEX + 4;

    public static final int SET_SELECTED = VIE_SET_INDEX + 5;

    public static final int SET_NOT_SELECTED = VIE_SET_INDEX + 6;

    public static final int SET_OCCUPIED = VIE_SET_INDEX + 7;

    // ===========================RetCode Command==========================//
    private static final int VIE_COMMAND_INDEX = 300;

    public static final int COMMANDS_DATA_INVALID = VIE_COMMAND_INDEX + 1;

    public static final int COMMANDS_FILE_ILLEGAL = VIE_COMMAND_INDEX + 2;

    public static final int COMMANDS_NUM_EXCEED_LIMIT = VIE_COMMAND_INDEX + 3;

    // ===========================RetCode Search==========================//

    // =========================RetCode Passphrase==========================//

    // ============================Feature Type=============================//
    public static final int VIE_FEATURE_COMMAND = 1;
    public static final int VIE_FEATURE_SEARCH = 2;
    public static final int VIE_FEATURE_PASSPHRASE = 3;

    public static final String VIE_FEATURE_COMMAND_NAME = "Command";
    public static final String VIE_FEATURE_SEARCH_NAME = "Search";
    public static final String VIE_FEATURE_PASSPHRASE_NAME = "Passphrase";

    // ===========================API Type Command========================//
    public static final int VIE_API_COMMAND_IDLE = 1;
    public static final int API_COMMAND_START_RECOGNITION = VIE_API_COMMAND_IDLE + 1;
    public static final int API_COMMAND_STOP_RECOGNITION = VIE_API_COMMAND_IDLE + 2;
    public static final int API_COMMAND_PAUSE_RECOGNITION = VIE_API_COMMAND_IDLE + 3;
    public static final int API_COMMAND_RESUME_RECOGNITION = VIE_API_COMMAND_IDLE + 4;
    public static final int API_COMMAND_RECOGNIZE_RESULT = VIE_API_COMMAND_IDLE + 5;
    public static final int API_COMMAND_NOTIFY_ERROR = VIE_API_COMMAND_IDLE + 6;
    public static final int API_COMMAND_SET_COMMANDS = VIE_API_COMMAND_IDLE + 7;

    // ============================API Type Search===========================//

    // ===========================API Type Passphrase=========================//

}
