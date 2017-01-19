'''
Created on 2014/04/10

@author: MTK04199
'''
import os, sys, getopt
import time
import subprocess

from utils import *

import signal



def signal_handler(signal, frame):
        print('Ctrl+C to exit!')
        sys.exit(0)



def externel_cmd(ex_cmd, log = 0):
    
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
        

def run_test_folder(rootpath, folder, isDry , verbose, prepare = True) :
    
    test_path = os.path.abspath(folder)
    
    os.chdir(test_path)
    
    test_cmd = test_path + '\\run.py' 
    
    ## run it ##
    if isDry == False :
        if prepare == True :
            externel_cmd('push_data.bat', verbose)
        externel_cmd(test_cmd, verbose)
    
    os.chdir(rootpath)

def usage() :
    print 'python auotRunUnitTest.py [option] -f [testFolder] '
    print '[option]'
    print '\t-h : help'
    print '\t-d : dry run '
    print '\t-v : verbose'
    print '\t-e : user event'
    print '\t-n : normal only'
    
    

if __name__ == '__main__':

    isFail = False 
    test_idx = 0
    autoFolderList = []
    isDryRun = False
    isTestDB = False
    isVerbose = 0
    isRunExtEvent = True
    islogIon = False

    try:
        opts, args = getopt.getopt(sys.argv[1:],"f:vhdtnem")
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    
    
    for opt, arg in opts:
        if opt in ("-h") :
            usage()
            sys.exit(0)
        elif opt in ("-f"):
            autoFolderList.append(arg) 
        elif opt in ("-d"):
            isDryRun = True
        elif opt in ("-t"):
            isTestDB = True
        elif opt in ("-v"):
            isVerbose = 1
        elif opt in ("-n"):
            isRunExtEvent = False
        elif opt in ("-e"):
            isRunExtEvent = True
        elif opt in ("-m"):
            islogIon = True
            
 
    signal.signal(signal.SIGINT, signal_handler)

    if len(autoFolderList) == 0 :
        autoFolderList = ['autoVideoTelephony','autoVDSaddVideo', 'autoVDSvlw']
    
    
    root_path = os.path.abspath('.')
    
    rstFolder = root_path + time.strftime("\\SF_UT_LOG_%y%m%d_%H%M%S")
    
    os.makedirs(rstFolder)
    
    logDB = log_db()
    #logDB.clean_db()
    
    logFD = log_fd(outfile = rstFolder+'\\logfd.csv')
    logFD.log_level = 0
    logFD.start(async = True)
    
    if islogIon == True :
        log_ion = log_ion_heap(rstFolder+'\\log_ion_heap.csv')
    
    
    #logDB.start_sample()
    
    #print 'Unit Test Start'
    print '==== test normal case ===='
    
    for testFolder in autoFolderList :
        
        
        print '[%d] Test : %s ...' % (test_idx,testFolder),
        
        run_test_folder(root_path, testFolder, isDryRun, isVerbose, True)
        
        test_idx += 1

        if isTestDB == True :
            logDB.utTestDB()
        
        time.sleep(2)
        if logDB.isNewDB() == False and logFD.isOverThreshold() == False :
            print 'Pass'
        else :
            isFail = True
            print 'Fail, ',
            logDB.aee_case_report()
            print ', FD_diff(%d)' % (logFD.diffFd()),
            print '!!'
    
    
    if isRunExtEvent == True :
      
        print '==== test suspend/resume ===='
        
        apm = autoPowerManager()
        
        
        
        for testFolder in autoFolderList :
            
            apm.suspend_interval = 3
            #time.sleep(4)
                
            print '[%d] Test : %s ...' % (test_idx,testFolder),
            
            run_test_folder(root_path, testFolder, isDryRun, isVerbose, False)
            
            test_idx += 1
            
            #apm.suspend_interval = -1
            #time.sleep(9)

            if isTestDB == True :
                logDB.utTestDB()
            
            time.sleep(2)
            if logDB.isNewDB() == False and logFD.isOverThreshold() == False :
                print 'Pass'
            else :
                isFail = True
                print 'Fail, ',
                logDB.aee_case_report()
                print ', FD_diff(%d)' % (logFD.diffFd()),
                print '!!'
                
               
    



    print '==== test long suspend/resume ====' 
    
    apm.suspend_interval = 20

    print '[%d] Test : test suspend %d (sec) ...' % (test_idx, apm.suspend_interval),    
    time.sleep(30)    
    
    test_idx += 1

    if logDB.isNewDB() == False and logFD.isOverThreshold() == False :
        print 'Pass'
    else :
        isFail = True
        print 'Fail, ',
        logDB.aee_case_report()
        print ', FD_diff(%d)' % (logFD.diffFd()),
        print '!!'




    apm.suspend_interval = 0
    time.sleep(4)    
    
    my_html = csv2html()
    my_html.write_html(rstFolder+'\\logfd.csv','FD_USAGE',rstFolder+'\\logfd.html')
    if islogIon == True :
        my_html.write_html(rstFolder+'\\log_ion_heap.csv','ION_MM_HEAP',rstFolder+'\\log_ion_heap.html')
    
    print '\n========================================================================'
    
    print 'Unit Test Result Summary:',
    if isFail == True :
        print 'Fail!!'
    else:
        print 'Pass!!'
    
    logDB.aee_report()
    
    logFD.stop()
    
    print 'Check FD : diff is %d !!' % (logFD.diffFd()) 

    print '========================================================================'       
    
    print 'Unit Test done!! (press ENTER to exit)'
    raw_input()
    
    