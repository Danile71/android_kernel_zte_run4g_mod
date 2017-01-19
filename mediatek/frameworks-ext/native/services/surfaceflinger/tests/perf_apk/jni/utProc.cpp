#define LOG_TAG "SFT"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <dlfcn.h>
#include <stdlib.h>
#include <utils/Timers.h>
#include <utils/Trace.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <cutils/memory.h>
#include <math.h>

//#include <ui/DisplayInfo.h>
#include <gui/Surface.h>
#include <hardware/gralloc.h>

#include "utProc.h"

//#define USE_GRALLOC_EXTRA
#ifdef USE_GRALLOC_EXTRA
#include <ui/gralloc_extra.h>
#endif

namespace android {

#define LOGV(x, ...) XLOGV("[SFT_ut] "x, ##__VA_ARGS__)
#define LOGD(x, ...) XLOGD("[SFT_ut] "x, ##__VA_ARGS__)
#define LOGI(x, ...) XLOGI("[SFT_ut] "x, ##__VA_ARGS__)
#define LOGW(x, ...) XLOGW("[SFT_ut] "x, ##__VA_ARGS__)
#define LOGE(x, ...) XLOGE("[SFT_ut] "x, ##__VA_ARGS__)

#define FRAME_NUM           4

const uint32_t gRGBColorTable[] = {0x7F0000FF, 0x7F00FF00, 0x7FFF0000, 0x7FFFFFFF,
                                   0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF};
#define TABLE_SIZE (sizeof(gRGBColorTable)/sizeof(uint32_t))
#define DEFAULT_FIXED_COLOR 0x7F00FFFF

SfUtProc::SfUtProc() :
    mId(0),
    mFixedColor(DEFAULT_FIXED_COLOR),
    mPrintDetailLog(false)
{
}

SfUtProc::~SfUtProc()
{
}

void SfUtProc::onFirstRef()
{
    LOGD("onFirstRef");
    run("SfUtProc", PRIORITY_DISPLAY);
}

status_t SfUtProc::readyToRun()
{
    LOGD("readyToRun");
    return NO_ERROR;
}

bool SfUtProc::threadLoop()
{
    char value[PROPERTY_VALUE_MAX];
    bool isUpdate = false;
    uint32_t ssize = 0;
    {
        Mutex::Autolock _l(mLock);
        //LOGD("threadLoop");
        sp<ANativeWindow> window;

        ssize = surfaceList.size();
        static uint32_t frameCount = 0;

        property_get("debug.sftest.count", value, "-1");
        int _count = atoi(value);
        uint32_t color;

        LOGV("list size = %d, frameCount=%d", ssize, frameCount);

        for (uint32_t i = 0; i < ssize; i++) {
            bool reDraw = true;
            sp<SurfaceItem> item = surfaceList[i];
            if (frameCount > FRAME_NUM * ssize)
                reDraw = false;

            if (!item->isUpdateScreen) {
                color = mFixedColor;
                continue;
            } else {
                if (_count >= 0) {
                    reDraw = true;
                    frameCount = 0;
                    color = gRGBColorTable[(_count+i) % TABLE_SIZE];
                }
                else {
                    if (item->colorIndex >= 0)
                        color = gRGBColorTable[item->colorIndex % TABLE_SIZE];
                    else
                        color = gRGBColorTable[(frameCount+i) % TABLE_SIZE];
                }
            }
            LOGV("render[%d], color=%x", i, color);
            window = item->window;
            render(window.get(), color, reDraw);
        }

        if (ssize > 0) {
            frameCount++;
            isUpdate = true;
        } else {
            frameCount = 0;
        }
    }

    if (isUpdate)
    {
        if (true == mFpsCounter.update()) {
            LOGI("Layer[n=%d]: fps=%.2f dur=%.2f max=%.2f min=%.2f",
                ssize,
                mFpsCounter.getFps(),
                mFpsCounter.getLastLogDuration() / 1e6,
                mFpsCounter.getMaxDuration() / 1e6,
                mFpsCounter.getMinDuration() / 1e6);

            if (mFpsCounter.getFps() < 50)
                mPrintDetailLog = true;
            else
                mPrintDetailLog = false;

            property_get("debug.sftest.log", value, "0");
            mPrintDetailLog |= atoi(value);

        }
        usleep(1 * 1000);
    }
    else
        usleep(1000 * 1000);
    return true;
}

void SfUtProc::connect(int id, sp<ANativeWindow> window, bool useDefaultSize, int w, int h, bool updateScreen, int api, int format, int colorIndex)
{
    Mutex::Autolock _l(mLock);

    sp<SurfaceItem> item = new SurfaceItem();
    item->window = window;
    item->colorIndex = colorIndex;
    item->isUpdateScreen = updateScreen;
    surfaceList.add(id, item);
    init(window.get(), useDefaultSize, w, h, api, format);
    render(window.get(), mFixedColor);

    LOGI("connect, id=%d, window=%p", id, window.get());
}

void SfUtProc::disconnect(int id)
{
    Mutex::Autolock _l(mLock);

    sp<SurfaceItem> item;
    sp<ANativeWindow> window;
    item = surfaceList.valueFor(id);
    if (item == NULL) {
        LOGE("disconnect fail, id=%d", id);
        return;
    }
    window = item->window;
    native_window_api_disconnect(window.get(), NATIVE_WINDOW_API_CPU);
    surfaceList.removeItem(id);

    uint32_t ssize = surfaceList.size();
    LOGI("disconnect, id=%d, window=%p, surface size=%d", id, window.get(), ssize);
}

float SfUtProc::getFps()
{
    return mFpsCounter.getFps();
}

void SfUtProc::init(ANativeWindow *window, bool useDefaultSize, int requestWidth, int requestHeight, int api, int format)
{
    char value[PROPERTY_VALUE_MAX];

    // set api connection type as register
//    native_window_api_connect(window, NATIVE_WINDOW_API_CPU);
    native_window_api_connect(window, api);

    // set buffer size
    int w = requestWidth;
    int h = requestHeight;
    int width, height;
    window->query(window, NATIVE_WINDOW_DEFAULT_WIDTH, &width);
    window->query(window, NATIVE_WINDOW_DEFAULT_HEIGHT, &height);

    LOGI("init, win=%p, default [w, h]=(%d x %d), use default size=%d, api=%d, format=%x",
        window, width, height, useDefaultSize, api, format);
    if (useDefaultSize) {
        w = width;
        h = height;
    }

    native_window_set_buffers_dimensions(window, w, h);

    // set format
//    native_window_set_buffers_format(window, HAL_PIXEL_FORMAT_RGBA_8888);
    native_window_set_buffers_format(window, format);

    // set usage software write-able and hardware texture bind-able
    native_window_set_usage(window, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

    // set scaling to match window display size
    native_window_set_scaling_mode(window, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    // set buffer rotation
    property_get("debug.sftest.orientation", value, "0");
    int orientation = atoi(value);
    switch (orientation)
    {
        case 1:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_ROT_90);
            LOGD("rot 90");
            break;

        case 2:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_ROT_180);
            LOGD("rot 180");
            break;

        case 3:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_ROT_270);
            LOGD("rot 270");
            break;

        case 4:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_FLIP_H);
            LOGD("flip H");
            break;

        case 5:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_FLIP_V);
            LOGD("flip V");
            break;

        case 6:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_ROT_90 | HAL_TRANSFORM_FLIP_H);
            LOGD("rot 90 + flip H");
            break;

        case 7:
            native_window_set_buffers_transform(window, HAL_TRANSFORM_ROT_90 | HAL_TRANSFORM_FLIP_V);
            LOGD("rot 90 + flip V");
            break;

        default:
            LOGD("rot 0 and no flip");
    }

    // set buffer count
    native_window_set_buffer_count(window, FRAME_NUM);
}

void SfUtProc::render(ANativeWindow *window, uint32_t color, bool reDraw)
{
    char value[PROPERTY_VALUE_MAX];
    uint64_t checkPoint[10] = {0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0};

    ANativeWindowBuffer *buf;
    sp<GraphicBuffer>   gb;
    void                *ptr;

    int err;
    int fenceFd = -1;

    {
        if (mPrintDetailLog) checkPoint[0] = systemTime();
        err = window->dequeueBuffer(window, &buf, &fenceFd);                     // dequeue to get buffer handle
        if (mPrintDetailLog) checkPoint[1] = systemTime();
    }

    {
        sp<Fence> fence1(new Fence(fenceFd));
        fence1->wait(Fence::TIMEOUT_NEVER);
        if(err) {
            LOGE("%s", strerror(-err));
        }
        if (mPrintDetailLog) checkPoint[2] = systemTime();
    }

    gb = new GraphicBuffer(buf, false);
    const Rect rect(gb->getWidth(), gb->getHeight());

    // set extra buff value
#ifdef USE_GRALLOC_EXTRA
    {
        gralloc_extra_ion_sf_info_t sf_info;

        gralloc_extra_query(gb->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        gralloc_extra_sf_set_status(&sf_info,
            GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
            GRALLOC_EXTRA_BIT_DIRTY);

        gralloc_extra_perform(gb->handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
    }
#endif

    LOGV("gb info, w=%d, h=%d, s=%d, f=%d, bpp=%d, color=%x",
        gb->getWidth(), gb->getHeight(), gb->getStride(), gb->getPixelFormat(), getBitPerPixel(gb->getPixelFormat()), color);

    if (reDraw) {
        if (mPrintDetailLog) checkPoint[3] = systemTime();
        gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
        if (mPrintDetailLog) checkPoint[4] = systemTime();
        {
            ssize_t bpp = getBitPerPixel(gb->getPixelFormat());
            if (bpp == 32)
                android_memset32((uint32_t*)ptr, color, gb->getStride() * gb->getHeight() * bpp / 8);
            else if (bpp == 16)
                android_memset16((uint16_t*)ptr, (uint16_t)color, gb->getStride() * gb->getHeight() * bpp / 8);
            else if (bpp != BAD_VALUE)
                memset((void*)ptr, 0x00, gb->getStride() * gb->getHeight() * bpp / 8);
        }
        gb->unlock();
        if (mPrintDetailLog) checkPoint[5] = systemTime();
    } else {
        if (mPrintDetailLog)
            checkPoint[5] = checkPoint[4]= checkPoint[3] = systemTime();
    }

    err = window->queueBuffer(window, buf, -1);                                    // queue to display

    if (mPrintDetailLog) checkPoint[6] = systemTime();

    if (mPrintDetailLog) {
        const size_t SIZE = 256;
        char b[SIZE];
        snprintf(b, SIZE, "Total Time=%lld ms\n"
                          "         dequeue=%lld, \n"
                          "         wait-fence=%lld, \n"
                          "         new gb=%lld \n"
                          "         lock=%lld \n"
                          "         memset & unlock=%lld \n"
                          "         queue=%lld \n",
                          ns2ms(checkPoint[6] - checkPoint[0]),
                          ns2ms(checkPoint[1] - checkPoint[0]),
                          ns2ms(checkPoint[2] - checkPoint[1]),
                          ns2ms(checkPoint[3] - checkPoint[2]),
                          ns2ms(checkPoint[4] - checkPoint[3]),
                          ns2ms(checkPoint[5] - checkPoint[4]),
                          ns2ms(checkPoint[6] - checkPoint[5]));
        LOGD("%s", b);
    }
}

int SfUtProc::getBitPerPixel(int format)
{
    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_sRGB_A_8888:
        case HAL_PIXEL_FORMAT_sRGB_X_8888:
            return 32;
        case HAL_PIXEL_FORMAT_RGB_888:
            return 24;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
//        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_Y16:
            return 16;
//        case HAL_PIXEL_FORMAT_I420:
//        case HAL_PIXEL_FORMAT_NV12_BLK:
//        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
//        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_YV12:
            return 12;
        case HAL_PIXEL_FORMAT_Y8:
            return 8;
    }
    return BAD_VALUE;
}

}
