
import sys, os, time

import operator

import subprocess

# aee controller: May  8 2014 15:36:27
# Show running exception
# Time                Worker PID    class
# 2014-01-01 06:19:10 6083   0      ManualDump
# 1970-01-01 00:00:00 -1     -1     UNDEF
# 1970-01-01 00:00:00 -1     -1     UNDEF
# 1970-01-01 00:00:00 -1     -1     UNDEF


def externel_cmd(ex_cmd, log = 0):
    
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


class log_db(object) :
    
    def __init__ (self):
        self.clean_db()
        self._aee_init_state = self._check_exception_db()
        self._aee_start_state = self._aee_init_state
        self._aee_diff_state = self._aee_init_state
        self._idx = 0
        
        
    def clean_db(self) : 
        print 'clean previous DB file...'
        externel_cmd('adb shell rm -rf /sdcard/mtklog/aee_exp/*')
    
    def start_sample(self, name=None) :
        
        if name != None :
            self._name = name
        else:
            self._name = 'unname_case'
        self._aee_start_state = self._check_exception_db()
        
    def utTestDB(self) :
        
        db_file = 'adb shell touch /sdcard/mtklog/aee_exp/db.%d.NE' % (self._idx)
        self._idx += 1
        externel_cmd('adb shell mkdir /sdcard/mtklog/aee_exp')
        externel_cmd(db_file)

    @staticmethod
    def isAeeRun() :
        
        #result = os.popen('adb shell aee -r').read()
        
        # t = subprocess.Popen('adb shell aee -r', shell=True, stdout=subprocess.PIPE).stdout
        # result = t.read()
        
        result = externel_cmd('adb shell aee -r')
        
        if result.count('aee controller') == 1 :
            if result.count('UNDEF') < 4 :
                return True
            else :
                return False        
        else : 
            return False
            
    
    def _check_exception_db(self):
        
        #result = os.popen('adb shell ls /sdcard/mtklog/aee_exp').read()

        #t = subprocess.Popen('adb shell ls /sdcard/mtklog/aee_exp', shell=True, stdout=subprocess.PIPE).stdout
        #result = t.read()
        
        result = externel_cmd('adb shell ls /sdcard/mtklog/aee_exp')        
        
        ke_cnt = result.count('KE') 
        ne_cnt = result.count('NE') 
        anr_cnt = result.count('ANR') 
        swt_cnt = result.count('SWT')
        je_cnt = result.count('JE')
        return (ke_cnt, ne_cnt, anr_cnt, swt_cnt, je_cnt) 
    
    
    #@staticmethod
    def isNewDB(self):
                
        found_new_db = False
        
        
        result = self._check_exception_db()
        
        wait_max = 60
        wait_unit = 10
        loop = wait_max/wait_unit
        i = 0
        
        if result == self._aee_start_state :
            while self.isAeeRun() == True :
                result2 = self._check_exception_db()
               
                if result == result2 :
                    print ("wait AEE dump, wait max {0} sec!!".format(wait_max))
                    time.sleep(10)
                else : 
                    result = result2
                    break

                if i >= loop :
                    break 
                i += 1
        
        #os.system('adb shell ls /sdcard/mtklog/aee_exp')
        
        #result = self._check_exception_db()
        
        #self._aee_end_state = result
        
        self._aee_diff_state = tuple(map(operator.sub, result, self._aee_start_state))
        
        (ke_cnt, ne_cnt, anr_cnt, swt_cnt, je_cnt) = result 
        
        # ke_cnt = result.count('KE') 
        # ne_cnt = result.count('NE') 
        # anr_cnt = result.count('ANR') 
        # swt_cnt = result.count('SWT')
        # je_cnt = result.count('JE')
        
        #print ''
        
        if self._aee_start_state != result :
            if ke_cnt > 0 or ne_cnt > 0 or anr_cnt > 0 or swt_cnt > 0 :
                found_new_db = True
        #        print("UnitTest {0}: Result => FAIL, found new create AEE_DB (KE:{1},NE:{2},ANR:{3},SWT{4})".format(self._name,ke_cnt, ne_cnt, anr_cnt, swt_cnt))
        #    elif je_cnt > 0:
        #        print("UnitTest {0}: Result => PASS, but found Java Exception:{0}!!".format(self._name, je_cnt))
        #    else:
        #        print("UnitTest {0}: Result => PASS, no Exception DB found!!".format(self._name))    
        #else :
        #    print ("UnitTest {0}:No New DB create, Result => PASS!!".format(self._name))
        
        #print '========================================================================'
        
        self._aee_start_state = result
        
        return found_new_db


    def aee_case_report(self) :
        
        diff = self._aee_diff_state 
        
        (ke_cnt, ne_cnt, anr_cnt, swt_cnt, je_cnt) = diff


        #name = self._name
        
        #print 'UnitTest %s: Result =>' % (name),
        #if diff != (0,0,0,0,0) :
        
        
        # if ke_cnt > 0 or ne_cnt > 0 or anr_cnt > 0 or swt_cnt > 0 :
        #     print 'FAIL, ', 
        # else :
        #     print 'PASS, ',
        
        print 'New DB (KE:%d,NE:%d,ANR:%d,SWT:%d)' % (ke_cnt, ne_cnt, anr_cnt, swt_cnt),
        
        
        #     elif je_cnt > 0:
        #         print 'PASS, but found Java Exception:%d!!' % (je_cnt)
        #     else:
        #         print 'PASS, no Exception DB found!!'
        # else :
        #     print 'PASS, No New DB create!!'
        # 
        # print '========================================================================'           
        
    
    def aee_report(self) :

        
        final = self._check_exception_db()
        diff = tuple(map(operator.sub, final, self._aee_init_state))
        
        (ke_cnt, ne_cnt, anr_cnt, swt_cnt, je_cnt) = diff


        #if total == False :
        #    name = self._name
        #else : 
        name = 'TOTAL'
        
        print 'Check DB :',
        if diff != (0,0,0,0,0) :
            if ke_cnt > 0 or ne_cnt > 0 or anr_cnt > 0 or swt_cnt > 0 :
                print 'FAIL, found new create AEE_DB (KE:%d,NE:%d,ANR:%d,SWT:%d)' % (ke_cnt, ne_cnt, anr_cnt, swt_cnt)
            elif je_cnt > 0:
                print 'PASS, but found Java Exception:%d!!' % (je_cnt)
            else:
                print 'PASS, no Exception DB found!!'
        else :
            print 'PASS, No New DB create!!'
             
        
    
    
    

if __name__ == '__main__' :
    
    
    print log_db.isAeeRun()
    
    aee = log_db()
    
    aee.start_sample('test')
    
    print aee.isNewDB()
    


