package com.mediatek.sensorhub;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public class ConditionItem implements Parcelable {
    private boolean mBracketLeft;
    private boolean mBracketRight;
    private int mCombine;
    private int mIndex1;
    private boolean mIsLast1;
    private int mIndex2;
    private boolean mIsLast2;
    private int mOperation;
    private int mDataType = DataCell.DATA_TYPE_INVALID;//data type
    private int mIntValue;
    private long mLongValue;
    private float mFloatValue;
    private int mConditionType = CONDITION_TYPE_INVALID;

    public static final int CONDITION_TYPE_INVALID = 0x00;
    public static final int CONDITION_TYPE_CMP = 0x20;
    public static final int CONDITION_TYPE_DIFF = 0x10;
    public static final int CONDITION_TYPE_DATA = 0x0F;

    public ConditionItem(int index, boolean isLast, int operation, int value) {
        mConditionType = CONDITION_TYPE_DATA;
        mIndex1 = index;
        mIsLast1 = isLast;
        mOperation = operation;
        mDataType = DataCell.DATA_TYPE_INT;
        mIntValue = value;
    }

    public ConditionItem(int index, boolean isLast, int operation, long value) {
        mConditionType = CONDITION_TYPE_DATA;
        mIndex1 = index;
        mIsLast1 = isLast;
        mOperation = operation;
        mDataType = DataCell.DATA_TYPE_LONG;
        mLongValue = value;
    }

    public ConditionItem(int index, boolean isLast, int operation, float value) {
        mConditionType = CONDITION_TYPE_DATA;
        mIndex1 = index;
        mIsLast1 = isLast;
        mOperation = operation;
        mDataType = DataCell.DATA_TYPE_FLOAT;
        mFloatValue = value;
    }

    public ConditionItem(int index1, boolean isLast1, int operation, int index2, boolean isLast2) {
        mConditionType = CONDITION_TYPE_CMP;
        mIndex1 = index1;
        mIsLast1 = isLast1;
        mOperation = operation;
        mIndex2 = index2;
        mIsLast2 = isLast2;
    }

    public ConditionItem(int index1, boolean isLast1, int operation, int index2, boolean isLast2, int value) {
        mConditionType = CONDITION_TYPE_DIFF;
        mIndex1 = index1;
        mIsLast1 = isLast1;
        mOperation = operation;
        mIndex2 = index2;
        mIsLast2 = isLast2;
        mDataType = DataCell.DATA_TYPE_INT;
        mIntValue = value;
    }

    public ConditionItem(int index1, boolean isLast1, int operation, int index2, boolean isLast2, long value) {
        mConditionType = CONDITION_TYPE_DIFF;
        mIndex1 = index1;
        mIsLast1 = isLast1;
        mOperation = operation;
        mIndex2 = index2;
        mIsLast2 = isLast2;
        mDataType = DataCell.DATA_TYPE_LONG;
        mLongValue = value;
    }

    public ConditionItem(int index1, boolean isLast1, int operation, int index2, boolean isLast2, float value) {
        mConditionType = CONDITION_TYPE_DIFF;
        mIndex1 = index1;
        mIsLast1 = isLast1;
        mOperation = operation;
        mIndex2 = index2;
        mIsLast2 = isLast2;
        mDataType = DataCell.DATA_TYPE_FLOAT;
        mFloatValue = value;
    }

    public void setCombine(int combineType) {
        mCombine = combineType;
    }

    public void setBracketLeft() {
        mBracketLeft = true;
    }

    public boolean isBracketLeft() {
        return mBracketLeft;
    }

    public void setBracketRight() {
        mBracketRight = true;
    }

    public boolean isBracketRight() {
        return mBracketRight;
    }

    @Override
    public String toString() {
        return "ConditionItem [mBracketLeft=" + mBracketLeft
                + ", mBracketRight=" + mBracketRight + ", mCombine=" + mCombine
                + ", mIndex1=" + mIndex1 + ", mIsLast1=" + mIsLast1
                + ", mIndex2=" + mIndex2 + ", mIsLast2=" + mIsLast2
                + ", mOperation=" + mOperation + ", mDataType=" + mDataType
                + ", mIntValue=" + mIntValue + ", mLongValue=" + mLongValue
                + ", mFloatValue=" + mFloatValue + ", mConditionType="
                + mConditionType + "]";
    }

    public ConditionItem(Parcel source) {
        readFromParcel(source);
    }

    public static final Parcelable.Creator<ConditionItem> CREATOR
            = new Parcelable.Creator<ConditionItem>() {
        @Override
        public ConditionItem createFromParcel(Parcel source) {
            return new ConditionItem(source);
        }

        @Override
        public ConditionItem[] newArray(int size) {
            return new ConditionItem[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    public void readFromParcel(Parcel source) {
        mBracketLeft = source.readInt() > 0;
        mBracketRight = source.readInt() > 0;
        mCombine = source.readInt();
        mIndex1 = source.readInt();
        mIsLast1 = source.readInt() > 0;
        mIndex2 = source.readInt();
        mIsLast2 = source.readInt() > 0;
        mOperation = source.readInt();
        mDataType = source.readInt();
        mIntValue = source.readInt();
        mLongValue = source.readLong();
        mFloatValue = source.readFloat();
        mConditionType = source.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mBracketLeft ? 1: 0);
        dest.writeInt(mBracketRight ? 1: 0);
        dest.writeInt(mCombine);
        dest.writeInt(mIndex1);
        dest.writeInt(mIsLast1 ? 1: 0);
        dest.writeInt(mIndex2);
        dest.writeInt(mIsLast2 ? 1: 0);
        dest.writeInt(mOperation);
        dest.writeInt(mDataType);
        dest.writeInt(mIntValue);
        dest.writeLong(mLongValue);
        dest.writeFloat(mFloatValue);
        dest.writeInt(mConditionType);
    }
}
