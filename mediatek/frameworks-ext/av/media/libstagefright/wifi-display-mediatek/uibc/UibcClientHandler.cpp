

#define LOG_TAG "UibcClientHandler"
#include <utils/Log.h>

#include "UibcClientHandler.h"
#include "WifiDisplayUibcType.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/hexdump.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 5096

namespace android {

int UibcClientHandler::mStopListenHidc = 0;
pthread_mutex_t UibcClientHandler::mStopHidcMutex;

UibcClientHandler::UibcClientHandler(   sp<ANetworkSession> netSession)
    : mSessionID(0),
      mNetSession(netSession) {
}

UibcClientHandler::~UibcClientHandler() {
}

status_t UibcClientHandler::init() {
    int version;

    UibcHandler::init();
    return OK;
}

status_t UibcClientHandler::destroy() {

    return OK;
}

status_t UibcClientHandler::sendUibcMessage(        sp<ANetworkSession> netSession,
        UibcMessage::MessageType type,
        const char *eventDesc) {
    status_t err;
    UibcMessage* message = new UibcMessage(type, eventDesc, m_wfdWidthScale, m_wfdHeightScale);
    if (message != NULL && message->isDataValid()) {
        ALOGI("sendUibcMessage Sending msg");
        err = netSession->sendDirectRequest(
                  mSessionID, message->getPacketData(), message->getPacketDataLen());
    }
    delete message;
    return err;
}

void UibcClientHandler::setSessionID(int32_t SessionID) {
    ALOGI("setSessionID (%d)", SessionID);
    mSessionID = SessionID;
}

int32_t UibcClientHandler::getSessionID() {
    ALOGI("getSessionID (%d)", mSessionID);
    return mSessionID;
}

int sendlen, receivelen;
int received = 0;
char buffer[BUFFSIZE];
struct sockaddr_in receivesocket;
struct sockaddr_in sendsocket;
int sock;
int i;
int ret = 0;
int SessionID;
sp<ANetworkSession> netSession;


void* hidcThread(void *arg) {
    ALOGI("[UibcClientHandler] hidcThread [+]");
    char* uibc_out;
    /* Create the UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        return NULL;
    }

    static bool appleMagicConn = false;
    /* my address */
    memset(&receivesocket, 0, sizeof(receivesocket));
    receivesocket.sin_family = AF_INET;
    receivesocket.sin_addr.s_addr = htonl(INADDR_ANY);
    receivesocket.sin_port = htons(9999);

    receivelen = sizeof(receivesocket);
    if (bind(sock, (struct sockaddr *) &receivesocket, receivelen) < 0) {
        ALOGE("bind");
        return NULL;
    }

    /* kernel address */
    memset(&sendsocket, 0, sizeof(sendsocket));
    sendsocket.sin_family = AF_INET;
    sendsocket.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendsocket.sin_port = htons(5555);

    /* Send message to the server */
    memcpy(buffer, "Start UIBC HIDC", strlen("Start UIBC HIDC") + 1);
    sendlen = strlen(buffer) + 1;

    if (sendto(sock, buffer, sendlen, 0, (struct sockaddr *) &sendsocket, sizeof(sendsocket)) != sendlen) {
        ALOGE("sendto");
        return NULL;
    }

    memset(buffer, 0, BUFFSIZE);
    if ((received = recvfrom(sock, buffer, BUFFSIZE, 0, NULL, NULL)) < 0) {
        ALOGE("Command recvfrom");
        return NULL;
    }
    buffer[BUFFSIZE - 1] = 0x0;
    ALOGD("Command received: %s\n", buffer);

    while (UibcClientHandler::isListenHidcStop() != 1) {
        memset(buffer, 0, BUFFSIZE);
        if ((received = recvfrom(sock, buffer, BUFFSIZE, 0, NULL, NULL)) < 0) {
            ALOGE("recvfrom");
            return NULL;
        }
        ALOGD("message received: %d\n", received);
        hexdump(buffer, received);
        buffer[BUFFSIZE - 1] = 0x0;

        if (strstr(buffer, "Stop UIBC HIDC") != 0x00) {
            ALOGD("Command received: %s\n", buffer);
            break;
        }

        int messagetype;
        int hidType;
        int busId;
        int size;

        memcpy (&messagetype, &buffer[0], sizeof(int));
        memcpy (&hidType, &buffer[4], sizeof(int));
        memcpy (&busId, &buffer[8], sizeof(int));
        memcpy (&size, &buffer[12], sizeof(int));

        ALOGD("message messagetype: %d, hidType: %d, busId: %d, size: %d\n",
              messagetype, hidType, busId, size);

        int totalLen = size + 9; // 4 + 5 + hidc

        uibc_out = (char*)malloc(totalLen);

        // UIBC header
        uibc_out[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
        uibc_out[1] = 0x10; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
        uibc_out[2] = (totalLen >> 8) & 0xFF; //Length(16 bits)
        uibc_out[3] = totalLen & 0xFF; //Length(16 bits)

        if (busId == 3) {
            uibc_out[4] = 1;
        } else if (busId == 5) {
            uibc_out[4] = 2;
        }

        switch (hidType) {
        case 0: // Keyboard
            uibc_out[5] = 0;
            break;
        case 1://Mouse
            uibc_out[5] = 1;
            // Apple MagicPad
            if (size > 0x44 && 
               buffer[0x54] == 0x02 && 
               buffer[0x55] == 0xFF) {
               uibc_out[5] = 3;
               appleMagicConn = true;
            }
            if (appleMagicConn)
               uibc_out[5] = 3;
            break;
        }

        switch (messagetype) {
        case 0: // Keyboard
            uibc_out[6] = 0;
            break;
        case 1://Mouse
            uibc_out[6] = 1;
            break;

        }
        uibc_out[7] = (size >> 8) & 0xFF;
        uibc_out[8] = size & 0xFF;
        memcpy (&uibc_out[9], &buffer[16], size);
        ALOGI("sendUibcHidcMessage Sending msg");
        hexdump(uibc_out, totalLen);
        netSession->sendDirectRequest(
            SessionID, uibc_out, totalLen);
        free(uibc_out);
    }
    appleMagicConn = false;
    close(sock);

    ALOGD("[UibcClientHandler] hidcThread [-]");
    return NULL;
}


status_t UibcClientHandler::startListenHidc() {
    status_t err;
    pthread_t tid;

    mStopListenHidc = 0;
    SessionID = mSessionID;
    netSession = mNetSession;
    err = pthread_create(&tid, NULL, &hidcThread, (void *)NULL);
    if (err != 0) {
        printf("\ncan't create listenHidc thread :[%s]", strerror(err));
        return -1;
    } else {
        printf("\n Thread created listenHidc successfully\n");
    }

    return err;
}

status_t UibcClientHandler::stopListenHidc() {
    pthread_mutex_lock(&mStopHidcMutex);
    mStopListenHidc = 1;
    pthread_mutex_unlock(&mStopHidcMutex);

    /* Send message to the server */
    memcpy(buffer, "Stop UIBC HIDC", strlen("Stop UIBC HIDC") + 1);
    sendlen = strlen(buffer) + 1;

    if (sendto(sock, buffer, sendlen, 0, (struct sockaddr *) &sendsocket, sizeof(sendsocket)) != sendlen) {
        ALOGE("sendto");
        return -1;
    }

    return 0;
}

int UibcClientHandler::isListenHidcStop() {
    int ret = 0;
    pthread_mutex_lock(&mStopHidcMutex);
    ret = mStopListenHidc;
    pthread_mutex_unlock(&mStopHidcMutex);
    return ret;
}
}
