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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.videoplayer;

import android.database.Cursor;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * The class is for mutli select helper
 */
public class SelectionManager {
    private static final String TAG = "Videos/SelectionManager";
    private static final String VALUE_IS_DRM = "1";
    private ArrayList<VideoItem> mSelectVideos = new ArrayList<VideoItem>();
    private ArrayList<VideoItem> mAllVideos = new ArrayList<VideoItem>();
    private OnChangedListener mListener = null;

    public SelectionManager() {
    }

    // selection change listener
    public interface OnChangedListener {
        void onChanged();
    }

    public void addListener(OnChangedListener listener) {
        mListener = listener;
    }

    public void removeListener(OnChangedListener listener) {
    }

    public synchronized void setAllItemChecked(boolean checked) {
        if (checked) {
            mSelectVideos.clear();
            mSelectVideos.addAll(mAllVideos);
        } else {
            mSelectVideos.clear();
        }
        notifyListener();
    }

    private synchronized void notifyListener() {
        if (mListener != null) {
            mListener.onChanged();
        }
    }

    public synchronized void addSelectItem(VideoItem selectItem) {
        boolean isExist = false;
        for (VideoItem item : mSelectVideos) {
            if (item.mId == selectItem.mId) {
                // Update if exist
                item.mTitle = selectItem.mTitle;
                item.mData = selectItem.mData;
                item.mIsDrm = selectItem.mIsDrm;
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            mSelectVideos.add(selectItem);
        }

        notifyListener();
    }

    public synchronized void removeSelectItem(long id) {
        for (VideoItem item : mSelectVideos) {
            if (item.mId == id) {
                mSelectVideos.remove(item);
                break;
            }
        }
        notifyListener();
    }

    public int getCheckedItemsCount() {
        return mSelectVideos.size();
    }

    public ArrayList<VideoItem> getSelectItemList() {
        return mSelectVideos;
    }

    public boolean isSelected(long id) {
        boolean selected = false;
        for (VideoItem item : mSelectVideos) {
            if (item.mId == id) {
                selected = true;
                break;
            }
        }
        return selected;
    }

    public synchronized void clearSelected() {
        mSelectVideos.clear();
        notifyListener();
    }

    public void onItemClick(VideoItem clickItem) {
        if (isSelected(clickItem.mId)) {
            removeSelectItem(clickItem.mId);
        } else {
            addSelectItem(clickItem);
        }
    }

    public synchronized  void refreshCheckedItem(Cursor c) {
        MtkLog.d(TAG, "refreshCheckedItem cursor is " + c);
        if (c == null) {
            MtkLog.d(TAG, "refreshCheckedItem cursor is null");
            mSelectVideos.clear();
            mAllVideos.clear();
        } else if (c.getCount() == 0) {
            MtkLog.d(TAG, "refreshCheckedItem cursor count is 0");
            mSelectVideos.clear();
            mAllVideos.clear();
        } else {
            // Add or remove cursor
            // Check the video in mItemChecked if also in cursor,
            // if not in cursor, means delete it, then remove.
            mAllVideos.clear();
            for (c.moveToFirst(); !c.isAfterLast(); c.moveToNext()) {
                VideoItem item = new VideoItem();
                item.mId  = c.getLong(MovieListActivity.INDEX_ID);
                item.mTitle = c.getString(MovieListActivity.INDEX_DISPLAY_NAME);
                item.mData = c.getString(MovieListActivity.INDEX_DATA);
                item.mIsDrm = VALUE_IS_DRM.equals(c.getString(MovieListActivity.INDEX_IS_DRM));
                mAllVideos.add(item);
            }
            // remove not exist videos
            removeNotExistVideos();
            MtkLog.d(TAG, "refreshCheckedItem all:" + mAllVideos.size() +
                    ", selected:" + mSelectVideos.size());
        }
    }

    private void removeNotExistVideos() {
        // Make ArrayList more simple
        ArrayList<Long> mAllVideoIds = new ArrayList<Long>();
        for (VideoItem videoItem : mAllVideos) {
            if (videoItem != null) {
                mAllVideoIds.add(videoItem.mId);
            }
        }

        Iterator<VideoItem> it = mSelectVideos.iterator();
        while (it.hasNext()) {
            VideoItem item = it.next();
            if (!mAllVideoIds.contains(item.mId)) {
                MtkLog.d(TAG, "removeNotExistVideos remove:" + item.mId);
                it.remove();
            }
        }
        notifyListener();
    }

    public static class VideoItem {
        long mId;
        String mTitle;
        String mData;
        boolean mIsDrm;
    }
}
