#define LOG_TAG "stresstool"

#include "Frame.h"

#include <utils/Log.h>

Frame::Frame(const Config& cfg)
    :   mCfg(cfg) {
    FILE* fp = fopen(mCfg.mPath.string(), "rb");
    if (fp == NULL) {
        fprintf(stderr, "fopen(%s) failed %s\n", mCfg.mPath.string(), strerror(errno));
        return ;
    }

    mData = new char[mCfg.mPitch * mCfg.mH];
    if (mData == NULL) {
        ALOGE("Frame::Frame()- allocating data failed");
    }
    if (fread(mData, 1, mCfg.mPitch * mCfg.mH, fp) != mCfg.mPitch * mCfg.mH) {
        ALOGE("Frame::Frame()- fread(%s) size:%d failed", mCfg.mPath.string(), mCfg.mPitch * mCfg.mH);
        return ;
    }
    fclose(fp);
}
