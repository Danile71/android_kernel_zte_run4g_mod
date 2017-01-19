#define LOG_TAG "GuiExt"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#define HAVE_PTHREADS
#define USE_DP

#include <string.h>
#include <cutils/xlog.h>
#include <hardware/gralloc.h>
#include <hardware/gralloc_extra.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <gui/BufferQueue.h>
#include <gui/GraphicBufferAlloc.h>

#if defined(GPU_AUX_USE_DP_FRAMEWORK)
#include <DpBlitStream.h>
#endif

#include <GuiExtAux.h>
#include <GuiExtClientProducer.h>
#include <GuiExtClientConsumer.h>

#define LOCK_FOR_DP (GRALLOC_USAGE_SW_READ_RARELY | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE)
#define ALIGN(x,a)      (((x) + (a) - 1L) & ~((a) - 1L))

#define MTK_LOGV(fmt, ...)      XLOGV("%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGD(fmt, ...)      XLOGD("%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGI(fmt, ...)      XLOGI("%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGW(fmt, ...)      XLOGW("%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGE(fmt, ...)      XLOGE("%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MTK_ATRACE_BEGIN(name)  atrace_begin(ATRACE_TAG, name)
#define MTK_ATRACE_END(name)    atrace_end(ATRACE_TAG)

using namespace android;

static struct
{
    Mutex                 lock;
    hw_module_t const*    module;
    extra_device_t*       extraDev;
} mtu_aux_global;

typedef struct GuiExtAuxBufferItem
{
    int                   mBuf;
    sp<GraphicBuffer>     mGraphicBuffer;
    sp<Fence>             mFence;
    int                   mFrameNumber;

    int                   mPoolId;
    android_native_buffer_t* mPoolBuffer;
    android_native_buffer_t* mSrcBuffer;
} GuiExtAuxBufferItem_t;

typedef struct GuiExtAuxBufferQueue
{
    sp<BufferQueue>       mBufferQueue;
    GuiExtAuxBufferItem*  mSlots;

    int                   num_slots;
    int                   width;
    int                   height;
    int                   format;
    int                   mScalingMode;
    int                   mTransform;
    int                   mSwapIntervalZero;

    int                   mPoolId;
    int                   mCount;

    Mutex*                mMutex;
#if defined(GPU_AUX_USE_DP_FRAMEWORK)
    DpBlitStream *bltStream;
#endif
} GuiExtAuxBufferQueue_t;

typedef struct guiExtAuxBufferInfo
{
    int                   ion_fd;
    int                   width;
    int                   height;
    int                   format;
    int                   usage;
    int                   stride;
    int                   vertical_stride;
    int                   alloc_size;
    int                   status;
} GuiExtAuxBufferInfo_t;

static buffer_handle_t guiExtAuxGetNativehandle(android_native_buffer_t * buffer)
{
    if ((buffer->common.magic   == ANDROID_NATIVE_BUFFER_MAGIC) &&
        (buffer->common.version == sizeof(android_native_buffer_t)))
    {
        return buffer->handle;
    }
    return 0;
}

static hw_module_t const * guiExtAuxGetGrallocDevice(void)
{
    if (mtu_aux_global.module == NULL)
    {
        Mutex::Autolock l(mtu_aux_global.lock);

        if (mtu_aux_global.module == NULL)
        {
            int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mtu_aux_global.module);
            if (err)
            {
                MTK_LOGE("FATAL: can't find the GRALLOC_HARDWARE_MODULE_ID module");
            }
        }
    }
    return mtu_aux_global.module;
}

static int guiExtAuxLockBuffer(android_native_buffer_t * buffer, int usage, GuiExtAuxBufferInfo_t *info, void **va)
{
    gralloc_module_t const *grallocDev = (gralloc_module_t const *)guiExtAuxGetGrallocDevice();
    buffer_handle_t hnd = guiExtAuxGetNativehandle(buffer);
    int err = grallocDev->lock(grallocDev, hnd, usage, 0, 0, info->width, info->height, va);
    if (err)
    {
        MTK_LOGE("lockBuffer(...) failed %d (%s)", err, strerror(-err));
    }
    return err;
}

static int guiExtAuxUnlockBuffer(android_native_buffer_t * buffer)
{
    gralloc_module_t const *grallocDev = (gralloc_module_t const *)guiExtAuxGetGrallocDevice();
    buffer_handle_t hnd = guiExtAuxGetNativehandle(buffer);
    int err = grallocDev->unlock(grallocDev, hnd);
    if (err)
    {
        MTK_LOGE("unlockBuffer(...) failed %d (%s)", err, strerror(-err));
    }
    return err;
}

void guiExtAuxSetWidthHieghtFromSrcBuffer(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t * src_buffer)
{
    if (bq == NULL)
    {
        MTK_LOGE("bq == NULL");
        return;
    }

    buffer_handle_t src = guiExtAuxGetNativehandle(src_buffer);
    int w, h;
    int err = 0;

    src = guiExtAuxGetNativehandle(src_buffer);

    err |= gralloc_extra_query(src, GRALLOC_EXTRA_GET_WIDTH,  &w);
    err |= gralloc_extra_query(src, GRALLOC_EXTRA_GET_HEIGHT, &h);

    if (GRALLOC_EXTRA_OK != err)
    {
        MTK_LOGE("error found");
    }

    GuiExtAuxSetSize(bq, w, h);
}

static DP_PROFILE_ENUM guiExtAuxGetYUVColorSpace(GuiExtAuxBufferInfo_t * info, DP_PROFILE_ENUM default_value)
{
    switch (info->status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE)
    {
    case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
        return DP_PROFILE_BT601;
    case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
        return DP_PROFILE_FULL_BT601;
    case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
        return DP_PROFILE_BT709;
    }

    return default_value;
}

static void guiExtAuxSetYUVColorSpace(android_native_buffer_t * buffer, DP_PROFILE_ENUM value)
{
    int bit = 0;

    switch (value)
    {
    case DP_PROFILE_BT601:
        bit = GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
        break;
    case DP_PROFILE_FULL_BT601:
        bit = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
        break;
    case DP_PROFILE_BT709:
        bit = GRALLOC_EXTRA_BIT_YUV_BT709_NARROW;
        break;

    default: return;
    }

    buffer_handle_t hnd;
    gralloc_extra_ion_sf_info_t sf_info;

    hnd = guiExtAuxGetNativehandle(buffer);
    if (hnd)
    {
        gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
        gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, bit);
        gralloc_extra_perform(hnd, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
    }
}

static void guiExtAuxDokickConversion(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t * src_buffer)
{
    /* update the w/h for output buffer */
    guiExtAuxSetWidthHieghtFromSrcBuffer(bq, src_buffer);

    /* get pool id */
    {
        gralloc_extra_ion_sf_info_t sf_info;

        if (GRALLOC_EXTRA_OK == gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info))
        {
            if (bq->mPoolId != sf_info.pool_id)
            {
                MTK_LOGD("PoolID change to %d from %d", sf_info.pool_id, bq->mPoolId);

                /* disconnect before we change poolid */
                if (bq->mPoolId != 0)
                {
                    GuiExtClientConsumer &consumer(GuiExtClientConsumer::getInstance());
                    consumer.disconnect(bq->mPoolId, GUI_EXT_USAGE_GPU);
                }

                bq->mPoolId = sf_info.pool_id;
            }
        }
    }

    int pre_allocate_found = 0;

    if (bq->mPoolId > 0)
    {
        GuiExtClientConsumer &consumer(GuiExtClientConsumer::getInstance());
        android_native_buffer_t* pbuf = NULL;
        int slotIdx = bq->mCount;
        int32_t ret = 0;

        ret = consumer.acquire(&pbuf, bq->mPoolId, GUI_EXT_USAGE_GPU);

        if (NO_ERROR == ret)
        {
            GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[slotIdx];

            auxitem->mGraphicBuffer = new GraphicBuffer(pbuf, false);
            auxitem->mFence = Fence::NO_FENCE;
            auxitem->mPoolId = bq->mPoolId;
            auxitem->mPoolBuffer = pbuf;
            auxitem->mSrcBuffer = 0;

            pre_allocate_found = 1;
        }
    }

    if (bq->mPoolId > 0 && pre_allocate_found == 0)
    {
        MTK_LOGW("pre_allocate poll freed, change to local buffer");
    }

    if (pre_allocate_found == 0)
    {
        int bufSlot;
        int fence_fd;

        GuiExtAuxDequeueBuffer(bq, &bufSlot, &fence_fd);

        sp<Fence> fence(new Fence(fence_fd));
        fence->waitForever("GPU_AUX");

        GuiExtAuxQueueBuffer(bq, bufSlot, -1);
    }
}

static GuiExtAuxBufferItemHandle guiExtAuxFindItem(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t * buffer)
{
    for (int i = 0; i < bq->num_slots; ++i)
    {
        android_native_buffer_t * b = GuiExtAuxRequestBuffer(bq, i);

        if (b == buffer)
        {
            return &bq->mSlots[i];
        }
    }

    return NULL;
}

static int guiExtAuxGetBufferInfo(android_native_buffer_t * buffer, GuiExtAuxBufferInfo_t * p_info)
{
    GuiExtAuxBufferInfo_t &info = *p_info;
    buffer_handle_t hnd;
    int err = GRALLOC_EXTRA_OK;

    hnd = guiExtAuxGetNativehandle(buffer);

    info.ion_fd = -1;

    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_ION_FD, &info.ion_fd);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_WIDTH,  &info.width);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_HEIGHT, &info.height);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_FORMAT, &info.format);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_USAGE,  &info.usage);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_STRIDE, &info.stride);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_VERTICAL_STRIDE, &info.vertical_stride);
    err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_ALLOC_SIZE, &info.alloc_size);

    if (GRALLOC_EXTRA_OK == err)
    {
        gralloc_extra_ion_sf_info_t sf_info;
        err |= gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
        info.status = sf_info.status;
    }

    if (GRALLOC_EXTRA_OK != err)
    {
        MTK_LOGE("error found");
    }

    //MTK_LOGV("wxh(s)=%dx%d(%d) fd=%d status=0x%x", info.width, info.height, info.stride, info.ion_fd, info.status);

    return err;
}

/* ================================== */

int GuiExtAuxIsSupportFormat(android_native_buffer_t* buf)
{
    buffer_handle_t hnd;

    hnd = guiExtAuxGetNativehandle(buf);

    if (hnd != NULL)
    {
        int format;

        if (gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_FORMAT, &format) == GRALLOC_EXTRA_OK)
        {
            if ((format == HAL_PIXEL_FORMAT_I420) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK_FCM) ||
                (format == HAL_PIXEL_FORMAT_YUV_PRIVATE) ||
                (format == HAL_PIXEL_FORMAT_UFO))
            {
                return 1;
            }
        }
        else
        {
            MTK_LOGE("query buf format fail");
        }
    }

    return 0;
}

struct DummyConsumer : public BnConsumerListener {
    virtual void onFrameAvailable() {}
    virtual void onBuffersReleased() {}
};

GuiExtAuxBufferQueueHandle GuiExtAuxCreateBufferQueue(int width, int height, int output_format, int num_max_buffer)
{
    GuiExtAuxBufferQueueHandle bq;
#if defined(GPU_AUX_USE_DP_FRAMEWORK)
    DpBlitStream *bltStream;
#endif

    if ((output_format != HAL_PIXEL_FORMAT_YV12) &&
            (output_format != HAL_PIXEL_FORMAT_RGBA_8888))
    {
        MTK_LOGE("Unsupported color format %d", output_format);
        return NULL;
    }

    if (num_max_buffer < 1)
    {
        num_max_buffer = 2;
        MTK_LOGE("num_max_buffer(%d) < 1", num_max_buffer);
    }

    bq = (GuiExtAuxBufferQueueHandle)calloc(1, sizeof(GuiExtAuxBufferQueue));

    if (bq == NULL)
    {
        MTK_LOGE("GPU_AUX_createBufferQueue allocate fail, out of memory");
        return NULL;
    }

#if defined(GPU_AUX_USE_DP_FRAMEWORK)
    bltStream = new DpBlitStream();
#endif
    sp<BufferQueue> mBufferQueue = new BufferQueue(new GraphicBufferAlloc());;
    GuiExtAuxBufferItem * mSlots = (GuiExtAuxBufferItemHandle)calloc(num_max_buffer, sizeof(GuiExtAuxBufferItem_t));;

    if (mBufferQueue == NULL)
    {
        MTK_LOGE("allocate BufferQueue fail: out of memory");

        free(bq);
        return NULL;
    }

    if (mSlots == NULL)
    {
        MTK_LOGE("allocate mSlots fail: out of memory");

        free(bq);
        return NULL;
    }
    else
    {
        for (int i = 0; i < num_max_buffer; ++i)
        {
            mSlots[i].mBuf = i;
            mSlots[i].mPoolId = 0;
        }
    }

    IGraphicBufferProducer::QueueBufferOutput output;
    int producerControlledByApp = 0;

    sp<DummyConsumer> dc(new DummyConsumer);
    mBufferQueue->consumerConnect(dc, producerControlledByApp);
    //IGraphicBufferProducer::QueueBufferOutput qbo;
    //mBufferQueue->connect(NULL, NATIVE_WINDOW_API_CPU, false, &qbo);

    mBufferQueue->setConsumerName(String8("GPU_AUX_default"));
    mBufferQueue->setBufferCount(num_max_buffer);
    mBufferQueue->setMaxAcquiredBufferCount(1);

    bq->mBufferQueue    = mBufferQueue;
    bq->mSlots          = mSlots;
    bq->num_slots       = num_max_buffer;
    bq->width           = width;
    bq->height          = height;
    bq->format          = output_format;
    bq->mScalingMode    = NATIVE_WINDOW_SCALING_MODE_NO_SCALE_CROP;
    bq->mTransform      = 0;
    bq->mSwapIntervalZero = false;
    bq->mPoolId         = 0;

    bq->mMutex          = new Mutex();
#if defined(GPU_AUX_USE_DP_FRAMEWORK)
    bq->bltStream       = bltStream;
#endif

    return bq;

}

void GuiExtAuxDestroyBufferQueue(GuiExtAuxBufferQueueHandle bq)
{
    if (bq != NULL)
    {
        if (bq->mPoolId > 0)
        {
            GuiExtClientConsumer &consumer(GuiExtClientConsumer::getInstance());
            consumer.disconnect(bq->mPoolId, GUI_EXT_USAGE_GPU);
        }

        delete bq->mMutex;
#if defined(GPU_AUX_USE_DP_FRAMEWORK)
        delete bq->bltStream;
#endif

        for (int i = 0; i < bq->num_slots; ++i)
        {
            GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[i];

            auxitem->mGraphicBuffer = NULL;
            // TODO: wait for all buffer to finish
            auxitem->mFence = NULL;
        }

        //bq->mBufferQueue->disconnect(NATIVE_WINDOW_API_CPU);
        bq->mBufferQueue->consumerDisconnect();
        bq->mBufferQueue= NULL;

        free(bq->mSlots);
        free(bq);
    }
}

void GuiExtAuxSetSize(GuiExtAuxBufferQueueHandle bq, int width, int height)
{
    bq->width = width;
    bq->height = height;
}


void GuiExtAuxSetName(GuiExtAuxBufferQueueHandle bq, const char* name)
{
    if (bq == NULL)
    {
        MTK_LOGE("bq == NULL");
        return;
    }

    bq->mBufferQueue->setConsumerName(String8(name));
}

void GuiExtAuxKickConversion(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t* src_buffer)
{
    guiExtAuxDokickConversion(bq, src_buffer);
}

int GuiExtAuxDoConversionIfNeed(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t* dst_buffer, android_native_buffer_t * src_buffer)
{
#if defined(GPU_AUX_USE_DP_FRAMEWORK)
    ATRACE_CALL();

    int err = 0;

    void *src_yp;
    void *dst_yp;

    int lockret;

    DpColorFormat dp_out_fmt;
    DpBlitStream &bltStream = *(bq->bltStream);

    unsigned int src_offset[2] = {0, 0};
    unsigned int src_size[3];
    unsigned int dst_size[3];

    DpColorFormat dp_in_fmt;
    int plane_num;

    int idx = 0;
    int num = 0;

    GuiExtAuxBufferInfo_t src_info;
    GuiExtAuxBufferInfo_t dst_info;

    GuiExtAuxBufferItemHandle hnd;

    err |= guiExtAuxGetBufferInfo(src_buffer, &src_info);
    err |= guiExtAuxGetBufferInfo(dst_buffer, &dst_info);

    if (err != GRALLOC_EXTRA_OK)
    {
        MTK_LOGE("retrive info fail: src:%p dst:%p", src_buffer, dst_buffer);
        return -1;
    }

    hnd = guiExtAuxFindItem(bq, dst_buffer);

    if (hnd != NULL && hnd->mSrcBuffer == src_buffer)
    {
        MTK_LOGV("skip, hnd->mSrcBuffer == src_buffer");
        return 0;
    }

    if (src_info.usage & (GRALLOC_USAGE_PROTECTED | GRALLOC_USAGE_SECURE))
    {
        MTK_LOGV("skip, cannot convert protect / secure buffer");
        return 0;
    }

    hnd->mSrcBuffer = src_buffer;

    Mutex::Autolock l(bq->mMutex);

    /* set SRC config */
    {
        int src_format;
        int src_width = 0;
        int src_height = 0;
        int src_y_stride;
        int src_uv_stride;
        int src_size_luma;
        int src_size_chroma;

        if ((src_info.width  <= 0) ||
            (src_info.height <= 0) ||
            (src_info.stride <= 0))
        {
            MTK_LOGE("Invalid buffer width %d, height %d, or stride %d", src_info.width, src_info.height, src_info.stride);
            return -1;
        }

        src_format = src_info.format;

        if (src_info.format == HAL_PIXEL_FORMAT_YUV_PRIVATE)
        {
            switch (src_info.status & GRALLOC_EXTRA_MASK_CM)
            {
            case GRALLOC_EXTRA_BIT_CM_YV12:
                src_format    = HAL_PIXEL_FORMAT_YV12;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK:
                src_format    = HAL_PIXEL_FORMAT_NV12_BLK;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM:
                src_format    = HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                break;
            case GRALLOC_EXTRA_BIT_CM_YUYV:
                src_format    = HAL_PIXEL_FORMAT_YCbCr_422_I;
                break;

            default:
                MTK_LOGE("unexpected format for clear motion: 0x%x", src_info.status & GRALLOC_EXTRA_MASK_CM);
                return -1;
            }
        }

        switch (src_format)
        {
        case HAL_PIXEL_FORMAT_I420:
            src_format    = src_info.format;
            src_width     = src_info.width;
            src_height    = src_info.height;
            src_y_stride  = src_info.stride;
            src_uv_stride = ALIGN(src_y_stride / 2, 16);

            plane_num = 3;
            src_size_luma   = src_height * src_y_stride;
            src_size_chroma = src_height * src_uv_stride / 2;
            src_offset[0]   = src_size_luma;
            src_offset[1]   = src_size_luma + src_size_chroma;
            src_size[0]     = src_size_luma;
            src_size[1]     = src_size_chroma;
            src_size[2]     = src_size_chroma;
            dp_in_fmt       = DP_COLOR_I420;
            break;

        case HAL_PIXEL_FORMAT_YV12:
            src_width     = src_info.width;
            src_height    = src_info.height;
            src_y_stride  = src_info.stride;
            src_uv_stride = ALIGN(src_y_stride / 2, 16);

            plane_num = 3;
            src_size_luma   = src_height * src_y_stride;
            src_size_chroma = src_height * src_uv_stride / 2;
            src_offset[0]   = src_size_luma;
            src_offset[1]   = src_size_luma + src_size_chroma;
            src_size[0]     = src_size_luma;
            src_size[1]     = src_size_chroma;
            src_size[2]     = src_size_chroma;
            dp_in_fmt       = DP_COLOR_YV12;
            break;

        case HAL_PIXEL_FORMAT_NV12_BLK:
            src_width     = ALIGN(src_info.width, 16);
            src_height    = ALIGN(src_info.height, 32);
            src_y_stride  = src_width * 32;
            src_uv_stride = src_y_stride / 2;

            plane_num = 2;
            src_size_luma   = src_width * src_height;
            src_size_chroma = src_width * src_height / 2;
            src_offset[0]   = src_size_luma;
            src_size[0]     = src_size_luma;
            src_size[1]     = src_size_chroma;
            dp_in_fmt       = DP_COLOR_420_BLKP;
            break;

        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            src_width     = ALIGN(src_info.width, 16);
            src_height    = ALIGN(src_info.height, 32);
            src_y_stride  = src_width * 32;
            src_uv_stride = src_y_stride / 2;

            plane_num = 2;
            src_size_luma   = src_width * src_height;
            src_size_chroma = src_width * src_height / 2;
            src_offset[0]   = src_size_luma;
            src_size[0]     = src_size_luma;
            src_size[1]     = src_size_chroma;
            dp_in_fmt       = DP_COLOR_420_BLKI;
            break;

        case HAL_PIXEL_FORMAT_UFO:
            {
                typedef struct { int w, h; } LCU_ALIGN;
                const LCU_ALIGN lcu_align_table[] = { {16, 32}, {32, 32}, {64, 64}, };
                const LCU_ALIGN * lcu_align;

                switch (src_info.status & GRALLOC_EXTRA_MASK_UFO_ALIGN)
                {
                    case GRALLOC_EXTRA_BIT_UFO_16_32:
                        lcu_align = &lcu_align_table[0];
                        break;
                    case GRALLOC_EXTRA_BIT_UFO_32_32: 
                        lcu_align = &lcu_align_table[1];
                        break;
                    case GRALLOC_EXTRA_BIT_UFO_64_64: /* fall-through */ 
                    default:
                        lcu_align = &lcu_align_table[2];
                        break;
                }

                src_width     = ALIGN(src_info.width, lcu_align->w);
                src_height    = ALIGN(src_info.height, lcu_align->h);
                src_y_stride  = src_width;
                src_uv_stride = ALIGN(src_y_stride / 2, 16);

                plane_num = 2;
                {
                    int pic_size_y_bs = ALIGN(src_width * src_height, 4096);
                    int pic_size_ref = src_info.alloc_size;

                    src_offset[0] = pic_size_y_bs; 

                    src_size[0] = pic_size_y_bs;;
                    src_size[1] = pic_size_ref - pic_size_y_bs;
                    src_size[2] = 0;
                }
            }
            dp_in_fmt       = DP_COLOR_420_BLKP_UFO;
            break;

        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            src_y_stride    = src_info.width * 2;
            src_uv_stride   = 0;

            plane_num = 1;
            src_size_luma   = src_info.width * src_info.height * 2;
            src_size[0]     = src_size_luma;
            dp_in_fmt       = DP_COLOR_YUYV;
            break;

        default:
            MTK_LOGE("lock src buffer format not support %d\n", src_info.format);
            return -1;
        }

#ifdef MTK_PQ_SUPPORT
        int32_t tdshp_param = bq->mPoolId << 16 | 0x01;
        bltStream.setTdshp(tdshp_param);
#endif

        if (src_info.ion_fd >= 0)
        {
            bltStream.setSrcBuffer(src_info.ion_fd, src_size, plane_num);
        }
        else
        {
            lockret = guiExtAuxLockBuffer(src_buffer, LOCK_FOR_DP, &src_info, &src_yp);

            if (0 != lockret)
            {
                MTK_LOGE("lock src buffer fail");
                return -1;
            }

            unsigned int src_addr[3] = {0, 0, 0};
            src_addr[0] = (unsigned int)src_yp;
            src_addr[1] = src_addr[0] + src_offset[0];
            src_addr[2] = src_addr[0] + src_offset[1];
            bltStream.setSrcBuffer((void**)src_addr, src_size, plane_num);
        }

        DpRect src_roi;
        src_roi.x = 0;
        src_roi.y = 0;
        src_roi.w = src_info.width;
        src_roi.h = src_info.height;

        if (dp_in_fmt != DP_COLOR_420_BLKP_UFO)
        {
            bltStream.setSrcConfig(src_width, src_height, src_y_stride, src_uv_stride,
                    dp_in_fmt, guiExtAuxGetYUVColorSpace(&src_info, DP_PROFILE_BT601), eInterlace_None, &src_roi, DP_SECURE_NONE);
        }
        else
        {
            bltStream.setSrcConfig(src_width, src_height,
                    DP_COLOR_420_BLKP_UFO, eInterlace_None, &src_roi);
        }
    }

    /* set DST config */
    {
        int dst_stride;
        int dst_pitch_uv;

        switch(dst_info.format)
        {
            case HAL_PIXEL_FORMAT_YV12:
                plane_num = 3;
                dp_out_fmt = DP_COLOR_YV12;
                dst_stride =  dst_info.stride;
                dst_pitch_uv = ALIGN((dst_stride/2), 16);
                dst_size[0] = dst_stride * dst_info.height;
                dst_size[1] = dst_pitch_uv*(dst_info.height/ 2);
                dst_size[2] = dst_pitch_uv*(dst_info.height/ 2);
                break;

            case HAL_PIXEL_FORMAT_RGBA_8888:
                plane_num = 1;
                dp_out_fmt = DP_COLOR_RGBX8888;
                dst_stride = dst_info.stride * 4;
                dst_pitch_uv = 0;
                dst_size[0] = dst_stride * dst_info.height;
                break;

            default:
                if (!src_info.ion_fd < 0)
                {
                    guiExtAuxUnlockBuffer(src_buffer);
                }
                MTK_LOGE("Unsupported dst color format %d\n", dst_info.format);
                return -1;
        }

        if (dst_info.ion_fd >= 0)
        {
            bltStream.setDstBuffer(dst_info.ion_fd, dst_size, plane_num);
        }
        else
        {
            MTK_LOGE("dst is not a ion buffer");
            guiExtAuxUnlockBuffer(src_buffer);
            return -1;
        }

        int width_even;
        int height_even;
        DpRect dst_roi;

        /* Make sure the w and h are even numbers. */
        width_even = (dst_info.width % 2) ? dst_info.width - 1 : dst_info.width;
        height_even = (dst_info.height % 2) ? dst_info.height - 1 : dst_info.height;

        dst_roi.x = 0;
        dst_roi.y = 0;
        dst_roi.w = width_even;
        dst_roi.h = height_even;

        DP_PROFILE_ENUM out_dp_profile_enum = guiExtAuxGetYUVColorSpace(&src_info, DP_PROFILE_BT601);

        guiExtAuxSetYUVColorSpace(dst_buffer, out_dp_profile_enum);

        bltStream.setDstConfig(width_even, height_even, dst_stride, dst_pitch_uv,
                dp_out_fmt, out_dp_profile_enum, eInterlace_None, &dst_roi, DP_SECURE_NONE, false);
    }

    MTK_ATRACE_BEGIN("bltStream.invalidate()");
    if (bltStream.invalidate() != DP_STATUS_RETURN_SUCCESS)
    {
        MTK_LOGE("DpBlitStream invalidate failed");
        err = -1;
    }
    MTK_ATRACE_END();

    if (!(src_info.ion_fd >= 0))
    {
        guiExtAuxUnlockBuffer(src_buffer);
    }

    return err;
#else
    MTK_LOGE("do not support");
    return -1;
#endif
}

int GuiExtAuxAcquireBuffer(GuiExtAuxBufferQueueHandle bq, int * bufSlot, int * fence_fd)
{
    status_t err = 0;
    int pre_allocation_found = 0;

    if (bq->mPoolId > 0)
    {
        int slotIdx = bq->mCount;

        GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[slotIdx];

        if (auxitem->mPoolId == bq->mPoolId)
        {
            *bufSlot = auxitem->mBuf;
            *fence_fd = (auxitem->mFence != Fence::NO_FENCE) ? auxitem->mFence->dup() : -1;

            bq->mCount = (bq->mCount + 1) % bq->num_slots;

            pre_allocation_found = 1;

            MTK_LOGV("use pre-allocate buffer: idx=%d, poolId=%d", *bufSlot, auxitem->mPoolId);
        }
    }

    if (pre_allocation_found == 0)
    {
        BufferQueue::BufferItem item;

        err = bq->mBufferQueue->acquireBuffer(&item, 0);

        if (err != NO_ERROR) {
            MTK_LOGE("acquireBuffer fail(%d)", err);
            return -err;
        }

        GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[item.mBuf];

        if (item.mGraphicBuffer.get())
            auxitem->mGraphicBuffer = item.mGraphicBuffer;
        auxitem->mFence = item.mFence;
        auxitem->mFrameNumber = item.mFrameNumber;
        auxitem->mPoolId = 0;
        auxitem->mSrcBuffer = 0;

        *bufSlot = auxitem->mBuf;
        *fence_fd = (auxitem->mFence != Fence::NO_FENCE) ? auxitem->mFence->dup() : -1;

        MTK_LOGV("use private buffer idx=%d", *bufSlot);
    }

    return err;
}


int GuiExtAuxReleaseBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot, int fence_fd)
{
    status_t err = 0;

    sp<Fence> fence( (fence_fd >= 0) ? new Fence(fence_fd) : Fence::NO_FENCE );

    GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[bufSlot];

    if (auxitem->mPoolId > 0)
    {
        GuiExtClientConsumer &consumer(GuiExtClientConsumer::getInstance());
        err = consumer.release(auxitem->mPoolBuffer, auxitem->mPoolId, GUI_EXT_USAGE_GPU);
    }
    else
    {
        err = bq->mBufferQueue->releaseBuffer(
                auxitem->mBuf, auxitem->mFrameNumber,
                EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, fence);
    }

    return err;
}

int GuiExtAuxDequeueBuffer(GuiExtAuxBufferQueueHandle bq, int* bufSlot, int* fence_fd)
{
    ATRACE_CALL();

    int buf = -1;
    const int reqW = bq->width;
    const int reqH = bq->height;
    const int reqF = bq->format;
    sp<Fence> fence;
    const int SwapIntervalZero = bq->mSwapIntervalZero;
    sp<GraphicBuffer> gb;

    status_t err;

    if (reqW == -1 || reqH == -1)
    {
        MTK_LOGE("please call setSize() beforce dequeueBuffer().");
        return -1;
    }

    err = bq->mBufferQueue->dequeueBuffer(&buf, &fence, SwapIntervalZero, reqW, reqH, reqF, LOCK_FOR_DP);
    err = bq->mBufferQueue->requestBuffer(buf, &gb);

    GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[buf];

    auxitem->mGraphicBuffer = gb;
    auxitem->mFence = fence;
    auxitem->mPoolId = 0;

    *bufSlot = auxitem->mBuf;
    *fence_fd = (auxitem->mFence != Fence::NO_FENCE) ? auxitem->mFence->dup() : -1;

    return err;
}

int GuiExtAuxQueueBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot, int fence_fd)
{
    ATRACE_CALL();

    int64_t timestamp = systemTime(SYSTEM_TIME_MONOTONIC);
    bool isAutoTimestamp = true;
    const int ScalingMode = bq->mScalingMode;
    const int Transform = bq->mTransform;
    const int SwapIntervalZero = bq->mSwapIntervalZero;

    Rect crop(bq->width, bq->height);
    sp<Fence> fence( (fence_fd >= 0) ? new Fence(fence_fd) : Fence::NO_FENCE );
    IGraphicBufferProducer::QueueBufferOutput output;
    IGraphicBufferProducer::QueueBufferInput input(timestamp, isAutoTimestamp, crop, ScalingMode, Transform, SwapIntervalZero, fence);

    status_t err = bq->mBufferQueue->queueBuffer(bufSlot, input, &output);

    return err;
}
android_native_buffer_t * GuiExtAuxRequestBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot)
{
    return bq->mSlots[bufSlot].mGraphicBuffer->getNativeBuffer();
}

