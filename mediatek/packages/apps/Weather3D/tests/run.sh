#!/bin/sh

local TOPFILE=build/core/envsetup.mk
# We redirect cd to /dev/null in case it's aliased to
# a command that prints something as a side-effect
# (like pushd)
local HERE=`/bin/pwd`
T=
while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
	cd .. > /dev/null
	T=`PWD= /bin/pwd`
done
if [ -f "$T/$TOPFILE" ]; then
	SRC_ROOT=$T
else
	echo "Error: source tree was not found."
	exit
fi

PRODUCT=`cat $SRC_ROOT/.product`
PRODUCT_OUT=$SRC_ROOT/out/target/product/$PRODUCT
INTERMEDIATES=$SRC_ROOT/out/target/common/obj/APPS/Weather3DWidget_intermediates
TEST_RUNNER=com.zutubi.android.junitreport.JUnitReportTestRunner

if adb remount
then
  # Prevent insufficient storage space
  adb shell rm -r /data/core

  # Copy binary to device
  adb uninstall com.mediatek.weather3dwidget
  adb install -r $PRODUCT_OUT/system/app/Weather3DWidget.apk
  adb uninstall com.mediatek.weather3dwidget.tests
  adb install -r $PRODUCT_OUT/data/app/Weather3DWidgetTests.apk

  # restart VM then system will use new Weather3DWidget.apk
  adb shell "stop;sleep 5;start"
  sleep 30

  PACKAGE=com.mediatek.weather3dwidget

  # remove junit-report and coverage.ec
  adb shell rm /data/data/$PACKAGE/files/coverage.ec
  adb shell rm /data/data/$PACKAGE/files/junit-report.xml
  # Run instrumentation test
  adb shell am instrument -e coverage true -w $PACKAGE.tests/$TEST_RUNNER
  adb pull /data/data/$PACKAGE/files/junit-report.xml $SRC_ROOT/junit-report-weather3d.xml

  # Pull performance test data
  adb pull /data/data/$PACKAGE/app_perf $SRC_ROOT/perf-$PACKAGE

  # Generate emma code coverage report
  cd $INTERMEDIATES
  adb pull /data/data/$PACKAGE/files/coverage.ec
  java -cp ~/local/emma/lib/emma.jar emma report -r xml -in coverage.ec -in coverage.em
fi
