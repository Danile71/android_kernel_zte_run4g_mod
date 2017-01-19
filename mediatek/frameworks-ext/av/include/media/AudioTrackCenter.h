#ifndef AUDIO_TRACK_CENTER_H_
#define AUDIO_TRACK_CENTER_H_

#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/List.h>
#include <media/stagefright/foundation/ABase.h>

//#define AUDIO_TRACK_CENTER_DEBUG

namespace android {

class AudioTrackCenter : public RefBase {
public:	
	AudioTrackCenter();
	
	virtual ~AudioTrackCenter();

        status_t addTrack(intptr_t trackId, uint32_t frameCount, uint32_t sampleRate, void* trackPtr,  uint32_t afFrameCount, uint32_t afSampleRate);

	status_t removeTrack(void* trackPtr);

	status_t updateTrackMaps(void* trackPtr, void* sinkPtr);

	status_t updateServer(intptr_t trackId, uint32_t server, bool restore=false);

	intptr_t getTrackId(void* trackPtr=NULL, void* sinkPtr=NULL);
	
	status_t getRealTimePosition(intptr_t trackId, int64_t *position);

    status_t setTrackActive(intptr_t trackId, bool active);
    
    status_t setTimeStretch(uint32_t timeScaled);

	status_t reset(intptr_t trackId);

private:
	status_t init();
#ifdef AUDIO_TRACK_CENTER_DEBUG	
	float countDeltaUs(int64_t realTimeUs);
#endif
	
    struct TrackInfo {
        uint32_t server;
        uint32_t frameCount;
		int64_t framePlayed;
		uint32_t afFrameCount;
        uint32_t sampleRate;
        uint32_t middleServer;
        bool active;
        int64_t  ts;
    };

	struct TrackMaps {
		intptr_t trackId;
		void*    trackPtr;
		void*    sinkPtr;
	};
	
	Mutex mLock;
	KeyedVector<intptr_t, TrackInfo > mTrackList;
	List<TrackMaps> mTrackMaps;
	
	size_t mAfFrameCount;
	uint32_t mAfSampleRate;
	uint32_t mTimeScaled;

#ifdef AUDIO_TRACK_CENTER_DEBUG
	int64_t mSysTimeUs;
	int64_t mRealTimeUs;
	float   mDeltaUs;
#endif	
	
    DISALLOW_EVIL_CONSTRUCTORS(AudioTrackCenter);

};

}

#endif  //AUDIO_TRACK_CENTER_H_
