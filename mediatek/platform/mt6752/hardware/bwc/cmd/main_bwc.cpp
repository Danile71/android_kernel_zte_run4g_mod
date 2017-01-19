#define LOG_TAG "BWC"

#include <cutils/xlog.h>
#include <binder/BinderService.h>
#include <BWCService.h>

using namespace android;

int main(int argc, char** argv) 
{
    XLOGD("BWC service start");

    BWCService::publishAndJoinThreadPool(true);
    // When BWC is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    XLOGD("BWC service exit");
    return 0;
}
