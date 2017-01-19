package com.mediatek.calendarimporter.tests.functional.annotation;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

//use below command to run test case marked with annotation of "InternalApiAnnotation"
//adb shell am instrument  -w -e annotation com.mediatek.calendarimporter.tests.functional.annotation.InternalApiAnnotation com.mediatek.calendarimporter.tests/com.mediatek.calendarimporter.FullFunctionalTestRunner

@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD, ElementType.TYPE})
public @interface InternalApiAnnotation {

}
