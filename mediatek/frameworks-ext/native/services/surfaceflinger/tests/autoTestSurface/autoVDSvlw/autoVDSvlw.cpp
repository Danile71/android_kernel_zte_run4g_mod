/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "autoTestSurface"

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <cutils/properties.h>

#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferExtra.h>
#include <ui/gralloc_extra.h>

using namespace android;

#define DISPLAY_WIDTH  1080
#define DISPLAY_HEIGHT 1920




#define WINDOW_1_WIDTH  300
#define WINDOW_1_HEIGHT 300

#define WINDOW_2_WIDTH  300
#define WINDOW_2_HEIGHT 300


#define SF_LAYER_BG 21000

//low
//#define SF_LAYER_0 20100
//#define SF_LAYER_1 20200
//#define SF_LAYER_2 20300

#define SF_LAYER_0 22000
#define SF_LAYER_1 21019
#define SF_LAYER_2 24000
#define SF_LAYER_3 25000
#define SF_LAYER_4 26000

//#define SF_LAYER_0 221999
//#define SF_LAYER_1 222000
//#define SF_LAYER_2 223000

//#define UI_STATUSBAR_HEIGHT 75   
//#define UI_NAVIBAR_HEIGHT 144
#define UI_STATUSBAR_HEIGHT 0
#define UI_NAVIBAR_HEIGHT 0


static const uint32_t kMinBitRate = 100000;         // 0.1Mbps
static const uint32_t kMaxBitRate = 100 * 1000000;  // 100Mbps
static const uint32_t kMaxTimeLimitSec = 180;       // 3 minutes
static const uint32_t kFallbackWidth = 1280;        // 720p
static const uint32_t kFallbackHeight = 720;

// Command-line parameters.
static bool gVerbose = false;               // chatty on stdout
static bool gRotate = false;                // rotate 90 degrees
static bool gSizeSpecified = false;         // was size explicitly requested?
static uint32_t gVideoWidth  = 1080; //640;            // default width+height
static uint32_t gVideoHeight = 1920; //480;
static uint32_t gBitRate = 4000000;         // 4Mbps
static uint32_t gTimeLimitSec = kMaxTimeLimitSec;


struct FRAME {
    char name[128];     // file name
    uint32_t w;         // width
    uint32_t s;         // stride
    uint32_t h;         // height
    uint32_t fmt;       // format
    uint32_t api;       // api connection type
    uint32_t usageEx;
};

FRAME test_frames[] = {
    {"/data/vlw_w400_h256_s400_nv12_blk.yuv"   ,     400, 400, 256, HAL_PIXEL_FORMAT_NV12_BLK,   NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/AOA_w1920_h1088_s1920_nv12_blk.yuv",     1920, 1920, 1088, HAL_PIXEL_FORMAT_NV12_BLK,   NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
                                                                                                                      
    {"/data/LGE.yv12",                  400,  416,  240, HAL_PIXEL_FORMAT_YV12,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/BigShips_1280x720_1.i420", 1280, 1280,  720, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/football_qvga_1.i420",      320,  320,  240, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/indoor_slow_1.i420",        848,  848,  480, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/Kimono1_1920x1088_1.i420", 1920, 1920, 1088, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/mobile_qcif_1.i420",        176,  176,  144, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/newyork_640x368_1.i420",    640,  640,  368, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/out_176_144.i420",          176,  176,  144, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/school_640x480_1.i420",     640,  640,  480, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    {"/data/prv03.i420",                640,  640,  480, HAL_PIXEL_FORMAT_I420,   NATIVE_WINDOW_API_CAMERA ,  GRALLOC_EXTRA_BIT_TYPE_CAMERA},
    
    
    // because the MTKYUB raw data does not contain padding bits,
    // keep the stride be the same value as width.
    {"/data/ibmbw_720x480_mtk.yuv",     720,  720,  480, HAL_PIXEL_FORMAT_NV12_BLK,   NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    //{"/data/VT_top_w176_h144_s192.yv12", 176,  192,  144, HAL_PIXEL_FORMAT_YV12,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    //{"/data/VT_bot_w176_h144_s192.yv12", 176,  192,  144, HAL_PIXEL_FORMAT_YV12,   NATIVE_WINDOW_API_MEDIA  ,   GRALLOC_EXTRA_BIT_TYPE_VIDEO},
    
};

FRAME tesUIframes[] = {
    //{"/data/VT_dialer_w1080_h1776_s1088.raw",  1080,  1088,  1776, HAL_PIXEL_FORMAT_RGBA_8888,   0  ,   0},    
    {"/data/bg_grey_64x64_rgba.raw",  64,  64,  64, HAL_PIXEL_FORMAT_RGBX_8888,   0  ,   0},    
    //{"/data/bg_grey_64x64_rgba.raw",  64,  64,  64, HAL_PIXEL_FORMAT_RGB_888,   0  ,   0},    
    //{"/data/bg_grey_64x64_rgba.raw",  64,  64,  64, HAL_PIXEL_FORMAT_RGB_565,   0  ,   0},    
};



struct sfRect {
  uint32_t en;   
  //uint32_t x ;  
  //uint32_t y ;   
  //uint32_t width ; 
  //uint32_t height ;
  uint32_t left ;
  uint32_t top ;  
  uint32_t right ;
  uint32_t bottom ;
};


typedef struct testConfig {
    uint32_t testEn ;
    uint32_t z_order ; // enable when z_order > 0
    sfRect srcCrop;
    sfRect dstRect;
    //sfRect dstCrop;
} testConfig;


testConfig cfgWindow[][5] = {

    {
       { 1, SF_LAYER_1, { 1, 0, 0, 135, 240}, { 1,  0, 0, 1080, 1920}}, // 720p VT call video layer
       //{ 1, SF_LAYER_1, { 0, 0, 0, 0, 0}, { 1,  8, 260, 720, 849}}, // 720p VT call video layer
       //{ 1, SF_LAYER_1, { 0, 0, 0, 0, 0}, { 1,  0, 390, 1080, 1273}}, // 1080p VT call video layer
       { 0, SF_LAYER_1, { 0, 0, 0, 0, 0}, { 1,  1, 390, 1079, 1273}}, // 1080p VT call video layer
       //no_use
       { 0, SF_LAYER_1, { 0, 0, 0, 0, 0}, { 1, 50, 250, WINDOW_1_WIDTH-1, WINDOW_1_HEIGHT-1}}, 
       { 0, SF_LAYER_1, { 0, 0, 0, 0, 0}, { 1, DISPLAY_WIDTH-WINDOW_1_WIDTH, DISPLAY_HEIGHT-WINDOW_1_HEIGHT, WINDOW_1_WIDTH-1, WINDOW_1_HEIGHT-1}},  //[ALPS01478586][Sanity][VT call]
       { 0, SF_LAYER_1, { 0, 0, 0, 0, 0}, { 1, 0, 0, 0, 0}}
    },
    
    {
      { 1, SF_LAYER_2, { 0, 0, 0, 0, 0}, { 1, 8, 857, 417, 1184}}, // 720p VT call video layer
      { 0, SF_LAYER_2, { 0, 0, 0, 0, 0}, { 1, 0, 1285, 600, 1776}}, // 1080p VT call video layer
      //no_use
      { 0, SF_LAYER_2, { 0, 0, 0, 0, 0}, { 1, 400, 600, WINDOW_2_WIDTH-1, WINDOW_2_HEIGHT-1}},
      { 0, SF_LAYER_2, { 0, 0, 0, 0, 0}, { 1, 400, 600, WINDOW_2_WIDTH-1, WINDOW_2_HEIGHT-1}},  //[ALPS01478586][Sanity][VT call]
      { 0, SF_LAYER_2, { 0, 0, 0, 0, 0}, { 1, 0, 0, 0, 0}}
    }
    
};

// VT dialer
testConfig cfgUIwindow[][4] = {
    {
       //{ 1, SF_LAYER_0, { 0, 0, 0, 0, 0}, { 1,  8, 260, 720-8, 849-260}}, // 720p VT call UI layer
       //{ 1, SF_LAYER_0, { 1, 0, 75, 1080, 1776-75}, { 1,  0, 75, 1080, 1776-75}}, // 1080p VT call UI layer
       { 1, SF_LAYER_0, { 1, 0, 0, 64, 64}, { 1,  0, 75, 1080, 1776-75}}, // 64x64 VT dialer UI layer
       
       { 0, SF_LAYER_0, { 1, 0, 0, 0, 0}, { 1,  0, 75, 1080, 1776-75}}, 
       { 0, SF_LAYER_0, { 0, 0, 0, 0, 0}, { 1,  0, 75, 1080, 1776-75}},  //[ALPS01478586][Sanity][VT call]
       { 0, SF_LAYER_0, { 0, 0, 0, 0, 0}, { 1, 0, 0, 0, 0}}
    },    
    
};

#define MAX_SURFACE_SIZE 2

const static int MAX_TEST_FRAMES = sizeof(test_frames) / sizeof(struct FRAME);

const static int TEST_FRAMES = 2 ;//sizeof(test_frames) / sizeof(struct FRAME);

//const static int TEST_SETTING_NUM = 1 ;

const static int TEST_SETTING_NUM = sizeof(cfgWindow) / (sizeof(testConfig)*MAX_SURFACE_SIZE) ;

static bool isDeviceRotated(int orientation) {
    return orientation != DISPLAY_ORIENTATION_0 &&
            orientation != DISPLAY_ORIENTATION_180;
}

//
// use FRAME data to dispay with an ANativeWindow
// as we postBuffer before
//
status_t showTestFrame(ANativeWindow *w, const FRAME &f, int loop) { //, int repeat_times
    
    
    char value[PROPERTY_VALUE_MAX];
    int i ;

    // set api connection type as register
    //native_window_api_connect(w, 0);

    for (i = 0 ; i < loop ; i++){
        
        printf("\nShowFrame loop_idx %d ... ", i );
        // set buffer size
        native_window_set_buffers_dimensions(w, f.s, f.h);
        
        // set format
        native_window_set_buffers_format(w, f.fmt);
        
        // set usage software write-able and hardware texture bind-able
        native_window_set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);
        //native_window_set_usage(w, GRALLOC_USAGE_HW_VIDEO_ENCODER);
        
        
        // set scaling to match window display size
        native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        
        // set buffer rotation
        property_get("debug.sftest.orientation", value, "0");
        int orientation = atoi(value);
        switch (orientation)
        {
            case 1:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);
                ALOGD("rot 90");
                break;
        
            case 2:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_180);
                ALOGD("rot 180");
                break;
        
            case 3:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_270);
                ALOGD("rot 270");
                break;
        
            case 4:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_FLIP_H);
                ALOGD("flip H");
                break;
        
            case 5:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_FLIP_V);
                ALOGD("flip V");
                break;
        
            case 6:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90 | HAL_TRANSFORM_FLIP_H);
                ALOGD("rot 90 + flip H");
                break;
        
            case 7:
                native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90 | HAL_TRANSFORM_FLIP_V);
                ALOGD("rot 90 + flip V");
                break;
        
            default:
                ALOGD("rot 0 and no flip");
        }
        
        // set buffer count
        native_window_set_buffer_count(w, 10);
        
        ANativeWindowBuffer *buf;
        sp<GraphicBuffer>   gb;
        void                *ptr;
        const Rect          rect(f.w, f.h);
        
        int err;
        int fenceFd = -1;
        err = w->dequeueBuffer(w, &buf, &fenceFd);                     // dequeue to get buffer handle
        sp<Fence> fence1(new Fence(fenceFd));
        fence1->wait(Fence::TIMEOUT_NEVER);
        if(err) {
            ALOGE("%s", strerror(-err));
        }
        
        gb = new GraphicBuffer(buf, false);
        
        // set api type
        GraphicBufferExtra::get().setBufParameter(gb->handle, GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY, f.usageEx | GRALLOC_EXTRA_BIT_DIRTY);
        
        
        gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
        {
            FILE *file = fopen(f.name, "rb");                // read file into buffer
        
            if (file != NULL)
            {
                if(f.fmt == HAL_PIXEL_FORMAT_YV12 || f.fmt == HAL_PIXEL_FORMAT_I420 || f.fmt == HAL_PIXEL_FORMAT_NV12_BLK )
                  fread(ptr, f.s * f.h * 3 / 2, 1, file);      // bpp is 1.5
                else if(f.fmt == HAL_PIXEL_FORMAT_RGBA_8888 || f.fmt == HAL_PIXEL_FORMAT_RGBX_8888)
                  fread(ptr, f.s * f.h * 4, 1, file);      // bpp is 4
                else if(f.fmt == HAL_PIXEL_FORMAT_RGB_888)
                  fread(ptr, f.s * f.h * 3, 1, file);      // bpp is 4 
                else if(f.fmt == HAL_PIXEL_FORMAT_RGB_565)
                  fread(ptr, f.s * f.h * 2, 1, file);      // bpp is 2
            }
            else
            {
                ALOGE("open file %s failed", f.name);
            }
        
            fclose(file);
        }
        gb->unlock(); 
        
        err = w->queueBuffer(w, buf, -1);                                    // queue to display
        sp<Fence> fence2(new Fence(fenceFd));
        fence2->wait(Fence::TIMEOUT_NEVER);
        if(err) {
            ALOGE("%s", strerror(-err));
        }
        
        property_get("debug.sftest.sleep", value, "16");
        int delay = atoi(value);
        
        usleep(delay * 1000);
    }

    // disconnect as unregister
    //native_window_api_disconnect(w, 0);

    return NO_ERROR;
}




static status_t prepareVirtualDisplay(const DisplayInfo& mainDpyInfo,
        const sp<IGraphicBufferProducer>& bufferProducer,
        sp<IBinder>* pDisplayHandle) {
    status_t err;

    // Set the region of the layer stack we're interested in, which in our
    // case is "all of it".  If the app is rotated (so that the width of the
    // app is based on the height of the display), reverse width/height.
    bool deviceRotated = isDeviceRotated(mainDpyInfo.orientation);
    uint32_t sourceWidth, sourceHeight;
    if (!deviceRotated) {
        sourceWidth = mainDpyInfo.w;
        sourceHeight = mainDpyInfo.h;
    } else {
        ALOGV("using rotated width/height");
        sourceHeight = mainDpyInfo.w;
        sourceWidth = mainDpyInfo.h;
    }
    ALOGD("prepareVirtualDisplay: orientation %d, w/h %d %d!!", mainDpyInfo.orientation, mainDpyInfo.w, mainDpyInfo.h);
    
    Rect layerStackRect(sourceWidth, sourceHeight);

    // We need to preserve the aspect ratio of the display.
    float displayAspect = (float) sourceHeight / (float) sourceWidth;


    // Set the way we map the output onto the display surface (which will
    // be e.g. 1280x720 for a 720p video).  The rect is interpreted
    // post-rotation, so if the display is rotated 90 degrees we need to
    // "pre-rotate" it by flipping width/height, so that the orientation
    // adjustment changes it back.
    //
    // We might want to encode a portrait display as landscape to use more
    // of the screen real estate.  (If players respect a 90-degree rotation
    // hint, we can essentially get a 720x1280 video instead of 1280x720.)
    // In that case, we swap the configured video width/height and then
    // supply a rotation value to the display projection.

    gVideoWidth = sourceWidth ;
    gVideoHeight = sourceHeight ;

    uint32_t videoWidth, videoHeight;
    uint32_t outWidth, outHeight;
    if (!gRotate) {
        videoWidth = gVideoWidth;
        videoHeight = gVideoHeight;
    } else {
        videoWidth = gVideoHeight;
        videoHeight = gVideoWidth;
    }
    if (videoHeight > (uint32_t)(videoWidth * displayAspect)) {
        // limited by narrow width; reduce height
        outWidth = videoWidth;
        outHeight = (uint32_t)(videoWidth * displayAspect);
    } else {
        // limited by short height; restrict width
        outHeight = videoHeight;
        outWidth = (uint32_t)(videoHeight / displayAspect);
    }
    uint32_t offX, offY;
    offX = (videoWidth - outWidth) / 2;
    offY = (videoHeight - outHeight) / 2;
    Rect displayRect(offX, offY, offX + outWidth, offY + outHeight);

    if (gVerbose) {
        if (gRotate) {
            printf("Rotated content area is %ux%u at offset x=%d y=%d\n",
                    outHeight, outWidth, offY, offX);
        } else {
            printf("Content area is %ux%u at offset x=%d y=%d\n",
                    outWidth, outHeight, offX, offY);
        }
    }


    sp<IBinder> dpy = SurfaceComposerClient::createDisplay(
            String8("ScreenRecorder"), false /* secure */);

    SurfaceComposerClient::openGlobalTransaction();
    SurfaceComposerClient::setDisplaySurface(dpy, bufferProducer);
    SurfaceComposerClient::setDisplayProjection(dpy,
            gRotate ? DISPLAY_ORIENTATION_90 : DISPLAY_ORIENTATION_0,
            layerStackRect, displayRect);
    SurfaceComposerClient::setDisplayLayerStack(dpy, 0);    // default stack
    SurfaceComposerClient::closeGlobalTransaction();

    *pDisplayHandle = dpy;

    return NO_ERROR;
}

// ./run_test [scenario_num] [surface_num] [queue_frame_num] [show] [pause_end]
// autoTestSurface testFrames testSurface
status_t main(int argc, char** argv) {


    int c;
    
    int test_scenario_num = MAX_TEST_FRAMES;
    int test_surface_num = 2 ;
    int test_fill_surface_times = 1 ;
    int test_blocking_end = 0 ;
    int test_vds_show = 0;
    
    printf("argc %d!!\n", argc);
    
    if( argc > 1 )
      test_scenario_num = atoi(argv[1]) ;

    if( argc > 2 )
      test_surface_num = atoi(argv[2]) ;

    if( argc > 3 )
      test_fill_surface_times = atoi(argv[3]) ;
      
    if( argc > 4 )
      test_vds_show = atoi(argv[4]) ; 
        
    if( argc > 5 )
      test_blocking_end = atoi(argv[5]) ; 
      
    printf("get test cmd: scenario_num %d, create_surface_num %d, fill_surface_times %d!!\n", test_scenario_num, test_surface_num, test_fill_surface_times);

#if 1    
    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    if(test_blocking_end){
      ProcessState::self()->startThreadPool();
    }
#endif
    
    // create a client to surfaceflinger
    
    
   //=============================================================================    
   //=============================================================================    
    
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    DisplayInfo dpyInfo;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
        ISurfaceComposer::eDisplayIdMain);
    SurfaceComposerClient::getDisplayInfo(display, &dpyInfo);
    uint32_t dispw = dpyInfo.w;
    uint32_t disph = dpyInfo.h;

    if(dispw != 1080 || disph != 1920){
      printf("current not support size (%d %d)", dispw, disph);   
    }
        

    //===============================================================




    sp<SurfaceControl> surfaceControllerUI;
    sp<Surface> surfaceUI ;
    ANativeWindow* windowUI ;
    testConfig *pUITestConfig = (testConfig *)malloc(sizeof(testConfig)); //&cfgUIwindow[0][0] ;
    FRAME *dialer = &tesUIframes[0] ;

    pUITestConfig->dstRect.left      = 0 ;
    pUITestConfig->dstRect.top      = UI_STATUSBAR_HEIGHT ;
    pUITestConfig->dstRect.right  = dpyInfo.w ;
    pUITestConfig->dstRect.bottom = dpyInfo.h - UI_NAVIBAR_HEIGHT ;//+ UI_STATUSBAR_HEIGHT ) ;
    pUITestConfig->z_order        = SF_LAYER_BG ;

    surfaceControllerUI = client->createSurface( String8("autoTestUI"), 
    pUITestConfig->dstRect.right - pUITestConfig->dstRect.left, pUITestConfig->dstRect.bottom - pUITestConfig->dstRect.top, PIXEL_FORMAT_RGBX_8888);
    //pUITestConfig->dstRect.right - pUITestConfig->dstRect.left, pUITestConfig->dstRect.bottom - pUITestConfig->dstRect.top, PIXEL_FORMAT_RGB_565);
    //dpyInfo.w, dpyInfo.h, PIXEL_FORMAT_RGBA_8888);

    surfaceUI = surfaceControllerUI->getSurface();
    windowUI = surfaceUI.get();
    android_native_rect_t rect ;
    
    rect.left   = 0;        //pUITestConfig->srcCrop.x ;
    rect.top    = 0;        //pUITestConfig->srcCrop.y ;
    rect.right  = dialer->w; //pUITestConfig->srcCrop.x+pUITestConfig->srcCrop.width;
    rect.bottom = dialer->h; //pUITestConfig->srcCrop.y+pUITestConfig->srcCrop.height ;




#if 1
    SurfaceComposerClient::openGlobalTransaction();
        
    surfaceControllerUI->setPosition( pUITestConfig->dstRect.left , pUITestConfig->dstRect.top ); 
    //surfaceControllerUI->setSize(pUITestConfig->dstRect.width, pUITestConfig->dstRect.height); 
    surfaceControllerUI->setLayer(pUITestConfig->z_order);
    //surfaceControllerUI->setAlpha(1);
    //surfaceControllerUI->setFlags(1,1) ;
     
    SurfaceComposerClient::closeGlobalTransaction();

    native_window_api_connect(windowUI , 0);
    
    native_window_set_crop(windowUI, &rect);
        
    showTestFrame( windowUI , tesUIframes[0] , 2);
    
    native_window_api_disconnect(windowUI, 0);
#endif
    
    free(pUITestConfig);

    //===============================================================

    
    // 720p
    // uint32_t SurfaceDstWidth  = 720-8;  //300 -1;
    // uint32_t SurfaceDstHeight = 849-260;//300 -1;
    // 
    // uint32_t  SurfaceDstWidth_1  = 417-8;  //300 -1;
    // uint32_t SurfaceDstHeight_1 = 1184-857;//300 -1;

    // 1080p
    uint32_t SurfaceDstWidth     = 1079   ;  //300 -1;
    uint32_t SurfaceDstHeight    = 1273-390 ;//300 -1;
    
    uint32_t  SurfaceDstWidth_1  = 600   ;  //300 -1;
    uint32_t SurfaceDstHeight_1  = 1776-1285;//300 -1;







    ALOGD("display (w,h):(%d,%d)", dispw, disph);
    printf("display (w,h):(%d,%d)", dispw, disph);
    
    sp<SurfaceControl> surfaceController[MAX_SURFACE_SIZE];
    sp<Surface> surface[MAX_SURFACE_SIZE] ;
    ANativeWindow* window[MAX_SURFACE_SIZE] ;

    surfaceController[0] = client->createSurface( String8("autoTestSurface_1"), dpyInfo.w, dpyInfo.h, PIXEL_FORMAT_RGB_565);

    surface[0] = surfaceController[0]->getSurface();
    window[0] = surface[0].get();


   //=============================================================================    
   //=============================================================================    
   


    surfaceController[1] = client->createSurface( String8("autoTestSurface_2"), dpyInfo.w, dpyInfo.h, PIXEL_FORMAT_RGB_565);

    surface[1] = surfaceController[1]->getSurface();
    window[1] = surface[1].get();


   //=============================================================================    
   //                          VDS configuration
   //=============================================================================    
   

    //sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    sp<SurfaceControl> surfaceControllerVDS = client->createSurface( String8("autoTestVDS"),  gVideoWidth, gVideoHeight, PIXEL_FORMAT_RGBA_8888);
    sp<Surface> surfaceVDS = surfaceControllerVDS->getSurface();
    sp<IGraphicBufferProducer> bufferProducer = surfaceVDS->getIGraphicBufferProducer();
    ANativeWindow* windowVDS = surfaceVDS.get();
    

    native_window_api_connect(windowVDS , 0);



    native_window_set_buffers_dimensions(windowVDS, gVideoWidth, gVideoHeight);
    
    // set format
    native_window_set_buffers_format(windowVDS, HAL_PIXEL_FORMAT_RGBA_8888);
    
    // set usage software write-able and hardware texture bind-able
    native_window_set_usage(windowVDS, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);
    //native_window_set_usage(windowVDS, GRALLOC_USAGE_HW_VIDEO_ENCODER);
    
    // set scaling to match window display size
    native_window_set_scaling_mode(windowVDS, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    
    native_window_set_buffer_count(windowVDS, 5) ;
    

    sp<IBinder> dpy;
    //err = 
    prepareVirtualDisplay(dpyInfo, bufferProducer, &dpy);
    Rect crop(320 ,0, 960, 720);

    SurfaceComposerClient::openGlobalTransaction();
        
    surfaceControllerVDS->setPosition( 0 , 900 ); 
    //surfaceControllerVDS->setSize( 320 , 360);  
    //surfaceControllerVDS->setSize( 480 , 540);  
    //surfaceControllerVDS->setCrop(crop);
    surfaceControllerVDS->setLayer(27000); 
     
    SurfaceComposerClient::closeGlobalTransaction(); 

    if(test_vds_show == 0){
      SurfaceComposerClient::openGlobalTransaction();
      surfaceControllerVDS->hide();
      SurfaceComposerClient::closeGlobalTransaction(); 
    }

   //=============================================================================    
   //=============================================================================    



    int test_scenario_idx = 0;
    int config_idx = 0;
    int surface_x = 0 ;
    int test_surface_idx = 0;
    


    
    //while(true) 
    for( ; test_scenario_idx < test_scenario_num && test_scenario_idx < MAX_TEST_FRAMES; test_scenario_idx++)
    //for( ; test_scenario_idx < 1; )//test_scenario_idx++)
    {
        
        for( test_surface_idx = 0; test_surface_idx < test_surface_num ; test_surface_idx++)
          native_window_api_connect(window[test_surface_idx] , test_frames[test_scenario_idx].api);
              
        
        //configure with different window setting
        for(config_idx = 0; config_idx < TEST_SETTING_NUM ;config_idx++) {

            // printf("\n[%d] set posistion %d %d ... ", test_scenario_idx,dispw - SurfaceDstWidth, disph - SurfaceDstHeight);
            // SurfaceComposerClient::openGlobalTransaction();
            // surfaceController[0]->setPosition( dispw - SurfaceDstWidth, disph - SurfaceDstHeight); //[ALPS01478586][Sanity][VT call]
            // surfaceController[0]->setLayer(100000);
            // 
            // //surfaceController[1]->setPosition(300,300); 
            // //surfaceController[1]->setLayer(110000);    
            // SurfaceComposerClient::closeGlobalTransaction();    


            //configure each surface 
            for( test_surface_idx = 0; test_surface_idx < test_surface_num ; test_surface_idx++) {  
                
                testConfig *pWindowCfg = &cfgWindow[test_surface_idx][config_idx] ;
                
                if( pWindowCfg->testEn ) {
                    
                    Rect cropRect(pWindowCfg->srcCrop.left, pWindowCfg->srcCrop.top, pWindowCfg->srcCrop.right, pWindowCfg->srcCrop.bottom);
                    
                    //if(config_idx > 0) 
                    printf("\n[%d][%d] set posistion %d %d %d %d (%d %d %d %d)... ", test_scenario_idx, config_idx, pWindowCfg->dstRect.left , pWindowCfg->dstRect.top,pWindowCfg->dstRect.right - pWindowCfg->dstRect.left, pWindowCfg->dstRect.bottom - pWindowCfg->dstRect.top
                     , pWindowCfg->dstRect.left, pWindowCfg->dstRect.top, pWindowCfg->dstRect.right, pWindowCfg->dstRect.bottom  );
                    SurfaceComposerClient::openGlobalTransaction();
                    
                    surfaceController[test_surface_idx]->setPosition( pWindowCfg->dstRect.left , pWindowCfg->dstRect.top ); //[ALPS01478586][Sanity][VT call]
                    surfaceController[test_surface_idx]->setSize( pWindowCfg->dstRect.right - pWindowCfg->dstRect.left, pWindowCfg->dstRect.bottom - pWindowCfg->dstRect.top); 
                    surfaceController[test_surface_idx]->setLayer(pWindowCfg->z_order);
                    //if(pWindowCfg->srcCrop.en)
                    //  surfaceController[test_surface_idx]->setCrop(cropRect);
                     
                    SurfaceComposerClient::closeGlobalTransaction();
                    
                    
                    
                    printf("\nshowTestFrame frame %d, surface_idx %d/%d, cfg_idx %d, loop %d ... ", test_scenario_idx, test_surface_idx,test_surface_num, config_idx, test_fill_surface_times);
                
                    showTestFrame( window[test_surface_idx] , test_frames[test_scenario_idx] , test_fill_surface_times);
                    
                }
                
            }
        }
        printf("\ndone..");



        for( test_surface_idx = 0; test_surface_idx < test_surface_num ; test_surface_idx++)
          native_window_api_disconnect(window[test_surface_idx], test_frames[test_scenario_idx].api);        
          
          
        //surface_x++ ;
        
        //if (test_scenario_idx >= TEST_FRAMES) {
        //    printf("\n... loop again ...\n");
        //    test_scenario_idx = 0;
        //}
    }


    native_window_api_disconnect(windowVDS, 0);
    
    bufferProducer = NULL;
    SurfaceComposerClient::destroyDisplay(dpy);
    

    printf("\ntest complete.");
    //printf("\ntest complete. CTRL+C to finish.");
#if 1
    if(test_blocking_end){
      IPCThreadState::self()->joinThreadPool();
    }
#endif        
    return NO_ERROR;
}
