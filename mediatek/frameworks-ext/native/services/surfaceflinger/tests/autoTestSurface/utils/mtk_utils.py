

import sys, os, time

import re



def android_time() :
    
    logcat_cmd = 'adb logcat -v threadtime -t 1'
    
    androidlog = os.popen(logcat_cmd).read()
    
    m = re.search('\d{2}:\d{2}:\d{2}.\d{3}',androidlog)

    if m :
        logtime = m.group(0)
        logtime = logtime.replace('.',':')
    else :
        logtime = None
        
    return logtime

def externel_cmd(ex_cmd, log = 1):
    
    if log > 0 :
        

        #p = subprocess.Popen(ex_cmd, shell=True, stderr=subprocess.PIPE)
        p = subprocess.Popen(ex_cmd, shell=True, stdout=subprocess.PIPE).stdout
        t = p.read()
        
        ## But do not wait till netstat finish, start displaying output immediately ##
        # while True:
        #     out = p.stderr.read(1)
        #     if out == '' and p.poll() != None:
        #         break
        #     if out != '':
        #         sys.stdout.write(out)
        #         sys.stdout.flush()
    else :
        t = os.popen(ex_cmd).read()
        
    return t



def signal_handler(signal, frame):
        print('Ctrl+C to exit!')
        sys.exit(0)
