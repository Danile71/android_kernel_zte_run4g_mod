#define LOG_TAG "AudioMTKLosslessBTBroadcast"

#include "AudioLosslessBTBroadcast.h"

#define LOSSLESS_BT_UI_CMD_PLAYING   "am broadcast --user 0 -a android.intent.action.LOSSLESS_PLAYING --ei type 1 &"
#define LOSSLESS_BT_UI_CMD_CLOSE  "am broadcast --user 0 -a android.intent.action.LOSSLESS_CLOSE --ei type 1 &"
#define LOSSLESS_BT_UI_CMD_STOP "am broadcast --user 0 -a android.intent.action.LOSSLESS_STOP --ei type 1 &"
//#define LOSSLESS_BT_UI_CMD_ADD  "am broadcast --user 0 -a android.intent.action.LOSSLESS_ADD --ei type 1 &"

#define CMD "ps -t | grep ActivityManager"

#define MAX_LENGTH 1024
namespace android {

int systemServerStarted(const char *cmd)   
{
    ALOGD("systemServerStarted: %s \n", cmd);  
    int res = 0;
    char buf_ps[MAX_LENGTH];
    char ps[MAX_LENGTH]={0};
    FILE *ptr;   
    strcpy(ps, cmd);   
    if((ptr=popen(ps, "r")) != NULL)   
    {   
        while(fgets(buf_ps, MAX_LENGTH, ptr) != NULL)   
        {
           ALOGD("cmd result: %s", buf_ps);    
           if(strlen(buf_ps) > 0)
           {
               res = 1;   
               break;  
           } 
        }   
        pclose(ptr);   
        ptr = NULL;   
    }   
    else  
    {
        ALOGE("popen error");
    } 

    return res;  
}

void* AudioFlinger::ReadLLBTCommandThread(void *me)
{
    //SPEECH_MSG_DBG("%s", __FUNCTION__);
    //prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    return (void *) static_cast<AudioFlinger *>(me)->ReadLLBTCommand();
}

status_t AudioFlinger::ReadLLBTCommand()
{   
    ALOGD("AudioFlinger::ReadLLBTCommand()");
    const char *cmd;
    while(1){
        if(!mIsLosslessBTVaild){
            usleep(500*1000);
            continue;
        }
        mIsLosslessBTVaild = false;
        ALOGD("mIsLosslessBTOn:%d, mIsLosslessBTPlaying:%d", mIsLosslessBTOn, mIsLosslessBTPlaying);
        if(mIsLosslessBTOn){
            if(mIsLosslessBTPlaying){
                cmd = LOSSLESS_BT_UI_CMD_PLAYING;
            }else{
                cmd = LOSSLESS_BT_UI_CMD_STOP;                
            }
        }else{
            if(mIsLosslessBTPlaying){
                cmd = LOSSLESS_BT_UI_CMD_CLOSE;
            }else{
                cmd = LOSSLESS_BT_UI_CMD_CLOSE;            
            }            
        }

        if(!systemServerStarted(CMD)){
            ALOGE("systemServerStarted return falid");
            usleep(1000*1000);
            mIsLosslessBTVaild = true;
            continue;
        }

        int result = system(cmd);
        ALOGD("run: %s, result=%d",cmd , result);
    }
    
    return NO_ERROR;
}
// ----------------------------------------------------------------------------
}; // namespace android