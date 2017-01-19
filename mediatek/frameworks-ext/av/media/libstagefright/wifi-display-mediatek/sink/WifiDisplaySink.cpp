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
#define LOG_TAG "WifiDisplaySink"
#include <utils/Log.h>

#include "WifiDisplaySink.h"
///M: Add for parse M4
#include "Parameters.h"

#include "DirectRenderer.h"
#include "MediaReceiver.h"
#include "TimeSyncer.h"

#include <cutils/properties.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ParsedMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>

///M: Add For Notify
#include <media/IRemoteDisplayClient.h>
#include "uibc/UibcMessage.h"
#include "DataPathTrace.h"

///Trace debug
#ifdef MTB_SUPPORT
#define ATRACE_TAG ATRACE_TAG_MTK_WFD
#include <utils/Trace.h>
#endif

namespace android {

// static
const AString WifiDisplaySink::sUserAgent = MakeUserAgent();

///M: Add @{
WifiDisplaySink::WifiDisplaySink(
        uint32_t flags,
        const sp<ANetworkSession> &netSession,
        const sp<IRemoteDisplayClient> &client,
        const sp<IGraphicBufferProducer> &bufferProducer,       
        const sp<AMessage> &notify)
    : mState(UNDEFINED),
      mFlags(flags),
      mNetSession(netSession),
      mSurfaceTex(bufferProducer),
      mNotify(notify),
      mUsingTCPTransport(false),
      mUsingTCPInterleaving(false),
      mSessionID(0),
      mNextCSeq(1),
      mIDRFrameRequestPending(false),
      mTimeOffsetUs(0ll),
      mTimeOffsetValid(false),
      mSetupDeferred(false),
      mLatencyCount(0),
      mLatencySumUs(0ll),
      mLatencyMaxUs(0ll),
	  mFirstRTPData(true),
	  mClient(client),
	  mPause(false),
	  mRTPInit(false),
	  mUsingHDCP(false),
	  mComposerClient(NULL),
	  mNextPrintTimeUs(-1),
#ifdef MTK_WFD_SINK_UIBC_SUPPORT	  
	  mUibcClientHandler(NULL),
      mUibcSinkEnabled(false),
#endif
#if 0
         mPromoted(false),
#endif
      mMaxDelayMs(-1ll),
      mTestClientSessionID(0){
    // We support any and all resolutions, but prefer 720p30
  	/* 
   	mSinkSupportedVideoFormats.setNativeResolution(
            VideoFormats::RESOLUTION_CEA, 5);  // 1280 x 720 p30

    	mSinkSupportedVideoFormats.enableAll();
   	 */
}

/// @}

WifiDisplaySink::WifiDisplaySink(
        uint32_t flags,
        const sp<ANetworkSession> &netSession,
        const sp<IGraphicBufferProducer> &bufferProducer,
        const sp<AMessage> &notify)
    : mState(UNDEFINED),
      mFlags(flags),
      mNetSession(netSession),
      mSurfaceTex(bufferProducer),
      mNotify(notify),
      mUsingTCPTransport(false),
      mUsingTCPInterleaving(false),
      mSessionID(0),
      mNextCSeq(1),
      mIDRFrameRequestPending(false),
      mTimeOffsetUs(0ll),
      mTimeOffsetValid(false),
      mSetupDeferred(false),
      mLatencyCount(0),
      mLatencySumUs(0ll),
      mLatencyMaxUs(0ll),
	///M : Add @	{	
	  mFirstRTPData(true),
	  mUsingHDCP(false),
	/// @}
      mMaxDelayMs(-1ll) {
    // We support any and all resolutions, but prefer 720p30
  	/* 
   	mSinkSupportedVideoFormats.setNativeResolution(
            VideoFormats::RESOLUTION_CEA, 5);  // 1280 x 720 p30

    	mSinkSupportedVideoFormats.enableAll();
   	 */
}

WifiDisplaySink::~WifiDisplaySink() {
	if(((mFlags & FLAG_SIGMA_TEST_MODE) == FLAG_SIGMA_TEST_MODE)
		&& mComposerClient != NULL)
	{
		mComposerClient->dispose();
		mComposerClient = NULL;
	}

    ///M:@{
#ifdef MTK_WFD_SINK_UIBC_SUPPORT
    if (mUibcClientHandler != NULL) {
        mUibcClientHandler->destroy();
        mUibcClientHandler.clear();
        mUibcClientHandler = NULL;
    }
#endif
    ///@}

	
}

void WifiDisplaySink::start(const char *sourceHost, int32_t sourcePort) {
    sp<AMessage> msg = new AMessage(kWhatStart, id());
    msg->setString("sourceHost", sourceHost);
    msg->setInt32("sourcePort", sourcePort);

	///M: Add for Miracast Test @{
    if((mFlags & FLAG_SIGMA_TEST_MODE) == FLAG_SIGMA_TEST_MODE)
	{
		ALOGI("Run Sigma test mode");
		if(mComposerClient != NULL)
		{
			ALOGE("Sigma test surface have been inited !");
		}
		else
		{
			sp<SurfaceComposerClient> composerClient = new SurfaceComposerClient;
			CHECK_EQ(composerClient->initCheck(), (status_t)OK);
		
			mComposerClient = composerClient;
	
			sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
					ISurfaceComposer::eDisplayIdMain));
			DisplayInfo info;
			SurfaceComposerClient::getDisplayInfo(display, &info);
			ssize_t displayWidth = info.w;
			ssize_t displayHeight = info.h;
	#ifndef MTK_ALPS_BOX_SUPPORT
			if(info.orientation == DISPLAY_ORIENTATION_90 
				|| info.orientation == DISPLAY_ORIENTATION_270)
			{
				displayWidth = info.w;
				displayHeight = info.h;
			}
			else
			{
				displayWidth = info.h;
				displayHeight = info.w;
			}
	#else
			displayWidth = info.w*0.6;
			displayHeight = info.h*0.6;
	#endif
			ALOGI("display is %d x %d\n", displayWidth, displayHeight);
	
	
			mSurfaceControl = composerClient->createSurface(
						String8("A Surface"),
						displayWidth,
						displayHeight,
						PIXEL_FORMAT_RGB_565,
						0);
	
			CHECK(mSurfaceControl != NULL);
			CHECK(mSurfaceControl->isValid());
	
			SurfaceComposerClient::openGlobalTransaction();
			CHECK_EQ(mSurfaceControl->setLayer(INT_MAX), (status_t)OK);
			CHECK_EQ(mSurfaceControl->show(), (status_t)OK);
			SurfaceComposerClient::closeGlobalTransaction();
	
			mSurface = mSurfaceControl->getSurface();
			CHECK(mSurface != NULL);
			
			mSurfaceTex = mSurface->getIGraphicBufferProducer();

		}        
    }
	/// @}

    /// M : Add for enable UIBC
#ifdef MTK_WFD_SINK_UIBC_SUPPORT
    char val[PROPERTY_VALUE_MAX];
    if (property_get("media.wfd.sink.uibc-enabled", val, NULL)) {
        ALOGI("media.wfd.sink.uibc-enabled:%s", val);
        int uibcEnabled = atoi(val);
        if (uibcEnabled > 0){
            mUibcSinkEnabled = true;
        } else {
            mUibcSinkEnabled = false;
        }
    }

    if (mUibcClientHandler == NULL) {
        mUibcClientHandler = new UibcClientHandler(mNetSession);
        mUibcClientHandler->init();
    }
#endif
    /// @}

    msg->post();
	/// M : Add for  Latency issue
	#if 1
	if (mRenderer == NULL) {
		mRenderer = new DirectRenderer(mSurfaceTex);
		looper()->registerHandler(mRenderer);
	}
	#endif
	
}

void WifiDisplaySink::start(const char *uri) {
    sp<AMessage> msg = new AMessage(kWhatStart, id());
    msg->setString("setupURI", uri);
    msg->post();
}

// static
bool WifiDisplaySink::ParseURL(
        const char *url, AString *host, int32_t *port, AString *path,
        AString *user, AString *pass) {
    host->clear();
    *port = 0;
    path->clear();
    user->clear();
    pass->clear();

    if (strncasecmp("rtsp://", url, 7)) {
        return false;
    }

    const char *slashPos = strchr(&url[7], '/');

    if (slashPos == NULL) {
        host->setTo(&url[7]);
        path->setTo("/");
    } else {
        host->setTo(&url[7], slashPos - &url[7]);
        path->setTo(slashPos);
    }

    ssize_t atPos = host->find("@");

    if (atPos >= 0) {
        // Split of user:pass@ from hostname.

        AString userPass(*host, 0, atPos);
        host->erase(0, atPos + 1);

        ssize_t colonPos = userPass.find(":");

        if (colonPos < 0) {
            *user = userPass;
        } else {
            user->setTo(userPass, 0, colonPos);
            pass->setTo(userPass, colonPos + 1, userPass.size() - colonPos - 1);
        }
    }

    const char *colonPos = strchr(host->c_str(), ':');

    if (colonPos != NULL) {
        char *end;
        unsigned long x = strtoul(colonPos + 1, &end, 10);

        if (end == colonPos + 1 || *end != '\0' || x >= 65536) {
            return false;
        }

        *port = x;

        size_t colonOffset = colonPos - host->c_str();
        size_t trailing = host->size() - colonOffset;
        host->erase(colonOffset, trailing);
    } else {
        *port = 554;
    }

    return true;
}

void WifiDisplaySink::onMessageReceived(const sp<AMessage> &msg) {

#if 0 
   if(!mPromoted)
   {
       struct sched_param sched_p;
	   mPromoted = true;
       // Change the scheduling policy to SCHED_RR
       sched_getparam(0, &sched_p);
       sched_p.sched_priority = 1;
   
       if (0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
           ALOGE("@@[WFDSink_PROPERTY]sched_setscheduler fail...");
       }
       else {
           sched_p.sched_priority = 0;
           sched_getparam(0, &sched_p);
           ALOGD("@@[WFDSink_PROPERTY]sched_setscheduler ok..., priority:%d", sched_p.sched_priority);
       } 
   }
#endif   

    switch (msg->what()) {
        case kWhatStart:
        {
           // sleep(2);  // XXX

            int32_t sourcePort;
            CHECK(msg->findString("sourceHost", &mRTSPHost));
            CHECK(msg->findInt32("sourcePort", &sourcePort));
			ALOGD("kWhatStart[+] with %s:%d",mRTSPHost.c_str(),sourcePort);

			///M : Add for SetupURI
			mSetupURI = makeSetupURI(mRTSPHost);

            sp<AMessage> notify = new AMessage(kWhatRTSPNotify, id());

            status_t err = mNetSession->createRTSPClient(
                    mRTSPHost.c_str(), sourcePort, notify, &mSessionID);
			///M : Add for notify display info @{
			if(err<0)
			{
				if(mClient != NULL){
					mClient->onDisplayError(
						IRemoteDisplayClient::kDisplayErrorUnknown);
				}
				return;
			}
			ALOGD("kWhatStart[-] with %s:%d",mRTSPHost.c_str(),sourcePort);
			///M  @}
            //CHECK_EQ(err, (status_t)OK);


			///M: Enable for Mircast testing @{
			struct in_addr mInterfaceAddr;
			inet_aton("127.0.0.1", &mInterfaceAddr); 
            sp<AMessage> testNotify = new AMessage(kWhatTestNotify, id());                        
            mNetSession->createTCPTextDataSession(
                    mInterfaceAddr, WFD_TESTMODE_PORT, testNotify, &mTestSessionID);
        	ALOGD("listening on %s:%d, session id = %d",inet_ntoa(mInterfaceAddr),WFD_TESTMODE_PORT, mTestSessionID);
            mState = CONNECTING;
            break;
        }

        case kWhatRTSPNotify:
        {
            int32_t reason;
            CHECK(msg->findInt32("reason", &reason));

            switch (reason) {
                case ANetworkSession::kWhatError:
                {
                    int32_t sessionID;
                    CHECK(msg->findInt32("sessionID", &sessionID));

                    int32_t err;
                    CHECK(msg->findInt32("err", &err));

                    AString detail;
                    CHECK(msg->findString("detail", &detail));

                    ALOGE("An error occurred in session %d (%d, '%s/%s').",
                          sessionID,
                          err,
                          detail.c_str(),
                          strerror(-err));
					
					///M : Add for notify display info @{
					if(mClient != NULL){
						mClient->onDisplayDisconnected();
					}
					///M  @}


                    if (sessionID == mSessionID) {
                        ALOGI("Lost control connection.");

                        // The control connection is dead now.
                        mNetSession->destroySession(mSessionID);
                        mSessionID = 0;

                        if (mNotify == NULL) {
                          //  looper()->stop();
                        } else {
                            sp<AMessage> notify = mNotify->dup();
                            notify->setInt32("what", kWhatDisconnected);
                            notify->post();
                        }
                    }
                    break;
                }

                case ANetworkSession::kWhatConnected:
                {
                    ALOGI("We're now connected.");
                    mState = CONNECTED;

					///M : Add for notify display info @{
					if(mClient != NULL )
					{
						size_t width, height;
						mClient->onDisplayConnected(
								mSurfaceTex,
								width,
								height, 								
								mUsingHDCP
									? IRemoteDisplayClient::kDisplayFlagSecure
									: 0,
								0);
					}
					/// @}

                    if ((mFlags & FLAG_SPECIAL_MODE) == FLAG_SPECIAL_MODE) {
                        sp<AMessage> notify = new AMessage(
                                kWhatTimeSyncerNotify, id());

                        mTimeSyncer = new TimeSyncer(mNetSession, notify);
                        looper()->registerHandler(mTimeSyncer);

                        mTimeSyncer->startClient(mRTSPHost.c_str(), 8123);
                    }
                    break;
                }

                case ANetworkSession::kWhatData:
                {
                    onReceiveClientData(msg);
                    break;
                }

                default:
                    TRESPASS();
            }
            break;
        }

        case kWhatStop:
        {	
			ALOGD("kWhatStop[+]");
			CHECK(msg->senderAwaitsResponse(&mStopReplyID));

			if(mState >= CONNECTED)
			{
				sendTearDown( mSessionID,mSetupURI.c_str());
			}
			ALOGD("mRenderer->pause()");
			mRenderer->pause();
            looper()->stop();
			///M : Add for notify display info @{
			if(mClient != NULL){
				mClient->onDisplayDisconnected();
			}
			///M @}
			sp<AMessage> response = new AMessage;			
			int32_t err = OK;
		    response->setInt32("err", err);
		    response->postReply(mStopReplyID);
			ALOGD("kWhatStop[-]");
			
            break;
        }

        case kWhatMediaReceiverNotify:
        {
            onMediaReceiverNotify(msg);
            break;
        }

        case kWhatTimeSyncerNotify:
        {
            int32_t what;
            CHECK(msg->findInt32("what", &what));

            if (what == TimeSyncer::kWhatTimeOffset) {
                CHECK(msg->findInt64("offset", &mTimeOffsetUs));
                mTimeOffsetValid = true;

                if (mSetupDeferred) {
                    CHECK_EQ((status_t)OK,
                             sendSetup(
                                mSessionID,
                                "rtsp://x.x.x.x:x/wfd1.0/streamid=0"));

                    mSetupDeferred = false;
                }
            }
            break;
        }

        case kWhatReportLateness:
        {
            if (mLatencyCount > 0) {
                int64_t avgLatencyUs = mLatencySumUs / mLatencyCount;

                ALOGV("avg. latency = %lld ms (max %lld ms)",
                      avgLatencyUs / 1000ll,
                      mLatencyMaxUs / 1000ll);

                sp<AMessage> params = new AMessage;
                params->setInt64("avgLatencyUs", avgLatencyUs);
                params->setInt64("maxLatencyUs", mLatencyMaxUs);
                mMediaReceiver->informSender(0 /* trackIndex */, params);
            }

            mLatencyCount = 0;
            mLatencySumUs = 0ll;
            mLatencyMaxUs = 0ll;

            msg->post(kReportLatenessEveryUs);
            break;
        }
        ///M: Add by MTK  for Miracast Test @{
        case kWhatTestNotify:
        {   
			int32_t reason;
            CHECK(msg->findInt32("reason", &reason));

            ALOGI("kWhatTestNotify, reason:%d", reason);

            switch (reason) {
                case ANetworkSession::kWhatClientConnected:
                {
                     int32_t sessionID;
                     CHECK(msg->findInt32("sessionID", &sessionID));

                     if (mTestClientSessionID > 0) {
                         ALOGW("A test client tried to connect, but we already "
                               "have one.");

                         mNetSession->destroySession(sessionID);
                         break;
                     }

                     mTestClientSessionID = sessionID;

                     ALOGI("We now have a test client (%d) connected.", sessionID);
                     break;
                }
                case ANetworkSession::kWhatError:
                {
                     int32_t sessionID;
                     CHECK(msg->findInt32("sessionID", &sessionID));
                     int32_t err;
                     CHECK(msg->findInt32("err", &err));

                     AString detail;
                     CHECK(msg->findString("detail", &detail));

                     ALOGE("An error occurred in test session %d (%d, '%s/%s').",
                       sessionID,
                       err,
                       detail.c_str(),
                       strerror(-err));

                     mNetSession->destroySession(sessionID);
                     if (sessionID == mTestClientSessionID) {
                         mTestClientSessionID = 0;
                     }
                     break;
                }
                case ANetworkSession::kWhatTextData:
                {
                     onReceiveTestData(msg);
                     break;
                }
                default:
                     TRESPASS();
                     break;
            }
            break;
        }
		/// @}
#ifdef MTK_WFD_SINK_UIBC_SUPPORT
        case kWhatUIBCNotify:
        {
            int32_t reason;
            CHECK(msg->findInt32("reason", &reason));

            switch (reason) {
                case ANetworkSession::kWhatError:
                {
                    int32_t sessionID;
                    CHECK(msg->findInt32("sessionID", &sessionID));

                    int32_t err;
                    CHECK(msg->findInt32("err", &err));

                    AString detail;
                    CHECK(msg->findString("detail", &detail));

                    ALOGE("An error occurred in session %d (%d, '%s/%s').",
                          sessionID,
                          err,
                          detail.c_str(),
                          strerror(-err));

                    if (sessionID == mSessionID) {
                        ALOGI("Lost control connection.");

                        // The control connection is dead now.
                        mNetSession->destroySession(mSessionID);
                        mSessionID = 0;

                        if (mNotify == NULL) {
                          //  looper()->stop();
                        } else {
                            sp<AMessage> notify = mNotify->dup();
                            notify->setInt32("what", kWhatDisconnected);
                            notify->post();
                        }
                    }
                    break;
                }

                case ANetworkSession::kWhatConnected:
                {
                    ALOGI("We're now connected.");
                    mState = CONNECTED;
                    break;
                }

                default:
                    TRESPASS();
            }
            break;
        }
#endif
		///Add for Check Alive @{
		case kWhatCheckAlive:
		{
			ALOGI("kWhatCheckAlive...");
			int32_t sessionID;
	        CHECK(msg->findInt32("sessionID", &sessionID));
			if(mMediaReceiver->getRTPRecvNum(0/*trackIndex*/)== 0){
				ALOGI("No A/V packets disconnect...");
				///M : Add for notify display info @{
				if( mClient != NULL )
				{
					mClient->onDisplayError(
						IRemoteDisplayClient::kDisplayErrorUnknown);
				}
				/// @}
			}else{
					if(mState == PLAYING){
						mMediaReceiver->resetRTPRecvNum(0/*trackIndex*/);
					}			
					scheduleCheckAlive(sessionID);
			}		
			break;
		}
		///@}
        default:
            TRESPASS();
    }
}

void WifiDisplaySink::dumpDelay(size_t trackIndex, int64_t timeUs) {
    int64_t delayMs = (ALooper::GetNowUs() - timeUs) / 1000ll;

    if (delayMs > mMaxDelayMs) {
        mMaxDelayMs = delayMs;
    }

    static const int64_t kMinDelayMs = 0;
    static const int64_t kMaxDelayMs = 300;

    const char *kPattern = "########################################";
    size_t kPatternSize = strlen(kPattern);

    int n = (kPatternSize * (delayMs - kMinDelayMs))
                / (kMaxDelayMs - kMinDelayMs);

    if (n < 0) {
        n = 0;
    } else if ((size_t)n > kPatternSize) {
        n = kPatternSize;
    }

    ALOGI("[%lld]: (%4lld ms / %4lld ms) %s",
          timeUs / 1000,
          delayMs,
          mMaxDelayMs,
          kPattern + kPatternSize - n);
}

void WifiDisplaySink::onMediaReceiverNotify(const sp<AMessage> &msg) {
    int32_t what;
    CHECK(msg->findInt32("what", &what));
	
	///Trace debug
	#ifdef MTB_SUPPORT	
	ATRACE_CALL();
	#endif

    switch (what) {
        case MediaReceiver::kWhatInitDone:
        {
            status_t err;
            CHECK(msg->findInt32("err", &err));
			///M : Add for notify display info @{
			if(mClient != NULL )
			{

				if(err != OK)
				{
					mClient->onDisplayError(
						IRemoteDisplayClient::kDisplayErrorUnknown);

				}else{
				#if 0
					size_t width, height;
					
	                CHECK(VideoFormats::GetConfiguration(
	                            mChosenVideoResolutionType,
	                            mChosenVideoResolutionIndex,
	                            &width,
	                            &height,
	                            NULL /* framesPerSecond */,
	                            NULL /* interlaced */));
					
					
					mClient->onDisplayConnected(
										mSurfaceTex,
										width,
										height,                                 
										mUsingHDCP
	                                    	? IRemoteDisplayClient::kDisplayFlagSecure
	                                    	: 0,
	                                    0);
				#endif
				}

			}
			/// @}
			mRTPInit = true;
            ALOGI("MediaReceiver initialization completed w/ err %d", err);
            break;
        }

        case MediaReceiver::kWhatError:
        {
            status_t err;
            CHECK(msg->findInt32("err", &err));

			///M : Add for notify display info @{
			if( mClient != NULL )
			{
				mClient->onDisplayError(
					IRemoteDisplayClient::kDisplayErrorUnknown);
			}
			/// @}
            ALOGE("MediaReceiver signaled error %d", err);
            break;
        }

        case MediaReceiver::kWhatAccessUnit:
        {
			///M : for Latency issue	
			#if 0	
            if (mRenderer == NULL) {
                mRenderer = new DirectRenderer(mSurfaceTex);
                looper()->registerHandler(mRenderer);
            }
			#endif
						
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));

            int64_t timeUs;
			int64_t oldtimeUs;
            CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));
			oldtimeUs = timeUs;
			

            if (!mTimeOffsetValid && !((mFlags & FLAG_SPECIAL_MODE) == FLAG_SPECIAL_MODE)) {
                mTimeOffsetUs = timeUs - ALooper::GetNowUs();
                mTimeOffsetValid = true;
            }

            CHECK(mTimeOffsetValid);

            // We are the timesync _client_,
            // client time = server time - time offset.
            timeUs -= mTimeOffsetUs;
			

            size_t trackIndex;
            CHECK(msg->findSize("trackIndex", &trackIndex));
			
			
            int64_t nowUs = ALooper::GetNowUs();
            int64_t delayUs = nowUs - timeUs;

            mLatencySumUs += delayUs;
            if (mLatencyCount == 0 || delayUs > mLatencyMaxUs) {
                mLatencyMaxUs = delayUs;
            }
            ++mLatencyCount;
			///M : Latency time print log
			if(mNextPrintTimeUs < 0ll || nowUs >= mNextPrintTimeUs)
			{
             	dumpDelay(trackIndex, timeUs);
				mNextPrintTimeUs = nowUs + kPrintLatenessEveryUs;
	    }

           // timeUs += 220000ll;  // Assume 220 ms of latency
            accessUnit->meta()->setInt64("timeUs", timeUs);
			#if 0
			///M: Latency improve issue, print log			
			int32_t printTime;			
			CHECK(accessUnit->meta()->findInt32("printTime",&printTime));
			if(1 == printTime)
			{
				int64_t nowUs = ALooper::GetNowUs();
				int64_t recvTime;
				CHECK(accessUnit->meta()->findInt64("arrivalTimeUs",&recvTime));
				int64_t delayUs = nowUs - recvTime;	
				ALOGD("Elapse time from NetworkSession to WifiDisplaySink is: [%lld] Ms",delayUs / 1000);
			}
			#endif

			
			///M: Add Latency print log{
			sp<WfdDebugInfo> debugInfo= defaultWfdDebugInfo();
			int64_t now = ALooper::GetNowUs();	
			//ALOGI("sinkin oldtimeUs is %lld, timeus = %lld",oldtimeUs/1000, timeUs/1000);
			//debugInfo->addTimeInfoByKey(trackIndex == 0, oldtimeUs, "sinkin", now/1000);
			debugInfo->addTimeInfoByKey(trackIndex == 0, timeUs, "Offset",  mTimeOffsetUs);
			///@}
			
            sp<AMessage> format;
            if (msg->findMessage("format", &format)) {
                mRenderer->setFormat(trackIndex, format);
            }

#ifdef MTK_WFD_SINK_SUPPORT
			if(mFirstRTPData)
			{
				mFirstRTPData = false;		
				scheduleCheckAlive(mSessionID);
				ALOGD("WfdApp-1st_data Time:(%lld)us",nowUs);					
			}
#endif
			///M : Add for RTP control
			if(mPause == false)
			{
				mRenderer->queueAccessUnit(trackIndex, accessUnit);
			}
		#if 0
			///M: Add Latency print log{		
			int64_t now1 = ALooper::GetNowUs();		
			debugInfo->addTimeInfoByKey(trackIndex == 0, oldtimeUs, "sinkout", now1/1000);
			///@}
		   int64_t t1 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "rtprecvin");	
		   int64_t t2 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "atsparserin");
		   int64_t t3 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "mediarecvout");
		   int64_t t4 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "apacksourceout");
		   int64_t t5 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "sinkin");
		   int64_t t6 = debugInfo->getTimeInfoByKey(true, oldtimeUs, "sinkout");

		   if(t1 != -1 && t2!= -1 && t3!= -1 && t4 != -1&& t5!= -1 && t6!= -1 )
		   {		   	
		   		ALOGI("video timeUs is %lld, rtprecvin->atsparserin %lld, atsparserin->apacksourceout %lld,atsparserin->mediarecvout %lld,apacksourceout->sinkin %lld, sinkin->sinkout %lld,   total %lld ms",
                    oldtimeUs,t2-t1,t4-t2,t3-t2,t5-t4,t6-t5,t6-t1);
		   
		   		//debugInfo->printDebugInfoByKey(true,oldtimeUs);
		   }			
		#endif
            
            break;
        }

        case MediaReceiver::kWhatPacketLost:
        {
#if 0
            if (!mIDRFrameRequestPending) {
                ALOGI("requesting IDR frame");

                sendIDRFrameRequest(mSessionID);
            }
#endif
            break;
        }

        default:
            TRESPASS();
    }
}

void WifiDisplaySink::registerResponseHandler(
        int32_t sessionID, int32_t cseq, HandleRTSPResponseFunc func) {
    ResponseID id;
    id.mSessionID = sessionID;
    id.mCSeq = cseq;
    mResponseHandlers.add(id, func);
}

status_t WifiDisplaySink::sendM2(int32_t sessionID) {
    AString request = "OPTIONS * RTSP/1.0\r\n";
    AppendCommonResponse(&request, mNextCSeq);

    request.append(
            "Require: org.wfa.wfd1.0\r\n"
            "\r\n");


	ALOGD("===== Send M2 Request =====");
	dumpMessage(request);
	ALOGD("===== Send M2 Request =====");


    status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceiveM2Response);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::onReceiveM2Response(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    return OK;
}

status_t WifiDisplaySink::onReceiveSetupResponse(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    if (!msg->findString("session", &mPlaybackSessionID)) {
        return ERROR_MALFORMED;
    }

    if (!ParsedMessage::GetInt32Attribute(
                mPlaybackSessionID.c_str(),
                "timeout",
                &mPlaybackSessionTimeoutSecs)) {
        mPlaybackSessionTimeoutSecs = -1;
    }

    ssize_t colonPos = mPlaybackSessionID.find(";");
    if (colonPos >= 0) {
        // Strip any options from the returned session id.
        mPlaybackSessionID.erase(
                colonPos, mPlaybackSessionID.size() - colonPos);
    }

    status_t err = configureTransport(msg);

    if (err != OK) {
        return err;
    }

    mState = PAUSED;

#if 1
	return sendPlay(
	   sessionID,
	   mSetupURI.c_str());

#else
	return sendPlay(
	   sessionID,
	   "rtsp://x.x.x.x:x/wfd1.0/streamid=0");
#endif

}

status_t WifiDisplaySink::configureTransport(const sp<ParsedMessage> &msg) {
    if (mUsingTCPTransport && !((mFlags & FLAG_SPECIAL_MODE) == FLAG_SPECIAL_MODE)) {
        // In "special" mode we still use a UDP RTCP back-channel that
        // needs connecting.
        return OK;
    }

	ALOGD("===== Receive M6 Response =====");
	dumpMessage(msg->debugString());
	ALOGD("===== Receive M6 Response =====");


    AString transport;
    if (!msg->findString("transport", &transport)) {
        ALOGE("Missing 'transport' field in SETUP response.");
        return ERROR_MALFORMED;
    }

    AString sourceHost;
    if (!ParsedMessage::GetAttribute(
                transport.c_str(), "source", &sourceHost)) {
        sourceHost = mRTSPHost;
    }
	int rtpPort, rtcpPort;

	///M: Not need to parse sever_port, for IOT issue @{
	#if 0

    AString serverPortStr;
    if (!ParsedMessage::GetAttribute(
                transport.c_str(), "server_port", &serverPortStr)) {
        ALOGE("Missing 'server_port' in Transport field.");
        return ERROR_MALFORMED;
    }

    
#ifdef MTK_WFD_SINK_SUPPORT
	///M: Modify for NONE RTCP
    if (sscanf(serverPortStr.c_str(), "%d", &rtpPort) != 1
        || rtpPort <= 0 || rtpPort > 65535) {
    	ALOGE("Invalid server_port description '%s'.",
            	serverPortStr.c_str());

    	return ERROR_MALFORMED;
	}

#else
    if (sscanf(serverPortStr.c_str(), "%d-%d", &rtpPort, &rtcpPort) != 2
            || rtpPort <= 0 || rtpPort > 65535
            || rtcpPort <=0 || rtcpPort > 65535
            || rtcpPort != rtpPort + 1) {
        ALOGE("Invalid server_port description '%s'.",
                serverPortStr.c_str());

        return ERROR_MALFORMED;
    }
#endif

    if (rtpPort & 1) {
        ALOGW("Server picked an odd numbered RTP port.");
    }
    #endif
	///@}
    return mMediaReceiver->connectTrack(
            0 /* trackIndex */, sourceHost.c_str(), rtpPort, rtcpPort);
}

status_t WifiDisplaySink::onReceivePlayResponse(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    mState = PLAYING;
	///M: Modify for NONE RTCP
#ifdef MTK_WFD_SINK_SUPPORT

#else
	(new AMessage(kWhatReportLateness, id()))->post(kReportLatenessEveryUs);
#endif
    return OK;
}

status_t WifiDisplaySink::onReceiveIDRFrameRequestResponse(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    CHECK(mIDRFrameRequestPending);
    mIDRFrameRequestPending = false;

    return OK;
}

void WifiDisplaySink::onReceiveClientData(const sp<AMessage> &msg) {
    int32_t sessionID;
    CHECK(msg->findInt32("sessionID", &sessionID));
	
    sp<RefBase> obj;
    CHECK(msg->findObject("data", &obj));

    sp<ParsedMessage> data =
        static_cast<ParsedMessage *>(obj.get());

    ALOGE("session %d received '%s'",
          sessionID, data->debugString().c_str());

    AString method;
    AString uri;
    data->getRequestField(0, &method);

    int32_t cseq;
    if (!data->findInt32("cseq", &cseq)) {
        sendErrorResponse(sessionID, "400 Bad Request", -1 /* cseq */);
        return;
    }

    if (method.startsWith("RTSP/")) {
        // This is a response.

        ResponseID id;
        id.mSessionID = sessionID;
        id.mCSeq = cseq;

        ssize_t index = mResponseHandlers.indexOfKey(id);

        if (index < 0) {
            ALOGW("Received unsolicited server response, cseq %d", cseq);
            return;
        }

        HandleRTSPResponseFunc func = mResponseHandlers.valueAt(index);
        mResponseHandlers.removeItemsAt(index);

        status_t err = (this->*func)(sessionID, data);
        CHECK_EQ(err, (status_t)OK);
    } else {
        AString version;
        data->getRequestField(2, &version);
        if (!(version == AString("RTSP/1.0"))) {
            sendErrorResponse(sessionID, "505 RTSP Version not supported", cseq);
            return;
        }

        if (method == "OPTIONS") {
            onOptionsRequest(sessionID, cseq, data);
        } else if (method == "GET_PARAMETER") {
            onGetParameterRequest(sessionID, cseq, data);
        } else if (method == "SET_PARAMETER") {
            onSetParameterRequest(sessionID, cseq, data);
        } else {
            sendErrorResponse(sessionID, "405 Method Not Allowed", cseq);
        }
    }
}

void WifiDisplaySink::onOptionsRequest(
        int32_t sessionID,
        int32_t cseq,
        const sp<ParsedMessage> &data) {
    AString response = "RTSP/1.0 200 OK\r\n";
    AppendCommonResponse(&response, cseq);
    response.append("Public: org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER\r\n");
    response.append("\r\n");
	

	ALOGD("===== Response M1 Request =====");
	dumpMessage(response);
	ALOGD("===== Response M1 Request =====");


    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);

    err = sendM2(sessionID);
    CHECK_EQ(err, (status_t)OK);
}

void WifiDisplaySink::onGetParameterRequest(
        int32_t sessionID,
        int32_t cseq,
        const sp<ParsedMessage> &data) {

    const char *content = data->getContent();
    ///M : Add for keep alive @{
    if (strstr(content,"wfd_") == NULL) {
        AString responseAlive = "RTSP/1.0 200 OK\r\n";
        AppendCommonResponse(&responseAlive, cseq);
        responseAlive.append("\r\n");

        ALOGD("===== Response M16 Request =====");
        dumpMessage(responseAlive);
        ALOGD("===== Response M16 Request =====");

        status_t errAlive = mNetSession->sendRequest(sessionID, responseAlive.
                            c_str());
        CHECK_EQ(errAlive, (status_t)OK);
        return;
    }
    /// @}

    if (mState < CONNECTED) {
        ALOGD("===== Unknown state %d=====", mState);
        return;
    }
    
    mUsingTCPTransport = false;
    mUsingTCPInterleaving = false;

    char val[PROPERTY_VALUE_MAX];
    if (property_get("media.wfd-sink.tcp-mode", val, NULL)) {
        if (!strcasecmp("true", val) || !strcmp("1", val)) {
            ALOGI("Using TCP unicast transport.");
            mUsingTCPTransport = true;
            mUsingTCPInterleaving = false;
        } else if (!strcasecmp("interleaved", val)) {
            ALOGI("Using TCP interleaved transport.");
            mUsingTCPTransport = true;
            mUsingTCPInterleaving = true;
        }
    } else if ((mFlags & FLAG_SPECIAL_MODE) == FLAG_SPECIAL_MODE) {
        mUsingTCPTransport = true;
    }

    AString body = "";
            
    if (strstr(content,"wfd_video_formats") != NULL) {
        body = "wfd_video_formats: ";
        if((mFlags & FLAG_SIGMA_TEST_MODE) == FLAG_SIGMA_TEST_MODE){
                body.append("00 00 01 01 00000001 00000000 00000000 00 0000 0000 13 none none");

        } else {
            body.append(mSinkSupportedVideoFormats.getFormatSpec());
        }
        body.append("\r\n");
    }
        
    if (strstr(content,"wfd_audio_codecs") != NULL) {
#ifdef MTK_WFD_SINK_SUPPORT
    /*
	    LPCM:  48 kHZ,16b
	    AAC: 48 kHz, 16b, 2 channels; 
        */
        body.append("wfd_audio_codecs: ");
    	if((mFlags & FLAG_SIGMA_TEST_MODE) == FLAG_SIGMA_TEST_MODE){
            body.append("LPCM 00000002 00, AAC 00000001 00");  // 2 ch AAC 48kHz            
        } else {
            body.append(
				"LPCM 00000002 00, AAC 00000001 00");
        }

#else
        body.append("AAC 0000000F 00");
#endif
        body.append("\r\n");
    }

    if (strstr(content,"wfd_client_rtp_ports") != NULL) {
        body.append("wfd_client_rtp_ports: RTP/AVP/");
        if (mUsingTCPTransport) {
            body.append("TCP;");
            if (mUsingTCPInterleaving) {
                body.append("interleaved");
            } else {
                body.append("unicast 19000 0");
            }
        } else {
            if (property_get("wfdsink.sigmatest.rtcp", val, NULL)) {
                if (!strcasecmp("true", val) || !strcmp("1", val)) {
                    body.append("UDP;unicast 19000 19001");
                } else {
                    body.append("UDP;unicast 19000 0");
                }
            } else {
                body.append("UDP;unicast 19000 0");
            }
        }
        body.append(" mode=play\r\n");
    }

    ///M:@{
    if(strstr(content,"wfd_content_protection") != NULL)
    {
        body.append("wfd_content_protection: none\r\n");		
    }

    if(strstr(content,"wfd_3d_video_formats") != NULL)
    {
        body.append("wfd_3d_video_formats: none\r\n");		
    }
    
    if(strstr(content,"wfd_display_edid") != NULL)
    {
        body.append("wfd_display_edid: none\r\n");		
    }
    if(strstr(content,"wfd_coupled_sink") != NULL)
    {
        body.append("wfd_coupled_sink: none\r\n");		
    }

    if (strstr(content, WFD_UIBC_CAPABILITY) != NULL) {
#ifdef MTK_WFD_SINK_UIBC_SUPPORT
        if (mUibcSinkEnabled) {
            body.append(mUibcClientHandler->getLocalCapabilities());
            ALOGI("The UIBC sink is started");
        } else {
            body.append(UIBC_SINK_CAPABILTY_NONE);
            ALOGI("uibc-capability:%s", UIBC_SINK_CAPABILTY_NONE);
            ALOGI("The UIBC sink is disabled");
        }
#else
        body.append(UIBC_SINK_CAPABILTY_NONE);
        ALOGI("uibc-capability:%s", UIBC_SINK_CAPABILTY_NONE);
#endif
    }
    if(strstr(content,"wfd_standby_resume_capability") != NULL)
    {
        body.append("wfd_standby_resume_capability: none\r\n");	
    }

    if(strstr(content,"wfd_I2C") != NULL)
    {
        body.append("wfd_I2C: none\r\n");	
    }
    if(strstr(content,"wfd_connector_type") != NULL)
    {
        body.append("wfd_connector_type: 5\r\n");		
    }
    /// @}

    AString response = "RTSP/1.0 200 OK\r\n";
    AppendCommonResponse(&response, cseq);
    response.append("Content-Type: text/parameters\r\n");

    response.append(StringPrintf("Content-Length: %d\r\n", body.size()));
    response.append("\r\n");
    response.append(body);
    ALOGD("===== Response M3 Request =====");
    dumpMessage(response);
    ALOGD("===== Response M3 Request =====");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);
}

status_t WifiDisplaySink::sendSetup(int32_t sessionID, const char *uri) {

	sp<AMessage> notify = new AMessage(kWhatMediaReceiverNotify, id());

    mMediaReceiverLooper = new ALooper;
    mMediaReceiverLooper->setName("media_receiver");

    mMediaReceiverLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);

    mMediaReceiver = new MediaReceiver(mNetSession, notify);
    mMediaReceiverLooper->registerHandler(mMediaReceiver);

    RTPReceiver::TransportMode rtpMode = RTPReceiver::TRANSPORT_UDP;
    if (mUsingTCPTransport) {
        if (mUsingTCPInterleaving) {
            rtpMode = RTPReceiver::TRANSPORT_TCP_INTERLEAVED;
        } else {
            rtpMode = RTPReceiver::TRANSPORT_TCP;
        }
    }

    int32_t localRTPPort;
	#ifdef MTK_WFD_SINK_SUPPORT
	///M: Modify for NONE RTCP
    status_t err = mMediaReceiver->addTrack(
            rtpMode, RTPReceiver::TRANSPORT_NONE /* rtcpMode */, &localRTPPort);

	#else
	status_t err = mMediaReceiver->addTrack(
				rtpMode, RTPReceiver::TRANSPORT_UDP /* rtcpMode */, &localRTPPort);


	#endif
    if (err == OK) {
        err = mMediaReceiver->initAsync(MediaReceiver::MODE_TRANSPORT_STREAM);
		
	}
    if (err != OK) {
        mMediaReceiverLooper->unregisterHandler(mMediaReceiver->id());
        mMediaReceiver.clear();

        mMediaReceiverLooper->stop();
        mMediaReceiverLooper.clear();
	
        return err;
    }

    AString request = StringPrintf("SETUP %s RTSP/1.0\r\n", uri);

    AppendCommonResponse(&request, mNextCSeq);

    if (rtpMode == RTPReceiver::TRANSPORT_TCP_INTERLEAVED) {
        request.append("Transport: RTP/AVP/TCP;interleaved=0-1\r\n");
    } else if (rtpMode == RTPReceiver::TRANSPORT_TCP) {
        if ((mFlags & FLAG_SPECIAL_MODE) == FLAG_SPECIAL_MODE) {
            // This isn't quite true, since the RTP connection is through TCP
            // and the RTCP connection through UDP...
            request.append(
                    StringPrintf(
                        "Transport: RTP/AVP/TCP;unicast;client_port=%d-%d\r\n",
                        localRTPPort, localRTPPort + 1));
        } else {
            request.append(
                    StringPrintf(
                        "Transport: RTP/AVP/TCP;unicast;client_port=%d\r\n",
                        localRTPPort));
        }
    } else {

		#ifdef MTK_WFD_SINK_SUPPORT
		///M: Modify for NONE RTCP
        request.append(
            StringPrintf(
                "Transport: RTP/AVP/UDP;unicast;client_port=%d\r\n",
                localRTPPort));

		#else
        request.append(
                StringPrintf(
                    "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d\r\n",
                    localRTPPort,
                    localRTPPort + 1));

		#endif

    }

    request.append("\r\n");
	

	ALOGD("===== Send M6 Request =====");
	dumpMessage(request);
	ALOGD("===== Send M6 Request =====");


    ALOGV("request = '%s'", request.c_str());

    err = mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceiveSetupResponse);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::sendPlay(int32_t sessionID, const char *uri) {
    AString request = StringPrintf("PLAY %s RTSP/1.0\r\n", uri);

    AppendCommonResponse(&request, mNextCSeq);

    request.append(StringPrintf("Session: %s\r\n", mPlaybackSessionID.c_str()));
    request.append("\r\n");
	

	ALOGD("===== Send M7 Request =====");
	dumpMessage(request);
	ALOGD("===== Send M7 Request =====");


    status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceivePlayResponse);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::sendIDRFrameRequest(int32_t sessionID) {
    CHECK(!mIDRFrameRequestPending);

    AString request = "SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\n";

    AppendCommonResponse(&request, mNextCSeq);

    AString content = "wfd_idr_request\r\n";

    request.append(StringPrintf("Session: %s\r\n", mPlaybackSessionID.c_str()));
    request.append(StringPrintf("Content-Length: %d\r\n", content.size()));
    request.append("\r\n");
    request.append(content);

    status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID,
            mNextCSeq,
            &WifiDisplaySink::onReceiveIDRFrameRequestResponse);

    ++mNextCSeq;

    mIDRFrameRequestPending = true;

    return OK;
}

void WifiDisplaySink::onSetParameterRequest(
        int32_t sessionID,
        int32_t cseq,
        const sp<ParsedMessage> &data) {
    AString value;
    const char *content = data->getContent();

    AString response = "RTSP/1.0 200 OK\r\n";
    AppendCommonResponse(&response, cseq);
    response.append("\r\n");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);
	
	///M : Modify for trigger Method @{
    if (strstr(content, WFD_TRIGGER_METHOD) != NULL)
    {
        AString methodName;
        char rtspBody[WFD_MAX_BUFFER_SIZE];
        memset(rtspBody, 0, sizeof(rtspBody));
        strcpy(rtspBody, content);

        if (ParsedMessage::getHeaderFromBody(rtspBody, WFD_TRIGGER_METHOD, &
                                             methodName))
        {
            status_t err = OK;
            ALOGD("Method name:%s", methodName.c_str());

			ALOGD("===== Receive M5 Request =====");
			dumpMessage(data->debugString());
			ALOGD("===== Receive M5 Request =====");

            if (methodName.startsWith("SETUP"))
            {

                err = sendSetup(
                          sessionID,
                          mSetupURI.c_str());				
            }
            else if (methodName.startsWith("PLAY"))
            {
                err = sendPlay(
                          sessionID,
                          mSetupURI.c_str());				
            }
            else if (methodName.startsWith("PAUSE"))
            {
                err = sendPause(
                          sessionID,
                          mSetupURI.c_str());
            }
            else if (methodName.startsWith("TEARDOWN"))
            {
                err = sendTearDown(
                          sessionID,
                          mSetupURI.c_str());
            }
            CHECK_EQ(err, (status_t)OK);
			
        }
        else
        {
            ALOGE("Can't parse the trigger method from %s", content);
        }
		return;
    }
	///M @}
	
#if 0
    if (strstr(content, "wfd_trigger_method: SETUP\r\n") != NULL)
    {
        if ((mFlags & FLAG_SPECIAL_MODE) && !mTimeOffsetValid)
        {
            mSetupDeferred = true;
        }
        else
        {
            status_t err =
                sendSetup(
                    sessionID,
                    "rtsp://x.x.x.x:x/wfd1.0/streamid=0");

            CHECK_EQ(err, (status_t)OK);
        }
    }
#endif	


	///M : Parse M4 Request @{

	/**Parse video/audio format info**/

	AString wfd_video_formats;
	sp<Parameters> params =
		  Parameters::Parse(data->getContent(), strlen(data->getContent()));

	#if 0
	if (!params->findParameter("wfd_video_formats", &wfd_video_formats)) 
	{
		
        ALOGE("Missing 'wfd_video_formats' field in M4 Request.");
        
    }
	ALOGE("%s",wfd_video_formats.c_str());

	
	if  (!(wfd_video_formats == "none")) 
	{  
	    if (!mSinkSupportedVideoFormats.parseFormatSpec(wfd_video_formats.c_str())) 
		{
	        ALOGE("Failed to parse source provided wfd_video_formats (%s)",
	              wfd_video_formats.c_str());
	        return ;
	    }
	}
	
	mSinkSupportedVideoFormats.getNativeResolution(
		&mChosenVideoResolutionType,
		&mChosenVideoResolutionIndex);
	
	mSinkSupportedVideoFormats.getProfileLevel(
		mChosenVideoResolutionType,
		mChosenVideoResolutionIndex,
		&mChosenVideoProfile,
		&mChosenVideoLevel);

 	ALOGD("mChosenVideoResolutionType= %d," \
          "mChosenVideoResolutionIndex=%d," \
          "mChosenVideoProfile=%d," \
          "mChosenVideoLevel=%d",
			mChosenVideoResolutionType,
			mChosenVideoResolutionIndex,
			mChosenVideoProfile,
			mChosenVideoLevel);
	#endif
    /**Parse UIBC format info**/
#ifdef MTK_WFD_SINK_UIBC_SUPPORT
    if (params != NULL &&
        params->findParameter(WFD_UIBC_CAPABILITY, &value)) {
        unsigned port;
        int32_t port32;
        int32_t sessionID;

        ALOGI("Parse UIBC Cap String:%s", value.c_str());

        if (mUibcClientHandler->getPort() > 0) {
            ALOGD("===== Receive M14 Request =====");
        }
        mUibcClientHandler->parseRemoteCapabilities(value);
    }

    // Get the resolution parameter
    if (params != NULL &&
        params->findParameter(WFD_VIDEO_FORMATS, &value)) {
        if (!(value == "none")) {
            ALOGI("Parse video spec String:%s", value.c_str());
            unsigned native, dummy;
            unsigned profile, level;
            uint32_t resolutionEnabled[3];
            size_t index, width, height, framesPerSecond;
            bool interlaced;
            if (sscanf(
                    value.c_str(),
                    "%02x %02x %02x %02x %08X %08X %08X",
                    &native,
                    &dummy,
                    &profile,
                    &level,
                    &resolutionEnabled[0],
                    &resolutionEnabled[1],
                    &resolutionEnabled[2]) != 7) {
                    return;
            }
            size_t r = 0;
            bool _find = false; 
            for (r = 0; r<VideoFormats::kNumResolutionTypes; ++r) {
                for (index = 0; index < 32; ++index) {
                    if ( resolutionEnabled[r] & (1ul << index)) {
                        _find = true;
                        break;
                    }
                }
                if(_find) break;
            }
            //ALOGI("find r=%d index=%d", r, index);
            
            CHECK(VideoFormats::GetConfiguration(
                      (VideoFormats::ResolutionType)r,
                      index,
                      &width,
                      &height,
                      &framesPerSecond,
                      &interlaced));
            mUibcClientHandler->setWFDResolution(width, height, 1);  
        }
    }
    if (strstr(content, WFD_UIBC_SETTING) != NULL) {
        if (params != NULL &&
            params->findParameter(WFD_UIBC_SETTING, &value)) {
            ALOGI("Parse UIBC setting string:%s", value.c_str());
            if (strstr(value.c_str(), "enable") != NULL) {
                mUibcClientHandler->setUibcEnabled(true);
                int32_t port32 = mUibcClientHandler->getPort();
                if ( port32 > 1 && port32 < 65535) {
                    sp<AMessage> notify = new AMessage(kWhatUIBCNotify, id());
                    status_t err = mNetSession->createUIBCClient(
                                    mRTSPHost.c_str(), port32, notify, &sessionID);
                    CHECK_EQ(err, (status_t)OK);
                    mUibcClientHandler->setSessionID(sessionID);
                }
            } else {
                mUibcClientHandler->setUibcEnabled(false);
                int32_t uibcSession =  mUibcClientHandler->getSessionID();
                if (uibcSession > 0) {
                    mNetSession->destroySession(mUibcClientHandler->getSessionID());
                    mUibcClientHandler->setSessionID(0);
                }
            }
        }
        if (strstr(content, WFD_UIBC_CAPABILITY) == NULL) {
            ALOGD("===== Receive M15 Request =====");
            dumpMessage(data->debugString());
            ALOGD("===== Receive M15 Request =====");
            return;		
        }
    }
#endif

	ALOGD("===== Receive M4 Request =====");
	dumpMessage(data->debugString());
	ALOGD("===== Receive M4 Request =====");
	return;				
	///M @}
   
}

void WifiDisplaySink::sendErrorResponse(
        int32_t sessionID,
        const char *errorDetail,
        int32_t cseq) {
    AString response;
    response.append("RTSP/1.0 ");
    response.append(errorDetail);
    response.append("\r\n");

    AppendCommonResponse(&response, cseq);

    response.append("\r\n");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);
}

// static
void WifiDisplaySink::AppendCommonResponse(AString *response, int32_t cseq) {
    time_t now = time(NULL);
    struct tm *now2 = gmtime(&now);
    char buf[128];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %z", now2);

    //response->append("Date: ");
    //response->append(buf);
    //response->append("\r\n");
    //response->append(StringPrintf("User-Agent: %s\r\n", sUserAgent.c_str()));

    if (cseq >= 0) {
        response->append(StringPrintf("CSeq: %d\r\n", cseq));
    }
}
///M: Add by MTK @{
status_t WifiDisplaySink::sendPause(int32_t sessionID, const char *uri)
{
    AString request = StringPrintf("PAUSE %s RTSP/1.0\r\n", uri);

    AppendCommonResponse(&request, mNextCSeq);

    request.append(StringPrintf("Session: %s\r\n", mPlaybackSessionID.c_str()));
    request.append("\r\n");

	ALOGD("===== Send M9 Request =====");
	dumpMessage(request);
	ALOGD("===== Send M9 Request =====");


    status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK)
    {
        return err;
    }

    registerResponseHandler(
        sessionID, mNextCSeq, &WifiDisplaySink::onReceivePauseResponse);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::onReceivePauseResponse(
    int32_t sessionID, const sp<ParsedMessage> &msg)
{
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode))
    {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200)
    {
        return ERROR_UNSUPPORTED;
    }

    mState = PAUSED;

    return OK;
}

status_t WifiDisplaySink::sendTearDown(int32_t sessionID, const char *uri)
{
    AString request = StringPrintf("TEARDOWN %s RTSP/1.0\r\n", uri);

    AppendCommonResponse(&request, mNextCSeq);

    request.append(StringPrintf("Session: %s\r\n", mPlaybackSessionID.c_str()));
    request.append("\r\n");
	

	ALOGD("===== Send M8 Request =====");
	dumpMessage(request);
	ALOGD("===== Send M8 Request =====");


    status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK)
    {
        return err;
    }

    registerResponseHandler(
        sessionID, mNextCSeq, &WifiDisplaySink::onReceiveTearDownResponse);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::onReceiveTearDownResponse(
    int32_t sessionID, const sp<ParsedMessage> &msg)
{
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode))
    {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200)
    {
        return ERROR_UNSUPPORTED;
    }

    mState = UNDEFINED;

    // The control connection is dead now.
    mNetSession->destroySession(mSessionID);
    mSessionID = 0;

	if(mClient != NULL){
	    mClient->onDisplayDisconnected();
	}

    ALOGV("The session is ended by teardown msg");

    return OK;
}
/// M: Add by MTK @{
status_t WifiDisplaySink::sendGenericMsgByMethod(int32_t methodID) {
    AString method;

    if(mSessionID <= 0){
        ALOGE("No active client session for WFD client");
        return 0;
    } 

    switch(methodID){
        case PLAY:           
            sendPlay(mSessionID,mSetupURI.c_str());
            break;
        case PAUSE:
            sendPause(mSessionID,mSetupURI.c_str());
            break;
        case TEARDOWN:
            sendTearDown(mSessionID,mSetupURI.c_str());
            break;
        default:
            ALOGE("Unknown methodID:%d", methodID);
            return BAD_VALUE;
            break;
    }

    return OK;
}

void WifiDisplaySink::onReceiveTestData(const sp<AMessage> &msg)
{
	status_t err = OK;
	int32_t sessionID;
	AString response = "";
	CHECK(msg->findInt32("sessionID", &sessionID));

	AString data;
	CHECK(msg->findString("data", &data));

	mFlags|= FLAG_SIGMA_TEST_MODE;

	ALOGI("test session %d received '%s'",
		  sessionID, data.c_str());

	if(data.startsWith("reset") || data.startsWith("wfd_cmd wfd_reset")){
		//resetRtspClient();
		response = "OK,\r\n";
	}else if(data.startsWith("rtsp_get sessionids")){		
		if(mPlaybackSessionID.size() > 0) {
			response = StringPrintf("OK,%s, 0\r\n", mPlaybackSessionID.c_str());
		} else {
			response = "FAIL,0,0\r\n";
		}
	}else if(data.startsWith("rtsp PLAY")){
		err = sendGenericMsgByMethod(PLAY);
		response = "OK,PLAY\r\n";
	}else if(data.startsWith("rtsp PAUSE")){
		err = sendGenericMsgByMethod(PAUSE);
		response = "OK,PAUSE\r\n";
	}else if(data.startsWith("rtsp TEARDOWN")){
		err = sendGenericMsgByMethod(TEARDOWN);
		response = "OK,TEARDOWN\r\n";
	}else if(data.startsWith("rtsp_set")){
		response = "OK,\r\n";
	}else if(data.startsWith("event uibcGen 1")){
#ifdef MTK_WFD_SINK_UIBC_SUPPORT	
	   static bool mouseGened = false;
                    if (mouseGened) {
                        sendUIBCMiracastKeys();
                        mouseGened = !mouseGened;
                    } else {
                        sendUIBCMiracastMouseMotion();
                        mouseGened = !mouseGened;
	    }		
            response = "OK,\r\n";
#endif
	}else{
		ALOGD("No match test command");
	}

	if(response.size() > 0){
		ALOGD("test response:[%s]", response.c_str());
		int err = mNetSession->sendDirectRequest(sessionID, response.c_str(), response.size());
		ALOGD("send test response ret: %d, %s", err, strerror(err));
	}


}

AString WifiDisplaySink::makeSetupURI(AString sourceHost)
{
	AString uri;
	uri.append("rtsp://");
	uri.append(sourceHost);
	uri.append("/wfd1.0/streamid=0");
	return uri;
}
status_t WifiDisplaySink::dumpMessage(AString strMsg)
{
	ALOGD("%s",strMsg.c_str());
	return OK;
}

void WifiDisplaySink::stop() {
	ALOGD("WifiDisplaySink::stop[+]");	
    sp<AMessage> msg = new AMessage(kWhatStop, id());
	sp<AMessage> response;
	msg->postAndAwaitResponse(&response);
	ALOGD("WifiDisplaySink::stop[-]");
    
	
}

/// @}

#ifdef MTK_WFD_SINK_SUPPORT
void WifiDisplaySink::mtkSinkPause()
{	
	ALOGD("Pause Rendering");	
	mPause = true;
	if(mRTPInit)
	{		
		mMediaReceiver->mtkRTPPause(0/*trackIndex*/);
	}
	if(mRenderer != NULL)// render status is playing or pause
	{
		mRenderer->pause();
	}
}
void WifiDisplaySink::mtkSinkResume(const sp<IGraphicBufferProducer> &bufferProducer)
{
	
	ALOGD("Resume Rendering");
	mPause = false;
	//TODO
	if(mRTPInit)
	{		 
		 mMediaReceiver->mtkRTPResume(0/*trackIndex*/);
	} 
	if(mRenderer != NULL) // render status is playing or pause
	{
		mRenderer->resume(bufferProducer);
	}
}

#ifdef MTK_WFD_SINK_UIBC_SUPPORT
void WifiDisplaySink::sendUIBCMiracastKeys() {
    const char* keyDescs[16] = {"3,0x004d,0x0000", 
                          "4,0x004d,0x0000", 
                          "3,0x0069,0x0000", 
                          "4,0x0069,0x0000",
                          "3,0x0072,0x0000", 
                          "4,0x0072,0x0000"
                          "3,0x0061,0x0000", 
                          "4,0x0061,0x0000",
                          "3,0x0063,0x0000", 
                          "4,0x0063,0x0000",
                          "3,0x0061,0x0000", 
                          "4,0x0061,0x0000",
                          "3,0x0073,0x0000", 
                          "4,0x0073,0x0000",
                          "3,0x0074,0x0000", 
                          "4,0x0074,0x0000",}; 
    int i;
    for (i = 0 ; i < 16 ; i++) {
        ALOGD("sendUIBCMiracastKeys %s", keyDescs[i]);
        sendUIBCGenericKeyEvent(keyDescs[i]);
    }
}

void WifiDisplaySink::sendUIBCMiracastMouseMotion() {
    int i;
    char strTmp[32];
    int width, heifght;
    
    UibcMessage::getScreenResolution(&width, &heifght);
    // Mouse down
    sprintf(strTmp, "0,1,0,%d,0", heifght/2);
    sendUIBCGenericTouchEvent(strTmp);

    // Move to down and up
    for (i = 0; i <= 200; i++) {
        sprintf(strTmp, "2,1,0,%d,%d", heifght/2, i);
        sendUIBCGenericTouchEvent(strTmp);
    }
    
    for (i = 200; i >= 0; i--) {
        sprintf(strTmp, "2,1,0,%d,%d", heifght/2, i);
        sendUIBCGenericTouchEvent(strTmp);
    }

    // Mouse up
    sprintf(strTmp, "1,1,0,%d,0", heifght/2);
    sendUIBCGenericTouchEvent(strTmp);

}

status_t WifiDisplaySink::sendUIBCGenericTouchEvent(const char * eventDesc) {
    status_t err;
    //ALOGD("sendUIBCGenericTouchEvent'%s'", eventDesc);
        
    if (mUibcClientHandler == NULL ||
        eventDesc == NULL || 
        !mUibcSinkEnabled) 
        return -1;
    if (!mUibcClientHandler->getUibcEnabled())
        return -1;
        
    if (!(mUibcClientHandler->isGenericSupported(DEVICE_TYPE_MOUSE) ||
        mUibcClientHandler->isGenericSupported(DEVICE_TYPE_SINGLETOUCH)))
        return -1;

    err = mUibcClientHandler->sendUibcMessage(mNetSession,
            UibcMessage::GENERIC_TOUCH_DOWN,
            eventDesc);
    return err;
}

status_t WifiDisplaySink::sendUIBCGenericKeyEvent(const char * eventDesc) {
    status_t err;
    //ALOGD("sendUIBCGenericKeyEvent'%s'", eventDesc);

    if (mUibcClientHandler == NULL ||
        eventDesc == NULL || 
        !mUibcSinkEnabled) 
        return -1;
    if (!mUibcClientHandler->getUibcEnabled())
        return -1;

    if (!mUibcClientHandler->isGenericSupported(DEVICE_TYPE_KEYBOARD))
        return -1;

    err = mUibcClientHandler->sendUibcMessage(mNetSession,
            UibcMessage::GENERIC_KEY_DOWN,
            eventDesc);
    return err;
}
#endif

int WifiDisplaySink::getWfdParam(int paramType) {
    int paramValue = 0;

    if (paramType == 8) {
#ifdef MTK_WFD_SINK_UIBC_SUPPORT
        /*
         * bit 0: UIBC touch
         * bit 1: UIBC mouse
         * bit 2: UIBC keyboard
         */
         if (mUibcClientHandler != NULL &&
             mUibcClientHandler->getUibcEnabled()) {
            bool touchSupported = mUibcClientHandler->isGenericSupported(DEVICE_TYPE_SINGLETOUCH);
            bool mouseSupported = mUibcClientHandler->isGenericSupported(DEVICE_TYPE_MOUSE);
            bool keyboardSupported = mUibcClientHandler->isGenericSupported(DEVICE_TYPE_KEYBOARD);
             paramValue |= touchSupported ? 0x01:0x00;
             paramValue |= mouseSupported ? 0x02:0x00;
             paramValue |= keyboardSupported ? 0x04:0x00;
         }
#endif
    }
    return paramValue;
}

void WifiDisplaySink::scheduleCheckAlive(int32_t sessionID) {
    sp<AMessage> msg = new AMessage(kWhatCheckAlive, id());
    msg->setInt32("sessionID", sessionID);

	char checkAliveTime[PROPERTY_VALUE_MAX];
	if (property_get("media.wfd-sink.checkalivetime", checkAliveTime, NULL)){		

		int64_t setCheckAliveTime = atoi(checkAliveTime)*1000000ll;
		msg->post(setCheckAliveTime);
	}else{
	
		msg->post(kCheckAliveTimeoutUs);
	}	
    
}

#endif


}  // namespace android
