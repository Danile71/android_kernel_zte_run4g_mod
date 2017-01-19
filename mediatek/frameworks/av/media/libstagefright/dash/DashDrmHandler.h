#ifndef _DASH_DRM_H_
#define _DASH_DRM_H_

#include <media/stagefright/foundation/ABuffer.h>
#include <drm/DrmManagerClient.h>
#include <utils/RefBase.h>

namespace android {

struct ABuffer;
struct String8;
struct DecryptHandle;
struct MetaData;
class DrmManagerClient ;

struct DashDrmHandler : public RefBase {
   enum{
   	KeyDRM_InvalidMin =0,
	KeyDRM_PR=1,
	KeyDRM_WV,
	KeyDRM_ML_BB,
	KeyDRM_ML_MS3,
	KeyDRM_InvalidMax,
   };
   enum{
   	keyTrack_Video = 1,
	keyTrack_Audio = 2,	
	keyTrack_SubTitle    =4,	
   };

//if use the DrmManagerClient, contruct Drm Handler using this contrut function
//if use XXX drm decrypt type of method, (not DrmManagerClient ), will add another DashDrmHandler contruter
	
	DashDrmHandler(
		int32_t drmType ,
		sp<DecryptHandle> handle  ,
		DrmManagerClient *client );

	status_t setPSSH( sp<MetaData> &DrmMetaPssh);        

	status_t setTenc(int32_t trackId, sp<MetaData> &DrmMetaTenc);        

	status_t decrypt(
		int32_t trackId,
		const sp<ABuffer> &encBuffer,
		const sp<ABuffer> & decBuffer, //if "handle", will be svp
		const sp<ABuffer> & IV = NULL);	

	
protected:
    virtual ~DashDrmHandler();
private:
	int32_t  mDrmType;
	uint32_t mHandleTrack;
	sp<MetaData> mPSSH;
	KeyedVector<int32_t,sp<MetaData> >mTencs;
	KeyedVector<int32_t,bool >  mDecryptEnv;
	
	sp<DecryptHandle> mDecryptHandle;
	DrmManagerClient* mDrmManagerClient;
	


    DISALLOW_EVIL_CONSTRUCTORS(DashDrmHandler);
};

}  // namespace android

#endif  // _DASH_DRM_H_
