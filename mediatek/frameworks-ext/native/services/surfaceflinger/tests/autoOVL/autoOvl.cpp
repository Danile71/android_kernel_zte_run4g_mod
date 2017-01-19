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

#define LOG_TAG "test-perf"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

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


using namespace android;


#define MAX_LAYER_COUNT		(32)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct FRAME {
    char name[128];     // file name
    uint32_t x;
    uint32_t y;
    uint32_t w;         // width
    uint32_t s;         // stride
    uint32_t h;         // height
    uint32_t fmt;       // format
    uint32_t api;       // api connection type
    uint32_t layer_type;// layer type
    uint32_t z_order;     // z order relative to BG_LAYER_ORDER
};

enum {
	normal_layer = ISurfaceComposerClient::eFXSurfaceNormal,
	dim_layer = ISurfaceComposerClient::eFXSurfaceDim,
};

static FRAME test_frames[MAX_LAYER_COUNT];

class OVLLayer : public virtual RefBase {
public:
	OVLLayer(const sp<SurfaceComposerClient>& client,
			struct FRAME& input);
	~OVLLayer();

	int Init();
	int Test();

private:
	sp<SurfaceControl> mSurfaceControl;
	sp<Surface> mSurface;

	FRAME mInfo;
};

OVLLayer::OVLLayer(const sp<SurfaceComposerClient>& client,
                 struct FRAME& input) : mInfo(input)

{
    mSurfaceControl = client->createSurface(
                String8::format(mInfo.name),
                mInfo.w, mInfo.h, mInfo.fmt, mInfo.layer_type);
    mSurface = mSurfaceControl->getSurface();
}

OVLLayer::~OVLLayer() {
    ANativeWindow *window = mSurface.get();
    native_window_api_disconnect(window, mInfo.api);
}

int OVLLayer::Init() {
    #define BG_LAYER_ORDER		(220000)
    /* config surface attrbution */

    SurfaceComposerClient::openGlobalTransaction();
    mSurfaceControl->setLayer(BG_LAYER_ORDER + mInfo.z_order);
    //mSurfaceControl->setSize(mInfo.w, mInfo.h);
    mSurfaceControl->setPosition(mInfo.x, mInfo.y);
    mSurfaceControl->setMatrix(1.0, 0.0, 0.0, 1.0);
    if (mInfo.layer_type == dim_layer)
        mSurfaceControl->setAlpha(0.5);

    SurfaceComposerClient::closeGlobalTransaction();

    /* start to config window buffer */
    ANativeWindow *window = mSurface.get();

    /* set api connection type as register */
    native_window_api_connect(window, mInfo.api);

    /* set buffer size */
    native_window_set_buffers_dimensions(window, mInfo.w, mInfo.h);

    /* set format */
    native_window_set_buffers_format(window, mInfo.fmt);

    /* set usage software write-able and hardware texture bind-able */
    native_window_set_usage(window, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

    /* set scaling to match window display size */
    native_window_set_scaling_mode(window, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    return OK;
}

int OVLLayer::Test() {
    if (mInfo.layer_type == normal_layer)
    {
        ANativeWindow *window = mSurface.get();

        /* alloc buffer and fill it */
        ANativeWindowBuffer *buf;
        sp<GraphicBuffer> gb;
        const Rect rect(mInfo.w, mInfo.h);
        void *ptr;

        int err;
        int fenceFd = -1;
        err = window->dequeueBuffer(window, &buf, &fenceFd);
        if (err != NO_ERROR) {
            printf("dequeue buffer failed for surface%d\n", mInfo.z_order);
            //native_window_api_disconnect(window, mInfo.api);
            return err;
        }
        sp<Fence> fence(new Fence(fenceFd));
        fence->wait(Fence::TIMEOUT_NEVER);

        gb = new GraphicBuffer(buf, false);
        gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);

        uint32_t *pbuf = (uint32_t *)ptr;
        int color = 0xFFFFFFFF;

        if (mInfo.z_order == 0)
            color = 0x0;
        else if (mInfo.z_order % 3 == 0)
            color = 0xFFFF0000;
        else if (mInfo.z_order % 3 == 1)
            color = 0xFF00FF00;
        else
            color = 0xFF0000FF;

        //memset(ptr, color, mInfo.w * mInfo.h * bytesPerPixel(mInfo.fmt));
        for (uint32_t i = 0; i < mInfo.w * mInfo.h; i++)
        {
            *pbuf++ = color;
        }

        gb->unlock();

        err = window->queueBuffer(window, buf, -1);
        if (err != NO_ERROR) {
            printf("queue buffer failed for surface%d\n", mInfo.z_order);
            //native_window_api_disconnect(window, mInfo.api);
            return err;
        }
    }

    return OK;
}


void prepare_layer_info(FRAME *pframe, int layer_count)
{
    #define shift_offset        (50)
    #define layer_size          (512)
    #define NORMAL_LAYER_NAME   "normal_layer"
    #define DIM_LAYER_NAME      "dim_layer"

	DisplayInfo dinfo;
	sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
			ISurfaceComposer::eDisplayIdMain);
	SurfaceComposerClient::getDisplayInfo(display, &dinfo);
	uint32_t dispw = dinfo.w;
	uint32_t disph = dinfo.h;
    int start_x = 0, start_y = 0;

	for (int i = 0; i < layer_count; i++)
	{
        if (pframe[i].layer_type == dim_layer)
            sprintf(pframe[i].name, "%s_%d",DIM_LAYER_NAME,i);
        else
            sprintf(pframe[i].name, "%s_%d",NORMAL_LAYER_NAME,i);

        pframe[i].x = start_x;
        start_x += shift_offset;
        pframe[i].y = start_y;
        start_y += shift_offset;

        pframe[i].api = NATIVE_WINDOW_API_CPU;
        pframe[i].z_order = i;

        if ( i == 0 || pframe[i].layer_type == dim_layer)
        {
            pframe[i].w = dispw;
            pframe[i].h = disph;
            pframe[i].s = dispw;
            pframe[i].fmt = HAL_PIXEL_FORMAT_RGBX_8888;
        }
        else
        {
            pframe[i].w = layer_size;
            pframe[i].h = layer_size;
            pframe[i].s = layer_size;
            pframe[i].fmt = HAL_PIXEL_FORMAT_RGBA_8888;
        }
	}

}

static void usage(void)
{
	printf(
		"Usage: test-perf [options]\n\n"
		"Options:\n"
		" -l | --normal layer number\t\t\t[N]\n"
		" -d | --dim layer number\t\t\t[N]\n"
	);
}

int main(int argc, char** argv)
{
	int i;
	int acc_layer_count = 0, total_layer_count = 0;
    printf("autoOvl start\n");

	/* analyze the arguments */
	if (argc <= 1) {
		/* no argument */
		usage();
		return 0;
	}

	/* options structures */
	const char shortOptions[] = "l:d:";

	while (1) {
		int c = getopt(argc, argv, shortOptions);
		int input = 0;

		if (-1 == c)
			break;

		switch (c) {
		case 'l':
			input = atoi(optarg);
			if (input <= 0 || input > MAX_LAYER_COUNT) {
				usage();
				return 0;
			}

			for (i = 0; i < input; i++) {
				test_frames[acc_layer_count++].layer_type = normal_layer;
			}

			break;

		case 'd':
			input = atoi(optarg);
			if (input <= 0 || input > MAX_LAYER_COUNT) {
				usage();
				return 0;
			}

			for (i = 0; i < input; i++) {
				test_frames[acc_layer_count++].layer_type = dim_layer;
			}

			break;

		default:
			usage();
			return 0;
		}
	}

    total_layer_count = acc_layer_count;

    /* set up the thread-pool */
    //sp<ProcessState> proc(ProcessState::self());
    //ProcessState::self()->startThreadPool();

	/* create a client to surfaceflinger */
	sp<SurfaceComposerClient> client = new SurfaceComposerClient();

    prepare_layer_info(test_frames, total_layer_count);

	sp<OVLLayer> testSurfaces[total_layer_count];


	for (i = 0; i < total_layer_count; i++) {
		testSurfaces[i] = new OVLLayer(client, test_frames[i]);
        testSurfaces[i]->Init();
	}

    char cmds[256];

    sprintf(cmds, "rm -r /data/SF_dump");
    system(cmds);

    sprintf(cmds, "mkdir /data/SF_dump");
    system(cmds);

    //disable HWC
    sprintf(cmds, "service call SurfaceFlinger 1008 i32 1");
    system(cmds);

	/* start to test */
	for (i = 0; i < total_layer_count; i++) {
        testSurfaces[i]->Test();
	}

	usleep(1000 * 1000);

    //captrue ovl
    sprintf(cmds, "lcdc_screen_cap > data/SF_dump/gpu_compose.bin");
    system(cmds);

    //enable HWC
    sprintf(cmds, "service call SurfaceFlinger 1008 i32 0");
    system(cmds);

	/* start to test */
	for (i = 0; i < total_layer_count; i++) {
        testSurfaces[i]->Test();
	}

	usleep(1000 * 1000);

    //captrue ovl
    sprintf(cmds, "lcdc_screen_cap > data/SF_dump/hwc_compose.bin");
    system(cmds);

    printf("dumpsys SurfaceFlinger >> /data/SF_dump/SF_auto_ovl.log\n");
    sprintf(cmds, "dumpsys SurfaceFlinger >> /data/SF_dump/SF_auto_ovl.log");
    system(cmds);

	usleep(1000 * 1000);
    printf("autoOvl stop\n");


	//IPCThreadState::self()->joinThreadPool();

    return 0;
}
