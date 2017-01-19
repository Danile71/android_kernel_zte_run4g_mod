'''
Created on 2014/05/12

@author: MTK04199 Otis Huang
@purpose: log surfaceflinger fd usage
'''

import sys, os, time
import csv
import re

import signal

from threading import Thread, Lock

class log_fd(object):
    
    def __init__ (self, outfile = None):
        
        if outfile != None : 
            self._csvFile = outfile 
        else :
            self._csvFile = 'logfd.csv'
        
        self._process = 'surfaceflinger'
        self._period = 20
        self._init_fd_num = 0
        self._min_fd_num = 1024
        self._max_fd_num = 0
        self._cur_fd_num = 0
        self._async = False 
        self._signal_end = False 
        self._isDaemon = True 
        self._logheader = ["TIME","TOTAL", "FENCE", "G3D", "DMABUF", "SOCKET", "OTHERS"]
        self._mutex = Lock()
        self.loglist = []
        self._threshold = 50
        self._log_level = 0;

        mycmd = 'adb shell ps '+self._process
        result = os.popen(mycmd).read()
        m_pid = re.search('\d+',result)
        self._pid = m_pid.group(0)
        self._cmd_get_fd = 'adb shell ls -l /proc/'+ self._pid + '/fd'
        print ("logFd search process: {0}, pid: {1} \nlogfile: {2} !!".format(self._process, self._pid, self._csvFile))

    
    @property
    def log_level(self) :
        return self._log_level

    @log_level.setter
    def log_level(self,value) :
        self._log_level = value
    
    def diffFd(self) :
        return self._cur_fd_num - self._init_fd_num
        
    def isOverThreshold(self) :
        if (self._cur_fd_num - self._init_fd_num) >= self._threshold :
            return True
        else : 
            return False
    
    def _loopQueryFd(self):
        

        loop = 0
        
        logcat_cmd = 'adb logcat -v threadtime -t 1'
        #logcat_cmd = 'adb shell date -s 20140000'
        
        looptimes = self._period*1
        logtime = ''
        
        androidlog = '' #os.popen(logcat_cmd).read()
        with open(self._csvFile,'wb') as csv_file:
            
            csv_fp = csv.writer(csv_file, quoting=csv.QUOTE_NONNUMERIC)
            csv_fp.writerow(self._logheader)
            

            
            #while loop < looptimes :
            while True :
                
                self._mutex.acquire()
                endflag = self._signal_end
                self._mutex.release()
                if endflag == True :
                    break
                
                androidlog = os.popen(logcat_cmd).read()
                #androidlog = '01-02 16:09:48.608'
                result = os.popen(self._cmd_get_fd).read()
            
                fd_count = result.count('->')
                g3d_count = result.count('pvrsrvkm')
                sync_fence_count = result.count('sync_fence')
                dmabuf_count = result.count('dmabuf')
                socket_count = result.count('socket')
                
                if fd_count > 0 : 
                    
                    self._cur_fd_num = fd_count
                    
                    if self._init_fd_num == 0: self._init_fd_num = fd_count
                    
                    if self._min_fd_num > fd_count : self._min_fd_num = fd_count
                        
                    if self._max_fd_num < fd_count : self._max_fd_num = fd_count  
                    #print androidlog
                    
                    
                    m = re.search('\d{2}:\d{2}:\d{2}.\d{3}',androidlog)
                    #m = re.search('\d{2}:\d{2}:\d{2}',androidlog)
                    #print m.group(0)
                    if m :
                        logtime = m.group(0)
                        logtime = logtime.replace('.',':')
                        
                        #print logtime,':FD hisory (',self._init_fd_num, self._max_fd_num ,')', ', current TOTAL count is:',fd_count ,'fence:', sync_fence_count, 'pvr:', g3d_count, 'dmabuf:', dmabuf_count, 'socket:' , socket_count
                        if self._log_level > 0 :
                            print("{0}:FD (int:{1},Max:{2},cur:{3}), fence:{4},pvr:{5},dmabuf:{6},socket:{7}!".format(logtime, self._init_fd_num, self._max_fd_num, fd_count, sync_fence_count, g3d_count, dmabuf_count, socket_count))
                        other_cnt = fd_count - sync_fence_count - g3d_count - dmabuf_count - socket_count
                        logrow = [logtime ,fd_count, sync_fence_count, g3d_count, dmabuf_count, socket_count, other_cnt]
                        #loglist.append( (logtime ,fd_count, sync_fence_count, g3d_count, dmabuf_count, socket_count, other_cnt) )
                        
                        csv_fp.writerow(logrow)
                        csv_file.flush()
                    
                    
                    #time.sleep(0.5)
                    #loop+=1
                
    def start(self, async = False):
        
            
        #print ("start:{0}, {1}".format(async, outfile))
        if async == True :
            self._async = True
            self._t = Thread(target=self._loopQueryFd)
            self._t.daemon = self._isDaemon
            self._t.start()
        else: 
            self._loopQueryFd()
        return 
        
    def stop(self):
        if self._async == True :
            self._mutex.acquire()
            self._signal_end = True 
            self._mutex.release()
            
            if self._isDaemon == False : 
                self._t.join()
        
            


def signal_handler(signal, frame):
        #print('Ctrl+C pressed !')
        # myc.signal_end = True
        # if isDaemonThread == False :
        #         print 'wait join sub thread!!'        
        #         t.join()
        # print 'go here!!'
        mylogfd.stop()
        print 'log done!!'
        sys.exit(0)

#usage: log_fd.py [outfile]
if __name__ == '__main__':
    
   signal.signal(signal.SIGINT, signal_handler)

   
   if len(sys.argv) < 2 :
       output = None
   else:
       output = sys.argv[1]

    
   mylogfd = log_fd(outfile=output)
   
   mylogfd.log_level = 1
   mylogfd.start(async = True)
   
   while True:
        #print 'logging!!'
        time.sleep(1)
   
   

