

#define LOG_TAG "UibcServerHandler"
#include <utils/Log.h>

#include "UibcMessage.h"
#include "UibcServerHandler.h"
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
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <linux/input.h>

#include <binder/IServiceManager.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/DisplayInfo.h>
#include <gui/ISurfaceComposer.h>
#include <cutils/properties.h>

#include <ctype.h>
#include <sys/poll.h>
#include <pthread.h>
#include <linux/uhid.h>
#include <linux/input.h>

namespace android {

#define BTN_TOUCH   0x14a

#define WFD_UIBC_HIDC_USAGE_REPORT_INPUT        0x00
#define WFD_UIBC_HIDC_USAGE_REPORT_DESCRIPTOR   0x01

#define HID_LEFTSHIFT_BIT                       0x02
#define HID_RIGHTSHIFT_BIT                      0x20

#define UIBC_KBD_NAME  "uibc"
#define UIBC_KEY_PRESS      1
#define UIBC_KEY_RELEASE    0
#define UIBC_KEY_RESERVE	2
#define UIBC_POINTER_X	    3
#define UIBC_POINTER_Y	    4
#define UIBC_KEYBOARD	    5
#define UIBC_MOUSE	        6
#define UIBC_TOUCH_DOWN		7
#define UIBC_TOUCH_UP		8
#define UIBC_TOUCH_MOVE		9
#define UIBC_KEYBOARD_MIRACAST		10

#define MAX_POINTERS 5

uint8_t default_KB_Desc[] = {0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07,
                             0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
                             0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
                             0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01,
                             0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02,
                             0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06,
                             0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07,
                             0x19, 0x00, 0x29, 0x65, 0x81, 0x00, 0xC0
                            };

uint8_t default_Mouse_Desc[] = {0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 0x09, 0x01,
                                0xA1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03,
                                0x15, 0x00, 0x25, 0x01, 0x95, 0x03, 0x75, 0x01,
                                0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x01,
                                0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81,
                                0x25, 0x7F, 0x75, 0x08, 0x95, 0x02, 0x81, 0x06,
                                0xC0, 0xC0
                               };

UibcServerHandler::UibcServerHandler(sp<IRemoteDisplayClient> remoteClient)
    : mUibc_kbd_fd(-1),
      m_XCurCoord(-1),
      m_YCurCoord(-1),
      m_ShiftPressed(false),
      m_touchDown(false),
      m_mouseDown(false),
      m_deltaX(0),
      m_deltaY(0),
      m_XOffset(0),
      m_YOffset(0),
      m_XRevert(false),
      m_YRevert(false),
      m_XYSwitch(false),
      m_mouseCursorSupported(false),
      m_Orientation(-1),
      mRemoteClient(remoteClient),
      m_GenericDriverInited(false),
      m_hidcDefDescTest(false) {
    int i, j;
    for (int i = 0 ; i < 8 ; i++) {
        for (int j = 0 ; j < 5 ; j++) {
            if(m_uhidFd[i][j] > 0) {
                m_uhidFd[i][j] = -1;
            }
        }
    }
}

UibcServerHandler::~UibcServerHandler() {

}

status_t UibcServerHandler::init() {
    ALOGD("init()");
    int version;

    UibcHandler::init();

    char val[PROPERTY_VALUE_MAX];
    if (property_get("media.wfd.uibc-mouse-cursor", val, NULL)) {
        int value = atoi(val);
        if(value == 1) {
            ALOGD("media.wfd.uibc-mouse-cursor:%s", val);
            m_mouseCursorSupported = true;
        }
    }

    if (property_get("media.wfd.source.uibc-def-desc", val, NULL)) {
        ALOGD("media.wfd.source.uibc-default-hidc-desc:%s", val);
        int value = atoi(val);
        if (value > 0) {
            m_hidcDefDescTest = true;
        } else {
            m_hidcDefDescTest = false;
        }
    }
    return OK;
}

status_t UibcServerHandler::destroy() {

    if(mUibc_kbd_fd >= 0) {
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
    }

    int i, j;
    for (int i = 0 ; i < 8 ; i++) {
        for (int j = 0 ; j < 5 ; j++) {
            if(m_uhidFd[i][j] > 0) {
                close(m_uhidFd[i][j]);
                m_uhidFd[i][j] = -1;
            }
        }
    }
    return OK;
}

status_t UibcServerHandler::initGenericDrivers() {
    ALOGI("initGenericDrivers()");
    bool bKeyboardSupported = m_Capability->isGenericSupported(DEVICE_TYPE_KEYBOARD);
    bool bTouchSupported = m_Capability->isGenericSupported(DEVICE_TYPE_SINGLETOUCH);
    bool bMouseSupported = m_Capability->isGenericSupported(DEVICE_TYPE_MOUSE);

    ALOGD("initGenericDrivers() bKeyboardSupported=%d,bTouchSupported=%d,bMouseSupported=%d",
          bKeyboardSupported, bTouchSupported, bMouseSupported);

    if(mUibc_kbd_fd >= 0) {
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
    }
    mUibc_kbd_fd = open(UIBC_KBD_DEV_PATH, O_WRONLY);
    if(mUibc_kbd_fd < 0) {
        ALOGE("open UIBC driver error");
        return -1;
    }

    if (bKeyboardSupported ||
        bTouchSupported ||
        bMouseSupported) {
        if (ioctl(mUibc_kbd_fd, UIBC_KEYBOARD, 0) < 0) {
            ALOGE("Initial uibc ioctl failed!");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }
    return OK;
}

status_t UibcServerHandler::handleUIBCMessage(const sp<ABuffer> &buffer) {
    ALOGD("handleUIBCMessage mUibcEnabled=0x%04X", mUibcEnabled);

    if (mUibcEnabled == UIBC_DISABLED)
        return OK;

    size_t size = buffer->size();
    size_t payloadOffset = 0;
    UIBCInputCategoryCode  inputCategory = WFD_UIBC_INPUT_CATEGORY_UNKNOWN;

    if(size < UIBC_HEADER_SIZE) {
        ALOGE("The size of UIBC message is less than header size");
        return ERROR_MALFORMED;
    }

    const uint8_t *data = buffer->data();

    //Skip the timestamp
    bool hasTimeStamp = data[0] & UIBC_TIMESTAMP_MASK;
    if(hasTimeStamp) {
        payloadOffset = UIBC_HEADER_SIZE + UIBC_TIMESTAMP_SIZE;
    } else {
        payloadOffset = UIBC_HEADER_SIZE;
    }

    if (size < payloadOffset) {
        // Not enough data to fit the basic header
        ALOGE("Not enough data to fit the basic header");
        return ERROR_MALFORMED;
    }

    buffer->setRange(payloadOffset, size - payloadOffset);

    inputCategory = (UIBCInputCategoryCode) (data[1] >> 4);

    switch(inputCategory) {
    case WFD_UIBC_INPUT_CATEGORY_GENERIC:
        /* if ((mRemote_InputCat & INPUT_CAT_GENERIC) == UIBC_NONE) {
             ALOGD("INPUT_CAT_GENERIC not supported.");
             return OK;
         }*/

        handleGenericInput(buffer);
        break;
    case WFD_UIBC_INPUT_CATEGORY_HIDC:
        if (!m_Capability->isHidcSupported(0, 0)) {
            ALOGD("INPUT_CAT_HIDC not supported.");
            return OK;
        }

        handleHIDCInput(buffer);
        break;
    default:
        ALOGE("Uknown input category:%d", inputCategory);
        break;
    }

    return OK;
}

status_t UibcServerHandler::handleGenericInput(const sp<ABuffer> &buffer) {
    size_t size = buffer->size();
    WFD_UIBC_GENERIC_BODY_FORMAT_HDR *pHdr = (WFD_UIBC_GENERIC_BODY_FORMAT_HDR*) buffer->data();
    UINT16 bodyLength = ntohs(pHdr->length);

    if (size < bodyLength) {
        ALOGE("Error: not enough space for a complete generic body:%d", bodyLength);
        return ERROR;
    }

    //ALOGV("handleGenericInput with IE:%d", pHdr->ieID);

    if (!m_GenericDriverInited) {
        if (initGenericDrivers() == OK) {
            m_GenericDriverInited = true;
        }
    }

    switch (pHdr->ieID) {
    case WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN:
    case WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP:
    case WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE: {
        WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH *pBody = (WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH*)pHdr;
        ALOGD("uibc (Move,Down,Up) ieID: %d, numptr: %d",
              pBody->ieID,
              pBody->numPointers);

        if ((pBody->numPointers > 1) &&
            !m_Capability->isGenericSupported(DEVICE_TYPE_MULTITOUCH)) {
            ALOGD("GENERIC_MULTITOUCH not supported.");
            return OK;
        }
        sendMultipleTouchEvent(pBody);
        break;
    }
    case WFD_UIBC_GENERIC_IE_ID_KEY_DOWN:
    case WFD_UIBC_GENERIC_IE_ID_KEY_UP: {
        if (!m_Capability->isGenericSupported(DEVICE_TYPE_KEYBOARD))
            return OK;

        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_KEY)) {
            WFD_UIBC_GENERIC_BODY_KEY *pBody = (WFD_UIBC_GENERIC_BODY_KEY*)pHdr;
            ALOGD("uibc (Key,Down,Up) ieID: %d, code1: %d, code2: %d",
                  pBody->ieID,
                  ntohs(pBody->code1),
                  ntohs(pBody->code2));
            int isDown =   ( (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_KEY_DOWN) ? 1 : 0  );
            if( ntohs(pBody->code1) > 0) {
                sendKeyEvent(ntohs(pBody->code1), isDown);
            }

            if(ntohs(pBody->code2) > 0) {
                sendKeyEvent(ntohs(pBody->code2), isDown);
            }
        }
        break;
    }
    case WFD_UIBC_GENERIC_IE_ID_ZOOM:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_ZOOM)) {
            WFD_UIBC_GENERIC_BODY_ZOOM *pBody = (WFD_UIBC_GENERIC_BODY_ZOOM*)pHdr;
            ALOGD("uibc (ZOOM) ieID: %d, x: %d, y: %d, itimes: %d, ftimes: %d",
                  pBody->ieID,
                  pBody->x,
                  pBody->y,
                  pBody->intTimes,
                  pBody->fractTimes);
        }
        break;
    case WFD_UIBC_GENERIC_IE_ID_VSCROLL:
    case WFD_UIBC_GENERIC_IE_ID_HSCROLL:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_SCROLL)) {
            WFD_UIBC_GENERIC_BODY_SCROLL *pBody = (WFD_UIBC_GENERIC_BODY_SCROLL*)pHdr;
            ALOGD("uibc (SCROLL V/H) ieID: %d, amount: %d",
                  pBody->ieID,
                  pBody->amount);
        }
        break;
    case WFD_UIBC_GENERIC_IE_ID_ROTATE:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_ROTATE)) {
            WFD_UIBC_GENERIC_BODY_ROTATE *pBody = (WFD_UIBC_GENERIC_BODY_ROTATE*)pHdr;
            ALOGD("uibc (ROTATE V/H) ieID: %d, iamount: %d, famount: %d",
                  pBody->ieID,
                  pBody->intAmount,
                  pBody->fractAmount);
            break;
        }
    case WFD_UIBC_GENERIC_IE_ID_VENDOR_SPECIFIC:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_VENDOR_KEY)) {
            WFD_UIBC_GENERIC_BODY_VENDOR_KEY *pBody = (WFD_UIBC_GENERIC_BODY_VENDOR_KEY*)pHdr;
            ALOGD("uibc (VENDOR KEY) ieID: %d, keyAction: %d, scanCode1: %d, scanCode2: %d",
                  pBody->ieID,
                  pBody->keyAction,
                  pBody->scanCode1,
                  pBody->scanCode2);
            int isDown = ((pBody->keyAction == WFD_UIBC_GENERIC_IE_ID_KEY_DOWN) ? 1 : 0  );
            if( ntohs(pBody->scanCode1) > 0) {
                sendKeyScanCode(ntohs(pBody->scanCode1), isDown);
            }
            if(ntohs(pBody->scanCode2) > 0) {
                sendKeyScanCode(ntohs(pBody->scanCode2), isDown);
            }
            break;
        }
    default:
        ALOGE("Unknown User input for generic type");
        break;
    }

    return OK;
}

status_t UibcServerHandler::handleHIDCInput(const sp<ABuffer> &buffer) {
    const uint8_t *data = buffer->data();
    //ALOGI("handleHIDCInput raw buffer:");
    //hexdump(buffer->data(), buffer->size());

    size_t bufferSize = buffer->size();
    size_t payloadOffset = 0;

    if (bufferSize < sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR)) {
        ALOGE("Error: not enough space for a complete HIDC header:%d", bufferSize);
        return ERROR;
    }

    WFD_UIBC_HIDC_BODY_FORMAT_HDR *pHdr = (WFD_UIBC_HIDC_BODY_FORMAT_HDR*) buffer->data();
    UINT16 bodyLength = ntohs(pHdr->length);

    if (bufferSize < bodyLength + sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR)) {
        ALOGE("Error: not enough space for a complete HIDC body:%d", bufferSize);
        return ERROR;
    }

    ALOGD("handleHIDCInput with Info:(%d:%d:%d:%d)", pHdr->inputPath, pHdr->hidType, pHdr->usage, bodyLength);

    //Skip the timestamp
    bool hasTimeStamp = data[0] & UIBC_TIMESTAMP_MASK;
    if(hasTimeStamp) {
        payloadOffset = UIBC_HEADER_SIZE + UIBC_TIMESTAMP_SIZE;
    } else {
        payloadOffset = UIBC_HEADER_SIZE;
    }

    payloadOffset += sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR);
    buffer->setRange(payloadOffset, bufferSize - sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR)); // Move the buffer to the HIDC body part
    uint8_t* pBuffer = buffer->data();


    ALOGD("handleHIDCInput buffer->size()=%d, buffer:",  buffer->size());
    hexdump(pBuffer, buffer->size());

    if (pHdr->hidType > 7)
        return ERROR;

    if (pHdr->usage == WFD_UIBC_HIDC_USAGE_REPORT_DESCRIPTOR) {
        // A "HID report descriptor"
        if (m_uhidFd[pHdr->hidType][pHdr->inputPath] <= 0) {
            m_uhidFd[pHdr->hidType][pHdr->inputPath] = open(UIBC_UHID_DEV_PATH, O_RDWR | O_CLOEXEC);
        } else {
            close(m_uhidFd[pHdr->hidType][pHdr->inputPath]);
            m_uhidFd[pHdr->hidType][pHdr->inputPath] = open(UIBC_UHID_DEV_PATH, O_RDWR | O_CLOEXEC);
        }
        if(m_uhidFd[pHdr->hidType][pHdr->inputPath] < 0) {
            ALOGE("m_uhidFd %d error", pHdr->hidType);
            return ERROR;
        }
        if (m_hidcDefDescTest && pHdr->hidType == 0x00) {
            hidha_uhid_create(&m_uhidFd[pHdr->hidType][pHdr->inputPath], "MTK UIBC HID",
                              0x08ED, 0x03, 0, 0, pHdr->inputPath, sizeof(default_KB_Desc), &default_KB_Desc[0]);
        } else if (m_hidcDefDescTest && pHdr->hidType == 0x01) {
            hidha_uhid_create(&m_uhidFd[pHdr->hidType][pHdr->inputPath], "MTK UIBC HID",
                              0x08ED, 0x03, 0, 0, pHdr->inputPath, sizeof(default_Mouse_Desc), &default_Mouse_Desc[0]);
        } else if (pHdr->hidType == 0x03 && 
                   pBuffer[0x44] == 0x02 && 
                   pBuffer[0x45] == 0xFF) {
            // Apple MagicPad
            hidha_uhid_create(&m_uhidFd[pHdr->hidType][pHdr->inputPath], "Apple MagicPad",
                              0x05AC, 0x030E, 0, 0, pHdr->inputPath, buffer->size(), pBuffer);
        } else {
            hidha_uhid_create(&m_uhidFd[pHdr->hidType][pHdr->inputPath], "MTK UIBC HID",
                              0x08ED, 0x03, 0, 0, pHdr->inputPath, buffer->size(), pBuffer);
        }
    } else if (pHdr->usage == WFD_UIBC_HIDC_USAGE_REPORT_INPUT) {
        // A "HID report"
        if (m_uhidFd[pHdr->hidType][pHdr->inputPath] <= 0) {
            if (pHdr->hidType == 0x00) {
                hidha_uhid_create(&m_uhidFd[pHdr->hidType][pHdr->inputPath], "MTK UIBC HID",
                                  0x08ED, 0x03, 0, 0, pHdr->inputPath, sizeof(default_KB_Desc), &default_KB_Desc[0]);
            } else if (pHdr->hidType == 0x01) {
                hidha_uhid_create(&m_uhidFd[pHdr->hidType][pHdr->inputPath], "MTK UIBC HID",
                                  0x08ED, 0x03, 0, 0, pHdr->inputPath, sizeof(default_Mouse_Desc), &default_Mouse_Desc[0]);
            }
        }
        if(m_uhidFd[pHdr->hidType][pHdr->inputPath] > 0) {
            hidha_uhid_input(m_uhidFd[pHdr->hidType][pHdr->inputPath], pBuffer, buffer->size());
        }
    }

    return OK;
}

status_t UibcServerHandler::sendKeyEvent(UINT16 code, int isPress) {
    scanCodeBuild_t scanCodeBuild;
    uint16_t scenCodeShift = KEY_LEFTSHIFT;
    uint16_t scenCodeAlt = KEY_LEFTALT;

    if (code < 0x20 || code == 0x7F/*DEL*/) {
        if (code == 0x7F) {
            scanCodeBuild.scanCode = KEY_DELETE;
        } else {
            scanCodeBuild = UibcMessage::asciiToScancodeBuild((char)code);
        }
        // Send the target ascii character
        if (isPress) {
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scanCodeBuild.scanCode) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
        } else {
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scanCodeBuild.scanCode) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
        }
    } else {
        if(isPress == 1 && mRemoteClient != NULL) {
            ALOGD("sendKeyEvent: %d / %d", code, isPress);
            mRemoteClient->onDisplayKeyEvent(code, isPress);
        }
    }
    return OK;
}

status_t UibcServerHandler::sendKeyScanCode(UINT16 code, int isPress) {
    if (isPress) {
        if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &code) < 0) {
            ALOGE("sendKeyEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    } else {
        if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &code) < 0) {
            ALOGE("sendKeyEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }

    return OK;
}

void UibcServerHandler::updateScreenMode() {
    double localRatio;
    double wfdRatio;
    double wfdScale;

    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
                              ISurfaceComposer::eDisplayIdMain);

    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);

    if (m_Orientation == info.orientation)
        return;

    m_Orientation = info.orientation;

    if (info.orientation == DISPLAY_ORIENTATION_90 ||
        info.orientation == DISPLAY_ORIENTATION_270) {
        m_localWidth = info.h;
        m_localHeight = info.w;
    } else {
        m_localWidth = info.w;
        m_localHeight = info.h;
    }

    m_wfdWidthScale = (double)m_localWidth / (double)m_wfdWidth;
    m_wfdHeightScale = (double)m_localHeight / (double)m_wfdHeight;

    localRatio = (double)m_localWidth / (double)m_localHeight;
    wfdRatio = (double)m_wfdWidth / (double)m_wfdHeight;

    if (localRatio > wfdRatio) {
        wfdScale = m_wfdHeightScale = m_wfdWidthScale;
    } else {
        wfdScale = m_wfdWidthScale = m_wfdHeightScale;
    }

    m_XOffset = ((m_wfdWidth - (int)((float)m_localWidth / wfdScale)) / 2);
    m_YOffset = ((m_wfdHeight - (int)((float)m_localHeight / wfdScale)) / 2);


    switch (info.orientation) {
    case DISPLAY_ORIENTATION_0:
        m_XYSwitch = false;
        m_XRevert = false;
        m_YRevert = false;
        break;
    case DISPLAY_ORIENTATION_90:
        m_XYSwitch = true;
        m_XRevert = true;
        m_YRevert = false;
        break;
    case DISPLAY_ORIENTATION_180:
        m_XYSwitch = false;
        m_XRevert = true;
        m_YRevert = true;
        break;
    case DISPLAY_ORIENTATION_270:
        m_XYSwitch = true;
        m_XRevert = false;
        m_YRevert = true;
        break;
    default:
        break;
    }
    ALOGD("uibc touch info: m_localWidth:%d, m_localHeight:%d, " \
          "m_wfdWidth:%d, m_wfdHeight:%d, ",
          m_localWidth, m_localHeight,
          m_wfdWidth, m_wfdHeight);

    ALOGD("uibc touch info: localRatio:%f, wfdRatio:%f, " \
          "m_wfdWidthScale:%f, m_wfdHeightScale:%f, " \
          "m_XOffset:%d, m_YOffset:%d, ",
          localRatio, wfdRatio,
          m_wfdWidthScale, m_wfdHeightScale,
          m_XOffset, m_YOffset);
}

bool UibcServerHandler::transTouchToSourcePosition(short * x, short * y) {
    ALOGD("uibc XY trans+: x:%d, y:%d", *x, *y);
    short tmp;
    // in the black part
    if ((m_XOffset > 0) &&
        (*x < m_XOffset ||
         *x > (m_wfdWidth - m_XOffset))) {
        return false;
    }
    // in the black part
    if ((m_YOffset > 0) &&
        (*y < m_YOffset ||
         *y > (m_wfdHeight - m_YOffset))) {
        return false;
    }

    *x -= m_XOffset;
    *y -= m_YOffset;
    //ALOGD("transTouchToSourcePosition remove offset: * x:%d, * y:%d", *x, *y);

    *x *= m_wfdWidthScale;
    *y *= m_wfdHeightScale;
    //ALOGD("transTouchToSourcePosition map to source: * x:%d, * y:%d", *x, *y);

    if (m_XYSwitch) {
        tmp = *x;
        *x = *y;
        *y = tmp;
    }
    //ALOGD("transTouchToSourcePosition XY switch: * x:%d, * y:%d", *x, *y);

    if (m_XRevert) {
        if (m_XYSwitch)
            *x = m_localHeight - *x;
        else
            *x = m_localWidth - *x;
    }

    if (m_YRevert) {
        if (m_XYSwitch)
            *y = m_localWidth - *y;
        else
            *y = m_localHeight - *y;
    }
    ALOGD("uibc XY trans-: x:%d, y:%d", *x, *y);
    return true;
}

status_t UibcServerHandler::sendMultipleTouchEvent(WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH * pBody) {
    //ALOGD("sendMultipleTouchEvent ieID: %d, numptr: %d, m_touchDown = %d",
    //      pBody->ieID,
    //      pBody->numPointers,
    //      m_touchDown);
    WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH* pHdr = (WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH *)pBody;
    short touchPosition[16] = {0};
    int TouchPressed = 0;
    short deltaX = 0, deltaY = 0;
    short x, y;
    uint16_t scenCodeMouse = BTN_MOUSE;
    bool m_touchSupported = m_Capability->isGenericSupported(DEVICE_TYPE_SINGLETOUCH);
    bool m_mouseSupported = m_Capability->isGenericSupported(DEVICE_TYPE_MOUSE);

    if (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN && !m_mouseDown && !m_touchDown) {
        touchPosition[0] = (short)pBody->numPointers;
        //ALOGD("TOUCH_DOWN: pointerID:%d x:%d y:%d", pBody->coordinates[0].pointerID,
        //ntohs(pBody->coordinates[0].x), ntohs(pBody->coordinates[0].y));
        x = ntohs(pBody->coordinates[0].x);
        y = ntohs(pBody->coordinates[0].y);

        if (m_XCurCoord == x && m_YCurCoord == y) {
            if (m_mouseCursorSupported) {
                m_mouseDown = true;
                if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scenCodeMouse) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            } else if (m_mouseSupported) {
                m_touchDown = true;
                updateScreenMode();
                if (!transTouchToSourcePosition(&x, &y))
                    return -1;
                touchPosition[1] = (short)pBody->coordinates[0].pointerID;
                touchPosition[2] = x;
                touchPosition[3] = y;

                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_DOWN, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
        } else if (!(m_XCurCoord == x && m_YCurCoord == y)) {
            m_touchDown = true;
            if (m_touchSupported) {
                updateScreenMode();
                if (!transTouchToSourcePosition(&x, &y))
                    return -1;
                touchPosition[1] = (short)pBody->coordinates[0].pointerID;
                touchPosition[2] = x;
                touchPosition[3] = y;

                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_DOWN, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
        }
    } else  if (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP) {
        if (m_mouseDown) {
            if (m_mouseCursorSupported) {
                if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scenCodeMouse) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
            m_mouseDown = false;
        } else if (m_touchDown) {
            if (m_touchSupported || m_mouseSupported) {
                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_UP, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
            m_touchDown = false;
        }
    } else  if (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE)  {
        if (m_touchDown) {
            touchPosition[0] = (short)pBody->numPointers;
            for(int i = 0; i < pBody->numPointers && i < (short)pBody->numPointers; i++) {
                //ALOGD("TOUCH_MOVE %dth: pointerID:%d x:%d y:%d", i, pBody->coordinates[i].pointerID,
                //ntohs(pBody->coordinates[i].x), ntohs(pBody->coordinates[i].y));
                x = ntohs(pBody->coordinates[i].x);
                y = ntohs(pBody->coordinates[i].y);
                touchPosition[i * 3 + 1] = (short)pBody->coordinates[i].pointerID;
                touchPosition[i * 3 + 2] = x;
                touchPosition[i * 3 + 3] = y;
                if (!transTouchToSourcePosition(&touchPosition[i * 3 + 2], &touchPosition[i * 3 + 3]))
                    return -1;
            }
            if (m_touchSupported || m_mouseSupported) {
                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_MOVE, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
        } else {
            if (pBody->numPointers == 1) {
                x = ntohs(pBody->coordinates[0].x);
                y = ntohs(pBody->coordinates[0].y);
                if (m_mouseCursorSupported) {
                    if (m_XCurCoord == -1 && m_YCurCoord == -1) {
                        if (m_Orientation == DISPLAY_ORIENTATION_90 ||
                            m_Orientation == DISPLAY_ORIENTATION_270) {
                            m_XCurCoord = m_wfdHeight / 2;
                            m_YCurCoord = m_wfdWidth / 2;
                        } else {
                            m_XCurCoord = m_wfdWidth / 2;
                            m_YCurCoord = m_wfdHeight / 2;
                        }
                    }
                    deltaX = (x - m_XCurCoord);
                    deltaY = (y - m_YCurCoord);
                    // Align the cursor to the boundary
                    if (deltaX == 0 && deltaY != 0) {
                        if (x < 3) {
                            deltaX = -1280;
                        } else  if ((m_wfdWidth - x) < 3 ) { // x == m_wfdWidth
                            deltaX = 1280;
                        }
                    }
                    if (deltaY == 0 && deltaX != 0) {
                        if (y  < 3) {
                            deltaY = -1280;
                        } else  if ((m_wfdHeight - y) < 3 ) { // x == m_wfdHeight
                            deltaY = 1280;
                        }
                    }
                    //ALOGD("TOUCH_MOVE x:%d y:%d deltaX:%d deltaY:%d", x, y, m_deltaX, m_deltaY);
                    if (m_mouseSupported) {
                        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_X, &deltaX) < 0) {
                            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                            close(mUibc_kbd_fd);
                            mUibc_kbd_fd = -1;
                            return -1;
                        }
                        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_Y, &deltaY) < 0) {
                            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                            close(mUibc_kbd_fd);
                            mUibc_kbd_fd = -1;
                            return -1;
                        }
                    }
                    m_deltaX = deltaX;
                    m_deltaY = deltaY;
                }
                m_XCurCoord = x;
                m_YCurCoord = y;
            }
        }
    }
    return OK;
}

status_t UibcServerHandler::genericKeyboardThreadFunc() {
    uint16_t text[] = {KEY_M, KEY_I, KEY_R, KEY_A,
                       KEY_C, KEY_A, KEY_S, KEY_T
                      };
    int i;

    ALOGD("simulateKeyEvent");

    if(mUibc_kbd_fd >= 0) {
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
    }
    usleep(50000);
    mUibc_kbd_fd = open(UIBC_KBD_DEV_PATH, O_WRONLY);
    if(mUibc_kbd_fd < 0) {
        ALOGE("simulateKeyEvent error");
        return -1;
    }
    if(ioctl(mUibc_kbd_fd, UIBC_KEYBOARD_MIRACAST, 0) < 0) {
        ALOGE("simulateKeyEvent ioctl failed!");
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
        return -1;
    }
    usleep(400000);
    for (i = 0; i < sizeof(text) / sizeof(text[0]); i++) {
        ALOGD("simulateKeyEvent %d", text[i]);
        if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &text[i]) < 0) {
            ALOGE("simulateKeyEvent ioctl failed!");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
        usleep(200000);
        if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &text[i]) < 0) {
            ALOGE("simulateKeyEvent ioctl failed!");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }

    return OK;
}

// Static
void *UibcServerHandler::genericKeyboardThreadWrapper(void *me) {
    return (void *) static_cast<UibcServerHandler *>(me)->genericKeyboardThreadFunc();
}

status_t UibcServerHandler::simulateKeyEvent() {
    int err;
    pthread_t tid;

    err = pthread_create(&tid, NULL, &genericKeyboardThreadWrapper, this);
    if (err != 0) {
        ALOGE("\ncan't create thread :[%s]", strerror(err));
        return -1;
    } else {
        ALOGD("\n Thread created successfully\n");
    }

    return OK;
}

status_t UibcServerHandler::genericMouseThreadFunc() {
    int i;
    int temp;
    uint16_t scenCodeMouse = BTN_MOUSE;

    // Move to left bondary from right bondary
    for (i = 0; i < 500; i++) {
        temp = -1;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_X, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
        temp = 0;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_Y, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }

    if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scenCodeMouse) < 0) {
        ALOGE("sendKeyEvent Fail hid ioctl");
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
        return -1;
    }

    // Move to left bondary from right bondary
    for (i = 0; i < 1000; i++) {
        temp = 1;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_X, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
        temp = 0;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_Y, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }


    if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scenCodeMouse) < 0) {
        ALOGE("sendKeyEvent Fail hid ioctl");
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
        return -1;
    }


    return OK;
}

// Static
void *UibcServerHandler::genericMouseThreadWrapper(void *me) {
    return (void *) static_cast<UibcServerHandler *>(me)->genericMouseThreadFunc();
}

status_t UibcServerHandler::simulateMouseEvent() {
    int err;
    pthread_t tid;

    err = pthread_create(&tid, NULL, &genericMouseThreadWrapper, this);
    if (err != 0) {
        ALOGE("\ncan't create thread :[%s]", strerror(err));
        return -1;
    } else {
        ALOGD("\n Thread created successfully\n");
    }

    return OK;
}

/*Internal function to perform UHID write and error checking*/
int UibcServerHandler::hidha_uhid_write(int fd, const struct uhid_event * ev) {
    size_t ret;
    ret = write(fd, ev, sizeof(*ev));
    if (ret < 0) {
        int rtn = -errno;
        ALOGD("[HID]uhid_write: Cannot write to uhid:%s", strerror(errno));
        return rtn;

    } else if (ret != sizeof(*ev)) {
        ALOGD("[HID]uhid_write: Cannot write to uhid:%s", strerror(errno));
        return -EFAULT;
    }
    return 0;
}

int UibcServerHandler::hidha_uhid_input(int fd, unsigned char * rpt, unsigned short len) {
    struct uhid_event ev;
    int result;

    //hexdump(rpt, len);

    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_INPUT;
    ev.u.input.size = len;
    if(len > sizeof(ev.u.input.data)) {
        ALOGD("[HID]hidha_uhid_input: fd = %d", fd);

        return -1;
    }
    memcpy(ev.u.input.data, rpt, len);

    result = hidha_uhid_write(fd, &ev);

    if (result) {
        ALOGD("[HID]hidha_uhid_input: fail !");
    }
    return result;

}

void UibcServerHandler::hidha_uhid_create(int* fd, char * dev_name, unsigned short vendor_id, unsigned short product_id,
        unsigned short version, unsigned char ctry_code,unsigned char hidcBusId, unsigned int dscp_len, unsigned char * p_dscp) {
    int result;
    struct uhid_event ev;

    if (fd < 0) {
        ALOGD("[HID]hidha_uhid_create: Error: fd = %d", *fd);
        return;
    }
    ALOGD("[HID]hidha_uhid_create: fd = %d, name = [%s], dscp_len = %d", *fd, dev_name, dscp_len);
    ALOGD("[HID]hidha_uhid_create: vendor_id = 0x%04x, product_id = 0x%04x, version= 0x%04x, ctry_code= 0x%04x",
          vendor_id, product_id, version, ctry_code);

    //Create and send hid descriptor to kernel
    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_CREATE;
    strncpy((char*)ev.u.create.name, dev_name, sizeof(ev.u.create.name) - 1);
    ev.u.create.rd_data = p_dscp;
    ev.u.create.rd_size = dscp_len;

    switch (hidcBusId) {
    case HID_INPUT_PATH_USB:
    ev.u.create.bus = BUS_USB;
        break;
    case HID_INPUT_PATH_BLUETOOTH:
        ev.u.create.bus = BUS_BLUETOOTH;
        break;
    default:
        ev.u.create.bus = BUS_USB;
        break;
    }
    ev.u.create.vendor = vendor_id;
    ev.u.create.product = product_id;
    ev.u.create.version = version;
    ev.u.create.country = ctry_code;
    result = hidha_uhid_write(*fd, &ev);

    ALOGD("[HID]hidha_uhid_create: fd = %d, dscp_len = %d, result = %d", *fd, dscp_len, result);

    if (result) {
        ALOGD("[HID]hidha_uhid_create: Error: failed to send DSCP, result = %d", result);

        /* The HID report descriptor is corrupted. Close the driver. */
        close(*fd);
        *fd = -1;
    } else {
        ALOGD("[HID]hidha_uhid_create: success !");
    }

}

status_t UibcServerHandler::hidcKeyboardThreadFunc() {
    unsigned char descriptor[] = {0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, 0x25, 0x01,
                                  0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01,
                                  0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06,
                                  0x75, 0x08, 0x15, 0x00, 0x26, 0xa4, 0x00, 0x05, 0x07, 0x19, 0x00, 0x29, 0xa4, 0x81, 0x00, 0xc0,
                                 };

    unsigned char report[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char report_char[] = {0x17, 0x0e, 0x2c, 0x10, 0x0c, 0x15, 0x04, 0x06};
    int i, j;

    if (m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB] <= 0) {
        m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB] = open(UIBC_UHID_DEV_PATH, O_RDWR | O_CLOEXEC);
    }
    hidha_uhid_create(&m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB], "MTK UIBC HID",
                      0x08ED, 0x03, 0, 0, HID_INPUT_PATH_USB, sizeof(descriptor), &descriptor[0]);
    if(m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB] > 0) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < sizeof(report_char); i++) {
                report[2] = report_char[i];
                hidha_uhid_input(m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB], report, sizeof(report));
                usleep(20000);

                report[2] = 0x00;
                hidha_uhid_input(m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB], report, sizeof(report));
                usleep(20000);
            }
        }
        close(m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB]);
        m_uhidFd[DEVICE_TYPE_KEYBOARD][HID_INPUT_PATH_USB] = -1;
    }

    return OK;
}

// Static
void *UibcServerHandler::hidcKeyboardThreadWrapper(void *me) {
    return (void *) static_cast<UibcServerHandler *>(me)->hidcKeyboardThreadFunc();
}

status_t UibcServerHandler::simulateHidcKeyEvent() {
    int err;
    pthread_t tid;

    err = pthread_create(&tid, NULL, &hidcKeyboardThreadWrapper, this);
    if (err != 0) {
        ALOGE("\ncan't create thread :[%s]", strerror(err));
        return -1;
    } else {
        ALOGD("\n Thread created successfully\n");
    }

    return OK;

}

status_t UibcServerHandler::hidcMouseThreadFunc() {
    ALOGI("simulateHidcMouseEventThread [+]");
    unsigned char descriptor[] = {0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01,
                                  0x95, 0x03, 0x75, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x03, 0x05, 0x01, 0x09, 0x01,
                                  0xa1, 0x00, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x02, 0x81, 0x06,
                                  0xc0, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x01, 0x81, 0x06, 0xc0
                                 };
    unsigned char report[] = { 0x00, 0x00, 0x00, 0x00 };
    int i, j;

    if (m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB] <= 0) {
        m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB] = open(UIBC_UHID_DEV_PATH, O_RDWR | O_CLOEXEC);
    }
    hidha_uhid_create(&m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB], "MTK UIBC HID",
                      0x08ED, 0x03, 0, 0, HID_INPUT_PATH_USB, sizeof(descriptor), &descriptor[0]);
    if(m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB] > 0) {
        for (j = 0 ; j < 8 ; j++) {
            for (i = 0; i < 127 / 2; i++) {
                report[1] = 0xfe;
                report[2] = 0x01;
                hidha_uhid_input(m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB], report, sizeof(report));
                usleep(9000); // 0.009 s
            }
            for (i = 0; i < 127 / 2; i++) {
                report[1] = 0x01;
                report[2] = 0xfe;
                hidha_uhid_input(m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB], report, sizeof(report));
                usleep(9000); // 0.009 s
            }
        }
        close(m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB]);
        m_uhidFd[DEVICE_TYPE_MOUSE][HID_INPUT_PATH_USB] = -1;
    }
    ALOGI("simulateHidcMouseEventThread [-]");
    return NULL;
}

// Static
void *UibcServerHandler::hidcMouseThreadWrapper(void *me) {
    return (void *) static_cast<UibcServerHandler *>(me)->hidcMouseThreadFunc();
}

status_t UibcServerHandler::simulateHidcMouseEvent() {
    int err;
    pthread_t tid;

    err = pthread_create(&tid, NULL, &hidcMouseThreadWrapper, this);
    if (err != 0) {
        ALOGE("\ncan't create thread :[%s]", strerror(err));
        return -1;
    } else {
        ALOGD("\n Thread created successfully\n");
    }

    return OK;
}

}
