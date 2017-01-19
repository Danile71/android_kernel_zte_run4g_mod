#define LOG_TAG "MJC"

#include <cutils/xlog.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

int drvID = -1;

int main(int argc, char** argv) 
{
    int actionID=0, RegBase = 0, RegValue = 0, err = 0;
    char fileName[256];

    XLOGD("MJC test...");
    //add MJC test code here
    XLOGD("MJC init end !");
    return 0;
}
