LOCAL_PATH:= $(call my-dir)

objc_arc_opts_SRC_FILES := \
  ObjCARC.cpp \
  ObjCARCExpand.cpp \
  ObjCARCOpts.cpp \
  ObjCARCAPElim.cpp \
  ObjCARCAliasAnalysis.cpp \
  ObjCARCUtil.cpp \
  ObjCARCContract.cpp \
  DependencyAnalysis.cpp \
  ProvenanceAnalysis.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(objc_arc_opts_SRC_FILES)

LOCAL_MODULE:= libLLVMObjCARCOpts

LOCAL_MODULE_TAGS := optional

include $(LLVM_HOST_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(objc_arc_opts_SRC_FILES)

LOCAL_MODULE:= libLLVMObjCARCOpts

LOCAL_MODULE_TAGS := optional

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
