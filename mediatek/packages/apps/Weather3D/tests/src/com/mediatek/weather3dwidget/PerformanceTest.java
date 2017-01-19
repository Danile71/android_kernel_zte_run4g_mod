package com.mediatek.weather3dwidget;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.test.ActivityInstrumentationTestCase2;
import com.mediatek.ngin3d.android.StageTextureView;
import com.mediatek.weather.WeatherTable;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.String;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThanOrEqualTo;
import static org.hamcrest.Matchers.is;

/**
 * This is a simple framework for a test of an Application.  See
 * {@link android.test.ApplicationTestCase ApplicationTestCase} for more information on
 * how to write and extend Application tests.
 * <p/>
 * To run this test, you can type:
 * adb shell am instrument -w \
 * -e class com.mediatek.weahter3dwidget.PerformanceTest \
 * com.mediatek.weather3dwidget.tests/android.test.InstrumentationTestRunner
 */

public class PerformanceTest extends ActivityInstrumentationTestCase2<WeatherActivity> {
    private WeatherActivity mActivity;
    private static final float FPS_CRITERIA = 10.0f;
    ForecastData[] mForecast;

    public PerformanceTest() {
        super("com.mediatek.weather3dwidget", WeatherActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();

        /* use arbitrary data, we want to test 3D model FPS, which icon in forecast is not important */
        mForecast = new ForecastData[3];
        mForecast[0] = new ForecastData(1, 25, 24, 2);
        mForecast[1] = new ForecastData(2, 26, 23, 3);
        mForecast[2] = new ForecastData(3, 27, 22, 4);
    }

    @Override
    protected void tearDown() throws Exception {
        mActivity = null;
        super.tearDown();
    }

    private static void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ie) {
            // just end it.
        }
    }

    private float getPageFps(StageTextureView view, int msDuration, int msSampleInterval) {
        if (msSampleInterval < 0 || msDuration < 0) {
            return -1;
        }
        view.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        sleep(3000); // wait 3 sec to wait for create from glo assets, and stable

        int i = 0;
        float fpsTotal = 0;
        for (; msDuration > 0; msDuration -= msSampleInterval) {
            sleep(msSampleInterval);
            fpsTotal += view.getFPS();
            i++;
        }
        fpsTotal /= i;
        view.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        return fpsTotal;
    }

    private void writePerformanceData(Activity activity, String name, Object data) {
        File dataFile = new File(activity.getDir("perf", Context.MODE_PRIVATE), name);
        dataFile.delete();
        try {
            FileWriter writer = new FileWriter(dataFile);
            writer.write("YVALUE=" + data);
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void testFPS(WeatherTable.WeatherCondition condition, String fpsFileName) {
        assertNotNull(mActivity);
        getInstrumentation().waitForIdleSync();

        // prepare data for show the weather condition
        LocationWeather lw = new LocationWeather(0, "Taipei", "GMT+8", condition, 23, 21, 24, mForecast);

        Bundle bundle = Util.getWeatherBundle(0, 1, lw, 0);
        WeatherView weatherView = (WeatherView)(mActivity.findViewById(R.id.view_3d_weather));
        weatherView.updateWeatherView(bundle);
        assertNotNull(weatherView);

        float fps = getPageFps(weatherView,
                30000,      // measure 30 sec
                1000);      // sample each second
        writePerformanceData(mActivity, fpsFileName, fps);
        assertThat(fps, is(greaterThanOrEqualTo(FPS_CRITERIA)));
    }

    public void test01SunnyFPS() {
        testFPS(WeatherTable.WeatherCondition.Sunny, "01.sunny.fps.txt");
    }

    public void test02WindyFPS() {
        testFPS(WeatherTable.WeatherCondition.Windy, "02.windy.fps.txt");
    }

    public void test03BlusteryFPS() {
        testFPS(WeatherTable.WeatherCondition.Hurricane, "03.blustery.fps.txt");
    }

    public void test04CloudyFPS() {
        testFPS(WeatherTable.WeatherCondition.Cloudy, "04.cloudy.fps.txt");
    }

    public void test05RainFPS() {
        testFPS(WeatherTable.WeatherCondition.Rain, "05.rain.fps.txt");
    }

    public void test06HeavyRainFPS() {
        testFPS(WeatherTable.WeatherCondition.Downpour, "06.heavy_rain.fps.txt");
    }

    public void test07ThunderFPS() {
        testFPS(WeatherTable.WeatherCondition.Sunny, "07.thunder.fps.txt");
    }

    public void test08SnowFPS() {
        testFPS(WeatherTable.WeatherCondition.Snow, "08.snow.fps.txt");
    }

    public void test09HeavySnowFPS() {
        testFPS(WeatherTable.WeatherCondition.HeavySnow, "09.heavy_snow.fps.txt");
    }

    public void test10SnowRainFPS() {
        testFPS(WeatherTable.WeatherCondition.Sleet, "10.snow_rain.fps.txt");
    }

    public void test11FogFPS() {
        testFPS(WeatherTable.WeatherCondition.Fog, "11.fog.fps.txt");
    }

    public void test12SandFPS() {
        testFPS(WeatherTable.WeatherCondition.Sand, "12.sand.fps.txt");
    }
}