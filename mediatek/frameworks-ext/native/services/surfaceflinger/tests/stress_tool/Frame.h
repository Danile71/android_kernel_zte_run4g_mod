#ifndef MTK_TEST_SURFACE_FRAME_H
#define MTK_TEST_SURFACE_FRAME_H

#include <utils/RefBase.h>
#include <utils/String8.h>


class Frame : public android::RefBase {
    public:
        struct Config {
            android::String8 mPath;
            uint32_t mW, mS, mH;
            uint32_t mPitch;
            uint32_t mFormat;
            uint32_t mApi;
            uint32_t mUsageEx;
            Config(const android::String8& path,
                   const uint32_t w,
                   const uint32_t s,
                   const uint32_t h,
                   const uint32_t pitch,
                   const uint32_t format,
                   const uint32_t api,
                   const uint32_t usageEx)
                :   mPath(path),
                    mW(w),
                    mS(s),
                    mH(h),
                    mPitch(pitch),
                    mFormat(format),
                    mApi(api),
                    mUsageEx(usageEx) {}
        };

        Frame(const Config& cfg);

        virtual ~Frame() {
            delete[] mData;
        }
        inline const Config& getConfig() const { return mCfg; }
        inline char* data() { return mData; }

    private:
        const Config mCfg;
        char* mData;
};

#endif // MTK_TEST_SURFACE_FRAME_H
