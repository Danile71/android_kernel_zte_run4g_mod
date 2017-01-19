#define LOG_TAG "BWCService" 

#include <sys/types.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <utils/SortedVector.h>
#include <binder/PermissionCache.h>
#include "BWCService.h"

#include <cutils/memory.h>


namespace android {


#define BWCS_LOGD(fmt, arg...) XLOGD(fmt, ##arg);

    BWCService::BWCService()
    {   
        BWCS_LOGD("BWCService constructor");   
        setProfile(BWCPT_NONE, 1);
    }

    BWCService::~BWCService()
    {
        setProfile(BWCPT_NONE, 0);
    }

    // Adapt the Binder Service to legacy BWC implementation
    status_t BWCService::setProfile(int32_t profile, int32_t state)
    {
        Mutex::Autolock _l(mLock);
        BWCS_LOGD("setProfile:%d=%d", profile, state);
        if(state == 0){
            bwc.Profile_Change((BWC_PROFILE_TYPE)profile, false);
        }else{
            bwc.Profile_Change((BWC_PROFILE_TYPE)profile, true);
        }

        return NO_ERROR;
    }


    void BWCService::onFirstRef() 
    {
        // Set the priority to PRIORITY_BACKGROUND
        // since it is a background thread to collection system bandwidth information
        run("BWCServiceMain", PRIORITY_BACKGROUND);
    }

    status_t BWCService::readyToRun()
    {
        BWCS_LOGD("BWCService ready");
        return NO_ERROR;    
    }

    bool BWCService::threadLoop()
    {
        BWCS_LOGD("BWCService thread loop start");
        // To collect system bandwitdh informtation here
        // Currently it is a simple implementation
        while(1){
            sleep(60*15); 
        }
        return true;
    }

};
