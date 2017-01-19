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

#include <linux/rtpm_prio.h>
#include <fcntl.h>

#include <cutils/xlog.h>
#include <cutils/properties.h>

#include <ui/DisplayInfo.h>

#include "SurfaceFlinger.h"
#include "DisplayHardware/HWComposer.h"

namespace android {

// global static vars for SF properties and features
SurfaceFlinger::PropertiesState SurfaceFlinger::sPropertiesState;

// adjust thread priority
status_t SurfaceFlinger::adjustPriority() const {
    sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_SURFACEFLINGER;      // RR 84
    if (-1 == sched_setscheduler(0, SCHED_RR, &sched_p)) {
        XLOGW("    RR priority failed:%s", strerror(errno));
        return INVALID_OPERATION;
    }
    XLOGI("    set priority to RR:%d", RTPM_PRIO_SURFACEFLINGER);

    return NO_ERROR;
}

// BOOT functions
void SurfaceFlinger::checkEnableBootAnim() {
    char propVal[PROPERTY_VALUE_MAX];
    property_get("sys.boot.reason", propVal, "");
    XLOGI("[%s] boot reason = '%s'", __func__, propVal);
    if (strcmp(propVal, "")) {
        mBootAnimationEnabled = ('1' != propVal[0]);
    } else {
        /*
         * The sys.boot.reason will be updated by boot_logo_updater.
         * However, the init can't set the system properity
         * before surface flinger is initialized.
         * Determine the boot reason by check boot path and
         * boot package information directly
         */
        char boot_reason = '0'; // 0: normal boot, 1: alarm boot
        int fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
        if (fd < 0) {
            XLOGE("fail to open: boot path %s", strerror(errno));
            boot_reason = '0';
        } else {
            char boot_mode;
            size_t s = read(fd, (void *)&boot_mode, sizeof(boot_mode));
            close(fd);

            if (s <= 0)
                XLOGE("can't read the boot_mode");
            else if (boot_mode == '7')
                boot_reason = '1';
        }

        mBootAnimationEnabled = ('1' != boot_reason);
    }

    ALOG(LOG_INFO, "boot", "mBootAnimationEnabled = %d", mBootAnimationEnabled);

    if (true == mBootAnimationEnabled) {
        property_set("service.bootanim.exit", "0");
        property_set("ctl.start", "bootanim");

        // boot time profiling
        ALOG(LOG_INFO, "boot", "BOOTPROF:BootAnimation:Start:%ld", long(ns2ms(systemTime())));
        bootProf(1);
    } else {
        XLOGI("Skip boot animation!");
    }
}

void SurfaceFlinger::bootProf(int start) const {
    int fd         = open("/proc/bootprof", O_RDWR);
    int fd_nand    = open("/proc/driver/nand", O_RDWR);

    if (fd == -1){
		XLOGE("fail to open /proc/bootproffile : %s", strerror(errno));

		return;
    	}
    char buf[64];
    if (1 == start) {
        strcpy(buf,"BOOT_Animation:START");
        if (fd > 0) {
            write(fd, buf, 32);
            close(fd);
        }
        if (fd_nand > 0) {
            close(fd_nand);
        }
    } else {
        strcpy(buf, "BOOT_Animation:END");
        if (fd > 0) {
            write(fd, buf, 32);
            write(fd, "0", 1);    //end of bootprof
            close(fd);
        }
        if (fd_nand > 0) {
            write(fd_nand, "I1", 2);
            close(fd_nand);
        }
    }
}

status_t SurfaceFlinger::getDisplayInfoEx(const sp<IBinder>& display, DisplayInfoEx* info) {
    int32_t type = NAME_NOT_FOUND;
    for (int i=0 ; i<DisplayDevice::NUM_BUILTIN_DISPLAY_TYPES ; i++) {
        if (display == mBuiltinDisplays[i]) {
            type = i;
            break;
        }
    }

    if (type < 0) {
        return type;
    }

    const HWComposer& hwc(getHwComposer());

    info->subtype = hwc.getSubType(type);

    return NO_ERROR;
}

void SurfaceFlinger::scanMirrorDisplay() {
    for (size_t i = 0; i < mDisplays.size(); i++) {
        sp<DisplayDevice> hw(mDisplays[i]);
        uint32_t layerStack = hw->getLayerStack();
        hw->setHwcMirrorId(-1);
        for (size_t j = 0; j < i; j++) {
            sp<const DisplayDevice> hw2(mDisplays[j]);
            if (layerStack == hw2->getLayerStack()) {
                int32_t mirrorId = hw2->getHwcDisplayId();
                hw->setHwcMirrorId(mirrorId);
                break;
            }
        }
    }
}

void SurfaceFlinger::setMTKProperties() {
    String8 result;
    setMTKProperties(result);
    XLOGI("%s", result.string());
}

void SurfaceFlinger::setMTKProperties(String8 &result) {
    const size_t SIZE = 4096;
    char buffer[SIZE];

    char value[PROPERTY_VALUE_MAX];

    snprintf(buffer, sizeof(buffer), "[%s]\n", __func__);
    result.append(buffer);
    result.append("========================================================================\n");

    result.append("[AOSP]\n");

    property_get("debug.sf.showupdates", value, "0");
    mDebugRegion = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.showupdates (mDebugRegion): %d\n", mDebugRegion);
    result.append(buffer);

    property_get("debug.sf.ddms", value, "0");
    mDebugDDMS = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.ddms (mDebugDDMS): %d\n", mDebugDDMS);
    result.append(buffer);

#ifdef RUNNING_WITHOUT_SYNC_FRAMEWORK
    result.append("    Sync Framework: disabled\n");
#else
    result.append("    Sync Framework: enabled\n");
#endif
    result.appendFormat("        vsyncPhaseOffsetNs: %lld\n", (int64_t)VSYNC_EVENT_PHASE_OFFSET_NS);
    result.appendFormat("        sfVsyncPhaseOffsetNs: %lld\n", (int64_t)SF_VSYNC_EVENT_PHASE_OFFSET_NS);

#ifdef FORCE_HWC_COPY_FOR_VIRTUAL_DISPLAYS
    result.append("    Force HWC copy VDS: true\n");
#else
    result.append("    Force HWC copy VDS: false\n");
#endif

    result.append("[MediaTek SF]\n");

    // get info for panel physical rotation
    property_get("ro.sf.hwrotation", value, "0");
    sPropertiesState.mHwRotation = atoi(value);
    snprintf(buffer, sizeof(buffer), "    ro.sf.hwrotation (mHwRotation): %d\n", sPropertiesState.mHwRotation);
    result.append(buffer);

    // for internal screen composition update
    property_get("debug.sf.log_repaint", value, "0");
    sPropertiesState.mLogRepaint = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.log_repaint (mLogRepaint): %d\n", sPropertiesState.mLogRepaint);
    result.append(buffer);

    property_get("debug.sf.log_buffer", value, "0");
    sPropertiesState.mLogBuffer = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.log_buffer (mLogBuffer): %d\n", sPropertiesState.mLogBuffer);
    result.append(buffer);

    property_get("debug.sf.log_transaction", value, "0");
    sPropertiesState.mLogTransaction = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.log_transaction (mLogTransaction): %d\n", sPropertiesState.mLogTransaction);
    result.append(buffer);

    // debug utils
    property_get("debug.sf.line_g3d", value, "0");
    sPropertiesState.mLineG3D = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.line_g3d (mLineG3D): %d\n", sPropertiesState.mLineG3D);
    result.append(buffer);

    property_get("debug.sf.line_ss", value, "0");
    sPropertiesState.mLineScreenShot = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.line_ss (mLineScreenShot): %d\n", sPropertiesState.mLineScreenShot);
    result.append(buffer);

    property_get("debug.sf.dump_ss", value, "0");
    sPropertiesState.mDumpScreenShot = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.dump_ss (mDumpScreenShot): %d\n", sPropertiesState.mDumpScreenShot);
    result.append(buffer);

    property_get("debug.sf.slowmotion", value, "0");
    sPropertiesState.mDelayTime = atoi(value);
    snprintf(buffer, sizeof(buffer), "    debug.sf.slowmotion (mDelayTime): %d\n", sPropertiesState.mDelayTime);
    result.append(buffer);

    result.append("[MediaTek GUI]\n");
    // just get and print, real switches should be in libgui

    property_get("debug.bq.dump", value, "NULL");
    snprintf(buffer, sizeof(buffer), "    debug.bq.dump: %s\n", value);
    result.append(buffer);

    property_get("debug.bq.line", value, "0");
    snprintf(buffer, sizeof(buffer), "    debug.bq.line: %s\n", value);
    result.append(buffer);

    snprintf(buffer, sizeof(buffer), "    *** dynamic modification ***\n");
    result.append(buffer);

    property_get("debug.bq.line_p", value, "0");
    snprintf(buffer, sizeof(buffer), "        debug.bq.line_p (set fixed index) %s\n", value);
    result.append(buffer);

    property_get("debug.bq.line_g", value, "0");
    snprintf(buffer, sizeof(buffer), "        debug.bq.line_g (set drawing grid, ex: 16:1 / 1:16) %s\n", value);
    result.append(buffer);

    property_get("debug.bq.line_c", value, "0");
    snprintf(buffer, sizeof(buffer), "        debug.bq.line_c (set drawing color) %s\n", value);
    result.append(buffer);

    result.append("========================================================================\n\n");
}

}; // namespace android
