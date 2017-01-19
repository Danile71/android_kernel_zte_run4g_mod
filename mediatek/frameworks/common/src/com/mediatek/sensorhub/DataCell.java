package com.mediatek.sensorhub;


import android.os.Parcel;
import android.os.Parcelable;

/**
 * DataCell represents a context data slot.
 */
public class DataCell implements Parcelable {
    /**
     * A constant indicates data slot value type is invalid.
     */
    public static final int DATA_TYPE_INVALID = 0;

    /**
     * A constant indicates data slot value type is integer.
     */
    public static final int DATA_TYPE_INT = 1;

    /**
     * A constant indicates data slot value type is long.
     */
    public static final int DATA_TYPE_LONG = 2;

    /**
     * A constant indicates data slot value type is floating.
     */
    public static final int DATA_TYPE_FLOAT = 3;

    private int mIndex;
    private boolean mIsPrevious;
    private int mType = DATA_TYPE_INVALID;//data type
    private int mIntValue;
    private long mLongValue;
    private float mFloatValue;

    /*package*/ DataCell() {
    }

    /*package*/ DataCell(int dataIndex, boolean isPrevious, int value) {
        mIndex = dataIndex;
        mIsPrevious = isPrevious;
        mType = DATA_TYPE_INT;
        mIntValue = value;
    }

    /*package*/ DataCell(int dataIndex, boolean isPrevious, float value) {
        mIndex = dataIndex;
        mIsPrevious = isPrevious;
        mType = DATA_TYPE_FLOAT;
        mFloatValue = value;
    }

    /**
     * Long value type constructor.
     * 
     * @param dataIndex data slot index
     * @param isPrevious whether the data is the previous one or not.
     * @param value data value
     * 
     * @hide
     */
    public DataCell(int dataIndex, boolean isPrevious, long value) {
        mIndex = dataIndex;
        mIsPrevious = isPrevious;
        mType = DATA_TYPE_LONG;
        mLongValue = value;
    }

    /**
     * Indicates whether the data slot value is the previous one.
     * 
     * @return {@code true} if the value is the previous one, {@code false} otherwise.
     */
    public boolean isPrevious() {
        return mIsPrevious;
    }

    /**
     * Gets the index of this data slot.
     * 
     * @return The data slot index.
     */
    public int getIndex() {
        return mIndex;
    }

    /**
     * Gets the type of this data slot.
     * 
     * @return The value type of this data slot.
     * 
     * @see #DATA_TYPE_INVALID
     * @see #DATA_TYPE_INT
     * @see #DATA_TYPE_LONG
     * @see #DATA_TYPE_FLOAT
     */
    public int getType() {
        return mType;
    }

    /**
     * Gets the {@code integer} value of this data slot.
     * 
     * @return The {@code integer} value of this data slot. 
     * If current data slot value type is {@code float} or {@code long}, it will return 0.
     * 
     */
    public int getIntValue() {
        return mIntValue;
    }

    /**
     * Gets the {@code float} value of this data slot.
     * 
     * @return The {@code float} value of this data slot. 
     * If current data slot type is {@code integer} or {@code long}, it will return 0.0F.
     */
    public float getFloatValue() {
        return mFloatValue;
    }

    /**
     * Gets the {@code long} type value of this data slot.
     * 
     * @return The {@code long} type value of this data slot. 
     * If current data slot type is {@code integer} or {@code float}, it will return 0L.
     */
    public long getLongValue() {
        return mLongValue;
    }

    /*package*/ DataCell setDataIndex(int dataIndex) {
        mIndex = dataIndex;
        return this;
    }

    /*package*/ DataCell setLast(boolean isLast) {
        mIsPrevious = isLast;
        return this;
    }

    /*package*/ DataCell setValue(int value) {
        mType = DATA_TYPE_INT;
        mIntValue = value;
        return this;
    }

    /*package*/ DataCell setValue(long value) {
        mType = DATA_TYPE_LONG;
        mLongValue = value;
        return this;
    }

    /*package*/ DataCell setValue(float value) {
        mType = DATA_TYPE_FLOAT;
        mFloatValue = value;
        return this;
    }

    /**
     * @hide
     */
    public static final Parcelable.Creator<DataCell> CREATOR = new Parcelable.Creator<DataCell>() {
        @Override
        public DataCell createFromParcel(Parcel source) {
            return new DataCell(source);
        }
        @Override
        public DataCell[] newArray(int size) {
            return new DataCell[size];
        }
    };

    private DataCell(Parcel source) {
        mIndex = source.readInt();
        mIsPrevious = source.readInt() > 0 ? true : false;
        mType = source.readInt();
        switch(mType) {
        case DATA_TYPE_FLOAT:
            mFloatValue = source.readFloat();
            break;
        case DATA_TYPE_INT:
            mIntValue = source.readInt();
            break;
        case DATA_TYPE_LONG:
            mLongValue = source.readLong();
            break;
        default:
            break;
        }
    }

    /**
     * @hide
     */
    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * @hide
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mIndex);
        dest.writeInt(mIsPrevious ? 1 : 0);
        dest.writeInt(mType);
        switch(mType) {
        case DATA_TYPE_FLOAT:
            dest.writeFloat(mFloatValue);
            break;
        case DATA_TYPE_INT:
            dest.writeInt(mIntValue);
            break;
        case DATA_TYPE_LONG:
            dest.writeLong(mLongValue);
            break;
        default:
            break;
        }
    }
}
