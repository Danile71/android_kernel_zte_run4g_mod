#define LOG_TAG "autoTestSurface"

#include <stdio.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/DisplayInfo.h>

using namespace android;

status_t main(int argc, char** argv) {
    int isPrint;
    int dequeueCnt;
    int loopCnt;
    int err;
    int sleepus;

    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    err = true;
    if (argc == 5) {
        sscanf(argv[1], "%d", &dequeueCnt);
        sscanf(argv[2], "%d", &loopCnt);
        sscanf(argv[3], "%d", &isPrint);
        sscanf(argv[4], "%d", &sleepus);

        if ((dequeueCnt > 0) && (dequeueCnt < 20) && (loopCnt > 0) && (isPrint >= 0) && (sleepus >= 0)) {
            err = false;
        }   
    } 
        
    if (err) {
         printf("\nautoIonFdLifeCycleUnitTest [ 0 < dequeueCnt < 20 ] [ 0 < looCnt ] [ 0 < isPrint ] [ 0 < sleepus ]\n\n");
         return NO_ERROR;
    }

    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    DisplayInfo dinfo;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
    SurfaceComposerClient::getDisplayInfo(display, &dinfo);

    for (int loop = 0 ;loop < loopCnt; loop++) {
        printf("[LOOP] %d / %d  [dequeueCnt] %d\n", loop, loopCnt, dequeueCnt);

        sp<SurfaceControl> surfaceControl = client->createSurface(
            String8("autoTestSurface"), dinfo.w - 100, dinfo.h - 100, PIXEL_FORMAT_RGB_565);

        sp<Surface> surface = surfaceControl->getSurface();
        ANativeWindow* window = surface.get();

        SurfaceComposerClient::openGlobalTransaction();
        surfaceControl->setPosition(50, 50);
        surfaceControl->setLayer(100000);
        SurfaceComposerClient::closeGlobalTransaction();
        //----------------------------------------------------------------------------
        int oft = (loop % 4) * 64;
        native_window_api_connect(window, 0);
        native_window_set_buffers_dimensions(window, 4096 - oft, 2048 - oft);
        native_window_set_buffers_format(window, HAL_PIXEL_FORMAT_YV12);
        native_window_set_usage(window, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);
        native_window_set_scaling_mode(window, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        native_window_set_buffer_count(window, dequeueCnt);
        
        ANativeWindowBuffer* buf[10];
        printf("[dequeue] ...\n");
        for (int i = 0; i < dequeueCnt; i++) {
            int fenceFd = -1;

            if (isPrint){
                printf("[dequeue] %d\n", i);
            }
            err = window->dequeueBuffer(window, &buf[i], &fenceFd);
            sp<Fence> fence1(new Fence(fenceFd));
            fence1->wait(Fence::TIMEOUT_NEVER); 
            if(err) {
                printf("dequeue err!!\n");
            }
        }

        printf("[queue] ...\n");
        for (int i = 0; i < dequeueCnt; i++) {
            if (isPrint) {
                printf("[queue] %d\n", i);
            }
            err = window->queueBuffer(window, buf[i], -1);
            if(err) {
                printf("queue err!!\n");
            }
            usleep(sleepus);
        }
        printf("[queue] ... OK\n");
        native_window_api_disconnect(window, 0);

    }
    return NO_ERROR;

}
