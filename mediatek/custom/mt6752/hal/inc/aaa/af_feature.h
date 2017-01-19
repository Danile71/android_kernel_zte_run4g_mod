#ifndef _LIB3A_AF_FEATURE_H
#define _LIB3A_AF_FEATURE_H

// AF command ID: 0x2000 ~
typedef enum
{
    LIB3A_AF_CMD_ID_SET_AF_MODE  = 0x2000,     // Set AF Mode
    LIB3A_AF_CMD_ID_SET_AF_METER = 0x2001,     // Set AF Meter

} LIB3A_AF_CMD_ID_T;

// AF mode definition
typedef enum
{
    LIB3A_AF_MODE_OFF = 0,           // Disable AF
    LIB3A_AF_MODE_AFS,               // AF-Single Shot Mode
    LIB3A_AF_MODE_AFC,               // AF-Continuous Mode
    LIB3A_AF_MODE_AFC_VIDEO,         // AF-Continuous Mode (Video)
    LIB3A_AF_MODE_MACRO,               // AF Macro Mode
    LIB3A_AF_MODE_INFINITY,          // Infinity Focus Mode
    LIB3A_AF_MODE_MF,                // Manual Focus Mode
    LIB3A_AF_MODE_CALIBRATION,       // AF Calibration Mode
    LIB3A_AF_MODE_FULLSCAN,           // AF Full Scan Mode

    LIB3A_AF_MODE_NUM,               // AF mode number
    LIB3A_AF_MODE_MIN = LIB3A_AF_MODE_OFF,
    LIB3A_AF_MODE_MAX = LIB3A_AF_MODE_FULLSCAN

} LIB3A_AF_MODE_T;

// AF meter definition
typedef enum
{
    LIB3A_AF_METER_SPOT = 0,      // Spot Window
    LIB3A_AF_METER_MATRIX,                // Matrix Window
    LIB3A_AF_METER_FD,                 // FD Window
    LIB3A_AF_METER_CONTI,         // for AFC

    LIB3A_AF_METER_NUM,
    LIB3A_AF_METER_MIN = LIB3A_AF_METER_SPOT,
    LIB3A_AF_METER_MAX = LIB3A_AF_METER_CONTI

} LIB3A_AF_METER_T;

#endif
