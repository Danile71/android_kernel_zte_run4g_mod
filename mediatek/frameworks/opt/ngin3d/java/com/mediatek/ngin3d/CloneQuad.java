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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
import android.util.Log;

import com.mediatek.j3m.Texture2D;
import com.mediatek.ngin3d.presentation.ImageDisplay;

/**
 * A Plane that can show a quad cloned from other Plane.
 */
public final class CloneQuad extends Plane {
    static final Property<Plane> PROP_PLANE_SRC = new Property<Plane>("plane_source", null);

    public CloneQuad(boolean isYUp) {
        super(isYUp);
    }

    /**
     * Create an CloneQuad object with texture content from Plane.
     * <p>
     *
     * @param ctx    android context
     * @param src    Plane to be Cloned
     * @param width  object width
     * @param height object height
     * @return an CloneQuad object that cloned from other plane
     */
    public static CloneQuad createFromOtherPlane(Context ctx, Plane src, int width, int height) {
        return createFromOtherPlane(ctx, src, width, height, false);
    }

    /**
     * Create an CloneQuad object with texture content from Plane.
     * <p>
     *
     * @param ctx    android context
     * @param src    Plane to be Cloned
     * @param width  object width
     * @param height object height
     * @param isYUp  true for creating a Y-up quad, default is Y-down
     * @return an CloneQuad object that cloned from other plane
     */
    public static CloneQuad createFromOtherPlane(Context ctx, Plane src, int width, int height, boolean isYUp) {
        CloneQuad quad = new CloneQuad(isYUp);
        Dimension dim = new Dimension(width, height);
        if (src instanceof Video) {
            quad.setMaterial("ngin3d#vidquad.mat");
            quad.setValue(PROP_SIZE, dim);
        } else {
            quad.setMaterial("ngin3d#quad.mat");
        }
        quad.setValue(PROP_VISIBLE, true);
        quad.setValue(PROP_PLANE_SRC, src);
        return quad;
    }

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

        if (property.sameInstance(PROP_PLANE_SRC)) {
            Plane src = (Plane) value;
            if (src != null) {
                setPlaneSourceFromPlane(getPresentation(), src);
                return true;
            }
        }

        return false;
    }

    private boolean hasSetMaterial = false;

    /**
     * Sets the material for a node within this Actor.
     * Actors may be constructed from a hierachy of components, especially if the
     * Actor is a 3D object created by 3ds Max or Blender. This method allows
     * a material to be assigned to a specific node in that hierachy. You will need
     * to know the name of the node; ask the artist.
     *
     * @param nodeName Name of the node for which to set the material
     * @param name     Name of the material to set
     */
    @Override
    public void setMaterial(String nodeName, String name) {
        hasSetMaterial = true;
        super.setMaterial(nodeName, name);
    }

    /**
     * Set the texture of display source
     *
     * @param imageDisplay The Display to be applied
     * @param srcPlane the source plane to be cloned
     * @hide
     */
    private void setPlaneSourceFromPlane(ImageDisplay imageDisplay, Plane srcPlane) {
        if (srcPlane.getPresentation() != null) {
            // checking the shader program, Has it been set before ?
            if (false == hasSetMaterial) {
                // the CloneQuad may change the cloned target, so set it always
                if (srcPlane instanceof Video) {
                    this.setMaterial("ngin3d#vidquad.mat");
                } else {
                    this.setMaterial("ngin3d#quad.mat");
                }
            }
            Texture2D txt = srcPlane.getPresentation().getTexture2D();
            if (txt != null) {
                imageDisplay.setTexture2D(txt);
            } else {
                Log.w(TAG, "setPlaneSourceFromPlane() " + srcPlane + " has null Texture2D");
            }
        }
    }
}
