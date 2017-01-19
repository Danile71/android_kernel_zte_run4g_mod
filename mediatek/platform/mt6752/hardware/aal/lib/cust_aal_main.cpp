#define LOG_TAG "AALCust"

#include <cutils/xlog.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "ddp_drv.h"

extern "C" { // All symbols should be exported as C signature

extern int LCM_COUNT;

int getLcmIndex(void)
{
    static int lcmIdx = -1;

    if (lcmIdx == -1) {
        int drvID = open("/dev/mtk_disp_mgr", O_RDONLY, 0);
        if (drvID >= 0) {
            ioctl(drvID, DISP_IOCTL_GET_LCMINDEX, &lcmIdx);
            close(drvID);
            if (lcmIdx < 0 || LCM_COUNT <= lcmIdx)
            {
                XLOGE("Invalid LCM index %d, LCM count %d", lcmIdx, LCM_COUNT);
                lcmIdx = 0;
            }
        }
        else {
            XLOGE("Fail to open disp driver!");
            lcmIdx = 0;
        }
    }    

    XLOGI("LCM index: %d/%d", lcmIdx, LCM_COUNT);
    return lcmIdx;
}



// MUST HAVE parameters
extern int BrightnessLevel[];
extern int BrighteningSpeedLevel;
extern int DarkeningSpeedLevel;
extern int ReadabilityLevel[];
extern int SmartBacklightStrength[];
extern int SmartBacklightRange[];


void checkVariableNames(void)
{
    // If any link error here, means the cust_aal.cpp is not configured properly.
    // May be file lost(not linked) or incorrect variable name
    XLOGI("Levels = %d %d %d %d %d %d",
        BrightnessLevel[0], BrighteningSpeedLevel, DarkeningSpeedLevel, ReadabilityLevel[0],
        SmartBacklightStrength[0], SmartBacklightRange[0]);
}

} // end of extern "C"

