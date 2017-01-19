#define LOG_TAG "[Dash]pTest"
#include <utils/Log.h>
#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <cutils/properties.h>
#include "DashDataProcessor.h"
#include "AnotherPacketSource.h"
#include <media/stagefright/MetaData.h>
namespace android {

struct MyHandler : public AHandler {
	MyHandler( ){shouldStartNow =false;}	
    enum{
        kWhatSource,
        kWhatDDPNotify,
    }; 
    bool startNow(){return shouldStartNow;}

protected:
	virtual ~MyHandler(){}
    void onMessageReceived(const sp<AMessage> &msg){
        switch(msg->what()) { 
            case kWhatDDPNotify :
            {   int32_t what;
                if(msg->findInt32("what", &what) 
                    && what == DashDataProcessor::kWhatSidx){

                    ALOGI("receive ddp sidx info, should start to dequeue AU now");
                    usleep(50000);//fake msg delay
                    shouldStartNow =true;
                }
                 break;
            }
        }

    }
private:
    bool shouldStartNow;

    DISALLOW_EVIL_CONSTRUCTORS(MyHandler);

};

}
int main(int argc, char **argv) {
    using namespace android; 
    ALOGI("ptest sdcard test.mp4");

	ProcessState::self()->startThreadPool();

	sp<ALooper> my_looper = new ALooper;
	my_looper->setName("dash_looper");

	my_looper->start(
	false /* runOnCallingThread */,
	false /* canCallJava */,
	PRIORITY_AUDIO);   

    sp<MyHandler> msgH = new MyHandler();   
    my_looper->registerHandler(msgH);

    sp<AMessage> dataInfo  = new AMessage(MyHandler::kWhatSource,msgH->id());  ;
    sp<AMessage> notify = new AMessage(MyHandler::kWhatDDPNotify,msgH->id());

    sp<AnotherPacketSource> audioSource = new AnotherPacketSource(NULL );
    sp<AnotherPacketSource> videoSource = new AnotherPacketSource(NULL) ;
    sp<AnotherPacketSource> subtitleSource= new AnotherPacketSource(NULL) ;
    CHECK(audioSource.get() != NULL);
    CHECK(videoSource.get() != NULL);
    CHECK(subtitleSource.get() != NULL);
    dataInfo->setPointer("audioSource", (audioSource.get()));  // data has audio track
    dataInfo->setPointer("videoSource", (videoSource.get()));  // data has video track
    dataInfo->setPointer("subtitleSource", subtitleSource.get());// data has subtitle track

    dataInfo->setInt32("file-Format", 0);  //keyContariner_XXX
    dataInfo->setInt32("Drm-type", 0);

    sp<DashDataProcessor> ddp = new DashDataProcessor (notify,NULL,NULL,dataInfo,0);


	ALOGI("fopen file");

	FILE* mFD= fopen("/sdcard/test.mp4", "rb"); 
	CHECK(mFD != NULL);

	ALOGI("start ddp");

    ddp->start();

	bool bEOF= false;
	bool startNow=false;
	int64_t mOffset=0;
	int byts=0;
	ALOGI("feed ddp");
    while(1){

        sp<ABuffer> buffer = new ABuffer(1024*64);
        fseek(mFD, mOffset, SEEK_SET);

        int byts =fread( buffer->data(), 1, buffer->size(), mFD); 
        buffer->setRange(0, byts);
        ALOGI("feed %d data",byts);
        if(msgH->startNow() && !startNow){
            //buffer->setInt32Data(DashDataProcessor::kBufferFlag_StartDequeueAU);
            startNow =true;
        }
        ddp->feedMoreBuffer(buffer);
        mOffset  +=byts;

        if(byts < 1024*64 ) {
        	ALOGI("feed over , break, total %lld",mOffset); 
            sp<ABuffer> buffer = new ABuffer(0);
            buffer->setRange(0, 0);
            ddp->feedMoreBuffer(buffer);
        	break;
        }

        usleep(50000);
    }

     usleep(4000000);
	 ALOGI("stop ddp");
	ddp->stop();
	ddp.clear();

    ALOGI("exit test");
    return 1;


}







