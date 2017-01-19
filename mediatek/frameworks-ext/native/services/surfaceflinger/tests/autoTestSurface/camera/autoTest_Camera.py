
#Import class of MonkeyRunner, MonkeyDevice, and MonkeyImage.
import os, sys
from com.android.monkeyrunner import MonkeyRunner, MonkeyDevice, MonkeyImage
from com.android.monkeyrunner.easy import EasyMonkeyDevice
from com.android.monkeyrunner.easy import By




if __name__ == '__main__':

    device1 = MonkeyRunner.waitForConnection()
    easy_device = EasyMonkeyDevice(device1)
    print 'connected device success..'
    
    # test config
    image_loops = 1
    wait_app_launch = 9
    wait_cmd_time = 3
    wait_video_rec_time = 7
    
    #width = device1.getProperty('display.width')
    #if int(str(width)) == 720:
    #    play_camera_video = (360, 670)
    #    save_capture_pic = (400, 600)
    #else:
    #    print('Error screen size not support..')
    #    sys.exit()
    
    print '==========================================='
    print '=== test Camera capture image and video ==='
    print '==========================================='
    
    # back to home screen
    device1.press( 'KEYCODE_HOME', "DOWN_AND_UP")
    
    
    
    # test camera capture image 
    for i in range(image_loops):
        print 'Test image capture: ', i
        
        # launch camera image mode
        print 'Launch image capture..'
        os.popen('adb shell "am start -a android.media.action.IMAGE_CAPTURE"')
        MonkeyRunner.sleep(wait_app_launch)    
        
        # test camera shot
        device1.press( 'KEYCODE_CAMERA' , "DOWN_AND_UP")
        #easy_device.touch(By.id("id/camera_shutter"), MonkeyDevice.DOWN_AND_UP )
        MonkeyRunner.sleep(wait_cmd_time)
        
        # save capture picture
        easy_device.touch(By.id("id/btn_done"), MonkeyDevice.DOWN_AND_UP ) #=> ok
        #device1.touch( save_capture_pic[0], save_capture_pic[1], "DOWN_AND_UP")
        #easy_device.touch(By.id("id/remaining_view"), MonkeyDevice.DOWN_AND_UP )
        MonkeyRunner.sleep(wait_cmd_time)
        
    
    
    print 'Test image capture done..'  
      
    
    # launch camera video mode
    print 'Launch video capture..'
    os.popen('adb shell "am start -a android.media.action.VIDEO_CAPTURE"')
    MonkeyRunner.sleep(wait_app_launch)
    
    print 'Test video capture..'
    device1.press( 'KEYCODE_CAMERA' , "DOWN_AND_UP")
    
    # wait for video record
    MonkeyRunner.sleep(wait_video_rec_time)
    
    # stop video record
    device1.press( 'KEYCODE_CAMERA' , "DOWN_AND_UP")
    MonkeyRunner.sleep(wait_cmd_time)
    print 'Test video capture done..'
    
    # play record video
    print 'Test video playback..'
    easy_device.touch(By.id("id/btn_play"), MonkeyDevice.DOWN_AND_UP ) 
    #device1.touch( 360, 670, "DOWN_AND_UP")
    #device1.touch( play_camera_video[0], play_camera_video[1], "DOWN_AND_UP")
    MonkeyRunner.sleep(wait_video_rec_time+2)
    print 'Test video playback done..'
    
    # back to home 
    device1.press( 'KEYCODE_HOME', "DOWN_AND_UP")
    
    
    print '==========================================='
    print '===     test Camera capture done        ==='
    print '==========================================='
    
    
    # check if has DB file
    print 'check if DB file exist'
    MonkeyRunner.sleep(wait_cmd_time)
    
    
    
    result = os.popen('adb shell ls /sdcard/mtklog/aee_exp').read()
    if result.find('KE') >= 0 or result.find('NE') >= 0 or result.find('ANR') >= 0 or result.find('SWT') >= 0 :
        print('UnitTest FAIL!!')
    elif result.find('JE') >= 0:
        print('UnitTest PASS, but found Java Exception!!')
    else:
        print('UnitTest PASS!!')
    
    #device1.press( 'KEYCODE_MEDIA_PLAY' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_BUTTON_START' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_MEDIA_NEXT' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_MEDIA_PAUSE' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_MEDIA_PLAY_PAUSE' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_APP_SWITCH' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_APOSTROPHE' , "DOWN_AND_UP")
    #device1.press( 'KEYCODE_BUTTON_1' , "DOWN_AND_UP")
    #device1.getProperty('am.current.package')
    #device1.getProperty('am.current.action')
    #device1.getProperty('display.width')
    #device1.getProperty('display.height')
    #device1.getProperty('am.current.comp.class')
    #device1.getProperty('am.current.comp.package')
    
    #device1.press( 'KEYCODE_HOME', "DOWN_AND_UP")
    
    #CurrentDeviceX = float(device.getProperty("display.width"))
    #CurrentDeviceY = float(device.getProperty("display.height"))


