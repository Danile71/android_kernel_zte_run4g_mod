#!/bin/bash
# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2014. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

##############################################################
# Program:
# Program to create ARM trusted firmware and tee binary
#

### BUILD MODE SETTING ###
if [ "${TARGET_BUILD_VARIANT}" = "" ] || [ "${TARGET_BUILD_VARIANT}" = "eng" ] ; then
TRUSTZONE_BUILD_MODE=Debug
else
TRUSTZONE_BUILD_MODE=Release
fi

if [ "${TRUSTZONE_BUILD_MODE}" = "Debug" ] ; then
TRUSTZONE_BUILD_MODE_LC=debug
else
TRUSTZONE_BUILD_MODE_LC=release
fi

### MTK_FILE_PATH ###
MTK_BUILD_TOOL_PATH=${MKTOPDIR}/mediatek/build/tools
MTK_MKIMAGE_TOOL=${MTK_BUILD_TOOL_PATH}/mkimage
TRUSTZONE_SIGN_TOOL=${MTK_BUILD_TOOL_PATH}/SignTool/TeeImgSignEncTool
MTK_CUSTOM_DIR=${MKTOPDIR}/mediatek/custom/${MTK_PROJECT}
TRUSTZONE_OUTPUT_PATH=${MKTOPDIR}/${OUT_DIR}/target/product/${MTK_PROJECT}/trustzone
TRUSTZONE_IMAGE_OUTPUT_PATH=${TRUSTZONE_OUTPUT_PATH}
TRUSTZONE_TEMP_PADDING_FILE=${TRUSTZONE_OUTPUT_PATH}/bin/padding.txt
#TRUSTZONE_PROTECT_BSP_PATH=${MKTOPDIR}/vendor/mediatek/proprietary/protect-bsp/platform/${ARCH_MTK_PLATFORM}/external/trustzone/trustlets
TRUSTZONE_PROTECT_BSP_PATH=${MKTOPDIR}/mediatek/protect-bsp/platform/${ARCH_MTK_PLATFORM}/external/trustzone/trustlets
TRUSTZONE_PROTECT_PRIVATE_PATH=${MKTOPDIR}/mediatek/protect-private/security/trustonic/platform/${ARCH_MTK_PLATFORM}/trustlets

### COMMON SETTINGS ###
TRUSTONZE_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/trustzone.bin
MKIMAGE_HDR_SIZE=0x240

### ATF SETTING ###
ATF_BUILD_PATH=${MKTOPDIR}/vendor/arm/atf
ATF_RAW_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_atf_${TRUSTZONE_BUILD_MODE_LC}_raw.img
ATF_SIGNED_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_atf_${TRUSTZONE_BUILD_MODE_LC}_signed.img
ATF_PADDING_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_atf_${TRUSTZONE_BUILD_MODE_LC}_pad.img
ATF_COMP_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_atf.img

### TEE SETTING ###
TBASE_BUILD_PATH=${MKTOPDIR}/vendor/trustonic/platform/${ARCH_MTK_PLATFORM}/t-base
TEE_RAW_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_tee_${TRUSTZONE_BUILD_MODE_LC}_raw.img
TEE_SIGNED_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_tee_${TRUSTZONE_BUILD_MODE_LC}_signed.img
TEE_PADDING_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_tee_${TRUSTZONE_BUILD_MODE_LC}_pad.img
TEE_COMP_IMAGE_NAME=${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin/${ARCH_MTK_PLATFORM}_tee.img

### CUSTOMIZTION FILES ###
TEE_SIGN_CFG=${MTK_CUSTOM_DIR}/trustzone/TRUSTZONE_IMG_PROTECT_CFG.ini
ATF_SIGN_CFG=${MTK_CUSTOM_DIR}/trustzone/TRUSTZONE_IMG_PROTECT_CFG.ini
TEE_DRAM_SIZE_CFG=${MTK_CUSTOM_DIR}/trustzone/SECURE_DRAM_SIZE_CFG.ini
if [ ! -f ${TEE_SIGN_CFG} ]; then
TEE_SIGN_CFG=${MKTOPDIR}/mediatek/custom/common/trustzone/TRUSTZONE_IMG_PROTECT_CFG.ini
fi
if [ ! -f ${ATF_SIGN_CFG} ]; then
ATF_SIGN_CFG=${MKTOPDIR}/mediatek/custom/common/trustzone/TRUSTZONE_IMG_PROTECT_CFG.ini
fi
if [ ! -f ${TEE_DRAM_SIZE_CFG} ]; then
TEE_DRAM_SIZE_CFG=${MKTOPDIR}/mediatek/custom/common/trustzone/SECURE_DRAM_SIZE_CFG.ini
fi
TEE_DRAM_SIZE=`cat ${TEE_DRAM_SIZE_CFG}`
TEE_TOTAL_DRAM_SIZE=0x$(echo "obase=16; $((${TEE_DRAM_SIZE} + ${MKIMAGE_HDR_SIZE}))" | bc)
PART_DEFAULT_MEMADDR=0xFFFFFFFF

##############################################################
# FUNCTIONS
#
function dump_build_info () 
{
    echo ""
    echo "============================================"
    echo "${MTK_PROJECT} tbase load"
    echo "${TRUSTONZE_IMAGE_NAME} built at"
    echo "time : $(date)"
    echo "img  size : $(stat -c%s "${TRUSTONZE_IMAGE_NAME}")" byte    
    echo "secure dram size = ${TEE_TOTAL_DRAM_SIZE}"
    echo "============================================"
}

function clean_trustzone () 
{
    echo "clean all trustzone output files"    
    rm -r -f ${TRUSTZONE_OUTPUT_PATH}/*
}

function check_zero_padding ()
{
    local FILE_PATH=$1
    local ALIGNMENT=$2
    local PADDING_SIZE=0
    local FILE_SIZE=$(stat -c%s "${FILE_PATH}")
    local REMAINDER=$((${FILE_SIZE} % ${ALIGNMENT}))

    if [ ${REMAINDER} -ne 0 ]; then
        PADDING_SIZE=$((${ALIGNMENT} - ${REMAINDER}))
        echo "[ERROR] File '${FILE_PATH}' size '${FILE_SIZE}' is not ${ALIGNMENT} bytes aligned"
        exit 1
    fi
}

function build_zero_padding_predict ()
{
    local FILE_PATH=$1
    local OUTPUT_PATH=$2
    local ALIGNMENT=$3
    local PADDING_SIZE=0
    local FILE_SIZE=$(stat -c%s "${FILE_PATH}")
    local REMAINDER=0
    local MKIMAGE_HDR_SIZE=512
    local RSA_SIGN_HDR_SIZE=576

    FILE_SIZE=$((${FILE_SIZE} + ${MKIMAGE_HDR_SIZE} + ${RSA_SIGN_HDR_SIZE}))
    REMAINDER=$((${FILE_SIZE} % ${ALIGNMENT}))

    cat ${FILE_PATH} > ${OUTPUT_PATH}
    if [ ${REMAINDER} -ne 0 ]; then
        PADDING_SIZE=$((${ALIGNMENT} - ${REMAINDER}))
        dd if=/dev/zero of=${TRUSTZONE_TEMP_PADDING_FILE} bs=$PADDING_SIZE count=1
        cat ${TRUSTZONE_TEMP_PADDING_FILE} >> ${OUTPUT_PATH}
        rm ${TRUSTZONE_TEMP_PADDING_FILE}
    fi
}

function build_atf ()
{
    echo "============================================"
    echo "Build for ARM trusted firmware"
    echo "============================================"
    local CURRENT_WK_DIR=`pwd`
    if [ "${MTK_ATF_SUPPORT}" = "yes" ] ; then
        cd ${ATF_BUILD_PATH}
        ./build.sh ${TRUSTZONE_BUILD_MODE} ${ATF_RAW_IMAGE_NAME} ${TRUSTZONE_IMAGE_OUTPUT_PATH} ${ARCH_MTK_PLATFORM} ${MTK_TEE_SUPPORT}
        if [ $? -ne 0 ]; then 
            echo "[ERROR] ARM trusted firmware build failed!"
            exit 1; 
        fi
        if [ ! -f ${ATF_RAW_IMAGE_NAME} ] ; then
            echo "[ERROR] ATF build failed!"
            exit 1
        fi
        cd ${CURRENT_WK_DIR}
        build_zero_padding_predict ${ATF_RAW_IMAGE_NAME} ${ATF_PADDING_IMAGE_NAME} 512
        ${TRUSTZONE_SIGN_TOOL} ${ATF_SIGN_CFG} ${ATF_PADDING_IMAGE_NAME} ${ATF_SIGNED_IMAGE_NAME} ${TEE_DRAM_SIZE}
        check_zero_padding ${ATF_SIGNED_IMAGE_NAME} 512
        ${MTK_MKIMAGE_TOOL} ${ATF_SIGNED_IMAGE_NAME} ATF ${PART_DEFAULT_MEMADDR} 0 ${ATF_COMP_IMAGE_NAME}
        check_zero_padding ${ATF_COMP_IMAGE_NAME} 512
    fi
}

function build_tee ()
{
    echo "============================================"
    echo "Build for secure OS"
    echo "============================================"
    local CURRENT_WK_DIR=`pwd`
    if [ "${MTK_TEE_SUPPORT}" = "yes" ] ; then
        if [ "${TRUSTONIC_TEE_SUPPORT}" = "yes" ] ; then
            cd ${TBASE_BUILD_PATH}
            ./build.sh ${TRUSTZONE_BUILD_MODE} ${TEE_RAW_IMAGE_NAME} ${TRUSTZONE_IMAGE_OUTPUT_PATH} ${TRUSTZONE_PROTECT_BSP_PATH} ${TRUSTZONE_PROTECT_PRIVATE_PATH}
            if [ $? -ne 0 ]; then 
                echo "[ERROR] TRUSTONIC TEE build failed!"
                exit 1; 
            fi
            if [ ! -f ${TEE_RAW_IMAGE_NAME} ] ; then
                echo "[ERROR] TRUSTONIC TEE build failed, image is not generated!"
                exit 1
            fi
            cd ${CURRENT_WK_DIR}
            build_zero_padding_predict ${TEE_RAW_IMAGE_NAME} ${TEE_PADDING_IMAGE_NAME} 512
            ${TRUSTZONE_SIGN_TOOL} ${TEE_SIGN_CFG} ${TEE_PADDING_IMAGE_NAME} ${TEE_SIGNED_IMAGE_NAME} ${TEE_DRAM_SIZE}
            check_zero_padding ${TEE_SIGNED_IMAGE_NAME} 512
            ${MTK_MKIMAGE_TOOL} ${TEE_SIGNED_IMAGE_NAME} TEE ${TEE_TOTAL_DRAM_SIZE} 0 ${TEE_COMP_IMAGE_NAME}
            check_zero_padding ${TEE_COMP_IMAGE_NAME} 512
        fi
    fi
}

function build_trustzone_bin ()
{
    echo "============================================"
    echo "Build for trustzone binary"
    echo "============================================"

    rm -rf ${TRUSTONZE_IMAGE_NAME}
    echo ./${TRUSTONZE_IMAGE_NAME}

    if [ "${MTK_ATF_SUPPORT}" = "yes" ] ; then
        cat ${ATF_COMP_IMAGE_NAME} >> ${TRUSTONZE_IMAGE_NAME}
    fi
    
    if [ "${MTK_TEE_SUPPORT}" = "yes" ] ; then
        if [ "${TRUSTONIC_TEE_SUPPORT}" = "yes" ] ; then
            cat ${TEE_COMP_IMAGE_NAME} >> ${TRUSTONZE_IMAGE_NAME}
        fi
    fi

    if [ $? -ne 0 ]; then exit 1; fi

    if [ ! -f ${TRUSTONZE_IMAGE_NAME} ] ; then
        echo "[ERROR] TRUSTZONE partition build failed!"
        exit 1
    fi
}

function build_configuration ()
{
    echo "============================================"
    echo "Build Configurations"
    echo "============================================"
    echo "MTK_ATF_SUPPORT = ${MTK_ATF_SUPPORT}"
    echo "MTK_TEE_SUPPORT = ${MTK_TEE_SUPPORT}"
    echo "TRUSTONIC_TEE_SUPPORT = ${TRUSTONIC_TEE_SUPPORT}"
    echo "TEE_DRAM_SIZE = ${TEE_DRAM_SIZE}"
    echo "TEE_TOTAL_DRAM_SIZE = ${TEE_TOTAL_DRAM_SIZE}"
    echo "TARGET_BUILD_VARIANT = ${TARGET_BUILD_VARIANT}"
    echo "TRUSTZONE_BUILD_MODE = ${TRUSTZONE_BUILD_MODE}"

    mkdir -p ${TRUSTZONE_IMAGE_OUTPUT_PATH}/bin
}

##############################################################
# Main Flow
#
case "$1" in
    clean)
        clean_trustzone;
        exit 0;
    ;;
    *)
        build_configuration;
        build_atf;
        build_tee;
        build_trustzone_bin;
        dump_build_info;
    ;;
esac
