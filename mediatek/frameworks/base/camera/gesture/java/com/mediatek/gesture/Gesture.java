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

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.gesture;

import android.app.NativeActivity;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.lang.ref.WeakReference;

/**
 * A supported gesture is a specific hand pose, such as open palm, that can be detected
 * when displayed in front of a camera. Once a gesture is detected from camera images, 
 * listener can be notified of this event with supplementary information.
 * <p>When camera with gesture detection application is developed, 
 *    the tip goes through the following steps:</p>
 * <ol>
 * <li>Obtain an instance of Camera from 
 *     {<a href="http://developer.android.com/reference/android/hardware/Camera.html#open(int)">open(int)</a>}.
 *
 * <li>Get the existing (default) settings with 
 *     {<a href="http://developer.android.com/reference/android/hardware/Camera.html#getParameters()">getParameters()</a>}.
 *
 * <li>If necessary, modify the returned {<a href="http://developer.android.com/reference/android/hardware/Camera.Parameters.html">Parameters</a>} 
 *     object and call{<a href="http://developer.android.com/reference/android/hardware/Camera.html#setParameters(android.hardware.Camera.Parameters)">
 *     setParameters(<a href="http://developer.android.com/reference/android/hardware/Camera.Parameters.html">Parameters</a>)</a>}.
 *
 * <li>If desired, call {<a href="http://developer.android.com/reference/android/hardware/Camera.html#setDisplayOrientation(int)">setDisplayOrientation(int)</a>}.
 *
 * <li><b>Important</b>: Pass a fully initialized {<a href="http://developer.android.com/reference/android/view/SurfaceHolder.html">SurfaceHolder</a>} 
 *        to {<a href="http://developer.android.com/reference/android/hardware/Camera.html#setPreviewDisplay(android.view.SurfaceHolder)">
 *        setPreviewDisplay(<a href="http://developer.android.com/reference/android/view/SurfaceHolder.html">SurfaceHolder </a>)</a>}.  
 *        Without a surface, the camera cannot start the preview.
 *
 * <li><b>Important</b>: Call {<a href="http://developer.android.com/reference/android/hardware/Camera.html#startPreview()">startPreview()</a>} 
 *        to start updating the preview surface.  Preview must be started before you can take a picture. 
 *
 * <li>Call startFaceDetection(){<a href="http://developer.android.com/reference/android/hardware/Camera.html#startFaceDetection()">startFaceDetection()</a>} 
 *     to increase detection rate.
 * 
 * <li>Obtain an instance of Gesture from {@link #createGesture()}. 
 *
 * <li>Add an listener to listen to the hand detection result 
 *     {@link #addHandDetectionListener(HandDetectionListener,HandPose)}. 
 * 
 * </ol> 
 *
 * <p>Here is an example of prerequisite declaration:</p>
 * <pre class="prettyprint">
 * 
 *
 *  private final class MyHandDetectionListener implements HandDetectionListener {
 *      public void onHandDetected(HandDetectionEvent event) {
 *           //handle the detection result here ...
 *      }
 *  }
*
 *  public class MainActivity extends Activity {
 *
 *     public void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *         setContentView(R.layout.activity_main);
 *         Camera camera = Camera.open(0);
 *         Parameters parameters = camera.getParameters();
 *         camera.setPreviewDisplay(getActivity().getSurfaceView().getHolder());
 *         camera.startPreview()
 *         camera.startFaceDetection() 
 *         Gesture sGesture = Gesture.createGesture();
 *         MyHandDetectionListener handDetectionListener = new MyHandDetectionListener();
 *         sGesture.addHandDetectionListener(handDetectionListener, HandPose.POSE_OPENPLAM);
 *         //
 *         sGesture.removeHandDetectionListener(handDetectionListener, HandPose.POSE_OPENPLA);
 *
 *     }
 *  }
 * </pre>
 */
 
public class Gesture {
    private static final String TAG = "Gesture_Framework";
    private static Gesture sGesture;
    private HandDetectionListener mHandDetectionListenerr;
    
    Gesture() {
        native_setup(this);
    }
    
    /**
     * Creates a Gesture object.
     * @return a static Gesture object
     */
    public static Gesture createGesture() {
        if (sGesture == null) {
            sGesture = new Gesture();
        }
        return sGesture;
    }
    /**
     * Listener interface for receiving hand gesture events. The class  
     * interested in processing a hand pose event implements this interface, and the 
     * object created with that class is registered with a component using the 
     * component addHandDetectionListener(HandDetectionListener, HandPose) method. 
     * When the hand pose event occurs, that object's appropriate method will be invoked.
     */
    public interface HandDetectionListener {
        
        /**
         * Called when the hand pose event occurs.
         * @param event the detected gesture event;
         */
        public void onHandDetected(HandDetectionEvent event);
    }
    
    /**
     * Object used to report a detected hand event.
     */
    public class HandDetectionEvent {
        
        /**
         * Creates a HandDectionEvent instance.
         */
        public HandDetectionEvent() {
        }
        /**
         * Bounding box of the hand.
         */
        private Rect boundBox;
        
        /**
         * Hand detection confidence (0.0 = 0%, 100.0 = 100%). This is the confidence level 
         * of the detected handedness, based on which hand model is more prominent 
         * in the detection algorithm. The value range is [0,100]; 0 is low confidence 
         * and 100 is high confidence on the handedness (left/right).
         */
        private float confidence;
        
        /**
         * Identifier associated with this event.
         */
        private int id;
        
        /**
         * Identifier associated hand pose.
         */
        private HandPose pose;
        
        /**
         * Set bounding box of the hand.
         * @param boundBox bounding box of the hand
         */
        public void setBoundBox(Rect boundBox) {
            this.boundBox = boundBox;
        }
        
        /**
         * Get bounding box of the hand.
         * @return bounding box of the hand
         */
        public Rect getBoundBox() {
            return this.boundBox;
        }
        
        /**
         * Set hand detection confidence.
         * @param confidence hand detection confidence
         */
        public void setConfidence(float confidence) {
            this.confidence = confidence;
        }
        
        /**
         * Get hand detection confidence.
         * @return hand detection confidence
         */
        public float getConfidence() {
            return this.confidence;
        }
        
        /**
         * Set identifier associated with this event.
         * @param id identifier associated with this event
         */
        public void setId(int id) {
            this.id = id;
        }
        
        /**
         * Get identifier associated with this event.
         * @return identifier associated with this event
         */
        public int getId() {
            return this.id;
        }
        
        /**
         * Set identifier associated hand pose.
         * @param pose identifier associated hand pose
         */
        public void setPose(HandPose pose) {
            this.pose = pose;
        }
        
        /**
         * Get identifier associated hand pose.
         * @return identifier associated hand pose
         */
        public HandPose getPose() {
            return this.pose;
        }
        
    }
    
    /**
     * Indicates the detected pose.
     */
    public enum HandPose {
        /**
         * Supports open palm detection.
         */
        POSE_OPENPLAM(0),
        
        /**
         * Supports victory detection.
         */
        POSE_VICTORY(1);
        
        private int mValue;
        private HandPose(int value) {
            this.mValue = value;
        }
        
        private int getValue() {
            return this.mValue;
        }
    }
    
    /**
     * Registers callbacks for hand gesture detection with the associated hand pose.
     * @param listener a listener that receive hand gesture event
     * @param pose the pose want to detect
     */
    public void addHandDetectionListener(HandDetectionListener listener, HandPose pose) {
        native_addGesture(listener, pose);
    }
    
    /**
     * Unregisters callbacks for hand gesture detection with the associated hand pose.
     * @param listener a listener that receive hand gesture event
     * @param pose the pose do not want to detect
     */
    public void removeHandDetectionListener(HandDetectionListener listener, HandPose pose) {
        native_removeGesture(listener, pose);
    }
    
    static {
        System.loadLibrary("jni_gesture");
    }
    
    private native void native_setup(Gesture object);
    private static native void native_addGesture(HandDetectionListener listener, HandPose pose);
    private static native void native_removeGesture(HandDetectionListener listener, HandPose pose);
}
