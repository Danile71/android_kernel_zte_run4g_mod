
#define LOG_TAG "UibcHandler"
#include "UibcHandler.h"

#include <utils/Log.h>
#include "UibcMessage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>



namespace android {

UibcHandler::UibcHandler()
    : m_wfdWidth(0),
      m_wfdHeight(0),
      m_localWidth(0),
      m_localHeight(0),
      m_wfdWidthScale(0.0),
      m_wfdHeightScale(0.0) {
    m_Capability = new UibcCapability();
}

UibcHandler::~UibcHandler() {

}

status_t UibcHandler::init() {
    int version;

    UibcMessage::getScreenResolution(&m_localWidth, &m_localHeight);
    return OK;
}

status_t UibcHandler::destroy() {
    return OK;
}

void UibcHandler::setWFDResolution(int width, int heigh, int screenMode) {
    int temp;

    m_wfdWidth = width;
    m_wfdHeight = heigh;

    if ((screenMode == 1) && (m_localHeight > m_localWidth)) {
        // Landscape
        temp = m_localWidth;
        m_localWidth = m_localHeight;
        m_localHeight = temp;
    } else if ((screenMode == 0) && (m_wfdHeight < m_wfdWidth)) {
        // Protrait
        temp = m_wfdWidth;
        m_wfdWidth = m_wfdHeight;
        m_wfdHeight = temp;
    }

    m_wfdWidthScale = (double)m_localWidth / (double)m_wfdWidth;
    m_wfdHeightScale = (double)m_localHeight / (double)m_wfdHeight;

    ALOGD("setWFDResolution screenMode=%d", screenMode);
    ALOGD("setWFDResolution width=%d, heigh=%d", width, heigh);
    ALOGD("setWFDResolution m_wfdWidth=%d, m_wfdHeight=%d", m_wfdWidth, m_wfdHeight);
    ALOGD("setWFDResolution m_localWidth=%d, m_localHeight=%d", m_localWidth, m_localHeight);
    ALOGD("setWFDResolution m_wfdWidthScale=%f, m_wfdHeightScale=%f", m_wfdWidthScale, m_wfdHeightScale);
}

void UibcHandler::run_consol_cmd(char *command) {
    ALOGD("run_consol_cmd:\"%s\"", command);
    FILE *fpipe;
    char line[256];
    if ( !(fpipe = (FILE*)popen(command, "r")) )
        return;

    while ( fgets( line, sizeof line, fpipe)) {
        puts(line);
    }
    pclose(fpipe);
}

void UibcHandler::setUibcEnabled(bool enabled) {
    ALOGD("setUibcEnabled enabled=%d", enabled);
    mUibcEnabled = enabled;
}

bool UibcHandler::getUibcEnabled() {
    ALOGD("getUibcEnabled mUibcEnabled=%d", mUibcEnabled);
    return mUibcEnabled;
}

void UibcHandler::parseRemoteCapabilities(AString capStr) {
    ALOGD("parseRemoteCapabilities");
    m_Capability->parseRemoteCapabilities(capStr);
}

const char* UibcHandler::getLocalCapabilities() {
    ALOGD("getLocalCapabilities");
    return m_Capability->getLocalCapabilities();
}

const char* UibcHandler::getSupportedCapabilities() {
    ALOGD("getSupportedCapabilities");
    return m_Capability->getSupportedCapabilities();
}

int UibcHandler::getPort() {
    ALOGD("getPort");
    return m_Capability->getPort();
}

bool UibcHandler::isUibcSupported() {
    ALOGD("isUibcSupported");
    return m_Capability->isUibcSupported();
}

bool UibcHandler::isGenericSupported(int devType) {
    ALOGD("isGenericSupported");
    return m_Capability->isGenericSupported(devType);
}

bool UibcHandler::isHidcSupported(int devType, int path) {
    ALOGD("isHidcSupported");
    return m_Capability->isHidcSupported(devType, path);
}

}
