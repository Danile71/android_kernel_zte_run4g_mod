#define LOG_TAG "GuiExt"

#include <cutils/xlog.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include "GuiExtClient.h"
#include "IGuiExtService.h"

namespace android {

#define GUIEX_LOGV(x, ...) XLOGV("[GuiExtL] "x, ##__VA_ARGS__)
#define GUIEX_LOGD(x, ...) XLOGD("[GuiExtL] "x, ##__VA_ARGS__)
#define GUIEX_LOGI(x, ...) XLOGI("[GuiExtL] "x, ##__VA_ARGS__)
#define GUIEX_LOGW(x, ...) XLOGW("[GuiExtL] "x, ##__VA_ARGS__)
#define GUIEX_LOGE(x, ...) XLOGE("[GuiExtL] "x, ##__VA_ARGS__)

//#define RETRY_SUPPORT

GuiExtClient::GuiExtClient()
{
    assertStateLocked();
}

status_t GuiExtClient::assertStateLocked() const
{
    int count = 0;
    if (mGuiExtService == NULL)
    {
        // try for one second
        const String16 name("GuiExtService");
        do {
            status_t err = getService(name, &mGuiExtService);
            if (err == NAME_NOT_FOUND) {
#if RETRY_SUPPORT
                if (count < 3)
                {
                    ALOGW("GuiExtService not published, waiting...");
                    usleep(100000);
                    count++;
                    continue;
                }
#else
                ALOGW("GuiExtService not published...");
#endif
                return err;
            }
            if (err != NO_ERROR) {
                return err;
            }
            break;
        } while (true);

        class DeathObserver : public IBinder::DeathRecipient {
            GuiExtClient & mGuiExtClient;
            virtual void binderDied(const wp<IBinder>& who) {
                ALOGW("GuiEx Service died [%p]", who.unsafe_get());
                mGuiExtClient.serviceDied();
            }
        public:
            DeathObserver(GuiExtClient & service) : mGuiExtClient(service) { }
        };

        mDeathObserver = new DeathObserver(*const_cast<GuiExtClient*>(this));
        mGuiExtService->asBinder()->linkToDeath(mDeathObserver);
    }

    return NO_ERROR;
}

void GuiExtClient::serviceDied()
{
    Mutex::Autolock _l(mLock);
    GUIEX_LOGI("[%s]", __func__);

    serviceDiedLocked();

    mGuiExtService.clear();
}
};
