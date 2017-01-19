#include <utils/RefBase.h>
#include <ui/mediatek/RefBaseDumpTunnel.h>

namespace android
{

class RefBaseTest: public RefBase{
    public:
        RefBaseTest(){
            RefBaseMonitor::getInstance().monitor(this);
        }
        ~RefBaseTest(){
            RefBaseMonitor::getInstance().unmonitor(this);
        }
};

};
