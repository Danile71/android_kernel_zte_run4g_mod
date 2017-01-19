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

package com.mediatek.engineermode.wifi;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.widget.ArrayAdapter;

import com.mediatek.engineermode.Elog;
import com.mediatek.xlog.Xlog;

public class ChannelInfo {
    
    public static class ChannelData {
        public int id;
        public String name;
        public int frequency;
        
        public ChannelData() {}
        public ChannelData(int id, String name, int frequency) {
            this.id = id;
            this.name = name;
            this.frequency = frequency;
        }
    }
    private HashMap<Integer, ChannelData> mChannelDataDb = null;
    private boolean initChannelDataDatabase() {
        boolean result = true;
        if (mChannelDataDb == null) {
            mChannelDataDb = new HashMap<Integer, ChannelData>();
            for (int i = 0; i < mFullChannelName.length; i++) {
                String name = mFullChannelName[i];
                int frequency = mChannelFreq[i];
                String freqStr = String.valueOf(frequency / 1000);
                if (!name.contains(freqStr)) {
                    String msg = "UnMatch name & frequency at index:" + i + " name:" + name + " frequency:" + frequency;
                    Elog.e(TAG, msg);
                    continue;
                }
                int id = extractChannelIdFromName(name);
                ChannelData cd = new ChannelData(id, name, frequency);
                mChannelDataDb.put(id, cd);
            }
        }
        return result;
    }
    
    private static final String TAG = "EM/Wifi_ChannelInfo";
    public static final int CHANNEL_NUMBER_14 = 14;
    private static final int DEFAULT_CHANNEL_COUNT = 11;
    private static final int MAX_CHANNEL_COUNT = 75;
    static final int BW_20M = 0;
    static final int BW_40M = 1;
    static final int BW_80M = 2;
    static final int CHANNEL_2DOT4G = 10;
    static final int CHANNEL_5G = 11;
    protected String mChannelSelect = null;
    protected static long[] sChannels = null;
    protected static boolean sHas14Ch = false;
    protected static boolean sHasUpper14Ch = false;
    final String[] mChannelName = {
            // 2.4GHz frequency serials
            "Channel 1 [2412MHz]", "Channel 2 [2417MHz]",
            "Channel 3 [2422MHz]", "Channel 4 [2427MHz]",
            "Channel 5 [2432MHz]", "Channel 6 [2437MHz]",
            "Channel 7 [2442MHz]", "Channel 8 [2447MHz]",
            "Channel 9 [2452MHz]", "Channel 10 [2457MHz]",
            "Channel 11 [2462MHz]", "Channel 12 [2467MHz]",
            "Channel 13 [2472MHz]", "Channel 14 [2484MHz]", };

    final String[] mFullChannelName = {
            // 2.4GHz frequency serials
            "Channel 1 [2412MHz]", "Channel 2 [2417MHz]",
            "Channel 3 [2422MHz]", "Channel 4 [2427MHz]",
            "Channel 5 [2432MHz]", "Channel 6 [2437MHz]",
            "Channel 7 [2442MHz]", "Channel 8 [2447MHz]",
            "Channel 9 [2452MHz]", "Channel 10 [2457MHz]",
            "Channel 11 [2462MHz]", "Channel 12 [2467MHz]",
            "Channel 13 [2472MHz]", "Channel 14 [2484MHz]",
            // 5GHz frequency serials
            "Channel 36 [5180MHz]", "Channel 38 [5190MHz]",
            "Channel 40 [5200MHz]", "Channel 42 [5210MHz]", 
            "Channel 44 [5220MHz]", "Channel 46 [5230MHz]", 
            "Channel 48 [5240MHz]", "Channel 52 [5260MHz]", 
            "Channel 54 [5270MHz]", "Channel 56 [5280MHz]", 
            "Channel 58 [5290MHz]", "Channel 60 [5300MHz]",
            "Channel 62 [5310MHz]", "Channel 64 [5320MHz]",
            "Channel 68 [5340MHz]", "Channel 70 [5350MHz]",
            "Channel 72 [5360MHz]", "Channel 74 [5370MHz]", 
            "Channel 76 [5380MHz]", "Channel 78 [5390MHz]", 
            "Channel 80 [5400MHz]", "Channel 84 [5420MHz]", 
            "Channel 86 [5430MHz]", "Channel 88 [5440MHz]", 
            "Channel 90 [5450MHz]", "Channel 92 [5460MHz]",
            "Channel 94 [5470MHz]", "Channel 96 [5480MHz]",
            "Channel 100 [5500MHz]", "Channel 102 [5510MHz]",
            "Channel 104 [5520MHz]", "Channel 106 [5530MHz]", 
            "Channel 108 [5540MHz]", "Channel 110 [5550MHz]", 
            "Channel 112 [5560MHz]", "Channel 116 [5580MHz]", 
            "Channel 118 [5590MHz]", "Channel 120 [5600MHz]", 
            "Channel 122 [5610MHz]", "Channel 124 [5620MHz]", 
            "Channel 126 [5630MHz]", "Channel 128 [5640MHz]",
            "Channel 132 [5660MHz]", "Channel 134 [5670MHz]", 
            "Channel 136 [5680MHz]", "Channel 140 [5700MHz]",
            "Channel 149 [5745MHz]", "Channel 151 [5755MHz]", 
            "Channel 153 [5765MHz]", "Channel 155 [5775MHz]", 
            "Channel 157 [5785MHz]", "Channel 159 [5795MHz]", 
            "Channel 161 [5805MHz]", "Channel 165 [5825MHz]", 
            "Channel 167 [5835MHz]", "Channel 169 [5845MHz]", 
            "Channel 171 [5855MHz]", "Channel 173 [5865MHz]",
            "Channel 175 [5875MHz]", "Channel 177 [5885MHz]", 
            "Channel 181 [5905MHz]", "Channel 183 [4915MHz]", 
            "Channel 184 [4920MHz]", "Channel 185 [4925MHz]", 
            "Channel 187 [4935MHz]", "Channel 188 [4940MHz]", 
            "Channel 189 [4945MHz]", "Channel 192 [4960MHz]",
            "Channel 196 [4980MHz]" };
    int[] mChannelFreq = {
            // 2.4GHz frequency serials' channels
            2412000, 2417000, 2422000, 2427000, 2432000, 2437000, 2442000,
            2447000, 2452000, 2457000, 2462000, 2467000, 2472000, 2484000,
            // 5GHz frequency serials' channels
            5180000, 5190000, 5200000, 5210000, 5220000, 5230000, 5240000, 5260000, 5270000, 5280000, 
            5290000, 5300000, 5310000, 5320000, 5340000, 5350000, 5360000, 5370000, 5380000, 5390000, 
            5400000, 5420000, 5430000, 5440000, 5450000, 5460000, 5470000, 5480000, 5500000, 5510000, 
            5520000, 5530000, 5540000, 5550000, 5560000, 5580000, 5590000, 5600000, 5610000, 5620000, 
            5630000, 5640000, 5660000, 5670000, 5680000, 5700000, 5745000, 5755000, 5765000, 5775000, 
            5785000, 5795000, 5805000, 5825000, 5835000, 5845000, 5855000, 5865000, 5875000, 5885000, 
            5905000, 4915000, 4920000, 4925000, 4935000, 4940000, 4945000, 4960000, 4980000 };
    
    private static final Object[][] s5gBw40MChannels = {
        {38, 5190000, "Channel 38 [5190MHz]"}, 
        {46, 5230000, "Channel 46 [5230MHz]"}, 
        {54, 5270000, "Channel 54 [5270MHz]"}, 
        {62, 5310000, "Channel 62 [5310MHz]"}, 
        {70, 5350000, "Channel 70 [5350MHz]"}, 
        {78, 5390000, "Channel 78 [5390MHz]"}, 
        {86, 5430000, "Channel 86 [5430MHz]"}, 
        {94, 5470000, "Channel 94 [5470MHz]"}, 
        {102, 5510000, "Channel 102 [5510MHz]"}, 
        {110, 5550000, "Channel 110 [5550MHz]"}, 
        {118, 5590000, "Channel 118 [5590MHz]"},
        {126, 5630000, "Channel 126 [5630MHz]"},
        {134, 5670000, "Channel 134 [5670MHz]"}, 
        {151, 5755000, "Channel 151 [5755MHz]"}, 
        {159, 5795000, "Channel 159 [5795MHz]"}, 
        {167, 5835000, "Channel 167 [5835MHz]"},
        {175, 5875000, "Channel 175 [5875MHz]"},
    };
    
    private static final Object[][] s5gBw80MChannels = {
        {42, 5210000, "Channel 42 [5210MHz]"}, 
        {58, 5290000, "Channel 58 [5290MHz]"}, 
        {74, 5370000, "Channel 74 [5370MHz]"}, 
        {90, 5450000, "Channel 90 [5450MHz]"}, 
        {106, 5530000, "Channel 106 [5530MHz]"}, 
        {122, 5610000, "Channel 122 [5610MHz]"}, 
        {155, 5775000, "Channel 155 [5775MHz]"}, 
        {171, 5855000, "Channel 171 [5855MHz]"}, 
    };
    
    private static final int[] sBw40mUnsupported2dot4GChannels = {1, 2, 12, 13};
    
    public void removeBw40mUnsupported2dot4GChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < sBw40mUnsupported2dot4GChannels.length; i++) {
            int channel = sBw40mUnsupported2dot4GChannels[i];
            int index = channel - 1;
            adapter.remove(mFullChannelName[index]);
        }
    } 
    
    public void insertBw40mUnsupported2dot4GChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < sBw40mUnsupported2dot4GChannels.length; i++) {
            int channel = sBw40mUnsupported2dot4GChannels[i];
            String channelName = mFullChannelName[channel - 1];
            if (isContains(channel)) {
                insertChannelIntoAdapterByOrder(adapter, channelName);
            }
        }
    }
    
    public void resetSupportedChannels(ArrayAdapter<String> adapter) {
        adapter.clear();
        long[] supportedCh = getCachedSupportChannels();
        addChannelsIntoAdapter(supportedCh, adapter, false);
    }
    
    private static long[] sCachedSupportChannels = null;
    private static long[] getCachedSupportChannels() {
        if (sCachedSupportChannels == null) {
            int len = (int)sChannels[0];
            sCachedSupportChannels = new long[len];
            for (int i = 0; i < len; i++) {
                sCachedSupportChannels[i] = sChannels[i + 1];
            }
        }
        return sCachedSupportChannels;
    }
    
    private HashMap<Integer, int[]> mCachedSupportedChs = new HashMap<Integer, int[]>();

    public void addChannelsIntoAdapter(long[] channels, ArrayAdapter<String> adapter, boolean byOrder, boolean checkSupported) {
        String name = null;
        if (channels == null) {
            return;
        }
        for (int i = 0; i < channels.length; i++) {
            long id = channels[i];
            if (checkSupported) {
                if (!isContains((int)id)) {
                    continue;
                }
            }
            name = getChannelNameById(id);
            if (name == null) {
                continue;
            }
            if (byOrder) {
                insertChannelIntoAdapterByOrder(adapter, name);
            } else {
                adapter.add(name);
            }
        }
    }
    
    public void addChannelsIntoAdapter(long[] channels, ArrayAdapter<String> adapter, boolean byOrder) {
        addChannelsIntoAdapter(channels, adapter, byOrder, true);
    }
    
    public void removeChannels(int[] channels, ArrayAdapter<String> adapter) {
        for (int i = 0; i < channels.length; i++) {
            int ch = channels[i];
            String name = getChannelNameById(ch);
            if (name != null) {
                adapter.remove(name);
            }
        }
    }
    
    public void remove2dot4GChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < 14; i++) {
            String name = mFullChannelName[i];
            adapter.remove(name);
        }
    }

    public void insertBw80MChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < s5gBw80MChannels.length; i++) {
            int ch = getChannelIdIntern(i, s5gBw80MChannels);
            if (isChannelSupported(ch, BW_80M)) {
                String name = getChannelNameIntern(i, s5gBw80MChannels);
                insertChannelIntoAdapterByOrder(adapter, name);
            }
        }
    }
    
    private String getChannelNameIntern(int index, Object[][] datas) {
        String name = (String)datas[index][2];
        return name;
    }
    
    private int getChannelIdIntern(int index , Object[][] datas) {
        Integer id = (Integer)datas[index][0];
        return id.intValue();
    }
    
    public void removeBw40MChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < s5gBw40MChannels.length; i++) {
            adapter.remove(getChannelNameIntern(i, s5gBw40MChannels));
        }
    }
    
    public void remove5GChannels(ArrayAdapter<String> adapter) {
        for (int i = adapter.getCount() - 1; i >= 0; i--) {
            String name = adapter.getItem(i);
            int id = extractChannelIdFromName(name);
            if (id > CHANNEL_NUMBER_14) {
                adapter.remove(name);
            }
        }
    }
    
    public void insert5GChannels(ArrayAdapter<String> adapter) {
        long[] channels = getCachedSupportChannels();
        for (int i = 0; i <= channels.length; i++) {
            long id = channels[i];
            if (id > CHANNEL_NUMBER_14) {
                String tag = getChannelNameById(id);
                if (tag != null) {
                    insertChannelIntoAdapterByOrder(adapter, tag);
                } else {
                    Elog.d(TAG, "UNKnown channel:" + id);
                }
            }
        }
    }
    
    private int[] getSupported2dot4gChannels() {
        int[] cachedChs = mCachedSupportedChs.get(CHANNEL_2DOT4G);
        if (cachedChs != null) {
            return cachedChs;
        }
        long[] chsupported = getCachedSupportChannels();
        List<Long> list = new ArrayList<Long>();
        for (int i = 0; i < chsupported.length; i++) {
            long id = chsupported[i];
            if (id >= 1 && id <= 14) {
                list.add(id);
            }
        }
        if (list.size() > 0) {
            int[] target = new int[list.size()];
            for (int i = 0; i < target.length; i++) {
                target[i] = list.get(i).intValue();
            }
            mCachedSupportedChs.put(CHANNEL_2DOT4G, target);
            return target;
        } else {
            return null;
        }
    }
    
    public void addSupported2dot4gChannels(ArrayAdapter<String> adapter, boolean byOrder) {
        int[] ch2dot4g = getSupported2dot4gChannels();
        long[] target = ints2longs(ch2dot4g);
        addChannelsIntoAdapter(target, adapter, byOrder);
    }
    
    public void addSupported5gChannelsByBandwidth(ArrayAdapter<String> adapter, int bandwidth, boolean byOrder) {
        int[] channels = getSupported5gChannelsByBandwidth(bandwidth);
        if (channels == null) {
            return;
        }
        long[] target = ints2longs(channels);
        addChannelsIntoAdapter(target, adapter, byOrder, false);
    }
    
    private int[] getSupported5gChannelsByBandwidth(int bandwidth) {
        List<Integer> list = new ArrayList<Integer>();
        boolean isCached = true;
        if (bandwidth == BW_20M) {
            int[] cachedChs = mCachedSupportedChs.get(BW_20M);
            if (cachedChs != null) {
                return cachedChs;
            }
            int[] chsupported = longs2ints(getCachedSupportChannels());
            for (int i = 0; i < chsupported.length; i++) {
                int id = chsupported[i];
                if (isIn5gChannelBandwidth(id, bandwidth)) {
                    list.add(id);
                }
            }
        } else if (bandwidth == BW_40M) {
            int[] cachedChs = mCachedSupportedChs.get(BW_40M);
            if (cachedChs != null) {
                return cachedChs;
            }
            for (int i = 0; i < s5gBw40MChannels.length; i++) {
                int id = getChannelIdIntern(i, s5gBw40MChannels);
                if (isChannelSupported(id, BW_40M)) {
                    list.add(id);
                }
            }
        } else if (bandwidth == BW_80M) {
            int[] cachedChs = mCachedSupportedChs.get(BW_80M);
            if (cachedChs != null) {
                return cachedChs;
            }
            for (int i = 0; i < s5gBw80MChannels.length; i++) {
                int id = getChannelIdIntern(i, s5gBw80MChannels);
                if (isChannelSupported(id, BW_80M)) {
                    list.add(id);
                }
            }
        } else {
            Elog.d(TAG, "getSupported5gChannelsByBandwidth invalid bandwidth:" + bandwidth);
            isCached = false;
        }
        if (list.size() > 0) {
            int[] target = new int[list.size()];
            for (int i = 0; i < target.length; i++) {
                target[i] = list.get(i);
            }
            if (isCached) {
                mCachedSupportedChs.put(bandwidth, target);
            }
            return target;
        } else {
            return null;
        }
    }
    
    private int[] longs2ints(long[] array) {
        if (array == null || array.length == 0) {
            return null;
        }
        int[] ints = new int[array.length];
        for (int i = 0; i < array.length; i++) {
            ints[i] = (int)array[i];
        }
        return ints;
    }
    
    private long[] ints2longs(int[] array) {
        if (array == null || array.length == 0) {
            return null;
        }
        long[] longs = new long[array.length];
        for (int i = 0; i < array.length; i++) {
            longs[i] = array[i];
        }
        return longs;
    }
    
    public void insertBw40MChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < s5gBw40MChannels.length; i++) {
            int id = getChannelIdIntern(i, s5gBw40MChannels);
            if (isChannelSupported(id, BW_40M)) {
                String name = getChannelNameIntern(i, s5gBw40MChannels);
                insertChannelIntoAdapterByOrder(adapter, name);
            }
        }
    }
    
    private int computeInsertIndex(ArrayAdapter<String> adapter, int channel) {
        int targetIndex = -1;
        for (int i = 0; i < adapter.getCount(); i++) {
            String name = adapter.getItem(i);
            int id = extractChannelIdFromName(name);
            if (id > channel) {
                targetIndex = i;
                break;
            }
        }
        if(targetIndex == -1) {
            targetIndex = adapter.getCount();
        } 
        return targetIndex;
    }
    
    public String getChannelNameById(long channel) {
        int id = (int)channel;
        String name = null;
        ChannelData cd = mChannelDataDb.get(id);
        if (cd != null) {
            name = cd.name;
        }
        return name;
    }
    
    private int extractChannelIdFromName(String fullName) {
        int id = -1;
        String[] strs = fullName.split(" +");
        if (strs.length == 3) {
            try {
                id = Integer.valueOf(strs[1]);
            } catch (NumberFormatException e) {
                Elog.d(TAG, "NumberFormatException:" + e.getMessage());
            }
        } else {
            Elog.d(TAG, "extractChannelIdFromName(): " + fullName + " invalid name format!");
        }
        return id;
    }
    
    public void insertChannelIntoAdapterByOrder(ArrayAdapter<String> adapter, String channelName) {
        if (adapter.getPosition(channelName) == -1) {
            int id = extractChannelIdFromName(channelName);
            int targetIndex = computeInsertIndex(adapter, id);
            adapter.insert(channelName, targetIndex);
        }
    }
    
    private boolean isIn5gChannelBandwidth(int channel, int bandwidth) {
        if (channel >= 1 && channel <= 14) { // channel 1~14 is 2.4G
            return false;
        }
        
        if (bandwidth == BW_20M) {
            boolean flag = true;
            if (isInDetailObjArray(channel, s5gBw40MChannels)) {
                flag = false;
            }
            if(isInDetailObjArray(channel, s5gBw80MChannels)) {
                flag = false;
            }
            return flag;
        } else if (bandwidth == BW_40M) {
            return isInDetailObjArray(channel, s5gBw40MChannels);
        } else if (bandwidth == BW_80M) {
            return isInDetailObjArray(channel, s5gBw80MChannels);
        } else {
            Elog.d(TAG, "is5gChannel, Invalid bandwidth type:" + bandwidth);
            return false;
        }
    }
    
    private boolean isInDetailObjArray(int channel, Object[][] detailObjArr) {
        for (int i = 0; i < detailObjArr.length; i++) {
            int id = getChannelIdIntern(i, detailObjArr);
            if (channel == id) {
                return true;
            }
        }
        return false;
    }
    
    private boolean isChannelSupported(int channel, int bandwidth) {
        boolean supported = true;
        int[] testChannels = null;
        if (bandwidth == BW_20M) {
            testChannels = new int[]{channel};
        } else if (bandwidth == BW_40M) {
            testChannels = new int[]{channel - 2, channel + 2};
        } else if (bandwidth == BW_80M) {
            testChannels = new int[]{channel - 6, channel - 2, channel + 2, channel + 6};
        } else {
            Elog.d(TAG, "Invalid bandwidth:" + bandwidth);
            return false;
        }
        
        for (int i = 0; i < testChannels.length; i++) {
            int ch = testChannels[i];
            if (!isContains(ch)) {
                supported = false;
                break;
            }
        }
        //Elog.d(TAG, "isChannelSupported: channel:" + channel + " bandwidth:" + bandwidth + " supported:" + supported);
        return supported;
    }

    /**
     * Get channel frequency
     * 
     * @return Channel frequency
     */
    public int getChannelFreq() {
        return mChannelFreq[getChannelIndex()];
    }

    /**
     * Get index in the channel array by Channel name
     * 
     * @return The channel index
     */
    public int getChannelIndex() {
        for (int i = 0; i < mFullChannelName.length; i++) {
            if (mFullChannelName[i].equals(mChannelSelect)) {
                return i;
            }
        }
        return 0;
    }

    /**
     * Check the channel is support or not in the phone
     * 
     * @param channel
     *            Need to check the channel's number
     * @return True if phone support the channel, or false
     */
    public boolean isContains(int channel) {
        for (int i = 1; i <= sChannels[0]; i++) {
            if (channel == sChannels[i]) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Get WiFi chip support channels
     */
    public static void getSupportChannels() {
        sChannels = new long[MAX_CHANNEL_COUNT];
        if (EMWifi.sIsInitialed) {
            if (0 == EMWifi.getSupportChannelList(sChannels)) {
                Xlog.v(TAG, "LENGTH channels[0] = " + sChannels[0]);
                for (int i = 1; i <= sChannels[0]; i++) {
                    if (CHANNEL_NUMBER_14 == sChannels[i]) {
                        sHas14Ch = true;
                    }
                    if (sChannels[i] > CHANNEL_NUMBER_14) {
                        sHasUpper14Ch = true;
                    }
                    Xlog.v(TAG, "channels[" + (i) + "] = " + sChannels[i]);
                }
            } else {
                sChannels[0] = DEFAULT_CHANNEL_COUNT;
                for (int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
                    sChannels[i + 1] = i + 1;
                }
            }
        } else {
            Xlog.v(TAG, "Wifi is not initialed");
        }
    }

    /**
     * Constructor
     */
    public ChannelInfo() {
        mChannelSelect = mFullChannelName[0];
        initChannelDataDatabase();
    }
}
