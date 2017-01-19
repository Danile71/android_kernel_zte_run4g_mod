#ifndef AUDIO_MTK_DCREMOVE_PROCESS_H
#define AUDIO_MTK_DCREMOVE_PROCESS_H
#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <utils/threads.h>
#ifndef uint32_t
typedef unsigned int        uint32;
#endif

extern "C" {
#include <dc_removal_flt.h>
}
namespace android
{

class DcRemove
{
    public:
        enum {
            DCR_MODE_1 = 0,
            DCR_MODE_2,
            DCR_MODE_3
        };
        DcRemove();
        ~DcRemove();
        status_t init(uint32_t channel, uint32_t samplerate, uint32_t drcMode);
        size_t process(const void *inbuffer, size_t bytes, void *outbuffer);
        status_t close();
    private:
        DCRemove_Handle *mHandle;
        signed char *p_internal_buf;
        DcRemove(const DcRemove &);
        DcRemove &operator=(const DcRemove &);
        mutable Mutex  mLock;
        size_t mSamplerate;
};
}
#endif
