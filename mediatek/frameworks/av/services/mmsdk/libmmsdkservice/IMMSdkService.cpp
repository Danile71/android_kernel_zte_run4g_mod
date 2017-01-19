/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#define LOG_TAG "mmsdk/BpMMSdkService"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <mmsdk/IImageTransformUser.h>
#include <mmsdk/IEffectUser.h>
#include <mmsdk/IGestureUser.h>
//
#include <mmsdk/IMMSdkService.h>

#include <mtkcam/device/Cam1Device.h>


using namespace android; 
using namespace NSMMSdk; 

namespace android {
namespace {

enum {
    EX_SECURITY = -1,
    EX_BAD_PARCELABLE = -2,
    EX_ILLEGAL_ARGUMENT = -3,
    EX_NULL_POINTER = -4,
    EX_ILLEGAL_STATE = -5,
    EX_HAS_REPLY_HEADER = -128,  // special; see below
};

static bool readExceptionCode(Parcel& reply) {
    int32_t exceptionCode = reply.readExceptionCode();

    if (exceptionCode != 0) {
        const char* errorMsg;
        switch(exceptionCode) {
            case EX_SECURITY:
                errorMsg = "Security";
                break;
            case EX_BAD_PARCELABLE:
                errorMsg = "BadParcelable";
                break;
            case EX_NULL_POINTER:
                errorMsg = "NullPointer";
                break;
            case EX_ILLEGAL_STATE:
                errorMsg = "IllegalState";
                break;
            // Binder should be handling this code inside Parcel::readException
            // but lets have a to-string here anyway just in case.
            case EX_HAS_REPLY_HEADER:
                errorMsg = "HasReplyHeader";
                break;
            default:
                errorMsg = "Unknown";
        }

        ALOGE("Binder transmission error %s (%d)", errorMsg, exceptionCode);
        return true;
    }

    return false;
}

};

class BpMMSdkService: public BpInterface<IMMSdkService>
{
public:
    BpMMSdkService(const sp<IBinder>& impl)
        : BpInterface<IMMSdkService>(impl)
    {
    }

    // 
    virtual status_t connectImageTransformUser(sp<NSImageTransform::IImageTransformUser>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        remote()->transact(BnMMSdkService::CONNECT_IMAGETRANSFORMUSER, data, &reply);
        
        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32();
        if (reply.readInt32() != 0)
        {
            client = interface_cast<NSImageTransform::IImageTransformUser>(reply.readStrongBinder()); 
        }
        return status;
    }

    // 
    virtual status_t connectEffect(String8 const &clientName, sp<NSEffect::IEffectUser>& client )
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        data.writeString8(clientName); 
        remote()->transact(BnMMSdkService::CONNECT_EFFECT, data, &reply);

        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32();
        if (reply.readInt32() != 0)
        {
            client = interface_cast<NSEffect::IEffectUser>(reply.readStrongBinder()); 
        }
        return status;
    }

    // 
    virtual status_t connectGesture(sp<NSGesture::IGestureUser>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        remote()->transact(BnMMSdkService::CONNECT_GESTURE, data, &reply);

        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32(); 
        if (reply.readInt32() != 0)
        {
            client = interface_cast<NSGesture::IGestureUser>(reply.readStrongBinder());             
        }
 
        return status;
    }
    //
    virtual status_t registerCamera1Device(NSCam::ICamDevice *device)
    {
        ALOGD("Bp:registerCamera1Device");
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        data.writeInt32(reinterpret_cast<int>(device)); 
        remote()->transact(BnMMSdkService::REGISTER_CAMERA_1_DEVICE, data, &reply);
        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32();         
        return status; 
    }
    //
    virtual status_t unRegisterCamera1Device(NSCam::ICamDevice *device)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMMSdkService::getInterfaceDescriptor());
        data.writeInt32(reinterpret_cast<int>(device)); 
        remote()->transact(BnMMSdkService::UNREGISTER_CAMERA_1_DEVICE, data, &reply);
        if (readExceptionCode(reply)) return -EPROTO;
        status_t status = reply.readInt32(); 
        return status; 
    }

};

IMPLEMENT_META_INTERFACE(MMSdkService, "android.media.IMMSdkService");

// ----------------------------------------------------------------------

status_t BnMMSdkService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case CONNECT_IMAGETRANSFORMUSER: {
            CHECK_INTERFACE(IMMSdkService, data, reply);
            
            sp<NSImageTransform::IImageTransformUser> client; 
            status_t status = connectImageTransformUser(client);
            reply->writeNoException();
            reply->writeInt32(status);
            if (client != 0)
            {                
                reply->writeInt32(1); 
                reply->writeStrongBinder(client->asBinder()); 
            }
            else 
            {
                reply->writeInt32(0);
            }
            return NO_ERROR;
        } break;
        case CONNECT_EFFECT: {
            CHECK_INTERFACE(IMMSdkService, data, reply);
            String8 clientName(data.readString8());
            sp<NSEffect::IEffectUser> client;             
            status_t status = connectEffect(clientName, client);             
            reply->writeNoException();
            reply->writeInt32(status);
            if (client != 0)
            {
                reply->writeInt32(1);
                reply->writeStrongBinder(client->asBinder()); 
            }
            else 
            {
                reply->writeInt32(0); 
            }
            return NO_ERROR;
        } break;
        case CONNECT_GESTURE: {
            CHECK_INTERFACE(IMMSdkService, data, reply);
            sp<NSGesture::IGestureUser> client; 
            status_t status = connectGesture(client); 
            reply->writeNoException(); 
            reply->writeInt32(status);
            if (client != 0)
            {
                reply->writeInt32(1); 
                reply->writeStrongBinder(client->asBinder()); 
            }
            else 
            {
                reply->writeInt32(0); 
            }            
            return NO_ERROR;
        } break;
        case REGISTER_CAMERA_1_DEVICE: {
            int callingPid =  android::IPCThreadState::self()->getCallingPid();
            if (callingPid != getpid())
            {
                status_t status = NO_ERROR; 
                ALOGW("Reject, (calling, current) = (%d, %d) only trust the device on the same process", 
                	       callingPid, getpid()); 
                reply->writeNoException(); 
                reply->writeInt32(status);

                return NO_ERROR; 
            }
            CHECK_INTERFACE(IMMSdkService, data, reply);
            NSCam::ICamDevice *pDevice = reinterpret_cast<NSCam::ICamDevice*>(data.readInt32()); 
            status_t status = registerCamera1Device(pDevice); 
            reply->writeNoException(); 
            reply->writeInt32(status);
            return status; 
        } break;            
        case UNREGISTER_CAMERA_1_DEVICE: {
            int callingPid =  android::IPCThreadState::self()->getCallingPid();
            if (callingPid != getpid())
            {
                status_t status = NO_ERROR;             
                ALOGW("Reject, (calling, current) = (%d, %d) only trust the device on the same process", 
                	       callingPid, getpid()); 
                reply->writeNoException(); 
                reply->writeInt32(status);
                
                return NO_ERROR; 
            }
            //
            CHECK_INTERFACE(IMMSdkService, data, reply);
            NSCam::ICamDevice *pDevice = reinterpret_cast<NSCam::ICamDevice*>(data.readInt32()); 

            status_t status = unRegisterCamera1Device(pDevice); 

            reply->writeNoException(); 
            reply->writeInt32(status);
            return NO_ERROR;         	 
        }break; 
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
