#define LOG_TAG "[Dash]DrmHandler"
#include <utils/Log.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/Utils.h>
#include <media/stagefright/MetaData.h>
#include <utils/KeyedVector.h>
#include "DashDrmHandler.h"

#include <drm/drm_framework_common.h>
#include <cutils/xlog.h>
namespace android {


DashDrmHandler::DashDrmHandler(
	int32_t drmType,
	sp<DecryptHandle> handle,
	DrmManagerClient *client)
	:mDrmType(drmType),
	mHandleTrack(0),
	mDecryptHandle(handle),
	mDrmManagerClient(client){

	CHECK(mDecryptHandle != NULL && mDrmManagerClient !=NULL );
    ALOGI("+DashDrmHandler drmType %d,mDecryptHandle=%p",drmType,mDecryptHandle.get());

}
    
//16byte UUID+4 byte size+ size data
status_t DashDrmHandler::setPSSH( sp<MetaData> &DrmMetaPssh){
	mPSSH = DrmMetaPssh;
	ALOGI("setPSSH");
	return OK;
	

}     
/*meta->setInt32(kKeyCryptoMode, tencInfo->mIsEncrypted);
meta->setInt32(kKeyCryptoDefaultIVSize, Tenc->mIV_Size);    
meta->setData(kKeyCryptoKey, 'tenc',Tenc->mKID,sizeof(Tenc->mKID));//16 byte 
class DrmBuffer {
public:
    char* data;
    int length;

    DrmBuffer() :
        data(NULL),
        length(0) {
    }

    DrmBuffer(char* dataBytes, int dataLength) :
        data(dataBytes),
        length(dataLength) {
    }

};
*/
status_t DashDrmHandler::setTenc(int32_t trackId, sp<MetaData> &DrmMetaTenc){
	CHECK(trackId == keyTrack_Video  || trackId == keyTrack_Audio || trackId == keyTrack_SubTitle);
	ALOGI("setTenc trackId =%d ",trackId );
    mHandleTrack |= trackId;

	mTencs.add(trackId,DrmMetaTenc);
    DrmBuffer trackEnc;

    uint32_t type;
    const void *data;
    size_t size;
    CHECK(DrmMetaTenc->findData(kKeyCryptoKey, &type, &data, &size));
    trackEnc.data = (char*)data;
    trackEnc.length = size;

    status_t err = mDrmManagerClient->initializeDecryptUnit(
		mDecryptHandle, trackId, &trackEnc);
    ALOGI("setTenc trackId =%d initializeDecryptUnit err %d",trackId,err);
	return err;

}     

status_t DashDrmHandler::decrypt(
	int32_t trackId,
	const sp<ABuffer> &encBuffer,
	const sp<ABuffer> & decBuffer, //if "handle", will be svp
	const sp<ABuffer> & IV ){

	ALOGI(" decrypt");
    status_t err = OK;

    DrmBuffer* enc = new DrmBuffer((char*)(encBuffer->data()),encBuffer->size());
    DrmBuffer* dec = new DrmBuffer((char*)(decBuffer->data()),decBuffer->size());
    DrmBuffer* iv  = new DrmBuffer((char*)(IV->data()),IV->size());

    if ((err = mDrmManagerClient->decrypt(mDecryptHandle, trackId,
            enc, &dec, iv)) != NO_ERROR) {
        ALOGE("decrypt err %d",err);
    }
    free(enc);
    free(dec);
    free(iv);
	return err;
}

DashDrmHandler ::~DashDrmHandler(){
	ALOGI("~DashDrmHandler mHandleTrack 0x%x",mHandleTrack);
    if((mHandleTrack & keyTrack_Video) == keyTrack_Video){
        mDrmManagerClient->finalizeDecryptUnit(mDecryptHandle, keyTrack_Video);
    }else if((mHandleTrack & keyTrack_Audio) == keyTrack_Audio){
        mDrmManagerClient->finalizeDecryptUnit(mDecryptHandle, keyTrack_Audio);
    }else if((mHandleTrack & keyTrack_SubTitle) == keyTrack_SubTitle){
        mDrmManagerClient->finalizeDecryptUnit(mDecryptHandle, keyTrack_SubTitle);
    }
}

}



