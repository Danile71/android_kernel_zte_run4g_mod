#define LOG_TAG "stresstool"

#include <stdio.h>
#include <getopt.h>

#include <log/log.h>

#include <utils/List.h>
#include <utils/Vector.h>

#include <gui/ISurfaceComposerClient.h>
#include <ui/DisplayInfo.h>
#include <ui/gralloc_extra.h>

#include "Stretegy.h"
#include "Frame.h"
#include "TestSurface.h"
#include "Action.h"
#include "App.h"
#include <ui/GraphicBufferExtra.h>
//#include "Monkey.h"

using namespace android;

inline int getBitsPerPixel(int format) {
    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        //case SGX_COLOR_FORMAT_BGRX_8888:
            return 32;

        case HAL_PIXEL_FORMAT_RGB_565:
            return 16;

        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YV12:
            return 12;
    }
    return 0;
}

Frame::Config bgCfg[] =
{
    //{String8("/data/LGE.yv12"),                  400,  416,  240, 0, HAL_PIXEL_FORMAT_YV12,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/LGE.yv12"),                  400,  416,  240, 0, HAL_PIXEL_FORMAT_YV12,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    //{String8("/data/black.yv12"),                  16,  16,  2, 0, HAL_PIXEL_FORMAT_YV12,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    //{String8("/data/black.rgba"),                  1,  1,  1, 0, HAL_PIXEL_FORMAT_RGBX_8888,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
};

Frame::Config framesCfg[] =
{
    {String8("/data/LGE.yv12"),                  400,  416,  240, 0, HAL_PIXEL_FORMAT_YV12,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/BigShips_1280x720_1.i420"), 1280, 1280,  720, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/football_qvga_1.i420"),      320,  320,  240, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/indoor_slow_1.i420"),        848,  848,  480, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/Kimono1_1920x1088_1.i420"), 1920, 1920, 1088, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/mobile_qcif_1.i420"),        176,  176,  144, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/newyork_640x368_1.i420"),    640,  640,  368, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/out_176_144.i420"),          176,  176,  144, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/school_640x480_1.i420"),     640,  640,  480, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {String8("/data/prv03.i420"),                640,  640,  480, 0, HAL_PIXEL_FORMAT_I420,       NATIVE_WINDOW_API_CAMERA,  GRALLOC_EXTRA_BIT_TYPE_CAMERA},

    // because the MTKYUB raw data does not contain padding bits,
    // keep the stride be the same value as width.
    {String8("/data/ibmbw_720x480_mtk.yuv"),     720,  720,  480, 0, HAL_PIXEL_FORMAT_NV12_BLK,   NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
};

void createBackground(sp<TestSurface>* bgSurface, const Vector<sp<Frame> >& frames, const DisplayInfo& dinfo) {
    Vector<sp<Frame> > bg;
    TestSurface::Config cfg;
    TestSurface::State state;

    if (frames.size() == 0) {
        XLOGE(" size of  frames need to > 0");
    }
    bg.push_back(*(frames.begin()));

    cfg.mDispW = dinfo.w;
    cfg.mDispH = dinfo.h;
    cfg.mScale = true;
    cfg.mMove = true;
    cfg.mZOrder = true;
    cfg.mSurfaceCrop = true;
    cfg.mBufferCrop = true;
    cfg.mBlend = true;

    state.mX = 0;
    state.mY = 0;
    state.mW = dinfo.w;
    state.mH = dinfo.h;
    state.mZOrder = 300000;
    state.mAlpha = 1.f;
    state.mFlags = ISurfaceComposerClient::eOpaque;

    (*bgSurface) = new TestSurface(String8("test-bg"),
            bg,
            cfg,
            state);
    (*bgSurface)->beginFrame();
    SurfaceComposerClient::openGlobalTransaction();
    (*bgSurface)->refresh();
    SurfaceComposerClient::closeGlobalTransaction();
}

static void printHelpMessage() {
    printf("Available options:\n");
    printf("  -n[NUM], --num=[NUM]: the maximal surfaces\n");
    printf("  -m[MODE], --num=[MODE]: mode: free, clock\n");
    printf("  --minsize=[size]: the minimal size of surface after surface is changed\n");
}

int main(int argc, char* argv[]) {
    enum Mode{
        FREE,
        CLOCK,
    } mode = FREE;
    uint32_t surfaceNum = 1;

    const char* short_options = "n:m:";
    struct option long_options[] = {
        {"num", required_argument, NULL, 'n'},
        {"mode", required_argument, NULL, 'm'},
        {"help", no_argument, NULL, 'h'},
        {"minsize", required_argument, NULL, 1001},
    };

    int ch = 0;
    uint32_t minSize = 65;
    while ((ch = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (ch) {
            case 'n':
                surfaceNum = atoi(optarg);
                break;
            case 'm':
                if (strcmp(optarg, "free") == 0)
                    mode = Mode::FREE;
                else if (strcmp(optarg, "clock") == 0)
                    mode = Mode::CLOCK;
                break;
            case 'h':
                printHelpMessage();
                return 0;
            case 1001:
                minSize = atoi(optarg);
                break;
            default:
                printf("Invalid options\n");
                return 0;
        }
    }

    sp<App> app = new App();

    const unsigned int NUM_FRAMES = sizeof(framesCfg) / sizeof(Frame::Config);


    ALOGI("Load frames");
    Vector<sp<Frame> > frames;
    for (uint32_t i = 0; i < NUM_FRAMES; ++i) {
        uint32_t bpp = getBitsPerPixel(framesCfg[i].mFormat);
        if (bpp == 0) {
            XLOGE("Unknown format");
        }
        framesCfg[i].mPitch = framesCfg[i].mS * bpp / 8;
        sp<Frame> frame = new Frame(framesCfg[i]);
        if (frame == NULL) {
            XLOGE("create Frame failed");
        } else {
            frames.push_back(frame);
        }
    }

    Vector<Vector<sp<Frame> > > showFramesSet;
    for (uint32_t i = 0; i < surfaceNum; ++i) {
        Vector<sp<Frame> > showFrames;
        for (uint32_t i = 1; i < NUM_FRAMES; ++i) {
            showFrames.push_back(*(frames.begin() + i));
        }
        showFramesSet.push_back(showFrames);
    }

    DisplayInfo dinfo;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    SurfaceComposerClient::getDisplayInfo(display, &dinfo);

    sp<TestSurface> bgSurface = NULL;

    Vector<sp<Frame> > bgFrames;
    {
        uint32_t bpp = getBitsPerPixel(bgCfg[0].mFormat);
        if (bpp == 0) {
            XLOGE("Unknown format");
        }
        bgCfg[0].mPitch = bgCfg[0].mS * bpp / 8;
        sp<Frame> frame = new Frame(bgCfg[0]);
        if (frame == NULL) {
            XLOGE("create Frame failed");
        } else {
            bgFrames.push_back(frame);
        }
    }

    createBackground(&bgSurface, bgFrames, dinfo);

    ALOGI("Create Surface");
    Vector<sp<TestSurface> > surfaces;


    for (uint32_t i = 0; i < surfaceNum; ++i) {
        TestSurface::Config cfg;
        {
            cfg.mDispW = dinfo.w;
            cfg.mDispH = dinfo.h;
            cfg.mMinSize = minSize;
            cfg.mScale = true;
            cfg.mMove = true;
            cfg.mZOrder = true;
            cfg.mSurfaceCrop = true;
            cfg.mBufferCrop = true;
            cfg.mBlend = true;
        }
        TestSurface::State state;
        {
            state.mX = 0 + i * 100;
            state.mY = 0 + i * 100;
            state.mW = 400;
            state.mH = 400;
            state.mZOrder = 3000000 + i;
            state.mAlpha = 1.f;
        }
        app->addSurfaceData(new SurfaceData(String8("test"), cfg, state, showFramesSet[i]));
    }

    ALOGD("Run app");
    app->setActionFlag(App::ACTION_SCALE | App::ACTION_MOVE | App::ACTION_SHUFFLE | App::ACTION_BLEND);
    //app->setActionFlag(App::ACTION_CLOCKWISEMOVE);
    //app->setActionFlag(App::ACTION_JUMP);
    switch (mode) {
        case Mode::FREE:
            app->addStretegy(new FreeMove(60));
            app->addStretegy(new FreeScale(60));
            app->addStretegy(new Shuffle(60));
            app->addStretegy(new FreeBlend(60));
            app->addStretegy(new SelfBurst(120, app));
            break;
        case Mode::CLOCK:
            app->addStretegy(new ClockwiseMove(60));
    }
    app->run();
    app->join();
}
