package com.mediatek.configurecheck2;

import com.mediatek.common.featureoption.FeatureOption;

public class Utils {
    
    public static final boolean IS_LTE_PHONE = 
            FeatureOption.MTK_LTE_SUPPORT;
    
    //DC=Dual Call
    public static final boolean IS_SGLTE_PHONE = 
            FeatureOption.MTK_LTE_SUPPORT 
            && FeatureOption.MTK_UMTS_TDD128_MODE 
            && FeatureOption.MTK_LTE_DC_SUPPORT;
    
    public static final boolean IS_CSFB_PHONE = 
            FeatureOption.MTK_LTE_SUPPORT 
            && FeatureOption.MTK_UMTS_TDD128_MODE 
            && !FeatureOption.MTK_LTE_DC_SUPPORT;
    
    public static final boolean IS_3_MODEMS = !FeatureOption.MTK_WORLD_PHONE;
    public static final boolean IS_5_MODEMS = FeatureOption.MTK_WORLD_PHONE;
    
    public static final boolean IS_SGLTE_3M = IS_SGLTE_PHONE && IS_3_MODEMS;
    public static final boolean IS_SGLTE_5M = IS_SGLTE_PHONE && IS_5_MODEMS;
    public static final boolean IS_CSFB_3M = IS_CSFB_PHONE && IS_3_MODEMS;
    public static final boolean IS_CSFB_5M = IS_CSFB_PHONE && IS_5_MODEMS;
}
