#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_CVSD_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_CVSD_H

#include "AudioALSAPlaybackHandlerBase.h"

namespace android
{

class WCNChipController;
class AudioBTCVSDControl;

class AudioALSAPlaybackHandlerBTCVSD : public AudioALSAPlaybackHandlerBase
{
    public:
        AudioALSAPlaybackHandlerBTCVSD(const stream_attribute_t *stream_attribute_source);
        virtual ~AudioALSAPlaybackHandlerBTCVSD();


        /**
         * open/close audio hardware
         */
        virtual status_t open();
        virtual status_t close();
        virtual status_t routing(const audio_devices_t output_devices);


        /**
         * write data to audio hardware
         */
        virtual ssize_t  write(const void *buffer, size_t bytes);



    private:
        WCNChipController *mWCNChipController;
        AudioBTCVSDControl *mAudioBTCVSDControl;

        int mFd2;
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_BT_CVSD_H
