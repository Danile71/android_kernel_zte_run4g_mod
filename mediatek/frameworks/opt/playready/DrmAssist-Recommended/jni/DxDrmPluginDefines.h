#ifndef __DX_DRM__PLUGIN_DEFINES_H__
#define __DX_DRM__PLUGIN_DEFINES_H__

//! Holds the size of one encrypted NAL unit and the size of its clear header
typedef struct __tagDxSubSample
{
	uint32_t dwClearDataSize;           /*!< Size of the data to be copied from the encBuffer to the output FD (decBuffer) */
	uint32_t dwEncryptedDataSize;       /*!< Size of the data to be decrypted into the decBuffer */
} DxSubSample;

//! The 'IV Optional buffer' (IV) passed to onDecrypt.
//! IMPORTANT: A big-enough buffer should be allocated so DxDecryptionSettings followed by multiple DxSubSample structures will be located in a contiguous memory.
//! The count of the DxSubSample structures following DxMultiSampleHeader is indicated by dwSubSamplesNum.
typedef struct __tagDxMultiSampleHeader
{
	uint64_t qwInitializationVector;  /*!< 8 bytes of the constant part of the initialization vector (IV, from the PIFF file) */
	uint32_t dwOutBufferOffset;       /*!< Offset into the output buffer to which decryption will be performed */
	uint32_t dwMediaOffset ;          /*!< Normally Will be zero. Will be non zero in case NAL units in the same frame will be passed in different calls. */
	uint32_t dwSubSamplesNum;         /*!< Number of DxSubSample structures following DxDecryptionSettings on the same contiguous memory */
} DxMultiSampleHeader;

//! Gets the required size in bytes of the DxMultiSampleHeader struct (including the multiple DxSubSample following it), based on _dwSubSamplesNum parameter.
#define DX_GET_IV_BUFFER_ACTUAL_LEN(_dwSubSamplesNum) \
	(sizeof(DxMultiSampleHeader) + (sizeof(DxSubSample) * (_dwSubSamplesNum)))

//! Gets a pointer to the beginning of the DxSubSample array following of the _IV_BUFFER (DxMultiSampleHeader struct) on the same contiguous memory, based on _IV_BUFFER parameter.
#define DX_GET_SAMPLE_ARR_PTR(_IV_BUFFER) \
	((DxSubSample*)(((char*)(_IV_BUFFER)) + sizeof(DxMultiSampleHeader)))

#endif /* __DX_DRM__PLUGIN_DEFINES_H__ */