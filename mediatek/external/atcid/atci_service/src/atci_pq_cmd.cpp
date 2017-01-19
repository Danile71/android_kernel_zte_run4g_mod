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

#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#if defined(MTK_AAL_SUPPORT)
#include <AALClient.h>
#endif /* defined(MTK_AAL_SUPPORT) */

// If AAL support read/write fields, following 2 macros should be defined in AALClient.h
#ifndef AALClient_readField
#define AALClient_readField(field, value_ptr) (*(value_ptr) = 0, -1)
#endif
#ifndef AALClient_writeField
#define AALClient_writeField(field, value) (-1)
#endif


extern "C" {
#include "atci_pq_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"
#include "at_tok.h"
}


//4 types of operation codes: ACTION / TEST(=?) / READ(?) / SET(=)

int drvID = -1;

extern "C"
int pq_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    ALOGD("[PQ_CMD] pq cmd handler handles cmdline:%s", cmdline);

    int actionID=0, RegBase = 0, RegValue = 0, err = 0;
    char fileName[256];
    struct timespec tt1, tt2;

    clock_gettime(CLOCK_REALTIME, &tt1);

    if(drvID == -1) //initial
        drvID = open("/proc/mtk_disp", O_RDONLY, 0);
    if(drvID == -1)
        drvID = open("/proc/mtk_mira", O_RDONLY, 0);
    if(drvID == -1)
        drvID = open("/dev/mtk_disp", O_RDONLY, 0);

    if(drvID == -1)
    {
        ALOGD("[PQ_CMD] drvID = -1!!!");
    }

    switch(at_op){
        case AT_ACTION_OP:
        case AT_TEST_OP:
        case AT_READ_OP:
            sprintf(response,"\r\nPQ OK\r\n");
            break;

        case AT_SET_OP:

            at_tok_nexthexint(&cmdline, &actionID);

            if(actionID == 0) //action: READ
            {
                DISP_READ_REG params;
                unsigned int value;

                at_tok_nexthexint(&cmdline, &RegBase);

                if ((RegBase & 0xF0000000) == 0x10000000)
                {
                    params.reg = (RegBase | 0xF0000000); // For compatible
                }
                else
                {
                    params.reg = RegBase;
                }
                params.val = &value;
                params.mask = 0xFFFFFFFF;
                ioctl(drvID, DISP_IOCTL_READ_REG, &params);

                sprintf(response,"%08x\r\n", value);
                ALOGD("[PQ_CMD] R: 0x%08x = %08x", params.reg, value);
            }
            else if(actionID == 1)//action: WRITE
            {
                DISP_WRITE_REG params;
                unsigned int value = 0;

                at_tok_nexthexint(&cmdline, &RegBase);
                at_tok_nexthexint(&cmdline, &RegValue);

                if((unsigned int)RegBase == 0xEEEEEEEE)
                {
                    if(RegValue == 0)
                    {
                        ALOGD("PQ_DEBUG SCE program start...\n");
                        value = 1;
                        ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &value);
                    }
                    else
                    {
                        ALOGD("PQ_DEBUG SCE program done!\n");
                        value = 2;
                        ioctl(drvID, DISP_IOCTL_MUTEX_CONTROL, &value);
                    }
                }
                else
                {
                    if ((RegBase & 0xF0000000) == 0x10000000)
                    {
                        params.reg = (RegBase | 0xF0000000); // For compatible
                    }
                    else
                    {
                        params.reg = RegBase;
                    }
                    params.val = RegValue;
                    params.mask = 0xFFFFFFFF;
                    ioctl(drvID, DISP_IOCTL_WRITE_REG, &params);
                    ALOGD("[PQ_CMD] W: 0x%08x = %08x", params.reg, params.val);
                }

                sprintf(response,"%08x\r\n", value);
                //ALOGD("[PQ_CMD] response=%x", value);
            }
            else if(actionID == 2)//action: SW READ
            {
                DISP_READ_REG params;
                unsigned int value;

                at_tok_nexthexint(&cmdline, &RegBase);

                params.reg = (RegBase); // For compatible
                params.val = &value;
                params.mask = 0xFFFFFFFF;
                ioctl(drvID, DISP_IOCTL_READ_SW_REG, &params);

                sprintf(response,"%08x\r\n", value);
                ALOGD("[PQ_CMD] SR: 0x%08x = %08x", params.reg, value);
            }
            else if(actionID == 3)//action: SW WRITE
            {
                DISP_WRITE_REG params;
                unsigned int value = 0;

                at_tok_nexthexint(&cmdline, &RegBase);
                at_tok_nexthexint(&cmdline, &RegValue);

                params.reg = (RegBase); // For compatible
                params.val = RegValue;
                params.mask = 0xFFFFFFFF;
                ioctl(drvID, DISP_IOCTL_WRITE_SW_REG, &params);
                ALOGD("[PQ_CMD] SW: 0x%08x = %08x", params.reg, params.val);

                sprintf(response,"%08x\r\n", value);
                //ALOGD("[PQ_CMD] response=%x", value);
            }
            else if(actionID == 4) //action: AAL READ
            {
                unsigned int value = 0;
                
                at_tok_nexthexint(&cmdline, &RegBase);

                if (AALClient_readField((unsigned int)RegBase, &value) == 0) {
                    ALOGD("[AAL_CMD] SR: %u = %d(0x%08x)", RegBase, (int)value, value);
                } else {
                    ALOGD("[AAL_CMD] SR: %u read error", RegBase);
                }

                sprintf(response,"%08x\r\n", value);
            }
            else if(actionID == 5) //action: AAL WRITE
            {
                at_tok_nexthexint(&cmdline, &RegBase);
                at_tok_nexthexint(&cmdline, &RegValue);

                if (AALClient_writeField((unsigned int)RegBase, (unsigned int)RegValue) == 0) {
                    ALOGD("[AAL_CMD] SW: %u = %d(0x%08x)", RegBase, RegValue, (unsigned int)RegValue);
                } else {
                    ALOGD("[AAL_CMD] SW: %u = %d(0x%08x) write error", RegBase, RegValue, (unsigned int)RegValue);
                }

                sprintf(response,"%08x\r\n", RegValue);
            }
            break;
        default:
            break;
    }

    clock_gettime(CLOCK_REALTIME, &tt2);
    ALOGD("[PQ_CMD] pq cmd handler end... comsumes [%ld] nano-sec!", tt2.tv_nsec - tt1.tv_nsec);

    return 0;
}



