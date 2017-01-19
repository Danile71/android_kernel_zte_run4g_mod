
#define LOG_TAG "GraphicBufferExtra_hal"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdint.h>
#include <errno.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <hardware/gralloc.h>
#include <hardware/gralloc_extra.h>

#include <ui/GraphicBufferExtra.h>

namespace android {
// ---------------------------------------------------------------------------

/* ----  IMPLEMENTATION macro  ---- */

#define GRALLOC_EXTRA_PROTOTYPE_(...)  ( __VA_ARGS__ )
#define GRALLOC_EXTRA_ARGS_(...)       ( __VA_ARGS__ )
#define GRALLOC_EXTRA_ARGS_2(...)      ( mExtraDev, __VA_ARGS__ )
#define GRALLOC_EXTRA_CHECK_FAIL_RETURN_(API, ERR)                      \
do {                                                                    \
    if (!mExtraDev)                                                     \
    {                                                                   \
        ALOGD("gralloc extra device is not supported");                 \
        return ERR;                                                     \
    }                                                                   \
                                                                        \
    if (!mExtraDev->API)                                                \
    {                                                                   \
        ALOGW("gralloc extra device " #API "(...) is not supported");   \
        return ERR;                                                     \
    }                                                                   \
} while(0)

#define GRALLOC_EXTRA_IMPLEMENTATION_(RET, ERR, API, PROTOTYPE, ARGS)   \
RET GraphicBufferExtra:: API PROTOTYPE                                  \
{                                                                       \
    ATRACE_CALL();                                                      \
    RET err;                                                            \
                                                                        \
    GRALLOC_EXTRA_CHECK_FAIL_RETURN_(API, ERR);                         \
                                                                        \
    err = mExtraDev-> API GRALLOC_EXTRA_ARGS_2 ARGS ;                   \
                                                                        \
    if (err < 0) ALOGW(#API"(...) failed %d", err);                     \
                                                                        \
    return err;                                                         \
}                                                                       \
extern "C" RET gralloc_extra_##API PROTOTYPE                            \
{                                                                       \
    return GraphicBufferExtra::get(). API ARGS;                         \
}

/* ----  IMPLEMENTATION start  ---- */

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getIonFd, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *idx, int *num), 
    GRALLOC_EXTRA_ARGS_(handle, idx, num)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getSecureBuffer, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int *type, int *hBuffer), 
    GRALLOC_EXTRA_ARGS_(handle, type, hBuffer)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getBufInfo, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo), 
    GRALLOC_EXTRA_ARGS_(handle, bufInfo)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, setBufParameter, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, int mask, int value), 
    GRALLOC_EXTRA_ARGS_(handle, mask, value)
)
	
GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, getMVA, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, void** mvaddr), 
    GRALLOC_EXTRA_ARGS_(handle, mvaddr)
)

GRALLOC_EXTRA_IMPLEMENTATION_(int, -1, setBufInfo, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, const char * str), 
    GRALLOC_EXTRA_ARGS_(handle, str)
)

GRALLOC_EXTRA_IMPLEMENTATION_(GRALLOC_EXTRA_RESULT, GRALLOC_EXTRA_NOT_SUPPORTED, query, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void * out_pointer), 
    GRALLOC_EXTRA_ARGS_(handle, attribute, out_pointer)
)

GRALLOC_EXTRA_IMPLEMENTATION_(GRALLOC_EXTRA_RESULT, GRALLOC_EXTRA_NOT_SUPPORTED, perform, 
    GRALLOC_EXTRA_PROTOTYPE_(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void * in_pointer), 
    GRALLOC_EXTRA_ARGS_(handle, attribute, in_pointer)
)

/* ----  IMPLEMENTATION start end  ---- */

#undef GRALLOC_EXTRA_CHECK_FAIL_RETURN_
#undef GRALLOC_EXTRA_IMPLEMENTATION_
#undef GRALLOC_EXTRA_PROTOTYPE_
#undef GRALLOC_EXTRA_ARGS_
#undef GRALLOC_EXTRA_ARGS_2

// ---------------------------------------------------------------------------
}; // namespace android

extern "C" GRALLOC_EXTRA_RESULT gralloc_extra_sf_set_status(gralloc_extra_ion_sf_info_t * sf_info, int mask, int value)
{
    int old_status;
    int new_status;

    new_status = old_status = sf_info->status;
    new_status = ( old_status & (~mask));
    new_status |= ( value & mask );

    /* Force modify the const member */
    *const_cast<int*>(&sf_info->status) = new_status;
    
    return GRALLOC_EXTRA_OK;
}

