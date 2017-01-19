
import sys,os,time

import subprocess

import signal

import re

from threading import Thread

class state :
    NONE = 0
    SUSPEND = 1
    KEY = 2
    RESUME = 3
    

def signal_handler(signal, frame):
        print('Ctrl+C to exit!')
        sys.exit(0)

class autoPowerManager(object) :
    
    def __init__(self) :
        self.suspend_interval = 0
        self._t = Thread(target=self._loop)
        self._t.daemon = True
        self._t.start()       
        self._state = state.NONE 
        self._width = 1080
        self._height = 1920
        self._getInfo()
       
    
    def _getInfo(self) :
        rst = os.popen('adb shell dumpsys SurfaceFlinger').read()
        m = re.search('Display\[0\] : \d+x\d+',rst)
        rst = m.group(0).replace('Display[0] : ','')
        info = rst.split('x')
        self._width = int(info[0])
        self._height = int(info[1])
        
    
    @property
    def suspend_interval(self) :
        return self._suspend_interval 

    @suspend_interval.setter
    def suspend_interval(self, value) :
        self._suspend_interval = value

    @staticmethod
    def isSuspend() :
        return self._getState() == state.SUSPEND
        
    def _getState(self) :
        rst = os.popen('adb shell dumpsys SurfaceFlinger').read()
        hwc_layer = len( re.findall('HWC\s+\|',rst))
        gles_layer = len( re.findall('GLES\s+\|',rst))
        isKeyguard = rst.count('Keyguard') > 0
        
        if (hwc_layer, gles_layer) == (0,0) :
            self._state = state.SUSPEND
        elif isKeyguard > 0 :
            self._state = state.KEY
        else :
            self._state = state.RESUME
        
        return self._state
    
    
    def _power(self) : 
        #print 'press power...'
        os.popen('adb shell input keyevent KEYCODE_POWER')
    
    def _unlockScreen(self) :
        #get screen width and height
        #W = 1080 #int(device1.getProperty('display.width'))
        #H = 1920 #int(device1.getProperty('display.height'))
        
        start_x = self._width/2
        start_y = self._height*3/4
        
        #swipe_cmd = 'adb shell input swipe 535 1375 900 1375'
        swipe_cmd = 'adb shell input swipe %d %d %d %d' % (start_x, start_y , start_x , start_y-500)
        
        
        #swipe_cmd = 'adb shell input swipe 350 890 607 921'
        
        #print 'unlockScreen...'
        #return
        #swipe_cmd = 'adb shell dumpsys SurfaceFlinger'
        os.popen(swipe_cmd)
        
        #device1.drag((int(W) / 2, int(H) * 3 / 4 ), (int(W) / 2, (int(H) *3 / 4 ) - 200) )
        
        # MonkeyRunner.sleep(5)
        # device1.press('KEYCODE_BACK', MonkeyDevice.DOWN_AND_UP)
        # return 

    def _suspend(self) :
        if self._getState() != state.SUSPEND :
            self._power()
    
    def _resume(self) :
        self._getState()
        if self._state == state.SUSPEND : 
            self._power()
            time.sleep(3) 
            self._unlockScreen()
        elif self._state == state.KEY :
            self._unlockScreen()
        
    
    def _loop(self) : 
        while True :
            period = self.suspend_interval
            if period > 0 :
                self._suspend()
                time.sleep(period)
                self._resume()
                time.sleep(period)
            elif period == 0 : 
                self._resume()
                time.sleep(period)
            else :
                time.sleep(3)
    


if __name__ == '__main__' :

    signal.signal(signal.SIGINT, signal_handler)


     
    t = autoPowerManager()
    
    print 'toggle start!!'
    t.suspend_interval = 3
    
    time.sleep(23)
    print 'toggle stop!!'
    #t.suspend_interval = 0
    
    while True :
        time.sleep(15)
    
        
