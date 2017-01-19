import re

#---------------------------------------------------------------------------------------------------
HAL_PIXEL_FORMAT_YV12           = 0x32315659    # android yv12 format define
HAL_PIXEL_FORMAT_I420           = 0x00000100    # MTK I420
HAL_PIXEL_FORMAT_NV12_BLK       = 0x00000101    # MTK NV12 block progressive mode
HAL_PIXEL_FORMAT_NV12_BLK_FCM   = 0x00000102    # MTK NV12 block field mode
HAL_PIXEL_FORMAT_YUV_PRIVATE    = 0x00000103    # I420 or NV12_BLK or NV12_BLK_FCM


#---------------------------------------------------------------------------------------------------
class SFInfo:
    """SF info storage"""
    
class HWCListInfo:
    """HHWCList info storage"""
    def __init__(self, m):
        self.type = m.group( 'type' )
        self.handle = m.group( 'handle' )
        self.name = m.group( 'name' )

    def __str__(self):
        return ' %10s | %08s | %s' % \
               (self.type, self.handle, self.name)
    
class HWCDisplayInfo:
    """HHWCList info storage"""
    def __init__(self, m):
        self.list = []
        self.idx = m.group( 'idx' )
        self.width = m.group( 'width' )
        self.height = m.group( 'height' )

    def __str__(self):
        return 'Display[%s] : %sx%s' % \
               (self.idx, self.width, self.height)       
        
class HWCInfo:
    """HWC info storage"""
    
    def __init__(self, m):
        self.display = []
        self.version = m.group( 'version' )
        #self.version = -1
        
    def __str__(self):
        return 'Hardware Composer state(version %s)' % self.version
        
class DisplayDeviceInfo:
    """DisplayDevice info storage"""
        
class BufferInfo:
    """Buffer info storage"""

class LayerInfo:
    """layer info storage"""

    def __init__(self, m):
        self.name = m.group( 'name' )
        self.ptr = m.group( 'ptr' )        

        self.width = -1
        self.height = -1
        self.stride= -1
        self.fmt = -1

    def __str__(self):
        return '%s(ptr=%s): width=%d, height=%d, stride=%d, fmt=%d' % \
               (self.name, self.ptr, self.width, self.height, self.stride, self.fmt)

PARSE_STATE = [ 'layer', 'displays', 'hwc_state' ]
PARSE_SUB_STATE = [[ 'active_buffer' ],
                   [ 'display_device' ],
                   [ 'hwc_display', 'hwc_list' ]]
'''
clasee ParseHelper
    """layer info storage"""

    def __init__(self, state, regexp):
        self.parse_state = state    
        self.regexp = regexp
        self.parse_progress = 0

    def Parse(self, l):
        for i in range(self.parse_progress, len(self.parse_state))
            m = self.regexp[ self.parse_state[i] ].match( l )
            if ( m != None ) :
                self.parse_progress = i
'''                
        
class Parser:
    """parse the Android dumpsys SurfaceFlinger result into data struct"""

    def __init__( self, path = './SF_layerdump_all.log' ):
        """define regexp matching patterns, and parse first with given path"""

        self.regexp = {}
        self.regexp[ 'layer'   ] = '\+ Layer(Dim)* (?P<ptr>.*) \((?P<name>.*)\)'
        self.regexp[ 'active_buffer' ] = 'format.*activeBuffer=\[\s*(?P<width>\d+)x\s*(?P<height>\d+):\s*(?P<stride>\d+),\s*(?P<fmt>\d+)\]'

        self.regexp[ 'displays'   ] = 'not ready!'
        self.regexp[ 'display_device'   ] = 'not ready!'

        self.regexp[ 'hwc_state'   ] = 'Hardware Composer state \(version\s+(?P<version>\d+)\).*'
        self.regexp[ 'hwc_display'   ] = 'Display\[(?P<idx>\d+)\] :\s*(?P<width>\d+)x\s*(?P<height>\d+),.*'
        """accept Hardware Composer state (version  1000000->1030000)"""
        self.regexp[ 'hwc_list'   ] = '(?P<type>(HWC|GLES|FB TARGET)+)\s*\|\s*\|*\s*(?P<handle>\w+).*\s+(?P<name>\S+)'
        for k in self.regexp:
            self.regexp[ k ] = re.compile( self.regexp[ k ] )

        # setup info path, and releted file prefix for different dump type
        # there are different ways for dump package, and set to different file name ...
        self.path = path

        self.Parse()

    def Parse( self ):
        self.layer_info = []
        self.hwc_info = []
        layer_info = None
        hwc_info = None
        hwc_display_info = None
        hwc_list_info = None

        parse_progress = 0
        first_match = False

        f = open( self.path, 'r' )
        for l in f:
            l = l.strip()
            
            for i in range(parse_progress, len(PARSE_STATE)):
                
                m = self.regexp[ PARSE_STATE[i] ].match( l )
                if ( m != None ):
                    first_match = True
                    parse_progress = i
                    if ( PARSE_STATE[i] == 'layer'):
                        layer_info = LayerInfo(m)
                        self.layer_info.append( layer_info )
                        
                    elif ( PARSE_STATE[i] == 'displays'):
                        print 'displays not ready'
                        
                    elif ( PARSE_STATE[i] == 'hwc_state'):
                        hwc_info = HWCInfo(m)
                        self.hwc_info.append( hwc_info )
                        
                else:
                    if ( first_match != True):
                        continue
                    for j in range(0, len(PARSE_SUB_STATE[parse_progress])):
                        
                        m = self.regexp[ PARSE_SUB_STATE[parse_progress][j] ].match( l )
                        if ( m != None ):
                            if ( PARSE_SUB_STATE[parse_progress][j] == 'active_buffer'):
                                if ( layer_info != None):
                                    layer_info.width  = int( m.group( 'width'  ) )
                                    layer_info.height = int( m.group( 'height' ) )
                                    layer_info.stride = int( m.group( 'stride' ) )
                                    layer_info.fmt    = int( m.group( 'fmt'    ), 16 )
                                
                            elif ( PARSE_SUB_STATE[parse_progress][j] == 'display_device'):
                                print 'display_device not ready'

                            elif ( PARSE_SUB_STATE[parse_progress][j] == 'hwc_display'):
                                hwc_display_info = HWCDisplayInfo(m)
                                if ( hwc_info != None):
                                    hwc_info.display.append( hwc_display_info )                                
                                
                            elif ( PARSE_SUB_STATE[parse_progress][j] == 'hwc_list'):                            
                                hwc_list_info = HWCListInfo(m)
                                #print 'hwc_list_info type=%s handle=%s name=%s' % (m.group('type'), m.group('handle'), m.group('name'))
                                if ( hwc_display_info != None):
                                    hwc_display_info.list.append( hwc_list_info )


        f.close()


if ( __name__ == '__main__' ):
    dump_file = 'SF_auto_ovl.log'
    #dump_file = 'C:\\Program Files\\Android\\android-sdk\\platform-tools\\SF_bqdump\\SF_bqdump.log'
    #dump_file = 'C:\\Program Files\\Android\\android-sdk\\platform-tools\\SF_layerdump_all\\SF_layerdump_all.log'                
    p = Parser(dump_file)
    for i in p.layer_info:
        print i
    for j in p.hwc_info:
        print j
        for k in j.display:
            print k
            print '    type    |  handle  |  name      '
            print '------------+----------+------------'
            for l in k.list:
                print l

