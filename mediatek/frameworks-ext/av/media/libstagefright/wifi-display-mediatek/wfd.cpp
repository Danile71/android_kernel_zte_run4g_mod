/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "wfd"
#include <utils/Log.h>

#ifdef MTK_WFD_SINK_SUPPORT
#include "sink/WifiDisplaySink.h"
#endif
#include "source/WifiDisplaySource.h"


#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <media/AudioSystem.h>
#include <media/IMediaPlayerService.h>
#include <media/IRemoteDisplay.h>
#include <media/IRemoteDisplayClient.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ParsedMessage.h>

#include <ui/DisplayInfo.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "wfdUibcTest.h"
namespace android {

static void setStreamMode(const char *buffer){
/*    
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.player"));

    sp<IMediaPlayerService> service =
        interface_cast<IMediaPlayerService>(binder);

    CHECK(service.get() != NULL);
    CHECK(buffer != NULL);

    if(strlen(buffer) != 1){
        ALOGE("Bad arguments:%s", buffer);
        return;
    }
    
    int mode = buffer[0] - '0';    
    service->setBitrateControl(mode);
*/    
}

static void enableDisableRemoteDisplay(const char *iface) {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.player"));

    sp<IMediaPlayerService> service =
        interface_cast<IMediaPlayerService>(binder);

    CHECK(service.get() != NULL);

    service->enableRemoteDisplay(iface, WifiDisplaySource::kTestModeFlag);
}

static void enableFastRemoteDisplay(const char *iface) {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.player"));

    sp<IMediaPlayerService> service =
        interface_cast<IMediaPlayerService>(binder);

    CHECK(service.get() != NULL);

    service->enableRemoteDisplay(iface, WifiDisplaySource::kFastSetupFlag | WifiDisplaySource::kTestModeFlag);
}

static void enableFastRtpRemoteDisplay(const char *iface) {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.player"));

    sp<IMediaPlayerService> service =
        interface_cast<IMediaPlayerService>(binder);

    CHECK(service.get() != NULL);

    service->enableRemoteDisplay(iface, WifiDisplaySource::kFastRtpFlag | WifiDisplaySource::kTestModeFlag);
}

#ifdef MTK_WFD_SINK_SUPPORT
static void enableDisableMiracastSinkTest(const char *iface) {    
    ALOGE("enableDisableMiracastSinkTest(), iface:%s", iface);
    
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.player"));

    sp<IMediaPlayerService> service =
        interface_cast<IMediaPlayerService>(binder);

    CHECK(service.get() != NULL);

    service->enableRemoteDisplay(iface, WifiDisplaySink::FLAG_SIGMA_TEST_MODE);
}
#endif

int connectUibc(const char* remoteHost, int32_t port)
{
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
 
    if (-1 == SocketFD)
    {
      ALOGE("cannot create socket");
      exit(1);
    }
 
    memset(&stSockAddr, 0, sizeof(stSockAddr));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(WFD_UIBC_SERVER_PORT);
    Res = inet_pton(AF_INET, remoteHost, &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      ALOGE("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(1);
    }
    else if (0 == Res)
    {
      ALOGE("char string (second parameter does not contain valid ipaddress)");
      close(SocketFD);
      exit(1);
    }
 
    if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      ALOGE("connect failed:%d", errno);
      close(SocketFD);
      exit(1);
 
    }
 
    /* perform read write operations ... */    
    int i=PEER_START_INDEX;
    while(i<=PEER_END_INDEX){
        ALOGE("send UIBC message %d",i);        
            if(write(SocketFD,  peerList[i] , peerListLength[i]) == -1){
            ALOGE("wangfj write failed:%d", errno);
            close(SocketFD);
            exit(1);        
        }
        ALOGE("send UIBC message %d done ",i);
        sleep(1);
        i++;
    }
    shutdown(SocketFD, SHUT_RDWR);
 
    close(SocketFD);
    return EXIT_SUCCESS;
}

static void testUibcCmd(const char *testCmd){
    ALOGI("testUibcCmd:%s", testCmd);
    //connectUibc();
}

static void usage(const char *me) {   
    fprintf(stderr,
            "usage:\n"
            "           %s -l iface[:port]\tcreate a wifi display source\n"
            "               -f(ilename)  \tstream media\n"
            "           -e ip[:port]       \tenable remote display as source\n"
            "           -d            \tdisable remote display as source\n"
            "           -x ip[:port]       \tenable remote display as sink\n"
            "           -y            \tdisable remote display as sink\n"
            "           -f            \tenable fast remote display\n"
            "           -b host[:port]          \tsend UIBC test command\n",
            me);
}

struct RemoteDisplayClient : public BnRemoteDisplayClient {
    RemoteDisplayClient();

    virtual void onDisplayConnected(
            const sp<IGraphicBufferProducer> &bufferProducer,
            uint32_t width,
            uint32_t height,
            uint32_t flags,
            uint32_t session);
    
    virtual void onDisplayDisconnected();
    virtual void onDisplayError(int32_t error);

    virtual void onDisplayKeyEvent(uint32_t, uint32_t);
    virtual void onDisplayGenericMsgEvent(uint32_t);

    void waitUntilDone();

protected:
    virtual ~RemoteDisplayClient();

private:
    Mutex mLock;
    Condition mCondition;

    bool mDone;

    sp<SurfaceComposerClient> mComposerClient;
    sp<IGraphicBufferProducer> mSurfaceTexture;
    sp<IBinder> mDisplayBinder;

    DISALLOW_EVIL_CONSTRUCTORS(RemoteDisplayClient);
};

RemoteDisplayClient::RemoteDisplayClient()
    : mDone(false) {
    mComposerClient = new SurfaceComposerClient;
    CHECK_EQ(mComposerClient->initCheck(), (status_t)OK);
}

RemoteDisplayClient::~RemoteDisplayClient() {
}

void RemoteDisplayClient::onDisplayConnected(
        const sp<IGraphicBufferProducer> &bufferProducer,
        uint32_t width,
        uint32_t height,
        uint32_t flags,
        uint32_t session) {
    ALOGI("onDisplayConnected width=%u, height=%u, flags = 0x%08x, session=%d",
          width, height, flags, session);

    if (bufferProducer != NULL) {
        mSurfaceTexture = bufferProducer;
        mDisplayBinder = mComposerClient->createDisplay(
                String8("foo"), false /* secure */);

        SurfaceComposerClient::openGlobalTransaction();
        mComposerClient->setDisplaySurface(mDisplayBinder, mSurfaceTexture);

        Rect layerStackRect(1280, 720);  // XXX fix this.
        Rect displayRect(1280, 720);

        mComposerClient->setDisplayProjection(
                mDisplayBinder, 0 /* 0 degree rotation */,
                layerStackRect,
                displayRect);

#ifdef MTK_WFD_SINK_SUPPORT
        SurfaceComposerClient::setDisplayLayerStack(mDisplayBinder, 0);    // default stack
#endif
        SurfaceComposerClient::closeGlobalTransaction();
    }
}

void RemoteDisplayClient::onDisplayDisconnected() {
    ALOGI("onDisplayDisconnected");

    Mutex::Autolock autoLock(mLock);
    mDone = true;
    mCondition.broadcast();
}

void RemoteDisplayClient::onDisplayError(int32_t error) {
    ALOGI("onDisplayError error=%d", error);

    Mutex::Autolock autoLock(mLock);
    mDone = true;
    mCondition.broadcast();
}

void RemoteDisplayClient::waitUntilDone() {
    Mutex::Autolock autoLock(mLock);
    while (!mDone) {
        mCondition.wait(mLock);
    }
}

void RemoteDisplayClient::onDisplayKeyEvent(uint32_t, uint32_t) {
    ALOGI("onDisplayKeyEvent");
}

void RemoteDisplayClient::onDisplayGenericMsgEvent(uint32_t) {
    ALOGI("onDisplayGenericMsgEvent");
}

static status_t enableAudioSubmix(bool enable) {
    status_t err = AudioSystem::setDeviceConnectionState(
            AUDIO_DEVICE_IN_REMOTE_SUBMIX,
            enable
                ? AUDIO_POLICY_DEVICE_STATE_AVAILABLE
                : AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE,
            NULL /* device_address */);

    if (err != OK) {
        return err;
    }

    err = AudioSystem::setDeviceConnectionState(
            AUDIO_DEVICE_OUT_REMOTE_SUBMIX,
            enable
                ? AUDIO_POLICY_DEVICE_STATE_AVAILABLE
                : AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE,
            NULL /* device_address */);

    return err;
}

static void createSource(const AString &addr, int32_t port) {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.player"));
    sp<IMediaPlayerService> service =
        interface_cast<IMediaPlayerService>(binder);

    CHECK(service.get() != NULL);

    enableAudioSubmix(true /* enable */);

    String8 iface;
    iface.append(addr.c_str());
    iface.append(StringPrintf(":%d", port).c_str());

    sp<RemoteDisplayClient> client = new RemoteDisplayClient;
    sp<IRemoteDisplay> display = service->listenForRemoteDisplay(client, iface);

    client->waitUntilDone();

    display->dispose();
    display.clear();

    enableAudioSubmix(false /* enable */);
}

static void createFileSource(
        const AString &addr, int32_t port, const char *path) {
    sp<ANetworkSession> session = new ANetworkSession;
    session->start();

    sp<ALooper> looper = new ALooper;
    looper->start();

    sp<RemoteDisplayClient> client = new RemoteDisplayClient;
    sp<WifiDisplaySource> source = new WifiDisplaySource(session, client, path);
    looper->registerHandler(source);

    AString iface = StringPrintf("%s:%d", addr.c_str(), port);
    CHECK_EQ((status_t)OK, source->start(iface.c_str()));

    client->waitUntilDone();

    source->stop();
}

}  // namespace android

int main(int argc, char **argv) {
    using namespace android;

    ProcessState::self()->startThreadPool();

    DataSource::RegisterDefaultSniffers();

    AString connectToHost;
    int32_t connectToPort = -1;
    AString uri;

    AString listenOnAddr;
    int32_t listenOnPort = -1;

    AString path;
///M: @{
    int     isFastSetup = 0;
///@}
    bool specialMode = false;

    int res;
    while ((res = getopt(argc, argv, "hpic:b:l:t:f:r:u:e:s:d:x:y")) >= 0) {
        switch (res) {
            case 'b':
            {
                const char *colonPos = strrchr(optarg, ':');

                if (colonPos == NULL) {
                    connectToHost = optarg;
                    connectToPort = WFD_UIBC_SERVER_PORT;
                } else {
                    connectToHost.setTo(optarg, colonPos - optarg);

                    char *end;
                    connectToPort = strtol(colonPos + 1, &end, 10);

                    if (*end != '\0' || end == colonPos + 1
                            || connectToPort < 1 || connectToPort > 65535) {
                        fprintf(stderr, "Illegal port specified.\n");
                        exit(1);
                    }
                }
                connectUibc(connectToHost.c_str(), connectToPort);
                exit(1);
                break;
            }
#ifdef MTK_WFD_SINK_SUPPORT
            case 'c':
            {
                const char *colonPos = strrchr(optarg, ':');

                if (colonPos == NULL) {
                    connectToHost = optarg;
                    connectToPort = WifiDisplaySource::kWifiDisplayDefaultPort;
                } else {
                    connectToHost.setTo(optarg, colonPos - optarg);

                    char *end;
                    connectToPort = strtol(colonPos + 1, &end, 10);

                    if (*end != '\0' || end == colonPos + 1
                            || connectToPort < 1 || connectToPort > 65535) {
                        fprintf(stderr, "Illegal port specified.\n");
                        exit(1);
                    }
                }
                break;
            }
#endif
            case 't':
            {
                isFastSetup = 1;
            }

            case 'f':
            {
                path = optarg;
                break;
            }            

            case 'u':
            {
                uri = optarg;
                break;
            }

            case 'l':
            {
                const char *colonPos = strrchr(optarg, ':');

                if (colonPos == NULL) {
                    listenOnAddr = optarg;
                    listenOnPort = WifiDisplaySource::kWifiDisplayDefaultPort;
                } else {
                    listenOnAddr.setTo(optarg, colonPos - optarg);

                    char *end;
                    listenOnPort = strtol(colonPos + 1, &end, 10);

                    if (*end != '\0' || end == colonPos + 1
                            || listenOnPort < 1 || listenOnPort > 65535) {
                        fprintf(stderr, "Illegal port specified.\n");
                        exit(1);
                    }
                }
                break;
            }
            

            case 'r':
            {
                enableFastRtpRemoteDisplay(optarg);
                exit(0);
                break;
            }

            // Miracast source test
            case 'e':
            {
                enableDisableRemoteDisplay(optarg);
                exit(0);
                break;
            }

            case 'd':
            {
                enableDisableRemoteDisplay(NULL);
                exit(0);
                break;
            }
        #ifdef MTK_WFD_SINK_SUPPORT
            // Miracast sink test
            case 'x':
            {
                enableDisableMiracastSinkTest(optarg);
                exit(0);
                break;
            }

            case 'y':
            {
                enableDisableMiracastSinkTest(NULL);
                exit(0);
                break;
            }
        #endif    

            case 's':
            {
                //setStreamMode(optarg);
                exit(0);
                break;
            }
    

            case '?':
            case 'h':
            default:
                testUibcCmd(argv[1]);
                usage(argv[0]);
                exit(1);
        }
    }

    if (listenOnPort >= 0) {
        if (path.empty()) {
            createSource(listenOnAddr, listenOnPort);
        } else {
            createFileSource(listenOnAddr, listenOnPort, path.c_str());
        }

        exit(0);
    }

#ifdef MTK_WFD_SINK_SUPPORT
    if (connectToPort < 0 && uri.empty()) {
        fprintf(stderr,
                "You need to select either source host or uri.\n");

        exit(1);
    }

    if (connectToPort >= 0 && !uri.empty()) {
        fprintf(stderr,
                "You need to either connect to a wfd host or an rtsp url, "
                "not both.\n");
        exit(1);
    }

    sp<SurfaceComposerClient> composerClient = new SurfaceComposerClient;
    CHECK_EQ(composerClient->initCheck(), (status_t)OK);

    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);

    ssize_t displayWidth = info.w;
    ssize_t displayHeight = info.h;

    ALOGD("display is %d x %d\n", displayWidth, displayHeight);

    sp<SurfaceControl> control =
        composerClient->createSurface(
                String8("A Surface"),
                displayWidth,
                displayHeight,
                PIXEL_FORMAT_RGB_565,
                0);

    CHECK(control != NULL);
    CHECK(control->isValid());

    SurfaceComposerClient::openGlobalTransaction();
    CHECK_EQ(control->setLayer(INT_MAX), (status_t)OK);
    CHECK_EQ(control->show(), (status_t)OK);
    SurfaceComposerClient::closeGlobalTransaction();

    sp<Surface> surface = control->getSurface();
    CHECK(surface != NULL);

    sp<ANetworkSession> session = new ANetworkSession;
    session->start();

    sp<ALooper> looper = new ALooper;

    sp<WifiDisplaySink> sink = new WifiDisplaySink(
            specialMode ? WifiDisplaySink::FLAG_SPECIAL_MODE : 0 /* flags */,
            session,
            surface->getIGraphicBufferProducer());

    looper->registerHandler(sink);

    if (connectToPort >= 0) {
        sink->start(connectToHost.c_str(), connectToPort);
    } else {
        sink->start(uri.c_str());
    }

    looper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
    while(1) sleep(10);

    composerClient->dispose();
#else
    usage(argv[0]);
#endif

    return 0;
}
