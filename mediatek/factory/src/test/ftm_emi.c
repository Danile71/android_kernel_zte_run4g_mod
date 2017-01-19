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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
//#include <cutils/pmem.h>
#include <common.h>
#include <miniui.h>
#include <ftm.h>
#include <dlfcn.h>
#include "multimediaFactoryTest.h"
#include "MediaTypes.h"
#if defined(FEATURE_FTM_EMI)

#define TAG "[FTM_EMI] "
#define PAGE_SIZE 4096
enum { ITEM_PASS, ITEM_FAIL };

static item_t emi_items[] = 
{
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

struct emi_info 
{
    char    info[1024];
    bool    stress_result;
    bool    exit_thd;
    text_t    title;
    text_t    text;
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define STR_BUF_LEN 10

#define mod_to_emi(p)     (struct emi_info*)((char*)(p) + sizeof(struct ftm_module))
#if 0
static void emi_update_info(struct emi_info *emi, char *info)
{
    char *ptr;
    int rc;   

    /* preare text view info */
    ptr = info;
    ptr += sprintf(ptr, "%s: %s\n", uistr_info_stress_test_result, emi->stress_result ? uistr_pass : uistr_fail);
    return;
}
#endif
/*
 * emi_update_thread: status-update thread function.
 * @priv:
 */
static void *emi_update_thread(void *priv)
{
    struct emi_info *emi = (struct emi_info*)priv;
    struct itemview *iv = emi->iv;
    int count = 1, chkcnt = 5;  

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(200000);
        chkcnt--;

        if (emi->exit_thd)
            break;

        if (chkcnt > 0)
            continue;        

        /* Prepare the info data to display texts on screen */
        //emi_update_info(emi, emi->info);
        
        iv->set_text(iv, &emi->text);
        iv->redraw(iv);
        chkcnt = 5;
    }
    pthread_exit(NULL);
    
    return NULL;
}

/*
 * update_screen_thread: screen-update thread function.
 * @priv:
 */
static bool update_screen_exit = false;
static void *update_screen_thread(void *priv)
{
    LOGD(TAG "enter update_screen_thread\n");
    while (!update_screen_exit){
        ui_flip();
    }
    LOGD(TAG "exit update_screen_thread\n");
    pthread_exit(NULL);
    return NULL;
}

/*
 * emi_entry: factory mode entry function.
 * @param:
 * @priv:
 * Return error code.
 */
static int emi_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct emi_info *emi = (struct emi_info *)priv;
    struct itemview *iv;
    pthread_t update_screen;
    update_screen_exit = false;
    int ret;
 
    emi->stress_result = false;
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&emi->title, param->name, COLOR_YELLOW);
    init_text(&emi->text, &emi->info[0], COLOR_YELLOW);

    //emi_update_info(emi, emi->info);
  
    emi->exit_thd = false;  

    /* Create a itemview */
    if (!emi->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        emi->iv = iv;
    }
    
    iv = emi->iv;
    iv->set_title(iv, &emi->title);
    iv->set_items(iv, emi_items, 0);
    iv->set_text(iv, &emi->text);
    
    /* create a thread for the test pattern: frame buffer update update */
    pthread_create(&emi->update_thd, NULL, emi_update_thread, priv); 

    /* create a thread for screen update */
    if (pthread_create(&update_screen, NULL, update_screen_thread, NULL)) {
        LOGD(TAG "create update_screen_thread failed\n");
    }

    ptr = emi->info;
    
    /* run multimedia test patterns */
    LOGD(TAG "start to run multimedia test patterns\n");
    ret = mHalFactory(NULL);
    if(ret == 0)
        emi->stress_result = true;
    else
        emi->stress_result = false;
    LOGD(TAG "complete to run multimedia test patterns\n");
    usleep(200000);
    
    printf("DRAM: stress result = %d\n", emi->stress_result);
    printf("DRAM test all done\n"); //output to screen here...
    ptr += sprintf(ptr, "--> %s:\n %s\n", uistr_info_stress_test_result, emi->stress_result ? uistr_pass : uistr_fail);      

entry_exit:
    //recover_cpus();

    /* stop the test pattern: frame buffer update */
    update_screen_exit = true;

    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            /* report test results */
            if (chosen == ITEM_PASS) {
                emi->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                emi->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;
            break;
        default:
            break;
        }
        
        if (exit) {
            /* stop the screen-update thread */
            emi->exit_thd = true;
            break;
        }
    } while (1);

    pthread_join(emi->update_thd, NULL); 

    //update_screen_exit = true;
    pthread_join(update_screen, NULL);

    return 0;
}

/*
 * emi_init: factory mode initialization function.
 * Return error code.
 */
int emi_init(void)
{
    int index;
    int ret = 0;
    struct ftm_module *mod;
    struct emi_info *emi;
    //pid_t p_id;

    LOGD(TAG "%s\n", __FUNCTION__);

    /* Alloc memory and register the test module */
    mod = ftm_alloc(ITEM_EMI, sizeof(struct emi_info));
    if (!mod)
        return -ENOMEM;
    emi = mod_to_emi(mod);
    emi->mod = mod;
  
    /* register the entry function to ftm_module */
    ret = ftm_register(mod, emi_entry, (void*)emi);
    memset(emi->info,0,1024);
    return ret;
}

#endif  /* FEATURE_FTM_EMI */
