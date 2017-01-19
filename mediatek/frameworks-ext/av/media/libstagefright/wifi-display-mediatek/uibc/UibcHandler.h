
#ifndef UIBC_HANDLER_H
#define UIBC_HANDLER_H

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>
#include <linux/input.h>

#include "UibcCapability.h"

#define UIBC_ENABLE_COMMAND "wfd_uibc_setting: enable\r\n"
#define UIBC_DISABLE_COMMAND "wfd_uibc_setting: disable\r\n"

namespace android {

struct UibcHandler {
    UibcHandler();
    status_t init();
    status_t destroy();

    void setWFDResolution(int width, int heigh, int screenMode);
    void run_consol_cmd(char *command);

    void setUibcEnabled(bool enabled);
    bool getUibcEnabled();


    void parseRemoteCapabilities(AString capStr);
    const char* getLocalCapabilities();
    const char* getSupportedCapabilities();
    int getPort();
    bool isUibcSupported();
    bool isGenericSupported(int devType);
    bool isHidcSupported(int devType, int path);

protected:
    virtual ~UibcHandler();
    int m_wfdWidth;
    int m_wfdHeight;
    int m_localWidth;
    int m_localHeight;
    double m_wfdWidthScale;
    double m_wfdHeightScale;

    bool mUibcEnabled;
    sp<UibcCapability> m_Capability;

private:

    DISALLOW_EVIL_CONSTRUCTORS(UibcHandler);
};

}

#endif
