#ifndef MTK_GRALLOC_EXTRA_H
#define MTK_GRALLOC_EXTRA_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <system/window.h>

#include <hardware/gralloc.h>

__BEGIN_DECLS

/** Deprecated methods and struct.
 * DO NOT use it for new project, they will be removed after "L".
 * Please use query/perform instead of using the following get/set methods.
 */
typedef struct gralloc_buffer_info_t {
    /* static number, never change */
    int width;
    int height;
    int stride;
    int format;
    int vertical_stride;
    int usage;

    /* change by setBufParameter() */
    int status;
} gralloc_buffer_info_t;

int gralloc_extra_getIonFd(buffer_handle_t handle, int *idx, int *num);
int gralloc_extra_getBufInfo(buffer_handle_t handle, gralloc_buffer_info_t* bufInfo);
int gralloc_extra_getSecureBuffer(buffer_handle_t handle, int *type, int *hBuffer);
int gralloc_extra_setBufParameter(buffer_handle_t handle, int mask, int value);
int gralloc_extra_getMVA(buffer_handle_t handle, void ** mvaddr);
int gralloc_extra_setBufInfo(buffer_handle_t handle, const char * str);
/** Deprecated methods and struct. END */

/* enum gralloc_extra_setBufParameter for */
#define GRALLOC_EXTRA_MAKE_BIT(start_bit, index)        ( (index) << (start_bit) )
#define GRALLOC_EXTRA_MAKE_MASK(start_bit, end_bit)     ( ( ((unsigned int)-1) >> (sizeof(int) * __CHAR_BIT__ - 1 - (end_bit) + (start_bit) ) ) << (start_bit) )
enum {
    /* TYPE: bit 0~1 */
    GRALLOC_EXTRA_BIT_TYPE_CPU          = GRALLOC_EXTRA_MAKE_BIT(0,0),
    GRALLOC_EXTRA_BIT_TYPE_GPU          = GRALLOC_EXTRA_MAKE_BIT(0,1),
    GRALLOC_EXTRA_BIT_TYPE_VIDEO        = GRALLOC_EXTRA_MAKE_BIT(0,2),
    GRALLOC_EXTRA_BIT_TYPE_CAMERA       = GRALLOC_EXTRA_MAKE_BIT(0,3),
    GRALLOC_EXTRA_MASK_TYPE             = GRALLOC_EXTRA_MAKE_MASK(0,1),

    /* UFO alignment: bit 2~3 */
    GRALLOC_EXTRA_BIT_UFO_16_32         = GRALLOC_EXTRA_MAKE_BIT(2,1),
    GRALLOC_EXTRA_BIT_UFO_32_32         = GRALLOC_EXTRA_MAKE_BIT(2,2),
    GRALLOC_EXTRA_BIT_UFO_64_64         = GRALLOC_EXTRA_MAKE_BIT(2,3),
    GRALLOC_EXTRA_MASK_UFO_ALIGN        = GRALLOC_EXTRA_MAKE_MASK(2,3),

    /* ClearMotion: bit 4~6 */
    GRALLOC_EXTRA_BIT_CM_YV12           = GRALLOC_EXTRA_MAKE_BIT(4,0),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK       = GRALLOC_EXTRA_MAKE_BIT(4,1),
    GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM   = GRALLOC_EXTRA_MAKE_BIT(4,2),
    GRALLOC_EXTRA_BIT_CM_YUYV           = GRALLOC_EXTRA_MAKE_BIT(4,3),
    GRALLOC_EXTRA_BIT_CM_I420           = GRALLOC_EXTRA_MAKE_BIT(4,4),
    GRALLOC_EXTRA_BIT_CM_YV12_DI        = GRALLOC_EXTRA_MAKE_BIT(4,5),
    GRALLOC_EXTRA_BIT_CM_I420_DI        = GRALLOC_EXTRA_MAKE_BIT(4,6),
    GRALLOC_EXTRA_BIT_CM_UFO            = GRALLOC_EXTRA_MAKE_BIT(4,7),
    GRALLOC_EXTRA_MASK_CM               = GRALLOC_EXTRA_MAKE_MASK(4,6),

    /* Secure switch: bit 7 */
    GRALLOC_EXTRA_BIT_NORMAL            = GRALLOC_EXTRA_MAKE_BIT(7,0),
    GRALLOC_EXTRA_BIT_SECURE            = GRALLOC_EXTRA_MAKE_BIT(7,1),
    GRALLOC_EXTRA_MASK_SECURE           = GRALLOC_EXTRA_MAKE_MASK(7,7),

    /* Orientation: bit 12~15 */
    GRALLOC_EXTRA_MASK_ORIENT           = GRALLOC_EXTRA_MAKE_MASK(12,15),

    /* Alpha: bit 16~23 */
    GRALLOC_EXTRA_MASK_ALPHA            = GRALLOC_EXTRA_MAKE_MASK(16,23),

    /* Blending: bit 24~25 */
    GRALLOC_EXTRA_MASK_BLEND            = GRALLOC_EXTRA_MAKE_MASK(24,25),

    /* Dirty Param: bit 26 */
    GRALLOC_EXTRA_MASK_DIRTY_PARAM      = GRALLOC_EXTRA_MAKE_MASK(26,26),

    /* FLUSH: bit 27 */
    GRALLOC_EXTRA_BIT_FLUSH             = GRALLOC_EXTRA_MAKE_BIT(27,0),
    GRALLOC_EXTRA_BIT_NOFLUSH           = GRALLOC_EXTRA_MAKE_BIT(27,1),
    GRALLOC_EXTRA_MASK_FLUSH            = GRALLOC_EXTRA_MAKE_MASK(27,27),

    /* DIRTY: bit 28 */
    GRALLOC_EXTRA_BIT_UNDIRTY           = GRALLOC_EXTRA_MAKE_BIT(28,0),
    GRALLOC_EXTRA_BIT_DIRTY             = GRALLOC_EXTRA_MAKE_BIT(28,1),
    GRALLOC_EXTRA_MASK_DIRTY            = GRALLOC_EXTRA_MAKE_MASK(28,28),

    /* YUV Color SPACE: bit 29~31 */
    GRALLOC_EXTRA_BIT_YUV_NOT_SET       = GRALLOC_EXTRA_MAKE_BIT(29,0),
    GRALLOC_EXTRA_BIT_YUV_BT601_NARROW  = GRALLOC_EXTRA_MAKE_BIT(29,1),
    GRALLOC_EXTRA_BIT_YUV_BT601_FULL    = GRALLOC_EXTRA_MAKE_BIT(29,2),
    GRALLOC_EXTRA_BIT_YUV_BT709_NARROW  = GRALLOC_EXTRA_MAKE_BIT(29,3),
    GRALLOC_EXTRA_BIT_YUV_BT709_FULL    = GRALLOC_EXTRA_MAKE_BIT(29,4),
    GRALLOC_EXTRA_MASK_YUV_COLORSPACE   = GRALLOC_EXTRA_MAKE_MASK(29,31),
};

/** Deprecated enum. */
enum {
    GRALLOC_EXTRA_BUFFER_TYPE               = GRALLOC_EXTRA_MASK_TYPE,
    GRALLOC_EXTRA_BUFFER_TYPE_VIDEO         = GRALLOC_EXTRA_BIT_TYPE_VIDEO,
    GRALLOC_EXTRA_STATUS_BIT                = GRALLOC_EXTRA_MASK_DIRTY,
    GRALLOC_EXTRA_STATUS_BUFFER_DIRTY_BIT   = GRALLOC_EXTRA_BIT_DIRTY,
};
/** Deprecated enum. END */

typedef enum {
    GRALLOC_EXTRA_OK,
    GRALLOC_EXTRA_UNKNOWN,
    GRALLOC_EXTRA_NOT_SUPPORTED,
    GRALLOC_EXTRA_NO_IMPLEMENTATION,
    GRALLOC_EXTRA_NOT_INIT,
    GRALLOC_EXTRA_INVALIDE_PARAMS,
    GRALLOC_EXTRA_INVALIDE_OPERATION,
    GRALLOC_EXTRA_ERROR,
} GRALLOC_EXTRA_RESULT;

typedef enum {
    /* buffer handle, output: int */
    GRALLOC_EXTRA_GET_ION_FD = 1,
    GRALLOC_EXTRA_GET_FB_MVA,
    GRALLOC_EXTRA_GET_SECURE_HANDLE,

    /* const value, output: int */
    GRALLOC_EXTRA_GET_WIDTH = 10,
    GRALLOC_EXTRA_GET_HEIGHT,
    GRALLOC_EXTRA_GET_STRIDE,
    GRALLOC_EXTRA_GET_VERTICAL_STRIDE,
    GRALLOC_EXTRA_GET_ALLOC_SIZE,
    GRALLOC_EXTRA_GET_FORMAT,
    GRALLOC_EXTRA_GET_USAGE,

    /* output: gralloc_extra_sf_info_t */
    GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO = 100,

    /* output: gralloc_extra_buf_debug_t */
    GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG,

	/* output: buffer_handle_t */
	GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC
} GRALLOC_EXTRA_ATTRIBUTE_QUERY;

typedef enum {
    /* input: gralloc_extra_sf_info_t */
    GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO = 100,

    /* input: gralloc_extra_buf_debug_t */
    GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG,

	/* input: NULL */
	GRALLOC_EXTRA_ALLOC_SECURE_BUFFER_HWC
} GRALLOC_EXTRA_ATTRIBUTE_PERFORM;

typedef struct _crop_t {
    int x, y, w, h;

#ifdef __cplusplus
    _crop_t():x(0),y(0),w(0),h(0) {}
#endif
} _crop_t;

typedef struct gralloc_extra_ion_sf_info_t {
    int magic;
    _crop_t src_crop;
    _crop_t dst_crop;
#ifdef __cplusplus
    const int status;
#else
    int status;
#endif
    int pool_id;

    // used for profiling latency
    unsigned int sequence;

    // used for secure handle
    // do not use direcly, but use query function
    // with GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC
    unsigned int secure_handle_hwc;

    // used for video frame
    unsigned int timestamp;

    // reserved data
    int reserved[2];

#ifdef __cplusplus
    gralloc_extra_ion_sf_info_t():
        magic(0),src_crop(),dst_crop(),
        status(0),pool_id(0),sequence(0),
        secure_handle_hwc(0),
        timestamp(0)
    {}
#endif
} gralloc_extra_ion_sf_info_t;

GRALLOC_EXTRA_RESULT gralloc_extra_sf_set_status(gralloc_extra_ion_sf_info_t * sf_info, int mask, int value);

typedef struct gralloc_extra_ion_debug_t {
    int data[4];
    char name[16];

#ifdef __cplusplus
    gralloc_extra_ion_debug_t():
        data(),
        name()
    {}
#endif
} gralloc_extra_ion_debug_t;

GRALLOC_EXTRA_RESULT gralloc_extra_query(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void * out_pointer);

GRALLOC_EXTRA_RESULT gralloc_extra_perform(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void * in_pointer);

__END_DECLS

#endif /* MTK_GRALLOC_EXTRA_H */
