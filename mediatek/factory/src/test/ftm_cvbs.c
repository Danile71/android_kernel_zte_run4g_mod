
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

//#include <linux/tve_drv.h>
#include "tve_drv.h"

#ifdef FEATURE_FTM_TVE

#define TAG                 "[CVBS] "


static const char *devpath = "/dev/cvbs";

#define mod_to_cvbs(p)	(cvbs_module *)((char *)(p) + sizeof(struct ftm_module))

#define ARY_SIZE(x)     (sizeof((x)) / sizeof((x[0])))
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
static item_t cvbs_item[] = {
    item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1,          NULL),
};

typedef struct {
	struct ftm_module *module;
    char info[1024];

    /* item view */
	struct itemview *itm_view;
    text_t title;
    text_t text;

} cvbs_module;

static int cvbs_enable()
{
    int fd = -1;
    int ret = 0;
    int check_res = 0;

    fd = open(devpath, O_RDONLY);
    if (fd == -1)
    {
        LOGE(TAG "Error, Can't open /dev/cvbs\n");
        return -1;
    }

    ret = ioctl(fd, CMD_TVE_SET_DPI0_CB, 1);
    if(ret < 0)
    {
        check_res = -1;
        goto check_exit;
    }
    usleep(1000*1500);
    
check_exit:

    close(fd);
    return check_res;
}


int cvbs_entry(struct ftm_param *param, void *priv)
{
    bool exit = false;
    cvbs_module *cvbs = (cvbs_module *)priv;
    struct itemview *iv;

    LOGD(TAG "cvbs_entry\n");

     /* show text view */
    if (!cvbs->itm_view) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory for item view");
            return -1;
        }
        cvbs->itm_view = iv;
    }

    iv = cvbs->itm_view;

    //init item view
    memset(&cvbs->info[0], 0, sizeof(cvbs->info));
    memset(&cvbs->info[0], '\n', 10);
    init_text(&cvbs->title, param->name, COLOR_YELLOW);
    init_text(&cvbs->text, &cvbs->info[0], COLOR_YELLOW);

    iv->set_title(iv, &cvbs->title);
    iv->set_items(iv, cvbs_item, 0);
    iv->set_text(iv, &cvbs->text);

    //iv->redraw(iv);

    if(cvbs_enable())
    {
        LOGD(TAG "cvbs test fail\n");
        sprintf(cvbs->info, "cvbs "uistr_fail"\n");
    }
    else
    {
        LOGD(TAG "cvbs test pass\n");
        sprintf(cvbs->info, "cvbs Basic "uistr_pass" \n");
    }

    while(!exit)
    {
        switch(iv->run(iv, &exit))
        {
        case ITEM_PASS:
            cvbs->module->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        case ITEM_FAIL:
            cvbs->module->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case -1:
            exit = true;
            break;
        default:
            break;
        }
    }
    return 0;
}

int cvbs_init(void)
{
    int ret = 0;

    struct ftm_module *mod;
	cvbs_module *cvbs;

    LOGD(TAG "hdmi_init\n");

    mod = ftm_alloc(ITEM_CVBS, sizeof(cvbs_module));
    if (!mod)
    {
        LOGD(TAG "cvbs_init failed\n");
        return -ENOMEM;
    }

	cvbs = mod_to_cvbs(mod);
    cvbs->module = mod;

    ret = ftm_register(mod, cvbs_entry, (void*)cvbs);
    if (ret)
    {
		LOGE(TAG "register HDMI failed (%d)\n", ret);
    }

    return ret;
}
#endif



