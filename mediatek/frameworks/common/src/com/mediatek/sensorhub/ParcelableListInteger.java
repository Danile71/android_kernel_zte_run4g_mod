package com.mediatek.sensorhub;

import java.util.ArrayList;
import java.util.List;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public class ParcelableListInteger implements Parcelable {
    private List<Integer> mList = new ArrayList<Integer>();
    public ParcelableListInteger(List<Integer> list) {
        mList.clear();
        mList.addAll(list);
    }

    public ParcelableListInteger(int[] list) {
        mList.clear();
        if (list != null) {
            for (int i = 0, size = list.length; i < size; i++) {
                mList.add(list[i]);
            }
        }
    }

    public List<Integer> toList() {
        return new ArrayList<Integer>(mList);
    }

    private ParcelableListInteger(Parcel source) {
        int size = source.readInt();
        mList.clear();
        for (int i = 0; i < size; i++) {
            mList.add(source.readInt());
        }
    }

    public static final Parcelable.Creator<ParcelableListInteger> CREATOR
            = new Parcelable.Creator<ParcelableListInteger>() {
        @Override
        public ParcelableListInteger createFromParcel(Parcel source) {
            return new ParcelableListInteger(source);
        }
        @Override
        public ParcelableListInteger[] newArray(int size) {
            return new ParcelableListInteger[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        int size = mList.size();
        dest.writeInt(size);
        for (int i = 0; i < size; i++) {
            dest.writeInt(mList.get(i));
        }
    }
}
