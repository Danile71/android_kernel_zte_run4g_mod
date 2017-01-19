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

package com.mediatek.ngin3d.animation;

import com.mediatek.ngin3d.EulerOrder;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;

/**
 * The interpolator of keyframe.
 * @hide
 */
public class KeyframeInterpolator {
    private final int mSampleType;
    private Object mValue;
    // For optimization use
    private Object mOldValue;

    private boolean mNormalized;
    private boolean mIsStatic = false;
    private int mLen;
    private final int[] mCurveType;
    private final float[] mKfTime;
    private float[] mValX;
    private float[] mValY;
    private float[] mValZ;
    private float[] mInTanX;
    private float[] mInTanY;
    private float[] mInTanZ;
    private float[] mOutTanX;
    private float[] mOutTanY;
    private float[] mOutTanZ;

    private static final int FORMULA_NONE = 0x00;
    private static final int FORMULA_TEMPORAL_CONTINUOUS = 0x01;
    private static final int FORMULA_TEMPORAL_AUTOBEZIER = 0x02;
    private static final int FORMULA_SPATIAL_CONTINUOUS = 0x04;
    private static final int FORMULA_SPATIAL_AUTOBEZIER = 0x08;
    private static final int FORMULA_ROVING = 0x10;

    Rotation mEulerStart = new Rotation();
    Rotation mEulerEnd = new Rotation();

    public KeyframeInterpolator(Samples samples) {
        mSampleType = samples.getType();
        mCurveType = samples.getInt(Samples.CURVE_TYPE);
        mKfTime = samples.get(Samples.KEYFRAME_TIME);

        if (mKfTime == null) {
            mLen = 0;
        } else {
            mLen = mKfTime.length;
        }

        if (mSampleType == Samples.ALPHA) {
            mValX = samples.get(Samples.VALUE);
            // Dummy for alpha
            mValY = new float[mLen];
            mValZ = new float[mLen];
            mInTanX = samples.get(Samples.IN_TANVAL);
            mOutTanX = samples.get(Samples.OUT_TANVAL);
        } else {
            mValX = samples.get(Samples.X_AXIS);
            mValY = samples.get(Samples.Y_AXIS);
            mValZ = samples.get(Samples.Z_AXIS);
            mInTanX = samples.get(Samples.IN_TANX);
            mInTanY = samples.get(Samples.IN_TANY);
            mInTanZ = samples.get(Samples.IN_TANZ);
            mOutTanX = samples.get(Samples.OUT_TANX);
            mOutTanY = samples.get(Samples.OUT_TANY);
            mOutTanZ = samples.get(Samples.OUT_TANZ);
        }

        if (mSampleType == Samples.ALPHA || mSampleType == Samples.X_ROTATE) {
            mValue = mValX[0];
            mOldValue = -mValX[0];
        } else if (mSampleType == Samples.Y_ROTATE) {
            mValue = mValY[0];
            mOldValue = -mValY[0];
        } else if (mSampleType == Samples.Z_ROTATE) {
            mValue = mValZ[0];
            mOldValue = -mValZ[0];
        } else if (mSampleType == Samples.ROTATE) {
            mValue = new Rotation(mValX[0], mValY[0], mValZ[0]);
            mOldValue = new Rotation(-mValX[0], mValY[0], mValZ[0]);
        } else if (mSampleType == Samples.TRANSLATE || mSampleType == Samples.ANCHOR_POINT) {
            mValue = new Point(mValX[0], mValY[0], mValZ[0]);
            mOldValue = new Point(-mValX[0], mValY[0], mValZ[0]);
        } else if (mSampleType == Samples.SCALE) {
            mValue = new Scale(mValX[0] / 100, mValY[0] / 100, mValZ[0] / 100);
            mOldValue = new Scale(-mValX[0] / 100, mValY[0] / 100, mValZ[0] / 100);
        } else {
            throw new RuntimeException("Not excepted Sample type");
        }

    }

    public Object getValue(float currTime) {
        boolean sameValue = false;
        int nKf = 0;
        if (mLen == 0) {
            // [Optimization] It's static animation data, no need to update it every frame
            if (mIsStatic) {
                return null;
            } else {
                mIsStatic = true;
                return mValue;
            }
        }

        for (; nKf < mLen; nKf++) {
            if (currTime <= mKfTime[nKf]) {
                break;
            }
        }

        if (currTime <= mKfTime[0]) {
            sameValue = setValue(mValX[0], mValY[0], mValZ[0]);
        } else if (currTime >= mKfTime[mLen - 1]) {
            sameValue = setValue(mValX[mLen - 1], mValY[mLen - 1], mValZ[mLen - 1]);
        } else if (nKf != 0 && nKf < mLen) {
            if (mCurveType[nKf] == FORMULA_SPATIAL_CONTINUOUS && mCurveType[nKf - 1] == FORMULA_SPATIAL_CONTINUOUS) {
                bezier(currTime, nKf);
            } else {
                linear(currTime, nKf);
            }
        } else {
            return null;
        }
        // Optimization for animation, if the value is same as previous one, skip it.
        if (sameValue) {
            return null;
        }
        return mValue;
    }

    private boolean bezier(float currTime, int nKf) {
        // Cubic bezier curves
        // (1-t)^3 * P0 + 3 * (1-t)^2 * P1 + 3* (1-t) * t^2 * P2 + t^3 * P3
        float t = (currTime - mKfTime[nKf - 1]) / ((mKfTime[nKf] - mKfTime[nKf - 1]));
        float t2 = t * t;
        float t3 = t2 * t;
        float tc = (1 - t);
        float tc2 = tc * tc;
        float tc3 = tc2 * tc;
        float valX = 0;
        float valY = 0;
        float valZ = 0;
        float p0;
        float p1;
        float p2;
        float p3;

        p0 = mValX[nKf - 1];
        p1 = mValX[nKf - 1] + mOutTanX[nKf - 1];
        p2 = mValX[nKf] + mInTanX[nKf];
        p3 = mValX[nKf];
        valX = tc3 * p0 + 3 * tc2 * t * p1 + 3 * tc * t2 * p2 + t3 * p3;

        if (mSampleType != Samples.ALPHA) {
            p0 = mValY[nKf - 1];
            p1 = mValY[nKf - 1] + mOutTanY[nKf - 1];
            p2 = mValY[nKf] + mInTanY[nKf];
            p3 = mValY[nKf];
            valY = tc3 * p0 + 3 * tc2 * t * p1 + 3 * tc * t2 * p2 + t3 * p3;

            p0 = mValZ[nKf - 1];
            p1 = mValZ[nKf - 1] + mOutTanZ[nKf - 1];
            p2 = mValZ[nKf] + mInTanZ[nKf];
            p3 = mValZ[nKf];
            valZ = tc3 * p0 + 3 * tc2 * t * p1 + 3 * tc * t2 * p2 + t3 * p3;
        }
        return setValue(valX, valY, valZ);
    }

    private boolean linear(float currTime, int nKf) {
        float px0;
        float py0;
        float pz0;
        float px1;
        float py1;
        float pz1;
        float valX = 0;
        float valY = 0;
        float valZ = 0;
        float t = (currTime - mKfTime[nKf - 1]) / ((mKfTime[nKf] - mKfTime[nKf - 1]));

        if (mSampleType == Samples.ROTATE) {
            px0 = mValX[nKf - 1];
            py0 = mValY[nKf - 1];
            pz0 = mValZ[nKf - 1];
            px1 = mValX[nKf];
            py1 = mValY[nKf];
            pz1 = mValZ[nKf];

            Rotation rot = (Rotation) mValue;
            mEulerStart.set(EulerOrder.ZYX, px0, py0, pz0);
            mEulerEnd.set(EulerOrder.ZYX, px1, py1, pz1);
            Quaternion linearQ = mEulerStart.getQuaternion().slerp(mEulerEnd.getQuaternion(), t);
            rot.set(linearQ.getQ0(), linearQ.getQ1(), linearQ.getQ2(), linearQ.getQ3(), true);
            return false;
        } else {
            px0 = mValX[nKf - 1];
            px1 = mValX[nKf];
            valX = px0 * (1 - t) + px1 * t;

            if (mSampleType != Samples.ALPHA) {
                py0 = mValY[nKf - 1];
                py1 = mValY[nKf];
                valY = py0 * (1 - t) + py1 * t;

                pz0 = mValZ[nKf - 1];
                pz1 = mValZ[nKf];
                valZ = pz0 * (1 - t) + pz1 * t;
            }
        }
        return setValue(valX, valY, valZ);
    }

    private boolean isSameValue() {
        if (mOldValue == mValue) {
            return true;
        } else {
            mOldValue = mValue;
            return false;
        }
    }

    private boolean setValue(float x, float y, float z) {
        boolean sameValue = false;
        if (mSampleType == Samples.ALPHA) {
            mValue = x;
            sameValue = isSameValue();
        } else if (mSampleType == Samples.X_ROTATE) {
            mValue = x;
            sameValue = isSameValue();
        } else if ( mSampleType == Samples.Y_ROTATE) {
            mValue = y;
            sameValue = isSameValue();
        } else if ( mSampleType == Samples.Z_ROTATE) {
            mValue = z;
            sameValue = isSameValue();
        } else if (mSampleType == Samples.TRANSLATE || mSampleType == Samples.ANCHOR_POINT) {
            Point point = (Point)mValue;
            point.x = x;
            point.y = y;
            point.z = z;
            Point oldPoint = (Point)mOldValue;
            if (oldPoint.x == point.x && oldPoint.x == point.y && oldPoint.z == point.z) {
                sameValue = true;
            } else {
                oldPoint.x = x;
                oldPoint.y = y;
                oldPoint.z = z;
            }
        } else if (mSampleType == Samples.SCALE) {
            Scale scale = (Scale)mValue;
            scale.x = x / 100;
            scale.y = y / 100;
            scale.z = z / 100;
            Scale oldScale = (Scale)mOldValue;
            if (oldScale.x == scale.x && oldScale.x == scale.y && oldScale.z == scale.z) {
                sameValue = true;
            } else {
                oldScale.x = x;
                oldScale.y = y;
                oldScale.z = z;
            }
        }

        return sameValue;
    }

    public int getType() {
        return mSampleType;
    }

    public int getDuration() {
        if (mLen == 0)
            return 0;

        return (int)(mKfTime[mLen - 1] * 1000);
    }

    public void setNormalized(boolean isNormalized) {
        mNormalized = isNormalized;
    }

    public boolean isNormalized() {
        return mNormalized;
    }
}
