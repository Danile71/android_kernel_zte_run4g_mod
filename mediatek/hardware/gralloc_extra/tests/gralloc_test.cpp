
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <cutils/log.h>
#include <ui/GraphicBuffer.h>

#include <ui/gralloc_extra.h>

using namespace android;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

class timer
{
    public:
        timer():start(0), end(0) {}
        
        void reset() { end = 0; start = clock(); }
        void stop() { end = clock(); }
        
        float second() 
        { 
            if (end != 0) 
                return (end-start)/(double)CLOCKS_PER_SEC; 
            return (clock()-start)/(double)CLOCKS_PER_SEC; 
        }

    private:
        clock_t start, end;
};

class result
{
public:
    result():
        err(0),
        w(0),
        h(0),
        f(0),
        u(0),
        s(0),
        number(1),
        t_alloc(0.0f),
        t_free(0.0f),
        t_lock(0.0f),
        t_unlock(),
        t_lockYCbCr(0.0f),
        t_unlockYCbCr(0.0f)
#ifdef GRALLOC_EXTRA_SUPPORT
        ,
        e_w(0),
        e_h(0),
        e_s(0),
        e_vs(0),
        e_alloc_size(0),
        e_f(0),
        e_u(0)
#endif
        {}
    
    int err;

    int w, h, f, u, s;

    int number;
    float t_alloc, t_free;
    float t_lock, t_unlock;
    float t_lockYCbCr, t_unlockYCbCr;

#ifdef GRALLOC_EXTRA_SUPPORT
    int e_w, e_h, e_s, e_vs, e_alloc_size, e_f, e_u;
#endif

    struct android_ycbcr yuv_info;

    void add(const result &r2)
    {
        t_alloc += r2.t_alloc;
        t_free += r2.t_free;
        t_lock += r2.t_lock;
        t_unlock += r2.t_unlock;
        t_lockYCbCr += r2.t_lockYCbCr;
        t_unlockYCbCr += r2.t_unlockYCbCr;

        ++number;
    }
};

result allocGB(int w, int h, int f, int u)
{
    timer t;
    result r;

    t.reset();
    sp<GraphicBuffer> pGrBuffer = new GraphicBuffer(w, h, f, u);
    r.t_alloc = t.second();
    
    r.w = w;
    r.h = h;
    r.f = f;
    r.u = u;

    if( pGrBuffer == NULL || pGrBuffer.get() == NULL)
    {
        // should never be here.
        printf("!! Can't allocate GraphicBuffer:%dx%d-0x%x u-0x%x\n", w, h, f, u);
        r.err = -1;
        return r;
    }

    // Init the buffer
    r.err = pGrBuffer->initCheck();
    if (r.err != NO_ERROR)
    {
        // allocate fail, check the logcat for detail.
        return r;
    }

    pGrBuffer->dumpAllocationsToSystemLog();

    r.s = pGrBuffer->getStride();

#ifdef GRALLOC_EXTRA_SUPPORT
    // Retrieve andorid native buffera
    buffer_handle_t hnd = pGrBuffer->getNativeBuffer()->handle;

    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_WIDTH, &r.e_w);
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_HEIGHT, &r.e_h);
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_STRIDE, &r.e_s);
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_VERTICAL_STRIDE, &r.e_vs);
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_FORMAT, &r.e_f);
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_USAGE, &r.e_u);
#endif

    struct android_ycbcr &yuv_info = r.yuv_info;
    status_t err;

    t.reset();
    err = pGrBuffer->lock(u, &yuv_info.y);
    if (err == NO_ERROR)
    {
        r.t_lock = t.second();

        t.reset();
        pGrBuffer->unlock();
        r.t_unlock = t.second();
    }

    t.reset();
    err = pGrBuffer->lockYCbCr(u, &yuv_info);
    if (err == NO_ERROR)
    {
        r.t_lockYCbCr = t.second();

        t.reset();
        pGrBuffer->unlock();
        r.t_unlockYCbCr = t.second();
    }

    t.reset();
    pGrBuffer = NULL;
    r.t_free = t.second();

    return r;
}

result allocGBtime(int w, int h, int f, int u, int t)
{
    result ret = allocGB(w, h, f, u);
    while (--t)
        ret.add(allocGB(w, h, f, u));
    return ret;
}

class SIZE
{
    public:
        int w;
        int h;
};

class VS 
{
    public:
        int v;
        const char * s;

        static const char * getS(const VS * table, size_t size, int f)
        {
            for (size_t j = 0; j < size; ++j)
            {
                if (table[j].v == f)
                    return table[j].s;
            }
            return "";
        }
};

#define MAKE_VS(v)  {v, #v}

void testFormatSupport()
{
    const SIZE sizeList[] = {
        {64, 64}, 
        {176, 176}, 
        {1080, 1920}, 
        {4096, 4096},
    };
    const VS formatList[] = {
        MAKE_VS(HAL_PIXEL_FORMAT_RGBA_8888),
        MAKE_VS(HAL_PIXEL_FORMAT_RGBX_8888),
        MAKE_VS(HAL_PIXEL_FORMAT_RGB_888),
        MAKE_VS(HAL_PIXEL_FORMAT_BLOB), // hight must be 1
        MAKE_VS(HAL_PIXEL_FORMAT_YV12),
        MAKE_VS(HAL_PIXEL_FORMAT_I420),
        MAKE_VS(HAL_PIXEL_FORMAT_NV12_BLK),
        MAKE_VS(HAL_PIXEL_FORMAT_YUV_PRIVATE),
        MAKE_VS(HAL_PIXEL_FORMAT_UFO),
        MAKE_VS(HAL_PIXEL_FORMAT_YCrCb_420_SP), // NV21
        MAKE_VS(HAL_PIXEL_FORMAT_YCbCr_422_I), // YUY2

        MAKE_VS(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED), // real format ?
        MAKE_VS(HAL_PIXEL_FORMAT_YCbCr_420_888), // real format ?
    };
    const VS usageList[] = {
        MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_TEXTURE),
        MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_VIDEO_ENCODER),
        MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_CAMERA_ZSL),
        MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_CAMERA_ZSL),
    };

    int tests = 0;
    int pass = 0;

    for (size_t j = 0; j < ARRAYSIZE(formatList); ++j)
    {
        const VS &f = formatList[j];

        for (size_t k = 0; k < ARRAYSIZE(sizeList); ++k)
        {
            const SIZE &size = sizeList[k];

            for (size_t i = 0; i < ARRAYSIZE(usageList); ++i)
            {
                const VS &u = usageList[i];

                result r;

                tests ++;

                /* special case */
                if (f.v == HAL_PIXEL_FORMAT_BLOB)
                    r = allocGB(size.w * size.h, 1, f.v, u.v);
                else
                    r = allocGB(size.w, size.h, f.v, u.v);

                if (r.err != NO_ERROR)
                {
                    printf(" [!!FAIL] allocate fail, size:%dx%d format:0x%x(%s) usage:0x%x(%s)" "\n",
                        r.w, r.h, r.f, f.s, r.u, u.s);
                }
                else
                {
                    pass ++;

                    printf(" [PASS] size:%dx%d stride:%d format:0x%x usage:0x%x time:%fms" "\n",
                        r.w, r.h, r.s, r.f, r.u, r.t_alloc*1000.0f);
                }
            }
        }
    }

    if (pass == tests)
    {
        printf("testFormatSupport PASS all %d tests\n", tests);
    }
    else
    {
        printf("testFormatSupport FAIL %d tests of %d tests\n", tests - pass, tests);
    }
}

void testImplmentationDefine()
{
#ifdef GRALLOC_EXTRA_SUPPORT
    const VS format_implementation = MAKE_VS(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
    const VS format_ycbcr_420_888  = MAKE_VS(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);

    const VS usage                 = MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_TEXTURE);
    const VS usage_video           = MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_VIDEO_ENCODER);
    const VS usage_camera          = MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_CAMERA_ZSL);
    const VS usage_camera_texture  = MAKE_VS(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_CAMERA_ZSL);

    class test_t {
    public:
        int w, h;
        VS f;
        VS u;

        int expected_format;
    };
    
    const test_t testList[] = {
        {1024, 1024, format_implementation, usage,                HAL_PIXEL_FORMAT_RGBA_8888},
        {1024, 1024, format_implementation, usage_video,          HAL_PIXEL_FORMAT_YV12},
        {1024, 1024, format_implementation, usage_camera,         HAL_PIXEL_FORMAT_YCbCr_422_I},
        {1024, 1024, format_implementation, usage_camera_texture, HAL_PIXEL_FORMAT_YCrCb_420_SP},

        {1024, 1024, format_ycbcr_420_888,  usage,                HAL_PIXEL_FORMAT_YCrCb_420_SP},
        {1024, 1024, format_ycbcr_420_888,  usage_video,          HAL_PIXEL_FORMAT_YCrCb_420_SP},
        {1024, 1024, format_ycbcr_420_888,  usage_camera,         HAL_PIXEL_FORMAT_YCrCb_420_SP},
        {1024, 1024, format_ycbcr_420_888,  usage_camera_texture, HAL_PIXEL_FORMAT_YCrCb_420_SP},
    };

    int tests = 0;
    int pass = 0;

    for (size_t i = 0; i < ARRAYSIZE(testList); ++i)
    {
        const test_t &test = testList[i];

        result r;
        r = allocGB(test.w, test.h, test.f.v, test.u.v);
        //r = allocGB(test.w, test.h, test.f.v, test.u.v | GRALLOC_USAGE_NULL_BUFFER); // save time

        tests ++;

        if (r.err != NO_ERROR)
        {
            printf(" [!!FAIL] allocate fail, size:%dx%d format:0x%x(%s) usage:0x%x(%s)" "\n",
                    r.w, r.h, r.f, test.f.s, r.u, test.u.s);
        }
        else if (r.e_f != test.expected_format)
        {
            printf(" [!!FAIL] expected_format=0x%x vs format=0x%x : size:%dx%d format:0x%x(%s) usage:0x%x(%s)" "\n",
                    test.expected_format, r.e_f, r.w, r.h, r.f, test.f.s, r.u, test.u.s);
        }
        else
        {
            pass ++;

            printf(" [PASS] size:%dx%d stride:%d format:0x%x usage:0x%x real_format=0x%x time:%fms" "\n",
                    r.w, r.h, r.s, r.f, r.u, r.e_f, r.t_alloc*1000.0f);
        }
    }

    if (pass == tests)
    {
        printf("testImplmentationDefine PASS all %d tests\n", tests);
    }
    else
    {
        printf("testImplmentationDefine FAIL %d tests of %d tests\n", tests - pass, tests);
    }
#else
    printf("testImplmentationDefine FAIL, do not support GRALLOC_EXTRA\n");
#endif
}

void testNullBuffer()
{
    const int tests = 100;

    const int w = 4096, h = 4096;
    const int format = HAL_PIXEL_FORMAT_YV12;
    const int usage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE;
    const int usage_nullbuffer = usage | GRALLOC_USAGE_NULL_BUFFER;

    printf(" start alloc %d times ...\n", tests);

    result r;
    result r_normal = allocGBtime(w, h, format, usage, tests);

    r = r_normal;
    printf(" size:%dx%d format:0x%x usage:0x%x time:%fms" "\n",
            r.w, r.h, r.f, r.u, r.t_alloc/r.number*1000.0f);
    printf("  lock_time:%fms" "\n", r.t_lock/r.number*1000.0f);
    printf("  unlock_time:%fms" "\n", r.t_unlock/r.number*1000.0f);
    printf("  lockYCbCr_time:%fms" "\n", r.t_lockYCbCr/r.number*1000.0f);
    printf("  unlockYCbCr_time:%fms" "\n", r.t_unlockYCbCr/r.number*1000.0f);

    result r_null = allocGBtime(w, h, format, usage_nullbuffer, tests);
    
    r = r_null;
    printf(" size:%dx%d format:0x%x usage:0x%x time:%fms" "\n",
            r.w, r.h, r.f, r.u, r.t_alloc/r.number*1000.0f);


    if (r.t_alloc/r.number*1000.0f < 3.0f)
    {
        printf("testNullBuffer PASS\n");
    }
    else
    {
        printf("testNullBuffer FAIL, NULL_BUFFER take too long\n");
    }
}

int main(int argc, char * argv[])
{
    testFormatSupport();

    testImplmentationDefine();

    testNullBuffer();
}
