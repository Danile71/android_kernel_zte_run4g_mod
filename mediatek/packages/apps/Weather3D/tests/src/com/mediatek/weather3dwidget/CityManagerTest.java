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

public class CityManagerTest extends ActivityInstrumentationTestCase2<WeatherActivity> {

    public CityManagerTest() {
        super("com.mediatek.weather3dwidget", WeatherActivity.class);
    }

    public void test01SetCityIndex() {
        int cityIndex = 0;
        CityManager.setTotal(1);
        CityManager.setCurrentIndex(cityIndex);
        assertEquals(cityIndex, CityManager.getCurrentIndex());
    }

    public void test02TestPreviousCity() {
        CityManager.setTotal(5);
        CityManager.setCurrentIndex(4);
        assertEquals(3, CityManager.getPreviousCity());

        CityManager.setCurrentIndex(0);
        assertEquals(4, CityManager.getPreviousCity());
    }

    public void test03TestNextCity() {
        CityManager.setTotal(5);

        CityManager.setCurrentIndex(3);
        assertEquals(4, CityManager.getNextCity());

        CityManager.setCurrentIndex(4);
        assertEquals(0, CityManager.getNextCity());
    }
}
