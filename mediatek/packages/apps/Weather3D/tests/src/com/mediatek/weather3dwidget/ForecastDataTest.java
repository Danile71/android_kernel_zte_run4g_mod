package com.mediatek.weather3dwidget;

import android.test.ActivityInstrumentationTestCase2;

/**
 * This is a simple framework for a test of an Application.  See
 * {@link android.test.ApplicationTestCase ApplicationTestCase} for more information on
 * how to write and extend Application tests.
 * <p/>
 * To run this test, you can type:
 * adb shell am instrument -w \
 * -e class com.mediatek.weather3dwidget.CityMangerTest \
 * com.mediatek.weather3dwidget.tests/android.test.InstrumentationTestRunner
 */

public class ForecastDataTest extends ActivityInstrumentationTestCase2<WeatherActivity> {
    private ForecastData mForecastData;

    public ForecastDataTest() {
        super("com.mediatek.weather3dwidget", WeatherActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mForecastData = new ForecastData(3, 25.2, 24.2, 10);
    }

    @Override
    protected void tearDown() throws Exception {
        mForecastData = null;
        super.tearDown();
    }

    public void test01Constructor() {
        assertEquals(3, mForecastData.getDayOfWeek());
        assertEquals(25.2, mForecastData.getHighTemp());
        assertEquals(24.2, mForecastData.getLowTemp());
        assertEquals(10, mForecastData.getWeatherCondition());
    }

    public void test02ResetData() {
        mForecastData.resetForecastData();

        assertEquals(0, mForecastData.getDayOfWeek());
        assertEquals(0.0, mForecastData.getHighTemp());
        assertEquals(0.0, mForecastData.getLowTemp());
        assertEquals(0, mForecastData.getWeatherCondition());
    }

    public void test03SetMethod() {
        mForecastData.setForecastData(4, 28.2, 27.2, 5);

        assertEquals(4, mForecastData.getDayOfWeek());
        assertEquals(28.2, mForecastData.getHighTemp());
        assertEquals(27.2, mForecastData.getLowTemp());
        assertEquals(5, mForecastData.getWeatherCondition());;
    }

    public void test04ToString() {
        assertEquals("Forecast, DayOfWeek = 3, Low/High = 24.2/25.2, Condition = 10", mForecastData.toString());
    }
}
