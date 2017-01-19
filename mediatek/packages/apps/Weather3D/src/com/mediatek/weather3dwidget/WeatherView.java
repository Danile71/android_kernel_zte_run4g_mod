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

package com.mediatek.weather3dwidget;

import android.animation.ObjectAnimator;
import android.appwidget.AppWidgetManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.text.Layout;
import android.text.format.DateUtils;
import android.util.AttributeSet;
import android.util.SparseArray;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.widget.RemoteViews.RemoteView;

import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Object3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.Ngin3dLayoutInflater;
import com.mediatek.ngin3d.android.StageTextureView;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.PropertyAnimation;
import com.mediatek.weather.WeatherUpdateResult;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.LinkedList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

@RemoteView
public class WeatherView extends StageTextureView {
    private static final String TAG = "W3D/WeatherView";

    private static final Color COLOR_BLACK = new Color(0x00, 0x00, 0x00);
    private static final Color COLOR_WHITE = new Color(0xff, 0xff, 0xff);

    private static final Color COLOR_TEMP = COLOR_BLACK;
    private static final Color COLOR_HIGH_LOW_TEMP = COLOR_BLACK;
    private static final Color COLOR_DATE = COLOR_BLACK;
    private static final Color COLOR_LAST_UPDATE = COLOR_WHITE;

    private static final Typeface FONT_SANS_BOLD = Typeface.create(Typeface.SANS_SERIF, Typeface.BOLD);

    private static final int FORECAST_PANEL_NUM = 3;
    private static int[] sForecastPanelX = new int[FORECAST_PANEL_NUM];
    private static int[] sForecastPanelY = new int[FORECAST_PANEL_NUM];

    private static Point[] sForecastUpperPoint = new Point[FORECAST_PANEL_NUM];
    private static Point[] sForecastCenterPoint = new Point[FORECAST_PANEL_NUM];
    private static Point[] sForecastLowerPoint = new Point[FORECAST_PANEL_NUM];

    private PropertyAnimation[] mForecastPreviousDownAnimation = new PropertyAnimation[FORECAST_PANEL_NUM];
    private PropertyAnimation[] mForecastCurrentDownAnimation = new PropertyAnimation[FORECAST_PANEL_NUM];
    private PropertyAnimation[] mForecastPreviousUpAnimation = new PropertyAnimation[FORECAST_PANEL_NUM];
    private PropertyAnimation[] mForecastCurrentUpAnimation = new PropertyAnimation[FORECAST_PANEL_NUM];

    private Container[] mPreviousContainer = new Container[3];
    private Container[] mCurrentContainer = new Container[3];

    private int mCurrentWeather;
    private int mCurrentDayNight = DayNight.DAY;
    private int mTotalCity;

    private final Stage mStage;
    private Container mRootContainer;
    private Container mCityContainer;
    private Container mWeatherContainer;
    private Container mForecastContainer;
    private Container mForecastPanelContainer;
    private Container mUpdateContainer;
    private Container mScenarioContainer;
    private Text mNoWeatherText;
    private Image mCityBarArrowsImage;

    // glo model related
    private Glo3D mLandscapeGlo;
    private Glo3D mSunMoonGlo;
    private Glo3D mDayToNightGlo;
    private Glo3D mNightToDayGlo;
    private Glo3D mStarTwinkleGlo;
    private Glo3D mStarTwinkleSnowGlo;
    private Glo3D mExtraLightGlo;
    private BasicAnimation mSunMoonAni;
    private BasicAnimation mSunShowHideAni;
    private BasicAnimation mMoonShowHideAni;
    private BasicAnimation mDayToNightAni;
    private BasicAnimation mNightToDayAni;
    private BasicAnimation mStarTwinkleAni;
    private BasicAnimation mStarTwinkleSnowAni;

    private PropertyAnimation mChangeNextCityAnimation;
    private PropertyAnimation mChangePreviousCityAnimation;
    private static final String CHANGE_CITY_ANI_MARKER = "change";

    private PropertyAnimation mChangeSnowTextureAnimation;

    private static final int TAG_ANI_CHANGE_CITY_NEXT = 1;
    private static final int TAG_ANI_CHANGE_CITY_PREVIOUS = 2;
    private static final int TAG_ANI_CHANGE_SNOW_TEXTURE = 3;
    private static final int TAG_ANI_DAY_TO_NIGHT = 4;
    private static final int TAG_ANI_NIGHT_TO_DAY = 5;
    private static final int TAG_ANI_STAR_TWINKLE = 6;
    private static final int TAG_ANI_STAR_SNOW_TWINKLE = 7;

    private final SparseArray<Glo3D> mGloList = new SparseArray<Glo3D>();
    private final SparseArray<BasicAnimation> mGloAniList = new SparseArray<BasicAnimation>();
    private final LinkedList<Integer> mAniWaitingPlayQueue = new LinkedList<Integer>();
    private final List<Actor> mPremultiplyActorList = new ArrayList<Actor>();

    private final SparseArray<Glo3D> mGloObjLists = new SparseArray<Glo3D>();
    private final SparseArray<BasicAnimation> mAnimationLists = new SparseArray<BasicAnimation>();
    private final SparseArray<int[]> mWeatherTypeGloMap = new SparseArray<int[]>();

    private final WeatherInfo mWeatherInfo = new WeatherInfo();
    private Bundle mWeatherBundle;

    private final Context mContext;

    private GestureDetector mGestureDetector;

    private boolean mIsSnowing;

    private ObjectAnimator mAlphaAnimation;
    private boolean mFirstDraw;

    public enum STATE {
        INIT, NO_CITY, NO_NETWORK, NORMAL, UPDATING,
        SCROLLING, SCROLLED_WAIT_DATA, GOT_DATA_WAIT_SCROLL_END, SCROLLED_UPDATE_DATA
    }

    private STATE mState = STATE.INIT;

    public WeatherView(Context context) {
        this(context, null);
    }

    public WeatherView(Context context, AttributeSet attrs) {
        super(context, attrs, true);
        mContext = context;
        mStage = getStage();
        initialize();
        mGestureDetector = new GestureDetector(context, new WeatherGestureListener());
        mAlphaAnimation = ObjectAnimator.ofFloat(this, View.ALPHA, 0.0f, 1.0f).setDuration(200);
    }

    private void initialize() {
        LogUtil.v(TAG, "initialize(), width x height = (" + getWidth() + " x " + getHeight() + ")");

        // set opaque as false to let blending with the view behind
        this.setOpaque(false);

        mStage.setBackgroundColor(new Color(0x00, 0x00, 0x00, 0x00));
        mStage.setProjection(Stage.UI_PERSPECTIVE, 500.0f, 1500.0f, -1111.0f);
        mRootContainer = new Container();
        mStage.add(mRootContainer);

        Image cityBarImage = (Image) Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.image_city_bar, mRootContainer);
        cityBarImage.setMaterial("ngin3d#quad_premultiply.mat");
        mPremultiplyActorList.add(cityBarImage);

        mCityBarArrowsImage = (Image) Ngin3dLayoutInflater.inflateLayout(
                mContext, R.xml.image_city_bar_arrows, mRootContainer);
        mCityBarArrowsImage.setMaterial("ngin3d#quad_premultiply.mat");
        mPremultiplyActorList.add(mCityBarArrowsImage);

        Image temperatureCloudImage = (Image) Ngin3dLayoutInflater.inflateLayout(
                mContext, R.xml.image_temperature_cloud, mRootContainer);
        temperatureCloudImage.setMaterial("ngin3d#quad_premultiply.mat");
        mPremultiplyActorList.add(temperatureCloudImage);

        initForecast();

        // No Weather Text
        mNoWeatherText = (Text)Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.text_no_weather, mRootContainer);
        int textSize = getResources().getInteger(R.integer.size_text_no_weather);
        mNoWeatherText.setTextSize(textSize);
        mNoWeatherText.setTextColor(COLOR_LAST_UPDATE);
        mNoWeatherText.setShadowLayer(1, 2, 2, 0xff000000);
        mNoWeatherText.setTypeface(FONT_SANS_BOLD);
        int maxWidth = getResources().getInteger(R.integer.no_weather_max_width);
        mNoWeatherText.setMaxWidth(maxWidth);
        mNoWeatherText.setAlignment(Layout.Alignment.ALIGN_CENTER);

        mScenarioContainer = (Container) Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.container_3d_model, mStage);
        int scenarioXRotation = 15;
        int scenarioYRotation = 15;
        int scenarioZRotation = 5;
        mScenarioContainer.setRotation(new Rotation(scenarioXRotation, scenarioYRotation, scenarioZRotation));
        int scale = getResources().getInteger(R.integer.model_scale);
        mScenarioContainer.setScale(new Scale(1.0f * scale, -1.0f * scale, -1f * scale));

        mLandscapeGlo = Glo3D.createFromAsset("landscape.glo");
        // Replace materials to use custom shader
        mLandscapeGlo.setMaterial("ngin3d#weather.mat");
        // Make sure shadow uses alpha blending.
        mLandscapeGlo.setMaterial("sheep_shadow", "ngin3d#weather_blend.mat");

        mScenarioContainer.add(mLandscapeGlo);

        // Object 3D shadow
        Image shadowImage = (Image) Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.image_shadow, mRootContainer);
        shadowImage.setMaterial("ngin3d#quad_premultiply.mat");
        mPremultiplyActorList.add(shadowImage);
        shadowImage.setRotation(new Rotation(0, 0, -3));

        initObject3D();

        mChangeNextCityAnimation = new PropertyAnimation(mScenarioContainer, "rotation",
                new Rotation(scenarioXRotation, scenarioYRotation, scenarioZRotation),
                new Rotation(scenarioXRotation + 360, scenarioYRotation, scenarioZRotation));
        mChangeNextCityAnimation.setDuration(960).setMode(Mode.LINEAR).setTag(TAG_ANI_CHANGE_CITY_NEXT);
        mChangeNextCityAnimation.addMarkerAtTime(CHANGE_CITY_ANI_MARKER, 720);
        mChangeNextCityAnimation.addListener(mAnimationCompletedHandler);

        mChangePreviousCityAnimation = new PropertyAnimation(mScenarioContainer, "rotation",
                new Rotation(scenarioXRotation, scenarioYRotation, scenarioZRotation),
                new Rotation(scenarioXRotation - 360, scenarioYRotation, scenarioZRotation));
        mChangePreviousCityAnimation.setDuration(960).setMode(Mode.LINEAR).setTag(TAG_ANI_CHANGE_CITY_PREVIOUS);
        mChangePreviousCityAnimation.addMarkerAtTime(CHANGE_CITY_ANI_MARKER, 240);
        mChangePreviousCityAnimation.addListener(mAnimationCompletedHandler);

        initSnowScene();
        mTotalCity = 0;
    }

    private void initObject3D() {
        // create weather glo mapping with weather type
        buildWeatherMapping(GloList.SUNNY, WeatherType.ModelType.SUNNY);
        buildWeatherMapping(GloList.WINDY, WeatherType.ModelType.WINDY);
        buildWeatherMapping(GloList.BLUSTERY, WeatherType.ModelType.BLUSTERY);
        buildWeatherMapping(GloList.CLOUDY, WeatherType.ModelType.CLOUDY);
        buildWeatherMapping(GloList.RAINY, WeatherType.ModelType.RAIN);
        buildWeatherMapping(GloList.HEAVY_RAIN, WeatherType.ModelType.HEAVY_RAIN);
        buildWeatherMapping(GloList.THUNDER, WeatherType.ModelType.THUNDER);
        buildWeatherMapping(GloList.SNOW, WeatherType.ModelType.SNOW);
        buildWeatherMapping(GloList.HEAVY_SNOW, WeatherType.ModelType.HEAVY_SNOW);
        buildWeatherMapping(GloList.SNOW_RAIN, WeatherType.ModelType.SNOW_RAIN);
        buildWeatherMapping(GloList.FOG, WeatherType.ModelType.FOG);
        buildWeatherMapping(GloList.SAND,WeatherType.ModelType.SAND);

        mSunMoonGlo = Glo3D.createFromAsset("sunmoon.glo");
        mScenarioContainer.add(mSunMoonGlo);
        mSunMoonGlo.setVisible(false);
        mSunMoonAni = mSunMoonGlo.getAnimation();

        Glo3D sunShowHideGlo = Glo3D.createFromAsset("sun_show_hide.glo");
        mScenarioContainer.add(sunShowHideGlo);
        sunShowHideGlo.setVisible(false);
        mSunShowHideAni = sunShowHideGlo.getAnimation();

        Glo3D moonShowHideGlo = Glo3D.createFromAsset("moon_show_hide.glo");
        mScenarioContainer.add(moonShowHideGlo);
        moonShowHideGlo.setVisible(false);
        mMoonShowHideAni = moonShowHideGlo.getAnimation();

        mDayToNightGlo = Glo3D.createFromAsset("sunmoon_day_to_night.glo");
        mScenarioContainer.add(mDayToNightGlo);
        mDayToNightGlo.setVisible(false);
        mDayToNightAni = mDayToNightGlo.getAnimation();
        mDayToNightAni.setTag(TAG_ANI_DAY_TO_NIGHT);
        mDayToNightAni.addListener(mAnimationCompletedHandler);
        mGloList.put(TAG_ANI_DAY_TO_NIGHT, mDayToNightGlo);
        mGloAniList.put(TAG_ANI_DAY_TO_NIGHT, mDayToNightAni);

        mNightToDayGlo = Glo3D.createFromAsset("sunmoon_night_to_day.glo");
        mScenarioContainer.add(mNightToDayGlo);
        mNightToDayGlo.setVisible(false);
        mNightToDayAni = mNightToDayGlo.getAnimation();
        mNightToDayAni.setTag(TAG_ANI_NIGHT_TO_DAY);
        mNightToDayAni.addListener(mAnimationCompletedHandler);
        mGloList.put(TAG_ANI_NIGHT_TO_DAY, mNightToDayGlo);
        mGloAniList.put(TAG_ANI_NIGHT_TO_DAY, mNightToDayAni);

        mStarTwinkleGlo = Glo3D.createFromAsset("stars_twinkle.glo");
        mStarTwinkleGlo.setMaterial("ngin3d#weather.mat");
        mStarTwinkleGlo.setVisible(false);
        mScenarioContainer.add(mStarTwinkleGlo);
        mStarTwinkleAni = mStarTwinkleGlo.getAnimation();
        mStarTwinkleAni.setTag(TAG_ANI_STAR_TWINKLE);
        mStarTwinkleAni.addListener(mAnimationCompletedHandler);
        mGloList.put(TAG_ANI_STAR_TWINKLE, mStarTwinkleGlo);
        mGloAniList.put(TAG_ANI_STAR_TWINKLE, mStarTwinkleAni);

        mStarTwinkleSnowGlo = Glo3D.createFromAsset("stars_twinkle_snow.glo");
        mStarTwinkleSnowGlo.setMaterial("ngin3d#weather.mat");
        mStarTwinkleSnowGlo.setVisible(false);
        mScenarioContainer.add(mStarTwinkleSnowGlo);
        mStarTwinkleSnowAni = mStarTwinkleSnowGlo.getAnimation();
        mStarTwinkleSnowAni.setTag(TAG_ANI_STAR_SNOW_TWINKLE);
        mStarTwinkleSnowAni.addListener(mAnimationCompletedHandler);
        mGloList.put(TAG_ANI_STAR_SNOW_TWINKLE, mStarTwinkleSnowGlo);
        mGloAniList.put(TAG_ANI_STAR_SNOW_TWINKLE, mStarTwinkleSnowAni);

        mExtraLightGlo = Glo3D.createFromAsset("rain_light.glo");
        mExtraLightGlo.setVisible(false);
        mScenarioContainer.add(mExtraLightGlo);
    }

    private void buildWeatherMapping(int[] weather, int index) {
        LogUtil.v(TAG, "buildWeatherMapping, " + index);
        mWeatherTypeGloMap.put(index, weather);
    }

    private void initForecast() {
        Resources res = getResources();
        sForecastPanelX[0] = res.getInteger(R.integer.x_left_panel);
        sForecastPanelX[1] = res.getInteger(R.integer.x_center_panel);
        sForecastPanelX[2] = res.getInteger(R.integer.x_right_panel);
        sForecastPanelY[0] = res.getInteger(R.integer.y_left_panel);
        sForecastPanelY[1] = res.getInteger(R.integer.y_center_panel);
        sForecastPanelY[2] = res.getInteger(R.integer.y_right_panel);
        final int scaleChild = res.getInteger(R.integer.panel_scale_child);
        final int scaleMother = res.getInteger(R.integer.panel_scale_mother);
        final float scale = (float) scaleChild / (float) scaleMother;
        final Scale panelScale = new Scale(scale, scale);
        final int xOffset = res.getInteger(R.integer.x_offest_panel);
        final int yOffset = res.getInteger(R.integer.y_offest_panel);
        final int yRange = res.getInteger(R.integer.y_range_panel);

        int duration[] = {880, 1380, 1880};
        // different z value to prevent flick on panel
        float z[] = {0.01f, 0.02f, 0.03f};

        if (mForecastPanelContainer == null) {
            mForecastPanelContainer = new Container();
            mRootContainer.add(mForecastPanelContainer);
        }

        for (int i = 0; i < FORECAST_PANEL_NUM; i++) {
            // forecast panel image
            Point panelPoint = new Point(sForecastPanelX[i], sForecastPanelY[i], z[i]);
            Image panelImage = Image.createFromResource(getResources(), R.drawable.pnl_forecast);
            mForecastPanelContainer.add(panelImage);
            panelImage.setMaterial("ngin3d#quad_premultiply.mat");
            mPremultiplyActorList.add(panelImage);
            panelImage.setPosition(panelPoint);
            panelImage.setScale(panelScale);

            sForecastUpperPoint[i] = new Point(sForecastPanelX[i] + xOffset, sForecastPanelY[i] + yOffset - yRange);
            sForecastCenterPoint[i] = new Point(sForecastPanelX[i] + xOffset, sForecastPanelY[i] + yOffset);
            sForecastLowerPoint[i] = new Point(sForecastPanelX[i] + xOffset, sForecastPanelY[i] + yOffset + yRange);

            // initForecastAnimation
            mForecastCurrentDownAnimation[i] = new PropertyAnimation(
                    "position", sForecastUpperPoint[i], sForecastCenterPoint[i]);
            mForecastCurrentDownAnimation[i].setDuration(duration[i]).setMode(Mode.EASE_OUT_BOUNCE);

            mForecastPreviousDownAnimation[i] = new PropertyAnimation(
                    "position", sForecastCenterPoint[i], sForecastLowerPoint[i]);
            mForecastPreviousDownAnimation[i].setDuration(duration[i]).setMode(Mode.EASE_OUT_BOUNCE);

            mForecastPreviousUpAnimation[i] = new PropertyAnimation(
                    "position", sForecastCenterPoint[i], sForecastUpperPoint[i]);
            mForecastPreviousUpAnimation[i].setDuration(duration[i]).setMode(Mode.EASE_OUT_BOUNCE);

            mForecastCurrentUpAnimation[i] = new PropertyAnimation(
                    "position", sForecastLowerPoint[i], sForecastCenterPoint[i]);
            mForecastCurrentUpAnimation[i].setDuration(duration[i]).setMode(Mode.EASE_OUT_BOUNCE);
        }
    }

    private void startForecastDownAnimation() {
        // AnimationGroup doesn't support Property Animations with different duration, so start separately
        for (int i = 0; i < FORECAST_PANEL_NUM; i++) {
            mForecastCurrentDownAnimation[i].start();
            mForecastPreviousDownAnimation[i].start();
        }
    }

    private void startForecastUpAnimation() {
        for (int i = 0; i < FORECAST_PANEL_NUM; i++) {
            mForecastCurrentUpAnimation[i].start();
            mForecastPreviousUpAnimation[i].start();
        }
    }

    private void initSnowScene() {
        mChangeSnowTextureAnimation = new PropertyAnimation(new Empty(), "position",
                new Point(0, 0, 0), new Point(10, 0, 0));
        mChangeSnowTextureAnimation.setDuration(SnowTextureList.SNOW_ANI_TOTAL_DURATION).setMode(Mode.LINEAR).
                setTag(TAG_ANI_CHANGE_SNOW_TEXTURE);
        for (int i = SnowTextureList.SNOW_PILE_005; i < SnowTextureList.TEXTURE_TIME.length; i++) {
            mChangeSnowTextureAnimation.addMarkerAtTime(String.valueOf(i), SnowTextureList.TEXTURE_TIME[i]);
        }
        mChangeSnowTextureAnimation.addListener(mAnimationCompletedHandler);
    }

    private void updateCityInfo(String cityName, int index, int total) {
        LogUtil.v(TAG, "updateCityInfo");

        int cityNameTag = 1;
        int indexTag = 2;

        if (mCityContainer == null) {
            mCityContainer = new Container();
            mRootContainer.add(mCityContainer);

            int textSize;
            Text cityNameText = (Text)Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.text_city, mCityContainer);
            cityNameText.setTypeface(FONT_SANS_BOLD);
            textSize = getResources().getInteger(R.integer.size_text_city);
            cityNameText.setTextSize(textSize);
            cityNameText.setTag(cityNameTag);
            cityNameText.setText(cityName);
            int maxWidth = getResources().getInteger(R.integer.city_max_width);
            cityNameText.setMaxWidth(maxWidth);
            cityNameText.setMaxLines(1);

            Text indexText = (Text)Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.text_city_index, mCityContainer);
            indexText.setTypeface(FONT_SANS_BOLD);
            textSize = getResources().getInteger(R.integer.size_text_city_index);
            indexText.setTextSize(textSize);
            indexText.setTag(indexTag);
            indexText.setText(getCityIndexString(index, total));
        } else {
            ((Text)(mCityContainer.findChildByTag(cityNameTag))).setText(cityName);
            ((Text)(mCityContainer.findChildByTag(indexTag))).setText(getCityIndexString(index, total));
        }
    }

    private void updateTemp(double temp, double tempHigh, double tempLow, int tempType) {
        LogUtil.v(TAG,"updateTemp");

        int tempTag = 1;
        int highLowTempTag = 2;

        if (mWeatherContainer == null) {
            mWeatherContainer = new Container();
            mRootContainer.add(mWeatherContainer);

            int textSize;
            Text tempText = (Text)Ngin3dLayoutInflater.inflateLayout(mContext, R.xml.text_temperature, mWeatherContainer);
            tempText.setTypeface(FONT_SANS_BOLD);
            tempText.setTextColor(COLOR_TEMP);
            textSize = getResources().getInteger(R.integer.size_text_temp);
            tempText.setTextSize(textSize);
            tempText.setTag(tempTag);
            tempText.setText(getTempString(temp, tempType));

            Text highLowTempText = (Text)Ngin3dLayoutInflater.inflateLayout(
                    mContext, R.xml.text_high_low_temperature, mWeatherContainer);
            highLowTempText.setTypeface(FONT_SANS_BOLD);
            highLowTempText.setTextColor(COLOR_HIGH_LOW_TEMP);
            textSize = getResources().getInteger(R.integer.size_text_high_low_temp);
            highLowTempText.setTextSize(textSize);
            highLowTempText.setTag(highLowTempTag);
            highLowTempText.setText(getHighLowTempString(tempHigh, tempLow));
        } else {
            ((Text)(mWeatherContainer.findChildByTag(tempTag))).setText(getTempString(temp, tempType));
            ((Text)(mWeatherContainer.findChildByTag(highLowTempTag))).setText(getHighLowTempString(tempHigh, tempLow));
        }
    }

    private void updateForecast(ForecastData[] data, int scrollType) {
        LogUtil.v(TAG, "updateForecast");

        int totalForecast = 3;
        int containerTag[] = {1, 2, 3};

        int dayTag = 4;
        int iconTag = 5;
        int tempTag = 6;

        if (mForecastContainer == null) {
            mForecastContainer = new Container();
            mRootContainer.add(mForecastContainer);

            final int xOffsetDayText = getResources().getInteger(R.integer.x_offset_day_in_panel);
            final int yOffsetDayText = getResources().getInteger(R.integer.y_offset_day_in_panel);
            final int xOffsetHighLowTemp = getResources().getInteger(R.integer.x_offset_temp_in_panel);
            final int yOffsetHighLowTemp = getResources().getInteger(R.integer.y_offset_temp_in_panel);
            final int displayAreaWidth = getResources().getInteger(R.integer.display_area_width);
            final int displayAreaHeight = getResources().getInteger(R.integer.display_area_height);
            final int xOffsetDisplay = getResources().getInteger(R.integer.x_upper_offset_display);
            final int yOffsetDisplay = getResources().getInteger(R.integer.y_upper_offset_display);
            final Point dayPoint = new Point(xOffsetDayText, yOffsetDayText, -0.08f);
            final Point iconPoint = new Point(0, 0, -0.08f);
            final Point tempPoint = new Point(xOffsetHighLowTemp, yOffsetHighLowTemp, -0.08f);
            final Point dayAnchorPoint = new Point(0.0f, 0.0f);
            final Point tempAnchorPoint = new Point(1.0f, 1.0f);
            final int dayTextSize = getResources().getInteger(R.integer.size_text_date);
            final int tempTextSize = getResources().getInteger(R.integer.size_text_high_low_temp);

            for (int i = 0; i < totalForecast; i++) {
                Container subContainer = new Container();
                subContainer.setTag(containerTag[i]);

                int displayAreaX1 = sForecastPanelX[i] - xOffsetDisplay;
                int displayAreaY1 = sForecastPanelY[i] - yOffsetDisplay;
                int displayAreaX2 = displayAreaX1 + displayAreaWidth;
                int displayAreaY2 = displayAreaY1 + displayAreaHeight;

                Box displayArea = new Box(displayAreaX1, displayAreaY1, displayAreaX2, displayAreaY2);
                LogUtil.v(TAG, "box = (" + displayAreaX1 + ", " + displayAreaY1 + ", " +
                        displayAreaX2 + ", " + displayAreaY2 + ")");

                for (int j = 0; j < 2; j++) {
                    String dayOfWeek = getDayOfWeek(data[i].getDayOfWeek());
                    Text dayText = new Text(dayOfWeek);
                    dayText.setTypeface(FONT_SANS_BOLD);
                    dayText.setPosition(dayPoint);
                    dayText.setTextColor(COLOR_DATE);
                    dayText.setTextSize(dayTextSize);
                    dayText.setTag(dayTag);
                    dayText.setDisplayArea(displayArea);
                    dayText.setAnchorPoint(dayAnchorPoint);

                    Image weatherImage = Image.createFromResource(getResources(),
                            WeatherType.getWeatherIcon(data[i].getWeatherCondition()));
                    weatherImage.setMaterial("ngin3d#quad_premultiply.mat");
                    mPremultiplyActorList.add(weatherImage);
                    weatherImage.setPosition(iconPoint);
                    weatherImage.setTag(iconTag);
                    weatherImage.setDisplayArea(displayArea);
                    weatherImage.setScale(new Scale(0.95f, 0.95f));

                    Text highLowTempText = new Text(getHighLowTempString(data[i].getHighTemp(), data[i].getLowTemp()));
                    highLowTempText.setTypeface(FONT_SANS_BOLD);
                    highLowTempText.setPosition(tempPoint);
                    highLowTempText.setTextColor(COLOR_HIGH_LOW_TEMP);
                    highLowTempText.setTextSize(tempTextSize);
                    highLowTempText.setTag(tempTag);
                    highLowTempText.setDisplayArea(displayArea);
                    highLowTempText.setAnchorPoint(tempAnchorPoint);

                    Container dataContainer = new Container();
                    if (j == 1) {
                        // currentContainer
                        dataContainer.setPosition(sForecastCenterPoint[i]);
                        mCurrentContainer[i] = dataContainer;
                    } else {
                        // previousContainer
                        dataContainer.setPosition(sForecastUpperPoint[i]);
                        mPreviousContainer[i] = dataContainer;
                    }
                    dataContainer.add(dayText);
                    dataContainer.add(weatherImage);
                    dataContainer.add(highLowTempText);

                    subContainer.add(dataContainer);
                }
                mForecastContainer.add(subContainer);
            }
        } else {
            Container mTempContainer;
            for (int i = 0; i < totalForecast; i++) {
                // if not scroll up or down, then just update the currentContainer context.
                // if scroll up or down, then change the currentContainer and previousContainer,
                // update the currentContainer context, then play animation

                if (scrollType == ScrollType.SCROLL_DOWN || scrollType == ScrollType.SCROLL_UP) {
                    mTempContainer = mCurrentContainer[i];
                    mCurrentContainer[i] = mPreviousContainer[i];
                    mPreviousContainer[i] = mTempContainer;

                    if (scrollType == ScrollType.SCROLL_DOWN) {
                        mForecastCurrentDownAnimation[i].setTarget(mCurrentContainer[i]);
                        mForecastPreviousDownAnimation[i].setTarget(mPreviousContainer[i]);
                    } else if (scrollType == ScrollType.SCROLL_UP) {
                        mForecastCurrentUpAnimation[i].setTarget(mCurrentContainer[i]);
                        mForecastPreviousUpAnimation[i].setTarget(mPreviousContainer[i]);
                    }
                }

                ((Text)(mCurrentContainer[i].findChildByTag(dayTag))).setText(getDayOfWeek(data[i].getDayOfWeek()));
                ((Image)(mCurrentContainer[i].findChildByTag(iconTag))).setImageFromResource(
                        getResources(), WeatherType.getWeatherIcon(data[i].getWeatherCondition()));
                ((Text)(mCurrentContainer[i].findChildByTag(tempTag))).setText(
                        getHighLowTempString(data[i].getHighTemp(), data[i].getLowTemp()));
            }
        }
    }

    private void updateLastUpdate(long lastUpdate, boolean updating) {
        LogUtil.v(TAG, "updateLastUpdate");

        int updateTag = 1;

        String updateString;
        if (updating) {
            updateString = getResources().getString(R.string.updating);
        } else {
            updateString = getLastUpdateString(lastUpdate);
        }

        if (mUpdateContainer == null) {
            mUpdateContainer = new Container();
            mRootContainer.add(mUpdateContainer);

            Text lastUpdatedText = (Text)Ngin3dLayoutInflater.inflateLayout(
                    mContext, R.xml.text_last_update, mUpdateContainer);
            lastUpdatedText.setTag(updateTag);
            lastUpdatedText.setTextColor(COLOR_LAST_UPDATE);
            int textSize = getResources().getInteger(R.integer.size_text_last_updated);
            lastUpdatedText.setTextSize(textSize);
            lastUpdatedText.setTypeface(FONT_SANS_BOLD);
            lastUpdatedText.setShadowLayer(1, 2, 2, 0xff000000);
            lastUpdatedText.setText(updateString);
        } else {
            ((Text)(mUpdateContainer.findChildByTag(updateTag))).setText(updateString);
        }
    }

    private void updateWeatherObject(int condition, int dayNight) {
        int currentWeatherModelType = WeatherType.convertToModelType(mCurrentWeather);
        int conditionModelType = WeatherType.convertToModelType(condition);
        LogUtil.v(TAG, "updateWeatherObject - (current, new, currentDay, newDay) = (" +  mCurrentWeather + ", " +
                condition + ", " + mCurrentDayNight + ", " + dayNight + ")");

        if (mCurrentWeather != condition) {
            stopWeatherAni(currentWeatherModelType, true);
            playWeatherAni(conditionModelType);
        }

        resetWeatherObject();

        boolean isCurrentSnow = WeatherType.isSnowModelType(currentWeatherModelType);
        boolean isNewSnow = WeatherType.isSnowModelType(conditionModelType);

        if (WeatherType.isSnowModelType(conditionModelType)) {
            startSnow();
        } else if (WeatherType.isSandModelType(conditionModelType)) {
            startSandy();
        }

        // handle sun/moon show or not show start
        boolean isCurrentNeedSun = WeatherType.isSunMoonNeededModelType(currentWeatherModelType);
        boolean isNewNeedSun = WeatherType.isSunMoonNeededModelType(conditionModelType);

        // if new condition does not need sun/moon, then stop all the ani and hide glo
        if (!isNewNeedSun) {
            mSunMoonGlo.setVisible(false);
            mSunMoonAni.stop();
            mSunShowHideAni.stop();
            LogUtil.v(TAG, "sun_show_hide_stop");
            mMoonShowHideAni.stop();
            LogUtil.v(TAG, "moon_show_hide_stop");
            mDayToNightAni.stop();
            mNightToDayAni.stop();
            // sun_rise and moon_rise stop;
        }

        if (isNewNeedSun) {
            if (mCurrentWeather == 0 || !isCurrentNeedSun) {
                if (dayNight == DayNight.DAY) {
                    addAniToQueue(TAG_ANI_NIGHT_TO_DAY);
                    mSunShowHideAni.setLoop(true).start();
                    LogUtil.v(TAG, "sun_show_hide_start");
                } else {
                    addAniToQueue(TAG_ANI_DAY_TO_NIGHT);
                    mMoonShowHideAni.setLoop(true).start();
                    LogUtil.v(TAG, "moon_show_hide_start");
                }
            } else if (mCurrentDayNight != dayNight) {
                if (dayNight == DayNight.DAY) {
                    addAniToQueue(TAG_ANI_NIGHT_TO_DAY);
                } else {
                    addAniToQueue(TAG_ANI_DAY_TO_NIGHT);
                }
                if (!mSunShowHideAni.isStarted()) {
                    mSunShowHideAni.setLoop(true).start();
                    LogUtil.v(TAG, "sun_show_hide_start");
                }
                if (!mMoonShowHideAni.isStarted()) {
                    mMoonShowHideAni.setLoop(true).start();
                    LogUtil.v(TAG, "moon_show_hide_start");
                }
            }
        } else {
            removeAniFromQueue(TAG_ANI_DAY_TO_NIGHT);
            removeAniFromQueue(TAG_ANI_NIGHT_TO_DAY);
        }

        // if new condition need sun/moon, then set sun/moon glo visible and start animation
        if ((mCurrentWeather == 0  || !isCurrentNeedSun) && isNewNeedSun) {
            if (!mSunMoonGlo.getVisible()) {
                mSunMoonGlo.setVisible(true);
                mSunMoonAni.start();
            }
        }

        if (!(dayNight == DayNight.NIGHT && mCurrentDayNight == DayNight.NIGHT) ||
                !isCurrentNeedSun || (isNewSnow != isCurrentSnow)) {
            // ! is CurrentNeedSun: from a rain to sunny, rainy has no sun or moon, so when changing to sunny,
            // need show sun/moon, after moon show, the star will show. so need to hide star first.

            // !(new day night = night and current day night = night)
            // if true, then originally, the star is existed, so no need to set invisible.
            // if false, then day to night, or night to day, in both cases, need to set invisible first
            setStarTwinkleInvisible();
        }
        // if dayNight == night, then show day to night first then when day to night end, the star will show

        if (dayNight == DayNight.NIGHT) {
            if (isNewSnow) {
                if (!mStarTwinkleSnowGlo.getVisible()) {
                    addAniToQueue(TAG_ANI_STAR_SNOW_TWINKLE);
                }
            } else {
                if (!mStarTwinkleGlo.getVisible()) {
                    addAniToQueue(TAG_ANI_STAR_TWINKLE);
                }
            }
        } else if (dayNight == DayNight.DAY) {
            // if new DayNight = DAY, then remove the star twinkle animation if the animation queue have such animation
            removeAniFromQueue(TAG_ANI_STAR_SNOW_TWINKLE);
            removeAniFromQueue(TAG_ANI_STAR_TWINKLE);
        }

        addAniToQueue(0);
        playQueuedAni();
        mCurrentWeather = condition;
        mCurrentDayNight = dayNight;
    }

    private void initGlo(int[] weatherGloList, int index) {
        int gloIndex = weatherGloList[index];
        if (mGloObjLists.get(gloIndex) == null) {
            String gloFilename = GloList.ALLGLO[gloIndex];
            Glo3D glo = Glo3D.createFromAsset(gloFilename);
            glo.setMaterial("ngin3d#weather.mat");
            glo.setMaterial("fog_cloud_01", "ngin3d#fog.mat");
            glo.setVisible(false);
            mGloObjLists.put(gloIndex, glo);
            mScenarioContainer.add(glo);
            BasicAnimation ani = glo.getAnimation();
            mAnimationLists.put(gloIndex, ani);
        }
    }

    private void playWeatherAni(int modelType) {
        if (modelType == 0) {
            return;
        }
        int[] weatherGloList = mWeatherTypeGloMap.get(modelType);
        if (weatherGloList == null) {
            return;
        }
        BasicAnimation ani;
        Glo3D glo;
        for (int i = 0; i < weatherGloList.length; i++) {
            initGlo(weatherGloList, i);

            ani = mAnimationLists.get(weatherGloList[i]);
            if (ani != null) {
                ani.setLoop(true).start();
            }
            glo = mGloObjLists.get(weatherGloList[i]);
            if (glo != null) {
                glo.setVisible(true);
            }
        }
    }

    private void stopWeatherAni(int modelType, boolean hideGlo) {
        if (modelType == 0) {
            return;
        }
        int[] weatherGloList = mWeatherTypeGloMap.get(modelType);
        if (weatherGloList == null) {
            return;
        }
        Animation ani;
        Glo3D glo;
        for (int i = 0; i < weatherGloList.length; i++) {
            ani = mAnimationLists.get(weatherGloList[i]);
            if (ani != null) {
                ani.stop();
            }
            if (hideGlo) {
                glo = mGloObjLists.get(weatherGloList[i]);
                if (glo != null) {
                    glo.setVisible(false);
                }
            }
        }
    }

    private void addAniToQueue(int aniTag) {
        LogUtil.v(TAG, "total = " + mAniWaitingPlayQueue.size() + ", tag = " + aniTag + ", queue: " + mAniWaitingPlayQueue);
        if (mAniWaitingPlayQueue.contains(0)) {
            // end of animation token existed, means a set of animation waited to play
            if (aniTag == TAG_ANI_NIGHT_TO_DAY) {
                mAniWaitingPlayQueue.clear();
                mAniWaitingPlayQueue.add(aniTag);
            } else if (aniTag == TAG_ANI_DAY_TO_NIGHT) {
                mAniWaitingPlayQueue.clear();
                mAniWaitingPlayQueue.add(aniTag);
            } else if (aniTag == TAG_ANI_STAR_SNOW_TWINKLE || aniTag == TAG_ANI_STAR_TWINKLE) {
                if (mAniWaitingPlayQueue.contains(TAG_ANI_STAR_SNOW_TWINKLE)) {
                    int indexSnowStar = mAniWaitingPlayQueue.indexOf(TAG_ANI_STAR_SNOW_TWINKLE);
                    mAniWaitingPlayQueue.set(indexSnowStar, aniTag);
                } else if (mAniWaitingPlayQueue.contains(TAG_ANI_STAR_TWINKLE)) {
                    int indexStar = mAniWaitingPlayQueue.indexOf(TAG_ANI_STAR_TWINKLE);
                    mAniWaitingPlayQueue.set(indexStar, aniTag);
                } else {
                    mAniWaitingPlayQueue.clear();
                    mAniWaitingPlayQueue.add(aniTag);
                }
            } else if (aniTag == 0) {
                if (mAniWaitingPlayQueue.getLast() != aniTag) {
                    mAniWaitingPlayQueue.add(aniTag);
                }
            }
        } else {
            // add new aniTag directly
            mAniWaitingPlayQueue.add(aniTag);
        }
        LogUtil.v(TAG, "afterAddedQueue: " + mAniWaitingPlayQueue);
    }

    private void removeAniFromQueue(int aniTag) {
        LogUtil.v(TAG, "total = " + mAniWaitingPlayQueue.size() + ", tag = " + aniTag + ", queue: " + mAniWaitingPlayQueue);
        int index = mAniWaitingPlayQueue.indexOf(aniTag);
        if (index != -1) {
            mAniWaitingPlayQueue.remove(index);
        }
        LogUtil.v(TAG, "afterRemovedQueue: " + mAniWaitingPlayQueue);
    }

    private void playQueuedAni() {
        if (getVisibility() != VISIBLE) {
            // only play animation when VISIBLE, if play when != VISIBLE, then when VISIBLE, the animation may not be seen,
            // which will not be good to user
            return;
        }

        if (mAniWaitingPlayQueue.isEmpty()) {
            return;
        }

        int aniTag = mAniWaitingPlayQueue.remove();
        LogUtil.v(TAG, "playQueuedAni, tag = " + aniTag);

        if (aniTag == 0) {
            setState(STATE.NORMAL);
        } else {
            if (aniTag == TAG_ANI_STAR_TWINKLE) {
                mStarTwinkleSnowAni.stop();
                mStarTwinkleSnowGlo.setVisible(false);

                if (!mStarTwinkleGlo.getVisible()) {
                    mStarTwinkleGlo.setVisible(true);
                    mStarTwinkleAni.setLoop(true).start();
                }
            } else if (aniTag == TAG_ANI_STAR_SNOW_TWINKLE) {
                mStarTwinkleAni.stop();
                mStarTwinkleGlo.setVisible(false);

                if (!mStarTwinkleSnowGlo.getVisible()) {
                    mStarTwinkleSnowGlo.setVisible(true);
                    mStarTwinkleSnowAni.setLoop(true).start();
                }
            } else {
                mGloList.get(aniTag).setVisible(true);
                mGloAniList.get(aniTag).start();
            }
        }
    }

    private void setContainerVisible(boolean visible) {
        if (mCityContainer != null) {
            mCityContainer.setVisible(visible);
        }
        if (mWeatherContainer != null) {
            mWeatherContainer.setVisible(visible);
        }
        if (mForecastContainer != null) {
            mForecastContainer.setVisible(visible);
        }
        if (mUpdateContainer != null) {
            mUpdateContainer.setVisible(visible);
        }
    }

    private void setCityContainerVisible(boolean visible) {
        if (mCityContainer != null) {
            mCityContainer.setVisible(visible);
        }
    }

    private void setUpdateContainerVisible(boolean visible) {
        if (mUpdateContainer != null) {
            mUpdateContainer.setVisible(visible);
        }
    }

    private void setNoWeatherScenarioContainer() {
        setSpareWeatherGloInvisible();
        int currentWeatherModelType = WeatherType.convertToModelType(mCurrentWeather);

        if (WeatherType.isModelTypeInRange(currentWeatherModelType)) {
            stopWeatherAni(currentWeatherModelType, true);
        }
    }

    private void setSpareWeatherGloInvisible() {
        mSunMoonGlo.setVisible(false);
        mSunShowHideAni.stop();
        LogUtil.v(TAG, "sun_show_hide_stop");
        mMoonShowHideAni.stop();
        LogUtil.v(TAG, "moon_show_hide_stop");
        mDayToNightGlo.setVisible(false);
        mNightToDayGlo.setVisible(false);
        setStarTwinkleInvisible();
    }

    private void changeGroundTexture(String filename) {
        mLandscapeGlo.setMaterialProperty(
                "ground001", "M_DIFFUSE_TEXTURE", filename);
    }

    private void resetGroundSnowTexture() {
        changeGroundTexture(SnowTextureList.TEXTURE_MAP[SnowTextureList.SNOW_PILE_000]);
    }

    private void changeGroundSandyTexture() {
        changeGroundTexture("landscape_sandy.jpg");
    }

    private void resetGroundTexture() {
        changeGroundTexture("new_weather.jpg");
    }

    private void changeTreeTexture(String filename) {
        mLandscapeGlo.setMaterialProperty(
                "tree", "M_DIFFUSE_TEXTURE", filename);
    }

    private void changeTreeSnowTexture() {
        changeTreeTexture("tree_snow.jpg");
    }

    private void changeTreeSandyTexture() {
        changeTreeTexture("tree_sandy.jpg");
    }

    private void resetTreeTexture() {
        changeTreeTexture("tree.jpg");
    }

    private void changeWaterTexture(String filename) {
        mLandscapeGlo.setMaterialProperty(
                "water", "M_DIFFUSE_TEXTURE", filename);
    }

    private void changeWaterSnowTexture() {
        changeWaterTexture("water_snow.png");
    }

    private void changeWaterSandyTexture() {
        changeWaterTexture("water_sandy.png");
    }

    private void resetWaterTexture() {
        changeWaterTexture("water.png");
    }

    private void startSnow() {
        if (mIsSnowing) {
            mChangeSnowTextureAnimation.stop();
        } else {
            changeTreeSnowTexture();
            changeWaterSnowTexture();
        }
        mChangeSnowTextureAnimation.setLoop(true).start();
        resetGroundSnowTexture();

        mIsSnowing = true;
    }

    private void startSandy() {
        changeTreeSandyTexture();
        changeWaterSandyTexture();
        changeGroundSandyTexture();
    }

    private void resetWeatherObject() {
        // use to stop snow, stop sand, reset weather object
        if (mIsSnowing) {
            mChangeSnowTextureAnimation.stop();
        }
        resetGroundTexture();
        resetTreeTexture();
        resetWaterTexture();
        mIsSnowing = false;
    }

    private String getTempString(double temp, int unit) {
        String unitString;
        if (unit == 0) {
            unitString = "C";
        } else {
            unitString = "F";
        }
        return (int)temp + getResources().getString(R.string.degree) + unitString;
    }

    private String getHighLowTempString(double tempHigh, double tempLow) {
        return (int)tempLow + getResources().getString(R.string.degree) + "/" +
                (int)tempHigh + getResources().getString(R.string.degree);
    }

    private String getCityIndexString(int index, int total) {
        return index + "/" + total;
    }

    private String getLastUpdateString(long lastUpdateTime) {
        String update;
        if (Util.isSameDay(lastUpdateTime, System.currentTimeMillis())) {
            update = Util.getTimeString(mContext, lastUpdateTime);
        } else {
            update = Util.getDateString(lastUpdateTime);
        }

        return getResources().getString(R.string.last_update) + " : " + update;
    }

    private String getDayOfWeek(int dayOfWeek) {
        if (dayOfWeek < Calendar.SUNDAY || dayOfWeek > Calendar.SATURDAY) {
            LogUtil.e(TAG, "Invalid argument:" + dayOfWeek);
            return "";
        }
        return DateUtils.getDayOfWeekString(dayOfWeek, DateUtils.LENGTH_MEDIUM).toUpperCase();
    }

    private void updateWeather(WeatherInfo weather, int scrollType) {
        LogUtil.v(TAG, "updateWeather, name = " + weather.getCityName());
        LogUtil.v(TAG, "updateWeather, temp = " + weather.getCurrentTemp());
        LogUtil.v(TAG, "updateWeather, low/high = " + weather.getTempLow() + "/" + weather.getTempHigh());
        updateCityInfo(weather.getCityName(), weather.getCityIndex() + 1, weather.getTotalCity());
        updateTemp(weather.getCurrentTemp(), weather.getTempHigh(), weather.getTempLow(), weather.getTempType());
        updateForecast(weather.getForecastData(), scrollType);
        updateLastUpdate(weather.getLastUpdated(), false);
        updateWeatherObject(weather.getCondition(), Util.getDayNight(weather.getTimeZone()));
    }

    private void setState(STATE state) {
        LogUtil.v(TAG, "setState, (" + mState + " to " + state + ")");
        mState = state;
    }

    @android.view.RemotableViewMethod
    public void updateWeatherView(Bundle bundle) {
        LogUtil.v(TAG, "updateWeatherView - state = " + mState);
        mTotalCity = bundle.getInt("totalCity");

        if (mTotalCity == -1) {
            LogUtil.v(TAG, "updateWeatherView - totalCity = -1");

            setContainerVisible(false);
            setNoWeatherScenarioContainer();
            // when there is no city, cannot scroll, so should have no arrow
            mCityBarArrowsImage.setVisible(false);

            mNoWeatherText.setText(getResources().getString(R.string.update_fail));
            mNoWeatherText.setVisible(true);

            mExtraLightGlo.setVisible(true);
            setForecastPanelTransparent();

            // reset currentWeather and currentDayNight
            mCurrentWeather = 0;
            mCurrentDayNight = DayNight.DAY;

            setState(STATE.NO_CITY);
        } else if (mTotalCity == 0) {
            LogUtil.v(TAG, "updateWeatherView - totalCity = 0");
            setContainerVisible(false);
            setNoWeatherScenarioContainer();
            // when there is no city, cannot scroll, so should have no arrow
            mCityBarArrowsImage.setVisible(false);

            mNoWeatherText.setText(getResources().getString(R.string.no_weather));
            mNoWeatherText.setVisible(true);

            mExtraLightGlo.setVisible(true);
            setForecastPanelTransparent();

            // reset currentWeather and currentDayNight
            mCurrentWeather = 0;
            mCurrentDayNight = DayNight.DAY;

            setState(STATE.NO_CITY);
        } else {
            LogUtil.v(TAG, "updateWeatherView - totalCity > 0");
            // when there is city, can scroll, add arrow to give user a hint
            mCityBarArrowsImage.setVisible(true);
            if (mState == STATE.SCROLLING) {
                // if needAnimation == false, then throw exception
                setState(STATE.GOT_DATA_WAIT_SCROLL_END);
                mWeatherBundle = bundle;
            } else {
                // STATE.INIT is happening when start a new demo widget
                // STATE.UPDATING is happening when start a new real data widget
                // STATE.SCROLLED_WAIT_DATA is happening when user swipe the widget
                updateScreen(bundle);
            }
        }
    }

    private void updateScreen(Bundle bundle) {

        mWeatherInfo.setCityIndex(bundle.getInt("cityIndex"));
        mWeatherInfo.setTotalCity(bundle.getInt("totalCity"));
        mWeatherInfo.setCityName(bundle.getString("cityName"));
        mWeatherInfo.setLastUpdated(bundle.getLong("lastUpdated"));
        String timeZoneString = bundle.getString("timeZone");

        int result = bundle.getInt("result");
        LogUtil.v(TAG, "result = " + result);
        mWeatherInfo.setResult(result);

        if (result == WeatherUpdateResult.SUCCESS) {
            setState(STATE.SCROLLED_UPDATE_DATA);

            mWeatherInfo.setCondition(bundle.getInt("condition"));
            mWeatherInfo.setTempType(bundle.getInt("tempType"));
            mWeatherInfo.setCurrentTemp(bundle.getDouble("temp"));
            mWeatherInfo.setTempHigh(bundle.getDouble("highTemp"));
            mWeatherInfo.setTempLow(bundle.getDouble("lowTemp"));
            mWeatherInfo.setTimeZone(timeZoneString);
            ForecastData[] data = new ForecastData[3];
            data[0] = new ForecastData(bundle.getInt("firstDayOfWeek"), bundle.getDouble("firstHighTemp"),
                    bundle.getDouble("firstLowTemp"), bundle.getInt("firstForecast"));
            data[1] = new ForecastData(bundle.getInt("secondDayOfWeek"), bundle.getDouble("secondHighTemp"),
                    bundle.getDouble("secondLowTemp"), bundle.getInt("secondForecast"));
            data[2] = new ForecastData(bundle.getInt("thirdDayOfWeek"), bundle.getDouble("thirdHighTemp"),
                    bundle.getDouble("thirdLowTemp"), bundle.getInt("thirdForecast"));
            mWeatherInfo.setForecastData(data);
            int scrollType = bundle.getInt("order");
            LogUtil.v(TAG, "scroll_type = " + scrollType);

            mNoWeatherText.setVisible(false);
            setContainerVisible(true);
            mExtraLightGlo.setVisible(false);
            restoreForecastPanelTransparent();

            updateWeather(mWeatherInfo, scrollType);

            if (scrollType == ScrollType.SCROLL_UP) {
                startForecastUpAnimation();
            } else if (scrollType == ScrollType.SCROLL_DOWN) {
                startForecastDownAnimation();
            }
        } else {
            // no network case
            setContainerVisible(false);
            setNoWeatherScenarioContainer();
            mNoWeatherText.setVisible(false);

            if (result != WeatherUpdateResult.INIT_VALUE) {
                // when result == INIT_VALUE, city and last update information is null. hide these two fields.
                updateCityInfo(mWeatherInfo.getCityName(), mWeatherInfo.getCityIndex() + 1, mWeatherInfo.getTotalCity());
                updateLastUpdate(mWeatherInfo.getLastUpdated(), false);
                setCityContainerVisible(true);
                setUpdateContainerVisible(true);
            }

            mExtraLightGlo.setVisible(true);
            setForecastPanelTransparent();

            if (result == WeatherUpdateResult.ERROR_NETWORK_NOT_AVAILABLE) {
                mNoWeatherText.setText(getResources().getString(R.string.no_network));
            } else {
                mNoWeatherText.setText(getResources().getString(R.string.update_fail));
            }
            mNoWeatherText.setVisible(true);

            // directly set current day night = Day (initial value)
            // because when result = ERROR_UPDATE_WEATHER_FAILED, the timezone = null
            // so don't need to use timezone to get current day night
            // and we don't show day or night when result != success.
            mCurrentDayNight = DayNight.DAY;
            mCurrentWeather = 0;

            setState(STATE.NO_NETWORK);
        }
    }

    @android.view.RemotableViewMethod
    public void showUpdating(int i) {
        LogUtil.v(TAG, "showUpdating");
        setContainerVisible(false);
        setNoWeatherScenarioContainer();
        mNoWeatherText.setVisible(false);

        setCityContainerVisible(true);
        setUpdateContainerVisible(true);
        updateLastUpdate(0, true);

        mCurrentWeather = 0;
        setState(STATE.UPDATING);
    }

    @android.view.RemotableViewMethod
    public void switchDayNight(int switchType) {
        if (!(mState == STATE.NORMAL || mState == STATE.SCROLLED_UPDATE_DATA)) {
            // when in init, no city, no network -> no weather condition shown -> don't need to update day night
            return;
        }
        int dayNight = Util.getDayNight(mWeatherInfo.getTimeZone());
        LogUtil.v(TAG, "switchDayNight, (original, current) = (" + mCurrentDayNight + ", " + dayNight + ")");

        if (dayNight == mCurrentDayNight) {
            // do nothing
            return;
        }

        setStarTwinkleInvisible();
        int currentWeatherModelType = WeatherType.convertToModelType(mCurrentWeather);
        boolean isSunMoonNeeded = WeatherType.isSunMoonNeededModelType(currentWeatherModelType);

        if (dayNight == DayNight.DAY) {
            // handle sun/moon
            if (mMoonShowHideAni.isStarted()) {
                mMoonShowHideAni.stop();
            }
            removeAniFromQueue(TAG_ANI_DAY_TO_NIGHT);

            if (isSunMoonNeeded) {
                mSunShowHideAni.setLoop(true).start();
                addAniToQueue(TAG_ANI_NIGHT_TO_DAY);
            }
            removeAniFromQueue(TAG_ANI_STAR_SNOW_TWINKLE);
            removeAniFromQueue(TAG_ANI_STAR_TWINKLE);
        } else {
            // handle sun/moon
            if (mSunShowHideAni.isStarted()) {
                mSunShowHideAni.stop();
            }
            removeAniFromQueue(TAG_ANI_NIGHT_TO_DAY);

            if (isSunMoonNeeded) {
                mMoonShowHideAni.setLoop(true).start();
                addAniToQueue(TAG_ANI_DAY_TO_NIGHT);
            }

            // handle star
            if (WeatherType.isSnowModelType(currentWeatherModelType)) {
                addAniToQueue(TAG_ANI_STAR_SNOW_TWINKLE);
            } else {
                addAniToQueue(TAG_ANI_STAR_TWINKLE);
            }
        }

        addAniToQueue(0);
        playQueuedAni();

        mCurrentDayNight = dayNight;
    }

    private static final int TIMER_DURATION = 5000;
    private final Handler mHandlerTimer = new Handler();

    @android.view.RemotableViewMethod
    public void startTimer(int dummy) {
        LogUtil.v(TAG, "start timer");
        mHandlerTimer.removeCallbacks(mTimerRun);
        mHandlerTimer.postDelayed(mTimerRun, TIMER_DURATION);
    }

    @android.view.RemotableViewMethod
    public void stopTimer(int dummy) {
        LogUtil.v(TAG, "stop timer");
        mHandlerTimer.removeCallbacks(mTimerRun);
    }

    private final Runnable mTimerRun = new Runnable() {
        @Override
        public void run() {
            LogUtil.v(TAG, "timer out, widgetId = " + mAppWidgetId);
            Intent intent = new Intent(WeatherWidgetAction.ACTION_CHECK_WIDGET);
            intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetId);
            getContext().sendBroadcast(intent);

            startTimer(1);
        }
    };

    private void printWeatherBundle(Bundle bundle) {
        LogUtil.v(TAG,
                "cityIndex = " + bundle.getInt("cityIndex") +
                ", totalCity = " + bundle.getInt("totalCity") +
                ", cityName = " + bundle.getString("cityName") +
                ", condition = " + bundle.getInt("condition") +
                ", tempType = " + bundle.getInt("tempType") +
                ", temp = " + bundle.getDouble("temp") +
                ", tempHigh = " + bundle.getDouble("highTemp") +
                ", tempLow = " + bundle.getDouble("lowTemp") +
                ", lastUpdated = " + bundle.getLong("lastUpdated") +
                ", timeZone = " + bundle.getString("timeZone") +
                ", forecast-1 = " + bundle.getDouble("firstLowTemp") + "/" +
                        bundle.getDouble("firstHighTemp") + "/" + bundle.getInt("firstForecast") +
                ", forecast-2 = " + bundle.getDouble("secondLowTemp") + "/" +
                        bundle.getDouble("secondHighTemp") + "/" + bundle.getInt("secondForecast") +
                ", forecast-3 = " + bundle.getDouble("thirdLowTemp") + "/" +
                        bundle.getDouble("thirdHighTemp") + "/" + bundle.getInt("thirdForecast"));
    }

    private void setStarTwinkleInvisible() {
        mStarTwinkleAni.stop();
        mStarTwinkleGlo.setVisible(false);

        mStarTwinkleSnowAni.stop();
        mStarTwinkleSnowGlo.setVisible(false);
    }

    private void setForecastPanelTransparent() {
        // when no weather or no network connected, the forecast panel should be transparent
        mForecastPanelContainer.setOpacity(0x7F);
    }

    private void restoreForecastPanelTransparent() {
        // restore the forecast panel to original opacity
        mForecastPanelContainer.setOpacity(0xFF);
    }

    private final Animation.Listener mAnimationCompletedHandler = new Animation.Listener() {
        public void onStarted(Animation animation) {
            final int tag = animation.getTag();
            //LogUtil.v(TAG, "ani - onStarted: " + animation.getName() + " ," + animation);

            if (tag == TAG_ANI_STAR_TWINKLE || tag == TAG_ANI_STAR_SNOW_TWINKLE) {
                playQueuedAni();
            }
        }

        public void onMarkerReached(Animation animation, int direction, String marker) {
            final int tag = animation.getTag();
            //LogUtil.v(TAG, "ani - onMarkerReached: " + animation.getName() + " ," + animation);

            if (tag == TAG_ANI_CHANGE_SNOW_TEXTURE) {
                changeGroundTexture(SnowTextureList.TEXTURE_MAP[Integer.valueOf(marker)]);
            }
        }

        public void onCompleted(Animation animation) {
            final int tag = animation.getTag();
            //LogUtil.v(TAG, "ani - onCompleted: " + tag);

            if (tag == TAG_ANI_CHANGE_SNOW_TEXTURE) {
                resetGroundSnowTexture();
            } else if (tag == TAG_ANI_CHANGE_CITY_NEXT || tag == TAG_ANI_CHANGE_CITY_PREVIOUS) {
                LogUtil.v(TAG, "model_scroll_end");
                if (mState == STATE.SCROLLING) {
                    setState(STATE.SCROLLED_WAIT_DATA);
                    // do nothing
                } else if (mState == STATE.GOT_DATA_WAIT_SCROLL_END) {
                    // STATE.GOT_DATA_WAIT_SCROLL_END is happening when user swipe the widget
                    // update screen directly
                    updateScreen(mWeatherBundle);
                } else {
                    // ToDo:
                    // throw exception
                }
            } else if (tag == TAG_ANI_DAY_TO_NIGHT || tag == TAG_ANI_NIGHT_TO_DAY) {
                playQueuedAni();
                if (tag == TAG_ANI_DAY_TO_NIGHT) {
                    // from DAY to NIGHT, so stop SUN show hide
                    mSunShowHideAni.stop();
                    LogUtil.v(TAG, "sun_show_hide_stop");
                } else {
                    // from NIGHT to DAY, so stop MOON show hide
                    mMoonShowHideAni.stop();
                    LogUtil.v(TAG, "moon_show_hide_stop");
                }
            }
        }
    };

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        super.onSurfaceCreated(gl, config);
        LogUtil.v(TAG, "onSurfaceCreated(), width x height = (" + getWidth() + " x " + getHeight() + ")");
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        super.onSurfaceChanged(gl, width, height);
        LogUtil.v(TAG, "onSurfaceChanged(), width x height = (" + width + " x " + height + ")");
    }

    private int mAppWidgetId = AppWidgetManager.INVALID_APPWIDGET_ID;
    @android.view.RemotableViewMethod
    public void setWidgetId(int widgetId) {
        mAppWidgetId = widgetId;
    }

    private float mDragYDist = 0f;

    private boolean isScrollAllowed() {
        return (mState == STATE.NO_NETWORK || mState == STATE.NORMAL);
    }

    private void sendScrollIntent(boolean direction) {
        LogUtil.v(TAG, "sendScrollIntent - isNextDirection = " + direction + ", widgetId = " + mAppWidgetId);
        Intent intent = new Intent(WeatherWidgetAction.ACTION_SCROLL);
        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetId);
        intent.putExtra(WeatherWidgetAction.DIRECTION, direction
                ? WeatherWidgetAction.DIRECTION_NEXT
                : WeatherWidgetAction.DIRECTION_PREVIOUS);
        getContext().sendBroadcast(intent);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean handled = super.onTouchEvent(event);
        boolean isNextDirection;

        if (!handled) {
            handled =  mGestureDetector.onTouchEvent(event);
        }

        if (!handled) {
            if (event.getAction() == MotionEvent.ACTION_UP) {
                LogUtil.v(TAG, "onTouchEvent - YDrag = " + mDragYDist);
                if (Math.abs(mDragYDist) > 0f) {
                    LogUtil.v(TAG, "onTouchEvent - YDragRatio = " + Math.abs(calcDragRatio()));
                    getChangeCityAnimation().stopDragging();

                    if ((Math.abs(calcDragRatio()) * 1000) > TO_GO_THRESHOLD) {
                        LogUtil.v(TAG, "onTouchEvent - YDragRatio > ");
                        if (mDragYDist > 0) {
                            isNextDirection = false;
                        } else {
                            isNextDirection = true;
                        }
                        sendScrollIntent(isNextDirection);
                        setState(STATE.SCROLLING);
                    } else {
                        LogUtil.v(TAG, "onTouchEvent - YDragRatio < ");
                        getChangeCityAnimation().reverse();
                    }
                }
                mDragDirection = DRAG_DIRECTION.IDLE;
                mDraggingDirection = DRAG_DIRECTION.IDLE;
                mDragState = DRAG_STATE.FINISH;
                LogUtil.v(TAG, "onUP - updateDragState = FINISH; Direction = IDLE");
                mDragYDist = 0;
                return true;
            } else {
                return false;
            }
        }
        return true;
    }

    enum DRAG_STATE {
        INITIAL, START, DRAGGING, FINISH;
    }

    enum DRAG_DIRECTION {
        IDLE, HOTZONE, UP, DOWN;
    }

    DRAG_STATE mDragState = DRAG_STATE.INITIAL;
    DRAG_DIRECTION mDragDirection = DRAG_DIRECTION.IDLE;    // to represent the current up or down animation
    DRAG_DIRECTION mDraggingDirection = DRAG_DIRECTION.IDLE; // to represent the current gesture is up or down

    private static final int ON_DRAG_THRESHOLD = 10;
    private static final float DRAG_STANDARD_DISTANCE = 900f;
    private static final float HOT_ZONE_THRESHOLD = 0.005f * DRAG_STANDARD_DISTANCE;
    private static final float TO_GO_THRESHOLD = 0.05f * DRAG_STANDARD_DISTANCE;

    private float calcDragRatio() {
        return mDragYDist / DRAG_STANDARD_DISTANCE;
    }

    private void updateDragState() {
        if (mDragState == DRAG_STATE.FINISH) {
            mDragState = DRAG_STATE.INITIAL;
            LogUtil.v(TAG, "updateDragState = INITIAL");
        } else if (mDragState == DRAG_STATE.INITIAL) {
            mDragState = DRAG_STATE.START;
            LogUtil.v(TAG, "updateDragState = START");
        } else {
            mDragState = DRAG_STATE.DRAGGING;
            LogUtil.v(TAG, "updateDragState = DRAGGING");
        }
    }

    private void updateAnimation() {
        if (mDragState == DRAG_STATE.START) {
            getChangeCityAnimation().startDragging();
            mDragState = DRAG_STATE.DRAGGING;
            LogUtil.v(TAG, "updateAnimation - startDragging");
        } else if (mDragState == DRAG_STATE.DRAGGING) {
            float ratio = Math.abs(calcDragRatio());
            if (isAnimationAndYDistNotConsistent()) {
                getChangeCityAnimation().setProgress(0.f);
                mDragDirection = DRAG_DIRECTION.HOTZONE;
                mDragState = DRAG_STATE.FINISH;
                LogUtil.v(TAG, "updateAnimation - discrete sampling point");
            } else if (isAnimationAndDragNotConsistent() && ratio * DRAG_STANDARD_DISTANCE <= HOT_ZONE_THRESHOLD) {
                getChangeCityAnimation().setProgress(0.f);
                mDragDirection = DRAG_DIRECTION.HOTZONE;
                mDragState = DRAG_STATE.FINISH;
                LogUtil.v(TAG, "updateAnimation - less than threshold");
            } else {
                getChangeCityAnimation().setProgress(ratio);
                LogUtil.v(TAG, "updateAnimation - setProgress = " + ratio);
            }
        }
    }

    private PropertyAnimation getChangeCityAnimation() {
        if (mDragDirection == DRAG_DIRECTION.DOWN) {
            LogUtil.v(TAG, "getChangeCityAnimation - down");
            return mChangeNextCityAnimation;
        } else {
            LogUtil.v(TAG, "getChangeCityAnimation - up");
            return mChangePreviousCityAnimation;
        }
    }

    private class WeatherGestureListener extends GestureDetector.SimpleOnGestureListener {
        private boolean mDisallowIsRequested;

        @Override
        public boolean onDown(MotionEvent e) {
            // return true, otherwise we cannot get any other following events.
            mDisallowIsRequested = false;
            return true;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
                float distanceY) {
            if (!mDisallowIsRequested) {
                getParent().requestDisallowInterceptTouchEvent(true);
                mDisallowIsRequested = true;
            }

            if (!isScrollAllowed()) {
                return true;
            }

            mDragYDist += distanceY;
            if (distanceY >= 0) {
                mDraggingDirection = DRAG_DIRECTION.UP;
            } else {
                mDraggingDirection = DRAG_DIRECTION.DOWN;
            }

            LogUtil.v(TAG, "onScroll - DragY = " + mDragYDist + ", distY = " + distanceY + ", Direc = " + mDragDirection);
            if (mDragDirection == DRAG_DIRECTION.IDLE) {
                if (Math.abs(mDragYDist) > ON_DRAG_THRESHOLD) {
                    if (mDragYDist > 0) {
                        mDragDirection = DRAG_DIRECTION.UP;
                    } else {
                        mDragDirection = DRAG_DIRECTION.DOWN;
                    }
                    updateDragState();
                    updateAnimation();
                }
            } else if (mDragDirection == DRAG_DIRECTION.HOTZONE) {
                if (Math.abs(mDragYDist) > HOT_ZONE_THRESHOLD) {
                    if (mDragYDist > 0) {
                        mDragDirection = DRAG_DIRECTION.UP;
                    } else {
                        mDragDirection = DRAG_DIRECTION.DOWN;
                    }
                    updateDragState();
                    updateAnimation();
                }
            } else {
                updateDragState();
                updateAnimation();
            }
            return true;
        }
    }

    private boolean isAnimationAndDragNotConsistent() {
        // Example: of return ! case.
        // swipe up first, then swipe down, the YDist is still positive, but the delta distance is negative, means this time,
        return !((mDragDirection == DRAG_DIRECTION.DOWN && mDraggingDirection == DRAG_DIRECTION.DOWN) ||
                (mDragDirection == DRAG_DIRECTION.UP && mDraggingDirection == DRAG_DIRECTION.UP));
    }

    private boolean isAnimationAndYDistNotConsistent() {
        // Example: of return ! case,
        // mDragDirection = UP, and mDragYDist < 0, it is because the position sampling is discrete,
        // when you swipe up, then down, you may think the mDragYDist will be from a +number and decrease to 0,
        // then to -number, and when decrease to 0, it will be handled by hot zone case.
        // but this will not always happen. the position sampling is discrete, so you may have a case to from a +number,
        // directly jump to -number, no procedure of decreasing to 0.
        return !((mDragDirection == DRAG_DIRECTION.DOWN && mDragYDist <= 0) ||
                (mDragDirection == DRAG_DIRECTION.UP && mDragYDist >= 0));
    }

    private void reapplyMaterials() {
        List<Actor> list = mScenarioContainer.getChildren();
        for (Actor actor : list) {
            Object3D obj = (Object3D)actor;
            if (obj != null && !obj.equals(mSunMoonGlo)) {
                obj.setMaterial("ngin3d#weather.mat");
                obj.setMaterial("fog_cloud_01", "ngin3d#fog.mat");
            }
        }
    }

    private void reapplyPremultiplyMaterial() {
        // after reinitialize, should set again. Or the color will be over bright.
        for (Actor actor : mPremultiplyActorList) {
            if (actor != null) {
                actor.setMaterial("ngin3d#quad_premultiply.mat");
            }
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        super.onDrawFrame(gl);
        if (mFirstDraw) {
            LogUtil.v(TAG, "mFirstDraw - " + mFirstDraw);

            Stage.getUiHandler().post(new Runnable() {
                public void run() {
                    mAlphaAnimation.start();
                }
            });
            mFirstDraw = false;
        }
    }

    @Override
    protected void onVisibilityChanged(View changedView, int visibility) {
        LogUtil.v(TAG, "onVisibilityChanged - " + visibility);
        super.onVisibilityChanged(changedView, visibility);
        if (visibility == VISIBLE) {
            reapplyMaterials();
            reapplyPremultiplyMaterial();
            int currentWeatherModelType = WeatherType.convertToModelType(mCurrentWeather);
            if (WeatherType.isSnowModelType(currentWeatherModelType)) {
              changeTreeSnowTexture();
              changeWaterSnowTexture();
            } else if (WeatherType.isSandModelType(currentWeatherModelType)) {
                startSandy();
            }
            playQueuedAni();
        } else {
            // reset the Drag position back to initial point.
            // issue: drag to some position, then press power key to pause,
            // then resume, the position is not in initial point.
            if (mDragYDist != 0) {
                getChangeCityAnimation().setProgress(0.f);

                mDragState = DRAG_STATE.INITIAL;
                mDragDirection = DRAG_DIRECTION.IDLE;
                mDraggingDirection = DRAG_DIRECTION.IDLE;
                mDragYDist = 0;
            }
            setAlpha(0.0f);
            mFirstDraw = true;
            mAlphaAnimation.cancel();
        }
    }

    private void stopAllAnimation() {
        LogUtil.v(TAG, "stopAllAnimation");
        stopWeatherAni(WeatherType.convertToModelType(mCurrentWeather), false);

        mSunShowHideAni.stop();
        mMoonShowHideAni.stop();
        mStarTwinkleAni.stop();
        mStarTwinkleSnowAni.stop();
        mChangeSnowTextureAnimation.stop();
    }

    @Override
    protected void onAttachedToWindow() {
        // set cache dir
        String cacheDir = "/data/data/" + getRootView().getContext().getPackageName();
        LogUtil.v(TAG, "onAttachedToWindow - cacheDir = " + cacheDir);
        setCacheDir(getContext(), cacheDir);

        super.onAttachedToWindow();

        LogUtil.v(TAG, "onAttachedToWindow - current = " + mCurrentWeather + ", dayNight = " + mCurrentDayNight);
        if (mCurrentWeather != 0) {
            int condition = mCurrentWeather;
            int dayNight = mCurrentDayNight;
            mCurrentWeather = 0;
            mCurrentDayNight = DayNight.DAY;
            updateWeatherObject(condition, dayNight);
        } else {
            /* if current weather == 0, means that the weather view is just init, no weather update;
               send out ACTION_UPDATE to update weather information */
            Intent intent = new Intent(WeatherWidgetAction.ACTION_UPDATE);
            intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, AppWidgetManager.INVALID_APPWIDGET_ID);
            getContext().sendBroadcast(intent);
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        LogUtil.v(TAG, "onDetachedFromWindow");
        stopAllAnimation();
        stopTimer(1);
        super.onDetachedFromWindow();
    }

    @android.view.RemotableViewMethod
    public void onScreenOff(int dummy) {
        LogUtil.v(TAG, "onScreenOff - mDragYDist = " + mDragYDist);
        if (mDragYDist != 0) {
            getChangeCityAnimation().setProgress(0.f);

            mDragState = DRAG_STATE.INITIAL;
            mDragDirection = DRAG_DIRECTION.IDLE;
            mDraggingDirection = DRAG_DIRECTION.IDLE;
            mDragYDist = 0;
        }
    }
}
