#Import class of MonkeyRunner, MonkeyDevice, and MonkeyImage.
from com.android.monkeyrunner import MonkeyRunner, MonkeyDevice, MonkeyImage
import os, sys
from com.android.monkeyrunner.easy import EasyMonkeyDevice
from com.android.monkeyrunner.easy import By





if __name__ == '__main__':
    
    
    device1 = MonkeyRunner.waitForConnection()
    
    # Use the EasyMonkey API, all methods on device are available
    # in easy_device.
    MonkeyRunner.sleep(2);
    easy_device = EasyMonkeyDevice(device1)
    print 'Connected Device!'
    
    
    # init param
    wait_app_launch = 3
    wait_button_time = 1
    wait_cmd_time = 2
    test_vt_time = 20
        
    MonkeyRunner.sleep(wait_cmd_time);
    
    # launch VT working app
    print 'Launch Engineering Working Menu'
    os.popen('adb shell "am start -a android.intent.action.MAIN -n com.mediatek.engineermode/.videotelephone.WorkingMode"')
    MonkeyRunner.sleep(wait_app_launch);
    
    easy_device.touch(By.id("id/working_mode_media_loopback"), MonkeyDevice.DOWN_AND_UP )
    MonkeyRunner.sleep(wait_button_time);
    

    easy_device.touch(By.id("id/media_loopback_stack"), MonkeyDevice.DOWN_AND_UP )    
    #MonkeyRunner.sleep(wait_button_time);
    
    # MonkeyRunner.sleep(wait_app_launch);
    # print 'launch VT Activity'
    # os.popen('adb shell "am start -a com.android.vt.VTLoopbackActivity.LAUNCH -n com.android.phone/.VTLoopbackActivity"')
    
    
    # Launch VT Lookback Activity
    #  os.popen('adb shell "am start -a com.android.vt.VTLoopbackActivity.LAUNCH -n com.android.phone/.VTLoopbackActivity"')    
    #  #os.popen('adb shell "am start -a com.android.vt.VTLoopbackActivity.MAIN -n com.android.phone/.VTLoopbackActivity"')
    #  #os.popen('adb shell "am start -a android.intent.action.MAIN -n com.android.phone/.VTLoopbackActivity"')    
    #  # am start -a android.intent.action.MAIN -n com.android.phone/.VTLoopbackActivity 
    #  # am start -a com.android.vt.VTLoopbackActivity.LAUNCH -n com.android.phone/.VTLoopbackActivity
    
    # wait lunch VT done
    MonkeyRunner.sleep(wait_app_launch);
    
    print '[VT] Test Start ' + test_vt_time + '(s)'
    #MonkeyRunner.sleep(2);
    easy_device.touch(By.id("id/VTLBStart"), MonkeyDevice.DOWN_AND_UP )
    MonkeyRunner.sleep(wait_button_time);
    print '[VT] Testing...'
    MonkeyRunner.sleep(test_vt_time);
    
    easy_device.touch(By.id("id/VTLBEnd"), MonkeyDevice.DOWN_AND_UP )
    MonkeyRunner.sleep(wait_button_time);
    print '[VT] Test done'
        
    
    #  result = os.popen('adb shell ls /sdcard/mtklog/aee_exp').read()
    #  
    #  if result.find('KE') >= 0 or result.find('NE') >= 0 or result.find('ANR') >= 0 or result.find('SWT') >= 0 :
    #    print('UnitTest FAIL!!')
    #  elif result.find('JE') >= 0:
    #    print('UnitTest PASS, but found Java Exception!!')
    #  else:
    #    print('UnitTest PASS!!')
    
    #By.id("id/all_apps_button")
    #easy_device.visible(By.id("id/all_apps_button"))
    #    raise Error('Could not find the "all apps" button')
    
