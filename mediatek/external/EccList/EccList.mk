#
# Ecc List XML File
# 

LOCAL_PATH := mediatek/external/EccList

OPTR := NONE
ifdef OPTR_SPEC_SEG_DEF
   ifneq ($(OPTR_SPEC_SEG_DEF),NONE)
       OPTR := $(word 1,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
   endif
endif

ifneq ($(wildcard $(LOCAL_PATH)/ecc_list_$(OPTR).xml),)
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/ecc_list_$(OPTR).xml:system/etc/ecc_list.xml
else
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/ecc_list.xml:system/etc/ecc_list.xml
endif
