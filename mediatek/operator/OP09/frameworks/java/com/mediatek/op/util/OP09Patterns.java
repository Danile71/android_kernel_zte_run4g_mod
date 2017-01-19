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

package com.mediatek.op.util;

import com.mediatek.op.util.DefaultPatterns;
import com.mediatek.xlog.Xlog;

import java.util.regex.Pattern;

/**
 * M: Change for support operator's requirement.Support filename extension format. 
 * 1) Protocol name ignore case;
 * 2) Add ftp Protocol and ignore case
 * 3) Domain name ignore case;
 * 4) Support Chinese mp3 name;
 * 5) Mp3 name ignore case
 * 6) Chinese character following URL can be separated.
 *  Regular expression pattern to match most part of RFC 3987
 *  Internationalized URLs, aka IRIs.  Commonly used Unicode characters are
 *  added.
 */
public class OP09Patterns extends DefaultPatterns {
    private static final String FILENAME_EXTENSION = "((?i)(mp3|txt|mp4|aac|wma))";

    @Override
    public Pattern getWebUrl(final String engIriChar, final String goodIriChar) {
        Xlog.d("OP09Patterns", "getWebUrl");
        if (mPattern == null) {
            mPattern = Pattern
                    .compile(
                            "((?:((?i)ftp|http|https|rtsp):\\/\\/(?:(?:[a-zA-Z0-9\\$\\-\\_\\.\\+\\!\\*\\'\\(\\)"
                                    + "\\,\\;\\?\\&\\=]|(?:\\%[a-fA-F0-9]{2})){1,64}(?:\\:(?:[a-zA-Z0-9\\$\\-\\_"
                                    + "\\.\\+\\!\\*\\'\\(\\)\\,\\;\\?\\&\\=]|(?:\\%[a-fA-F0-9]{2})){1,25})?\\@)?)?"
                                    + "((?:(?:["
                                    + engIriChar
                                    + "]["
                                    + engIriChar
                                    + "\\-]{0,64}\\.)+"
                                    + TOP_LEVEL_DOMAIN_STR_FOR_WEB_URL_EXPAND
                                    + "|(?:(?:25[0-5]|2[0-4]"
                                    + "[0-9]|[0-1][0-9]{2}|[1-9][0-9]|[1-9])\\.(?:25[0-5]|2[0-4][0-9]"
                                    + "|[0-1][0-9]{2}|[1-9][0-9]|[1-9]|0)\\.(?:25[0-5]|2[0-4][0-9]|[0-1]"
                                    + "[0-9]{2}|[1-9][0-9]|[1-9]|0)\\.(?:25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}"
                                    + "|[1-9][0-9]|[0-9])))"
                                    + "(?:\\:\\d{1,5})?)"
                                    + "(\\/(?:(?:["
                                    + engIriChar
                                    + "\\;\\/\\?\\:\\@\\&\\=\\#\\~"
                                    + "\\-\\.\\+\\!\\*\\'\\(\\)\\_])|(?:\\%[a-fA-F0-9]{2}))*"
                                    + "(?:["
                                    + goodIriChar
                                    + "]*\\."
                                    + FILENAME_EXTENSION + ")?)?",
                            Pattern.CASE_INSENSITIVE);
        }
        return mPattern;
    }
}
