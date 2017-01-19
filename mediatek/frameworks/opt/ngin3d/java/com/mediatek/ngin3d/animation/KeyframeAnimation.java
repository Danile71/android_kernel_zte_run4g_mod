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

import android.util.Log;
import com.mediatek.ngin3d.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Keyframe animation that animate an actor by specified keyframe data.
 */
public class KeyframeAnimation extends BasicAnimation {
    private static final String TAG = "KeyframeAnimation";
    private float mXRotation = 0;
    private float mYRotation = 0;
    private float mZRotation = 0;
    private Rotation mOrientation;
    private Rotation mXYZRotation = new Rotation();
    private Rotation mAppliedRotation = new Rotation();
    private KeyframeDataSet mKfDataSet;
    private ArrayList<KeyframeInterpolator> mInterpolatorSet;
    private List<Actor> mTarget = Collections.synchronizedList(new ArrayList());


    public KeyframeAnimation(KeyframeDataSet kfDataSet) {
        this(null, kfDataSet);
    }

    public KeyframeAnimation(Actor target, KeyframeDataSet kfDataSet) {
        mKfDataSet = kfDataSet;
        setTarget(target);
        setupTimelineListener();
    }

    private void setupTimelineListener() {
        mTimeline.addListener(new Timeline.Listener() {
            public void onStarted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is started", MasterClock.getTime(), KeyframeAnimation.this));
                }

                if (!mTarget.isEmpty()) {
                    for (Actor actor : mTarget) {
                        actor.onAnimationStarted("Keyframe", KeyframeAnimation.this);
                    }
                }

                if ((mOptions & START_TARGET_WITH_INITIAL_VALUE) != 0) {
                    if (getDirection() == FORWARD) {
                        onAnimate(0);
                    } else {
                        onAnimate(timeline.getOriginalDuration());
                    }
                }
            }

            public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                onAnimate((float) elapsedMsecs);
            }

            public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
                // KeyframeAnimation onMarkerReached callback function
            }

            public void onPaused(Timeline timeline) {
                if (!mTarget.isEmpty()) {
                    for (Actor actor : mTarget) {
                        actor.onAnimationStopped("Keyframe");
                    }
                }
            }

            public void onCompleted(Timeline timeline) {
                if ((mOptions & DEBUG_ANIMATION_TIMING) != 0) {
                    Log.d(TAG, String.format("%d: Animation %s is completed, target is %s", MasterClock.getTime(), KeyframeAnimation.this, mTarget));
                }
                if ((mOptions & Animation.BACK_TO_START_POINT_ON_COMPLETED) == 0) {
                    if (getDirection() == Timeline.FORWARD) {
                        onAnimate(timeline.getOriginalDuration());
                    } else {
                        onAnimate(0);
                    }
                } else {
                    if (getDirection() == Timeline.FORWARD) {
                        onAnimate(0);
                    } else {
                        onAnimate(timeline.getOriginalDuration());
                    }
                }
                if (!mTarget.isEmpty()) {
                    for (Actor actor : mTarget) {
                        actor.onAnimationStopped("Keyframe");
                    }
                }

            }

            public void onLooped(Timeline timeline) {
                // do nothing now
            }
        });
    }

    private void onAnimate(float timeMs) {
        if (mTarget.isEmpty()) {
            return;
        }

        float currTime = timeMs / 1000;
        int size = mInterpolatorSet.size();
        for (int i = 0; i < size; i++) {
            KeyframeInterpolator interpolator = mInterpolatorSet.get(i);
            Object value = interpolator.getValue(currTime);
            if (value == null) {
                continue;
            }
            animateTarget(interpolator.getType(), value, interpolator.isNormalized());
        }
    }

    private void animateTarget(int type, Object value, boolean normalized) {
        switch (type) {
        case Samples.ANCHOR_POINT:
            for (Actor actor : mTarget) {
                if (actor instanceof Plane) {
                    ((Plane) actor).setAnchorPoint((Point) value);
                } else if (actor instanceof BitmapText) {
                    ((BitmapText) actor).setAnchorPoint((Point) value);
                }
            }
            break;
        case Samples.TRANSLATE:
            if (normalized) {
                ((Point) value).isNormalized = true;
            }
            for (Actor actor : mTarget) {
                actor.setPosition((Point) value);
            }
            break;
        case Samples.ROTATE:
            mOrientation = (Rotation) value;
            updateTargetRotation();
            break;
        case Samples.X_ROTATE:
            mXRotation = (Float) value;
            updateTargetRotation();
            break;
        case Samples.Y_ROTATE:
            mYRotation = (Float) value;
            updateTargetRotation();
            break;
        case Samples.Z_ROTATE:
            mZRotation = (Float) value;
            updateTargetRotation();
            break;
        case Samples.SCALE:
            for (Actor actor : mTarget) {
                actor.setScale((Scale) value);
            }
            break;
        case Samples.ALPHA:
            int opacity = (int) (2.55 * (Float) value);
            for (Actor actor : mTarget) {
                actor.setOpacity(opacity);
            }
            break;
        default:
            // do nothing.
            break;

        }
    }

    @Override
    public final Animation setTarget(Actor target) {
        mTarget.clear();
        if (target != null) {
            mTarget.add(target);
        }

        // Avoid two Actors reference the same Interpolator when animating
        // switch target actor and restart in one tick.
        mInterpolatorSet = new ArrayList<KeyframeInterpolator>();
        setDuration(0);
        for (KeyframeData kfData : mKfDataSet.getList()) {
            // Create Interpolator by keyframe samples
            KeyframeInterpolator interpolator = new KeyframeInterpolator(kfData.getSamples());
            interpolator.setNormalized(kfData.isNormalized());
            mInterpolatorSet.add(interpolator);
            if (interpolator.getDuration() > getDuration()) {
                setDuration(interpolator.getDuration());
            }
        }
        return this;
    }

    public final Animation setTarget(Actor... target) {
        mTarget.clear();
        mTarget.addAll(Arrays.asList(target));

        // Avoid two Actors reference the same Interpolator when animating
        // switch target actor and restart in one tick.
        mInterpolatorSet = new ArrayList<KeyframeInterpolator>();
        setDuration(0);
        for (KeyframeData kfData : mKfDataSet.getList()) {
            // Create Interpolator by keyframe samples
            KeyframeInterpolator interpolator = new KeyframeInterpolator(kfData.getSamples());
            interpolator.setNormalized(kfData.isNormalized());
            mInterpolatorSet.add(interpolator);
            if (interpolator.getDuration() > getDuration()) {
                setDuration(interpolator.getDuration());
            }
        }
        return this;
    }

    private void updateTargetRotation() {
        Quaternion appliedQ = mAppliedRotation.getQuaternion();
        Quaternion xyzQ = mXYZRotation.getQuaternion();
        mXYZRotation.set(EulerOrder.ZYX, mXRotation, mYRotation, mZRotation);
        appliedQ.set(xyzQ);
        appliedQ.multiply(mOrientation.getQuaternion());
        for (Actor actor : mTarget) {
            actor.setRotation(mAppliedRotation);
        }
    }

    @Override
    public Actor getTarget() {
        if (mTarget.isEmpty()) {
            return null;
        }
        return mTarget.get(0);
    }

    public List<Actor> getAllTargets() {
        return mTarget;
    }

    @Override
    public Animation reset() {
        super.reset();
        if (getDirection() == FORWARD) {
            onAnimate(0);
        } else {
            onAnimate(getOriginalDuration());
        }

        return this;
    }

    @Override
    public Animation complete() {
        super.complete();
        if (getDirection() == FORWARD) {
            onAnimate(getOriginalDuration());
        } else {
            onAnimate(0);
        }
        return this;
    }

    /**
     * Clone the KeyframeAnimation, value in each member of cloned animation is same of original one, except target.
     * Mew instance of KeyframeAnimation has no target in default.
     *
     * @return the cloned KeyframeAnimation
     */
    @Override
    public KeyframeAnimation clone() {
        KeyframeAnimation animation = (KeyframeAnimation) super.clone();
        animation.setupTimelineListener();
        animation.mTarget = new ArrayList<Actor>();
        return animation;
    }
}
