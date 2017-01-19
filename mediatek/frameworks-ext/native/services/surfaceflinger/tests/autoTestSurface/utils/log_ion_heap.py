
import sys, os, time
import csv
import re

import signal

from mtk_utils import *

from threading import Thread

from csv2html import csv2html

#1527606 fd leak

#write_file = 'log_ion_heap.csv' 

log_level = 0

def get_proc_ion_heap_size (client, dbg_name, ret) :
    p1 = client
    p2 = dbg_name
    regx_str = p1 + '\S+\s+' + p2 + '\S+\s+\d+\s+\d+'
    #regx_str = 'surfaceflinger\S+\s+gralloc\S+\s+\d+\s+\d+'
    
    m = re.findall(regx_str,ret)
    
    if m:
        #print m[0]
        pat = m[0].replace(client,'')
        t = re.findall('\d+',pat)    
        return int(t[1])/1024
    else :
        return 0
        



class log_ion_heap (object) :
    
    def __init__ (self, write_file):
        
        self._csvFile = write_file
        self._logheader = ["TIME", "GRALLOC", "SF", "MEDIASERVER", "GALLERY3D", "UI", "LAUNCHER"]

        self._t = Thread(target=self._loop)
        self._t.daemon = True
        self._t.start()               
        
    def _loop(self) :
        
        with open(self._csvFile,'wb') as csv_file:
            
            csv_fp = csv.writer(csv_file, quoting=csv.QUOTE_NONNUMERIC)
            csv_fp.writerow(self._logheader)
        
            while True :
                
                logtime = android_time()
                
                ret = os.popen('adb shell cat /sys/kernel/debug/ion/ion_mm_heap').read()
                
                sf_size = get_proc_ion_heap_size('surfaceflinger','gralloc',ret)
                
                pvr_size = get_proc_ion_heap_size('pvrsrvctl','', ret)
                
                ms_size = get_proc_ion_heap_size('mediaserver','gralloc', ret)
                
                gallery_size = get_proc_ion_heap_size('gallery3d','gralloc', ret)
                
                
                system_ui_size = get_proc_ion_heap_size('systemui','gralloc', ret)
                
                launcher_size = get_proc_ion_heap_size('launcher3','gralloc', ret)
                
                
                if logtime != None and sf_size != 0:
                    
                    if log_level > 0:
                        print 'logtime: %s, pvr %d, sf %d, mediaserver %d, gallery3d %d, ui %d, launcher %d!!' % (logtime, pvr_size, sf_size, ms_size, gallery_size, system_ui_size, launcher_size)
                    
                    logrow = [logtime, pvr_size, sf_size, ms_size, gallery_size, system_ui_size, launcher_size]
                    
                    #logtime += 1
                
                    csv_fp.writerow(logrow)
                    csv_file.flush()
                    
                    #time.sleep(1)
    
        
    
    
def signal_handler_html(signal, frame):
        
    print('Ctrl+C to exit!')

    t = csv2html()
    
    t.write_html('log_ion_heap.csv', 'ION_HEAP','log_ion_heap.html')
    
    sys.exit(0)
        


if __name__ == '__main__':
    
    signal.signal(signal.SIGINT, signal_handler_html)


    log_level = 1
    #t = log_ion_heap()
    t = log_ion_heap('log_ion_heap.csv')
    
    while True :
        time.sleep(5)

