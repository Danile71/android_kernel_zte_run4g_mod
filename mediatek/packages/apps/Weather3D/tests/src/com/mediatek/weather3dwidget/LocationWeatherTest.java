package com.mediatek.weather3dwidget;

import android.test.ActivityInstrumentationTestCase2;
import com.mediatek.weather.WeatherTable;
import com.mediatek.weather.WeatherUpdateResult;

/**
 * This is a simple framework for a test of an Application.  See
 * {@link android.test.ApplicationTestCase ApplicationTestCase} for more information on
 * how to write and extend Application tests.
 * <p/>
 * To run this test, you can type:
 * adb shell am instrument -w \
 * -e class com.mediatek.weather3dwidget.LocationWeatherTest \
 * com.mediatek.weather3dwidget.tests/android.test.InstrumentationTestRunner
 */

public class LocationWeatherTest extends ActivityInstrumentationTestCase2<WeatherActivity> {
    public LocationWeatherTest() {
        super("com.mediatek.weather3dwidget", WeatherActivity.class);
    }

    public void test01Constructor01() {
        int locationIndex = 0;
        ForecastData[] forecastData = {new ForecastData(1, 29, 25,
                LocationWeather.getWeather(WeatherTable.WeatherCondition.Sunny)),
                new ForecastData(2, 30, 24, LocationWeather.getWeather(WeatherTable.WeatherCondition.Windy)),
                new ForecastData(3, 28, 25, LocationWeather.getWeather(WeatherTable.WeatherCondition.Cloudy))};
        LocationWeather lw = new LocationWeather(0, "Taipei", "GMT+8", WeatherTable.WeatherCondition.Sunny,
                28, 24, 32, forecastData);

        assertEquals(locationIndex, lw.getLocationIndex());
        assertEquals("Taipei", lw.getLocationName());
        assertEquals("GMT+8", lw.getTimezone());
        assertEquals(28.0, lw.getCurrentTemp());
        assertEquals(24.0, lw.getTempLow());
        assertEquals(32.0, lw.getTempHigh());
        assertEquals(0, lw.getTempType());
        assertEquals(WeatherType.Type.SUNNY, lw.getWeather());
        assertEquals(WeatherUpdateResult.SUCCESS, lw.getResult());

        ForecastData[] data = lw.getForecastData();
        assertEquals(forecastData[0].getDayOfWeek(), data[0].getDayOfWeek());

        String lwString = lw.toString();
        assertTrue(lwString.startsWith("result = 0, cityID = -1, Timezone = GMT+8, Temp = 28.0, Low = 24.0, High = 32.0, lastUpdate ="));
    }


    public void test02Constructor02()  {
        int locationIndex = 0;
        int cityId = 2100;
        LocationWeather lw = new LocationWeather(locationIndex, cityId);
        assertEquals(cityId, lw.getCityId());
        assertEquals(WeatherUpdateResult.INIT_VALUE, lw.getResult());
    }
}
