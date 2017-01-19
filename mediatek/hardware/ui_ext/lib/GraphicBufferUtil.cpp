#define LOG_TAG "GraphicBufferUtil"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <utils/String8.h>

#include <cutils/xlog.h>

#include <ui_ext/GraphicBufferUtil.h>
#include <ui/GraphicBufferExtra.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>

#include <png.h>
#include <transform_scanline.h>

#define ALIGN_CEIL(x,a) (((x) + (a) - 1L) & ~((a) - 1L))
#define LOCK_FOR_SW (GRALLOC_USAGE_SW_READ_RARELY | GRALLOC_USAGE_SW_WRITE_RARELY | GRALLOC_USAGE_HW_TEXTURE)

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferUtil )

GraphicBufferUtil::GraphicBufferUtil()
    : mExtraDev(0)
{
    hw_module_t const* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);

    ALOGE_IF(err, "FATAL: can't find the %s module", GRALLOC_HARDWARE_MODULE_ID);
    if (err == 0)
    {
        gralloc_extra_open(module, &mExtraDev);
    }
}

GraphicBufferUtil::~GraphicBufferUtil()
{
    if (mExtraDev)
    {
        gralloc_extra_close(mExtraDev);
    }
}

static void write565Data(uint32_t width,
                         uint32_t height,
                         uint8_t* in,
                         png_structp out)
{
    // input is RGB565 for 2 bytes, and output is RGBA8888 for 4 bytes
    const uint32_t inBPP = 2;
    const uint32_t outBPP = 4;

    png_bytep tmp = new png_byte[width * outBPP];
    if (NULL == tmp)
    {
        XLOGE("cannot create temp row buffer for RGB565 conversion");
        return;
    }

    uint8_t* wptr;
    for (uint32_t i = 0; i < height; i++)
    {
        wptr = reinterpret_cast<uint8_t*>(tmp);
        for (uint32_t j = 0; j < width; ++j)
        {
            // get 16bits pixel value and split it into 32bits storage
            const uint16_t c = *(reinterpret_cast<uint16_t*>(in));
            wptr[0] = SkPacked16ToR32(c);
            wptr[1] = SkPacked16ToG32(c);
            wptr[2] = SkPacked16ToB32(c);
            wptr[3] = 255;

            wptr += outBPP;
            in += inBPP;
        }
        png_write_rows(out, &tmp, 1);
    }

    delete [] tmp;
}

static void writeData(uint32_t width,
                      uint32_t height,
                      uint8_t* in,
                      png_structp out)
{
    // should be RGBA for 4 bytes
    const uint32_t widthBytes = width * 4;

    for (uint32_t y = 0; y < height; y++)
    {
        png_write_rows(out, (png_bytepp)&in, 1);
        in = (uint8_t *)in + widthBytes;
    }
}

void GraphicBufferUtil::dump(const sp<GraphicBuffer>& gb,
                              const char* prefix,
                              const char* dir)
{
    if (CC_UNLIKELY(gb == NULL))
    {
        XLOGE("[%s] gb is NULL", __func__);
        return;
    }
    dump(gb->handle, prefix, dir);
}

void GraphicBufferUtil::dump(const buffer_handle_t &handle,
                              const char* prefix,
                              const char* dir)
{
    if (CC_UNLIKELY(handle == NULL))
    {
        XLOGE("[%s] handle is NULL", __func__);
        return;
    }

    gralloc_buffer_info_t info;
    if(gralloc_extra_getBufInfo(handle, &info) != NO_ERROR)
    {
        XLOGD("getting info of handle failed joen");
        return;
    }

    const uint32_t& width = info.width;
    const uint32_t& height = info.height;
    const uint32_t& stride = info.stride;
    PixelFormat inputFormat = PIXEL_FORMAT_UNKNOWN;
    if (getRealFormat(handle, &inputFormat) != GRALLOC_EXTRA_OK)
    {
        XLOGE("Getting format of handle failed");
        return;
    }

    XLOGD("[%s] handle:%p +", __func__, handle);
    XLOGD("    prefix:%s dir:%s", prefix, dir);

    // make file name, default path to /data/[handle]_[width]_[height]_[stride]
    String8 path;
    if ((NULL == dir) || (0 == strlen(dir)))
    {
        path.setPathName("/data/");
    }
    else
    {
        path.setPathName(dir);
    }

    if ((NULL == prefix) || (0 == strlen(prefix)))
    {
        path.append(String8::format("/H%p_w%d_h%d_s%d",
            handle, width, height, stride));
    }
    else
    {
        path.append(String8::format("/%s_H%p_w%d_h%d_s%d",
            prefix, handle, width, height, stride));
    }
    XLOGD("    path:%s", path.string());

    int dumpHeight = height;

    uint32_t bits = getBitsPerPixel(info.format);
    bool isRaw = false;
    bool is565 = false;
    bool stripAlpha = false;

    // switch case different pixel format process
    // only RGB? series will be saved into image file
    // others in RAW data only
    switch (inputFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            path += ".png";
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            path += ".png";
            stripAlpha = true;
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            path += "(RBswapped).png";
            break;
        case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
            path += "(RBswapped).png";
            stripAlpha = true;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            is565 = true;
            stripAlpha = true;
            path += ".png";
            break;
        case HAL_PIXEL_FORMAT_I420:
            isRaw = true;
            path += ".i420";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK:
            dumpHeight = ALIGN_CEIL(height, 32);
            isRaw = true;
            path += ".nv12_blk";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            dumpHeight = ALIGN_CEIL(height, 32);
            isRaw = true;
            path += ".nv12_blk_fcm";
            break;
        case HAL_PIXEL_FORMAT_YV12:
            isRaw = true;
            path += ".yv12";
            break;
        case HAL_PIXEL_FORMAT_YUYV:
            isRaw = true;
            path += ".yuyv";
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            isRaw = true;
            path += ".RGB";
            break;
        default:
            XLOGE("    CANNOT dump (format:0x%x)", inputFormat);
            return;
    }
    XLOGD("    path:%s", path.string());

    void        *ptr     = NULL;
    FILE        *f       = NULL;
    png_structp png_ptr  = NULL;
    png_infop   info_ptr = NULL;

    // dump to file with final path and file type

    int err = GraphicBufferMapper::getInstance().lock(handle,
                                                      GraphicBuffer::USAGE_SW_READ_OFTEN,
                                                      Rect(width, height),
                                                      &ptr);
    if (err == NO_ERROR) {
        if (NULL == ptr)
        {
            XLOGE("    lock() FAILED");
            goto finalize;
        }

        f = fopen(path.string(), "wb");
        if (NULL == f)
        {
            XLOGE("    fopen() FAILED");
            goto finalize;
        }

        if (isRaw)
        {
            // simplely write binary data to file
            fwrite(ptr, (stride * dumpHeight * bits) >> 3, 1, f);
        }
        else
        {
            // init write struct
            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (NULL == png_ptr)
            {
                XLOGE("    init png FAILED (1)");
                goto finalize;
            }

            // init info struct
            info_ptr = png_create_info_struct(png_ptr);
            if (NULL == info_ptr)
            {
                XLOGE("    init png FAILED (2)");
                goto finalize;
            }

            // set file to write
            png_init_io(png_ptr, f);

            // setup color format info
            if (true == stripAlpha)
            {
                png_set_IHDR(png_ptr, info_ptr,
                    stride, dumpHeight, 8, PNG_COLOR_TYPE_RGB,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            }
            else
            {
                png_set_IHDR(png_ptr, info_ptr,
                    stride, dumpHeight, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            }

            // set bits layout
            png_color_8 sigBit;
            if (is565)
            {
                sigBit.red   = 5;
                sigBit.green = 6;
                sigBit.blue  = 5;
                sigBit.alpha = 0;
            }
            else
            {
                sigBit.red   = 8;
                sigBit.green = 8;
                sigBit.blue  = 8;
                sigBit.alpha = 8;
            }
            png_set_sBIT(png_ptr, info_ptr, &sigBit);

            // start
            png_write_info(png_ptr, info_ptr);

            // set filler to get rid of alpha if strip needed
            if (true == stripAlpha)
            {
                png_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
            }

            // write data into png
            if (is565)
            {
                write565Data(stride, height, reinterpret_cast<uint8_t*>(ptr), png_ptr);
            }
            else
            {
                writeData(stride, height, reinterpret_cast<uint8_t*>(ptr), png_ptr);
            }

            // end
            png_write_end(png_ptr, info_ptr);
        }

finalize:
        // clean up
        if (NULL != f)        fclose(f);
        if (NULL != info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        if (NULL != png_ptr)  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    }
    GraphicBufferMapper::getInstance().unlock(handle);

    XLOGD("[%s] -", __func__);
}

uint32_t GraphicBufferUtil::getBitsPerPixel(int format)
{
    uint32_t bits = 32;
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
            bits = 32;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bits = 24;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_YUYV:
            bits = 16;
            break;
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
            bits = 12;
            break;
        default:
            XLOGE("    unknown format: 0x%x, use default as 4.0", format);
    }
    return bits;
}

uint32_t GraphicBufferUtil::getBitsPerPixel(buffer_handle_t handle)
{
    int32_t format = 0;
    if (GraphicBufferExtra::get().query(
                handle,
                GRALLOC_EXTRA_GET_FORMAT,
                static_cast<int*>(&format)) != GRALLOC_EXTRA_OK)
    {
        XLOGE("Getting format of handle failed");
        return 0;
    }
    return getBitsPerPixel(format);
}

int GraphicBufferUtil::drawLine(const sp<GraphicBuffer> &gb, uint8_t val, int ptn_w, int ptn_h, int pos)
{
    if (gb == NULL)
        return INVALID_OPERATION;

    // currently some internal format cancnot use SW rendering well, just skip them
    switch (gb->format)
    {
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
            XLOGW("[%s] unsupport buffer format %#x, just skip",
                __func__, gb->format);
            return INVALID_OPERATION;
    }

    if (ptn_w == 0)
        ptn_w = 1;
    if (ptn_h == 0)
        ptn_h = 1;

    uint32_t log_w = log2(ptn_w);
    if (log_w > 5)
        log_w = 5;
    uint32_t line_w = pow(2, log_w);

    uint32_t log_h = log2(ptn_h);
    if (log_h > 5)
        log_h = 5;
    uint32_t line_h = pow(2, log_h);

    uint8_t *ptr;
    status_t lockret = NO_ERROR;

    lockret = gb->lock(LOCK_FOR_SW, (void**)&ptr);
    if (NO_ERROR != lockret)
    {
        XLOGE("[%s] buffer lock fail: %s (gb:%p, handle:%p)",
            __func__, strerror(lockret), gb.get(), gb->handle);
    }
    else
    {
        // if custom format, just regard as one-byte-plane size
        // otherwise use accurate size aggressively
        uint32_t bits = 8;
        if (gb->format < 0x100)
        {
            bits = getBitsPerPixel(gb->format);
        }

        if (ptn_w == 1)
        {
            uint32_t bsize; // block size, will split intrested plane to 32 parts

            pos &= (line_h - 1);    // mod count by 32

            XLOGV("[debug] drawLine, pos=%d, log_h=%d, line_h=%d", pos, log_h, line_h);

            bsize = ((gb->stride * gb->height * bits) >> 3) >> log_h;
            memset(ptr + (bsize * pos), val, bsize);
        }
        else
        {
            uint32_t block_number = line_w * line_h;
            uint32_t block_w = gb->width >> log_w;
            uint32_t block_h = gb->height >> log_h;
            uint32_t bsize = block_w * block_h;
            uint32_t pos_x = pos & (line_w - 1);
            uint32_t pos_y = (pos / line_w) & (line_h - 1);
            uint32_t j;

            XLOGV("[debug] drawLine pos=%d, log_w=%d, line_w=%d, log_h=%d, line_h=%d, "
                  "block_number=%d, block_w=%d, block_h=%d, bsize=%d, pos_x=%d, pos_y=%d",
                pos, log_w, line_w, log_h, line_h, block_number, block_w, block_h, bsize, pos_x, pos_y);

            block_w = (block_w * bits) >> 3;
            ptr += ((gb->stride * pos_y * block_h * bits) >> 3) + (block_w * pos_x);
            for (j = 0; j < block_h; j++)
            {
                memset(ptr, val, block_w);
                ptr += (gb->stride * bits) >> 3;
            }
        }
    }
    gb->unlock();

    return NO_ERROR;
}

GRALLOC_EXTRA_RESULT GraphicBufferUtil::getRealFormat(buffer_handle_t handle, PixelFormat* format) {
    if (NULL == handle)
    {
        XLOGE("[%s] invalid argument", __func__);
        return GRALLOC_EXTRA_ERROR;
    }
    if (NULL == format)
    {
        XLOGE("[%s] invalid argument", __func__);
        return GRALLOC_EXTRA_ERROR;
    }
    GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;

    // for HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED
    if ((err = GraphicBufferExtra::get().query(
                    handle,
                    GRALLOC_EXTRA_GET_FORMAT,
                    format)) != GRALLOC_EXTRA_OK)
    {
        XLOGE("[%s] to get format failed", __func__);
        return err;
    }

    if (HAL_PIXEL_FORMAT_YUV_PRIVATE == *format)
    {
        gralloc_extra_ion_sf_info_t sf_info;
        if ((err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info))
                != GRALLOC_EXTRA_OK) {
            XLOGE("[%s] to query real format failed", __func__);
            return err;
        }
        // check real format within private format
        switch (sf_info.status & GRALLOC_EXTRA_MASK_CM)
        {
            case GRALLOC_EXTRA_BIT_CM_YV12:
                *format = HAL_PIXEL_FORMAT_YV12;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK:
                *format = HAL_PIXEL_FORMAT_NV12_BLK;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM:
                *format = HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                break;
            default:
                XLOGE("    CANNOT get real format: (format=0x%x, fillFormat=0x%x)",
                      *format,
                      sf_info.status & GRALLOC_EXTRA_MASK_CM);
                return GRALLOC_EXTRA_ERROR;
        }
    }
    return err;
}

// ---------------------------------------------------------------------------
}; // namespace android