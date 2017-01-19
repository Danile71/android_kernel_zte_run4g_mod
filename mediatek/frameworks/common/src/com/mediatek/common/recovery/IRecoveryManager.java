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
 

package com.mediatek.common.recovery;


public interface IRecoveryManager {

    /**
     * Recover successfully with no error
     * @hide 
     */
    public static final int RECOVER_SUCCESS = 0;

    /**
     * No backup file found
     * @hide
     */
    public static final int RECOVER_FAILED_NO_BACKUP_FILE = -1;

    /**
     * not enough space for recover file copy back
     * @hide
     */
    public static final int RECOVER_FAILED_INSUFFICIENT_STORAGE = -2;

    /**
     * Failed to delete broken file. This often caused by the file attribute or permission gets changed
     * @hide
     */
    public static final int RECOVER_FAILED_CANNOT_DELETE_ORIGINAL_FILE = -3;

    /**
     * Any exception happens when perform copy back of backup file
     * @hide
     */
    public static final int RECOVER_FAILED_EXCEPTION_COPY_BACK = -4;

    /**
     * The checksum of recovered file mismatch with backup file. This often happen when file system encounter error
     * @hide
     */
    public static final int RECOVER_FAILED_CHECKSUM_MISMATCH = -5;

    /**
     * Cannot find defined operation class in target jar file
     * @hide
     */
    public static final int RECOVER_FAILED_CLASS_NOT_FOUND = -6;

    /**
     * Cannot instantiation target class
     * @hide
     */
    public static final int RECOVER_FAILED_CLASS_INSTANTIATION_FAILED = -7;

    /**
     * Any exception happens during operation is running
     * @hide
     */
    public static final int RECOVER_FAILED_OPEARTION_EXCEPTION = -8;

    /**
     * Operation return failed value
     * @hide
     */
    public static final int RECOVER_FAILED_OPEARTION_FAILED = -9;

    /**
     * Requested module not exist 
     * @hide
     */
    public static final int RECOVER_FAILED_MODULE_UNDEFINED = -10;

    /**
     * Requested file not exist 
     * @hide
     */
    public static final int RECOVER_FAILED_FILE_UNDEFINED = -11;

    /**
     * Backup successfully with no error
     * @hide
     */
    public static final int BACKUP_SUCCESS = 0;

    /**
     * Not enough storage for backup file
     * @hide
     */
    public static final int BACKUP_FAILED_INSUFFICIENT_STORAGE = -1;

    /**
     * If any exception happened during I/O operation of make a copy of original file
     * @hide
     */
    public static final int BACKUP_FAILED_INVALID_IO = -2;

    /**
     * The checksum cannot match the original file
     * @hide
     */
    public static final int BACKUP_FAILED_CHECKSUM_MISMATCH = -3;

    /**
     * The backup target file doesn't exist
     * @hide
     */
    public static final int BACKUP_FAILED_FILE_UNDEFINED = -4;

    /**
     * Interface for modules need on-demand backup
     * @hide
     */
    public int backupSingleFile(String moduleName, String fileName);

    /**
     * Get the version of RecoveryManagerService
     *
     * @return Returns version of RecoveryManagerService.
     */
    public String getVersion();
    
    public void systemReady();

    public void parseException(String exceptionClassName, String throwMethodName, String throwClassName);

    public void startBootMonitor();

    public void stopBootMonitor();
}
