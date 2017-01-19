'''
Created on 2014/04/10

@author: MTK04199
'''
import os, sys
import time
import subprocess

import signal


def signal_handler(signal, frame):
        print('Ctrl+C to exit!')
        sys.exit(0)



def externel_cmd(ex_cmd, log = 1):
    
    if log > 0 :
        p = subprocess.Popen(ex_cmd, shell=True, stderr=subprocess.PIPE)
         
        ## But do not wait till netstat finish, start displaying output immediately ##
        while True:
            out = p.stderr.read(1)
            if out == '' and p.poll() != None:
                break
            if out != '':
                sys.stdout.write(out)
                sys.stdout.flush()
    else :
        t = os.popen(ex_cmd).read()



if __name__ == '__main__':


    signal.signal(signal.SIGINT, signal_handler)


    #externel_cmd('push_data.bat')
    
    externel_cmd('run_test.bat')
    

    
#    #print '(default)All (1)VT (2)Camera '
#    
#    print 'Start Test !!'    
#
#    db_before = os.popen('adb shell ls /sdcard/mtklog/aee_exp').read()    
#    
#    test_file_vt = 'monkeyScript_VideoTelephony.py'
#    
#    ####################################################
#    # start test VT loopback mode
#    
#    #os.popen(run_cmd)
#    
#    test_file = test_file_vt 
#    path = os.getcwd()
#    #print path 
#    run_cmd = ' monkeyrunner ' + path + '\\' + test_file
#        
#    ## run it ##
#    p = subprocess.Popen(run_cmd, shell=True, stderr=subprocess.PIPE)
#     
#    ## But do not wait till netstat finish, start displaying output immediately ##
#    while True:
#        out = p.stderr.read(1)
#        if out == '' and p.poll() != None:
#            break
#        if out != '':
#            sys.stdout.write(out)
#            sys.stdout.flush()
#
#
#    ####################################################    
#    
#    print 'VT LoopBack Test Done!!'
#    
#    # back to home
#    os.popen('adb shell input keyevent 03') 
#    
#    #run_cmd = 'push_data.bat'
#    #os.popen('push_data.bat')
#
#
#

    
    
#    ####################################################
#    ############### test ScreenRecord   ################
#    ####################################################
#    
#    # init param
#    test_screenRecord_time = 15
#    
#    
#    # launch camera app
#    print '\n[screenrecord] start test'
#    #os.popen('adb shell screenrecord --time-limit 15 /data/autoTestScreenRecord.mp4')    
#    #os.system('adb shell screenrecord /data/autoTestScreenRecord.mp4')    
#    rec_cmd = 'adb shell screenrecord /data/autoTestScreenRecord.mp4'
#    subprocess.Popen(rec_cmd, shell=True, stderr=subprocess.PIPE)
#    
#    #time.sleep(test_screenRecord_time+5)
#    
#    print '[screenrecord] pull video'
#    os.popen('adb pull /data/autoTestScreenRecord.mp4')
#    
#    time.sleep(test_screenRecord_time)
#    
#    
#    print '[screenrecord] remove video'
#    os.popen('adb shell rm /data/autoTestScreenRecord.mp4')


    #raw_input()
    #os.popen('pause') os.system('adb shell ls /sdcard/mtklog/aee_exp')
    
    