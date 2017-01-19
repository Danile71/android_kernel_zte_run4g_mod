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
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.ngin3d;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.mediatek.j3m.Texture2D;
import com.mediatek.ngin3d.presentation.ImageSource;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.presentation.VideoDisplay;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * A Plane that can show video steaming data.
 */
public final class Video extends Plane {

    static final Property<ImageSource> PROP_VIDEO_SRC = new Property<ImageSource>("video_source", null);
    static final Property<Boolean> PROP_PLAYING = new Property<Boolean>("playing", false, PROP_VIDEO_SRC);
    static final Property<Video> PROP_CLONE_SRC = new Property<Video>("video_clone_source", null);

    private final float[] mTransformMatrix = new float[16];
    // The y-axis flip matrix
    private final float[] mYFlipMatrix =
    {
        1, 0, 0, 0,
        0, -1, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 1
    };

    private boolean mApplyTransform;

    private Video(boolean isYUp) {
        super(isYUp);
    }

    private ExecutorService mVideoExecutorService;

    /**
     * Apply the video information data
     *
     * @param property property type to be applied
     * @param value    property value to be applied
     * @return if the property is successfully applied
     * @hide
     */
    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (property.sameInstance(PROP_CLONE_SRC)) {
            Video src = (Video) value;
            if (src != null) {
                setVideoSourceFromOtherVideo(getPresentation(), src);
            }
            return true;
        } else if (property.sameInstance(PROP_VIDEO_SRC)) {
            ImageSource src = (ImageSource) value;
            if (src == null) {
                if (getValue(PROP_CLONE_SRC) == null) {
                    return false;
                } else {
                    return true;
                }
            }
            getPresentation().setImageSource(src);
            initializeVideoPlayer();
            return true;
        } else if (property.sameInstance(PROP_PLAYING)) {
            VideoPlayer vp = getVideoPlayer();

            // Fix klockwork ALPS01419006 & ALPS01401307
            if (vp == null) {
                touchProperty(PROP_PLAYING);
                return true;
            }
            updateStreamingTexture();
            if ((Boolean) value) {
                // Keep PROP_PLAY dirty since the video is playing
                touchProperty(PROP_PLAYING);
                vp.start();
            } else {
                vp.pause();
            }
            /**
             * The texture coordinate transform matrix is updated by
             * the most recent call to updateTexImage, so we have to
             * pass it to native when playing and play status change.
             */
            if (mApplyTransform && vp.getTransformMatrix(mTransformMatrix)) {
                getPresentation().setTextureTransform(mTransformMatrix);
            } else {
                getPresentation().setTextureTransform(mYFlipMatrix);
            }

            return true;
        }
        return false;
    }

    /**
     * Set texture from others
     *
     * @param presentation the presentation of the actor
     * @hide
     */
    private void setVideoSourceFromOtherVideo(VideoDisplay presentation, Video SrcVideo) {
        if (SrcVideo.getPresentation() != null) {
            ImageSource is = (ImageSource) SrcVideo.getValue(PROP_VIDEO_SRC);
            Texture2D txt = SrcVideo.getPresentation().getTexture2D();
            if (is != null && txt != null) {
                Log.e(TAG, "setVideoSourceFromOtherVideo()" + txt);
                setValue(PROP_VIDEO_SRC, is, false);
                setValue(PROP_VISIBLE, true);
                presentation.setTexture2D(txt);
                setValue(PROP_CLONE_SRC, null, false);
            }
        }
    }

    /**
     * @hide
     */
    @Override
    protected VideoDisplay createPresentation(PresentationEngine engine) {
        VideoDisplay videoDisplay = engine.createVideoDisplay(mIsYUp);
        if (mRenderLayerForAttachment != null) {
            mRenderLayerForAttachment.setRenderTarget(videoDisplay);
            mRenderLayerForAttachment = null;
        }
        mVideoExecutorService = Executors.newSingleThreadExecutor();
        return videoDisplay;
    }

    /**
     * Create an Video object with video streaming content from URI.
     * <p>
     * Video streaming is paused by default after prepared and shows first frame.
     * Client must call play() specifically to start video playback.
     *
     * @param ctx    android context
     * @param uri    video uri
     * @param width  video object width
     * @param height video object height
     * @return an Video object that is created from video uri
     */
    public static Video createFromVideo(Context ctx, Uri uri, int width, int height) {
        return createFromVideo(ctx, uri, width, height, false);
    }

    /**
     * Create an Video object with video streaming content from uri.
     * <p>
     * Video streaming is paused by default after prepared and shows first frame.
     * Client must call play() specifically to start video playback.
     *
     * @param ctx    android context
     * @param uri    video uri
     * @param width  video object width
     * @param height video object height
     * @param isYUp  true for creating a Y-up quad, default is Y-down
     * @return an Video object that is created from video uri
     */
    public static Video createFromVideo(Context ctx, Uri uri, int width, int height, boolean isYUp) {
        Video video = new Video(isYUp);
        video.setVideoFromUri(ctx, uri, width, height);
        return video;
    }

    /**
     * Swap the Video Actor Source
     *
     * @param a Video Actor a
     * @param b Video Actor b
     */
    public static void swapVideoSource(Video a, Video b) {
        Texture2D ta = a.getPresentation().getTexture2D();
        Texture2D tb = b.getPresentation().getTexture2D();
        a.getPresentation().setTexture2D(tb);
        b.getPresentation().setTexture2D(ta);

        ImageSource isa = a.getValue(PROP_VIDEO_SRC);
        ImageSource isb = b.getValue(PROP_VIDEO_SRC);
        a.setValue(PROP_VIDEO_SRC, isb, false);
        b.setValue(PROP_VIDEO_SRC, isa, false);
    }

    /**
     * set the video source from other Video Actor
     *
     * @param src source Video Actor
     * @param dst Video Actor to be set
     */
    public static void setVideoSourceFromActor(Video src, Video dst) {
        Texture2D ta = src.getPresentation().getTexture2D();
        ImageSource isa = src.getValue(PROP_VIDEO_SRC);
        dst.getPresentation().setTexture2D(ta);
        dst.setValue(PROP_VIDEO_SRC, isa, false);
    }

    /**
     * Create an Video object with video streaming content from other Video Actor.
     *
     * @param ctx    android context
     * @param uri    source video
     * @param width  video object width
     * @param height video object height
     * @param isYUp  true for creating a Y-up quad, default is Y-down
     * @return an Video object that is created from video uri
     */
    public static Video createFromVideo(Context ctx, Video src, int width, int height, boolean isYUp) {
        Video video = new Video(isYUp);
        Dimension dim = new Dimension(width, height);
        video.setValue(PROP_SIZE, dim);
        video.setValue(PROP_VISIBLE, false);
        video.setValue(PROP_CLONE_SRC, src);
        return video;
    }

    protected void setVideoFromUri(Context ctx, Uri uri, int width, int height) {
        Dimension dim = new Dimension(width, height);
        setValue(PROP_SIZE, dim);
        setValue(PROP_VIDEO_SRC, new ImageSource(ImageSource.VIDEO_TEXTURE, new VideoPlayer(ctx, uri)));
        setValue(PROP_VISIBLE, false); // default is invisible until the content is ready
    }

    protected void updateStreamingTexture() {
        ImageSource src = getValue(PROP_VIDEO_SRC);
        boolean isFirstUpdate = ((VideoPlayer) src.srcInfo).applyUpdate();
        if (isFirstUpdate && !getVisible()) {
            setVisible(true);
        }
    }

    private class VideoPlayerUninitializer implements Runnable {
        private final VideoPlayer mVideoPlayer;

        VideoPlayerUninitializer(VideoPlayer src) {
            mVideoPlayer = src;
        }

        public void run() {
            if (mVideoPlayer != null) {
                mVideoPlayer.destroy();
            }
        }
    }

    private class VideoPlayerInitializer implements Runnable {
        private final VideoPlayer mVideoPlayer;

        VideoPlayerInitializer(VideoPlayer src) {
            mVideoPlayer = src;
        }

        public void run() {

            // After texture object name has been got from the engine,
            // generate surface texture, setup video player, and start to play.
            // Because SurfaceTexture can be created under any thread.
            // To accelerate display in GL thread, move initialization at UI thread.

            // This runnable is posted to ui thread, the ui thread might execute the
            // runnable while activity is in background and in this case the
            // presentation is null.
            if (mVideoPlayer != null && getPresentation() != null) {
                int textureName = getPresentation().getTexName();
                if (textureName > 0) {
                    mVideoPlayer.initialize(textureName);
                } else {
                    Log.w(TAG, "Video Texture texture name is invalid : " + textureName);
                }
            }
        }
    }

    /**
     * Get the video player image source info, or null if that is not available.
     */
    public VideoPlayer getVideoPlayer() {
        ImageSource src = getValue(PROP_VIDEO_SRC);
        if (src != null) {
            return (VideoPlayer) src.srcInfo;
        }
        return null;
    }

    /**
     * Un-realize this Video.
     *
     * @hide
     */
    public void unrealize() {
        super.unrealize();
        if (mVideoExecutorService != null) {
            mVideoExecutorService.submit(new VideoPlayerUninitializer(getVideoPlayer()));
            mVideoExecutorService.shutdown();
            mVideoExecutorService = null;
        }

    }

    /**
     * Initialisation.
     */
    public void initializeVideoPlayer() {
        VideoPlayer vp = getVideoPlayer();
        if (vp != null && !vp.isInitialized()) {
            mVideoExecutorService.submit(new VideoPlayerInitializer(vp));
        }
    }

    /**
     * Start video texture playback
     */
    public void play() {
        setValue(PROP_PLAYING, true);
    }

    /**
     * Pause video texture playback
     */
    public void pause() {
        setValue(PROP_PLAYING, false);
    }

    /**
     * Configure to play video once or to loop continuously.
     *
     * @param looping true for replay video automatically
     * @return an Video object
     */
    public Video setLooping(Boolean looping) {
        VideoPlayer vp = getVideoPlayer();
        if (vp != null) {
            vp.setLooping(looping);
        }
        return this;
    }

    /**
     * Sets the volume on this player.
     *
     * @param leftVolume  Sets the left volume on this player.
     * @param rightVolume Sets the right volume on this player.
     * @return an Video object
     */
    public Video setVolume(float leftVolume, float rightVolume) {
        VideoPlayer vp = getVideoPlayer();
        if (vp != null) {
            vp.setVolume(leftVolume, rightVolume);
        }
        return this;
    }

    /**
     * Query the 'dirty' state of the player.
     *
     * @return True if re-rendering is needed.
     */
    @Override
    public boolean isDirty() {
        VideoPlayer vp = getVideoPlayer();
        return vp != null;
    }

    /**
     * Query the playing state.
     *
     * @return True if the the video is playing
     */
    public boolean isPlaying() {
        VideoPlayer vp = getVideoPlayer();
        return vp != null && vp.isPlaying();
    }

    /**
     * Returns the Actor's presentation cast to the instantiated type.
     *
     * @hide Presentation API should be internal only
     *
     * @return Presentation object
     */
    @Override
    public VideoDisplay getPresentation() {
        return (VideoDisplay) mPresentation;
    }

    /**
     * Control use of source transform.
     *
     * There is a 4x4 texture coordinate transform matrix associated with the texture image set by
     * the most recent call to updateTexImage. You can decide to apply the transform matrix or not.
     *
     * @param apply True to apply the texture transform matrix
     */
    public void applyTransformMatrix(boolean apply) {
        mApplyTransform = apply;
    }
}
