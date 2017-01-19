#define LOG_TAG "GuiExt"

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <utils/SortedVector.h>
#include <binder/PermissionCache.h>

#include <android/native_window.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <ui/DisplayInfo.h>
#include <ui/mediatek/IDumpTunnel.h>

#include <cutils/memory.h>

#include "GuiExtService.h"
#include "GuiExtImpl.h"

namespace android {

#define GUIEXT_LOGV(x, ...) XLOGV("[GuiExtS] "x, ##__VA_ARGS__)
#define GUIEXT_LOGD(x, ...) XLOGD("[GuiExtS] "x, ##__VA_ARGS__)
#define GUIEXT_LOGI(x, ...) XLOGI("[GuiExtS] "x, ##__VA_ARGS__)
#define GUIEXT_LOGW(x, ...) XLOGW("[GuiExtS] "x, ##__VA_ARGS__)
#define GUIEXT_LOGE(x, ...) XLOGE("[GuiExtS] "x, ##__VA_ARGS__)

GuiExtService::GuiExtService()
{
    GUIEXT_LOGI("GuiExtService ctor");
    mPool = new GuiExtPool();
}

GuiExtService::~GuiExtService()
{
}

status_t GuiExtService::alloc(const sp<IBinder>& token, uint32_t gralloc_usage, uint32_t w, uint32_t h, uint32_t *poolId)
{
    GUIEXT_LOGV("alloc, gralloc_usage=%x, w=%d, h=%d, usage=%x", gralloc_usage, w, h);
    status_t ret = mPool->alloc(token, gralloc_usage, w, h, poolId);
    return ret;
}

status_t GuiExtService::free(uint32_t poolId)
{
    GUIEXT_LOGV("free, poolId=%d", poolId);
    status_t ret = mPool->free(poolId);
    return ret;
}

status_t GuiExtService::acquire(const sp<IBinder>& token, uint32_t poolId, uint32_t usage, uint32_t type, int *buf)
{
    GUIEXT_LOGV("acquire, poolId=%d, usage=%x, type=%d", poolId, usage, type);
    status_t ret = mPool->acquire(token, poolId, usage, type, buf);
    return ret;
}

status_t GuiExtService::request(uint32_t poolId, uint32_t usage, uint32_t type, int buf, sp<GraphicBuffer>* buffer)
{
    GUIEXT_LOGV("request, poolId=%d, usage=%x, type=%d, buf=%d", poolId, usage, type, buf);
    status_t ret = mPool->request(poolId, usage, type, buf, buffer);
    return ret;
}

status_t GuiExtService::release(uint32_t poolId, uint32_t usage, uint32_t type, int buf)
{
    GUIEXT_LOGV("release, poolId=%d, usage=%x, buf=%d, type=%d", poolId, usage, buf, type);
    status_t ret = mPool->release(poolId, usage, type, buf);
    return ret;
}

status_t GuiExtService::disconnect(uint32_t poolId, uint32_t usage, uint32_t type)
{
    GUIEXT_LOGV("disconnect, poolId=%d, usage=%x, type=%d", poolId, usage, type);
    status_t ret = mPool->disconnect(poolId, usage, type);
    return ret;
}

status_t GuiExtService::configDisplay(uint32_t type, bool enable, uint32_t w, uint32_t h, uint32_t bufNum)
{
    GUIEXT_LOGV("configDisplay, type=%d, enable=%d, w=%d, h=%d, bufNum=%d", type, enable, w, h, bufNum);
    mPool->configDisplay(type, enable, w, h, bufNum);
    return NO_ERROR;
}

static const String16 sDump("android.permission.DUMP");
status_t GuiExtService::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 4096;
    char buffer[SIZE];
    String8 result;

    if (!PermissionCache::checkCallingPermission(sDump)) {
        snprintf(buffer, SIZE, "Permission Denial: "
                "can't dump SurfaceFlinger from pid=%d, uid=%d\n",
                IPCThreadState::self()->getCallingPid(),
                IPCThreadState::self()->getCallingUid());
        result.append(buffer);
    } else {
        // Try to get the main lock, but don't insist if we can't
        // (this would indicate GuiExtService is stuck, but we want to be able to
        // print something in dumpsys).
        int retry = 3;
        while (mLock.tryLock()<0 && --retry>=0) {
            usleep(1000000);
        }
        const bool locked(retry >= 0);
        if (!locked) {
            snprintf(buffer, SIZE,
                    "GuiExtService appears to be unresponsive, "
                    "dumping anyways (no locks held)\n");
            result.append(buffer);
        }

        snprintf(buffer, SIZE,
                 "GuiExtService[pid=%d] state:\n\n",
                 getpid()
        );
        result.append(buffer);

        mPool->dump(result);

        if (locked) {
            mLock.unlock();
        }
    }

    {
        // check property for general buffer queue
        char value[PROPERTY_VALUE_MAX];
        property_get("debug.bq.general", value, "0");
        bool bqDump = (atoi(value) == 1);

        // scan each tunnel, and kick required tunnel
        Mutex::Autolock l(mDumpLock);
        for (uint32_t i = 0; i < mDumpTunnels.size(); ++i)
        {
            const sp<IDumpTunnel>& tunnel = mDumpTunnels.valueAt(i);
            const String8& key = mDumpTunnels.keyAt(i);
            if (!tunnel->asBinder()->isBinderAlive())
                continue;

            // if more types of tunnels are required,
            // please add filters to dump at here
            if (bqDump && key.find("BQ") == 0)
                tunnel->kickDump(result, key.string());
        }
    }

    {
        char value[PROPERTY_VALUE_MAX];
        property_get("debug.rb.kick", value, "0");
        bool isRbKick = (atoi(value) == 1);
        // scan each tunnel, and kick required tunnel
        Mutex::Autolock l(mDumpLock);
        if (isRbKick) {
            result.append("\n==============================================================\n");
            for (uint32_t i = 0; i < mDumpTunnels.size(); i++) {
                const sp<IDumpTunnel>& tunnel = mDumpTunnels.valueAt(i);
                const String8& key = mDumpTunnels.keyAt(i);
                if (!tunnel->asBinder()->isBinderAlive()) {
                    continue;
                }
                if (key.find("RB") == 0) {
                    tunnel->kickDump(result, key.string());
                }
            }
        }
    }
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t GuiExtService::regDump(const sp<IDumpTunnel>& tunnel, const String8& key)
{
    // check the tunnel does not come from GuiExtService
    if (!tunnel->asBinder()->remoteBinder())
        return NO_ERROR;

    if (!tunnel->asBinder()->isBinderAlive())
        return BAD_VALUE;

    class DeathNotifier : public IBinder::DeathRecipient
    {
        const String8 mKey;
        const wp<GuiExtService> mService;
    public:
        DeathNotifier(const String8& key, const wp<GuiExtService>& service)
            : mKey(key)
            , mService(service)
        {
        }

        virtual void binderDied(const wp<IBinder>& who)
        {
            GUIEXT_LOGW("binder of dump tunnel(%s) died", mKey.string());

            sp<GuiExtService> service = mService.promote();
            if (service != NULL)
                service->unregDump(mKey);
        }
    };

    sp<IBinder::DeathRecipient> notifier = new DeathNotifier(key, this);
    if (notifier != NULL)
        tunnel->asBinder()->linkToDeath(notifier);

    Mutex::Autolock l(mDumpLock);
    mDumpTunnels.add(key, tunnel);
    return NO_ERROR;
}

status_t GuiExtService::unregDump(const String8& key)
{
    Mutex::Autolock l(mDumpLock);
    mDumpTunnels.removeItem(key);
    return NO_ERROR;
}
};
