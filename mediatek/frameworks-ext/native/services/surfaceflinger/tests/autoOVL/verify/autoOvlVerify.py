#C:\Python27\python.exe
# -*- coding:utf-8 -*-

import os
import sys
import glob
import filecmp

from DumpsysParser import *
#---------------------------------------------------------------------------------------------------
MAX_OVL_LAYER = 4
#OVL_SUPPORT_DIM = False
OVL_SUPPORT_DIM = True
#---------------------------------------------------------------------------------------------------
def ConsoleCommand( cmd ):
    #print cmd
    #os.system( cmd )
    os.popen(cmd).read()
#---------------------------------------------------------------------------------------------------
print 'process 0%'
cmd = 'run.py'
ConsoleCommand( cmd )

#cmd = 'del SF_auto_ovl.log'
#ConsoleCommand( cmd )

print 'process 20%'
cmd = 'rmdir /S /Q data'
ConsoleCommand( cmd )

cmd = 'md data'
ConsoleCommand( cmd )

cmd = 'adb pull /data/SF_dump/SF_auto_ovl.log .\\data'
ConsoleCommand( cmd )

print 'process 40%'
cmd = 'adb pull /data/SF_dump/gpu_compose.bin .\\data'
ConsoleCommand( cmd )

print 'process 60%'
cmd = '..\\bin\\raw_bmp.exe .\\data\\gpu_compose.bin .\\data\\gpu_compose.bmp'
ConsoleCommand( cmd )

cmd = 'adb pull /data/SF_dump/hwc_compose.bin .\\data'
ConsoleCommand( cmd )

print 'process 80%'
cmd = '..\\bin\\raw_bmp.exe .\\data\\hwc_compose.bin .\\data\\hwc_compose.bmp'
ConsoleCommand( cmd )

p = Parser( './data/SF_auto_ovl.log' )


#---------------------------------------------------------------------------------------------------
if ( __name__ == '__main__' ):
    #for i in p.layer_info:
        #print i
    for j in p.hwc_info:
        #print j

        total_display_layer = 0
        hwc_layer = 0
        gpu_layer = 0
        hwc_expect_layer = 0
        dim_layer = 0
        gpu_exist = False
        for k in j.display:
            print k
            print '    type    |  handle  |  name      '
            print '------------+----------+------------'
            for l in k.list:
                if ( ( l.type.find( 'FB TARGET' ) < 0 ) ):
                    print l
                    
                    total_display_layer += 1
                    if ( ( l.type.find( 'HWC' ) >= 0 ) ):
                        hwc_layer += 1
                    else:
                        gpu_layer += 1

                    if ( gpu_exist == True):
                        continue
                    if ( total_display_layer > MAX_OVL_LAYER ):
                        if ( hwc_expect_layer == MAX_OVL_LAYER ):
                            hwc_expect_layer = MAX_OVL_LAYER - 1
                        gpu_exist = True
                        continue
                    
                    if ( OVL_SUPPORT_DIM == False ):
                        if ( ( l.name.find( 'dim' ) >= 0 ) ):
                            gpu_exist = True
                        else:
                            hwc_expect_layer += 1
                    else:
                        hwc_expect_layer += 1                                                                    

            print 'total display layer(%d), HWC layer(%d), HWC expect layer(%d)' % \
                  (total_display_layer, hwc_layer, hwc_expect_layer)

            compose_result = False

            print 'layer composition result : '
            if ( hwc_layer == hwc_expect_layer ):
                print 'PASS'
                compose_result = True
            else:
                print 'FAILURE'
                compose_result = False

            if ( compose_result == True ):
                compare_result = filecmp.cmp('./data/gpu_compose.bin', './data/hwc_compose.bin'\
                                                  , shallow = False)
                print 'golden comparision result : '
                if ( compare_result == True ):
                    print 'PASS'
                else:
                    print 'FAILURE'
                              
            
