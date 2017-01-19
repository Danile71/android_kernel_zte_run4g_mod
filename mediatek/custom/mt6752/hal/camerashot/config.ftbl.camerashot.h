#ifndef _MTK_CUSTOM_PROJECT_HAL_CAMERASHOT_CONFIGFTBLSHOT_H_
#define _MTK_CUSTOM_PROJECT_HAL_CAMERASHOT_CONFIGFTBLSHOT_H_
#if 1

/*******************************************************************************
 *
 ******************************************************************************/
#define CUSTOM_CAMERASHOT   "camerashot"
FTABLE_DEFINITION(CUSTOM_CAMERASHOT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
    //==========================================================================

#if 1
    //  C_Shot Indicator (CSI)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_CSHOT_INDICATOR), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::TRUE), 
            ITEM_AS_SUPPORTED_(
            #if 0
                MtkCameraParameters::FALSE
            #else
                MtkCameraParameters::TRUE
            #endif
            )
        ), 
    )
#endif

    //==========================================================================
END_FTABLE_SCENE_INDEP()
//------------------------------------------------------------------------------
END_FTABLE_DEFINITION()


#endif
#endif //_MTK_CUSTOM_PROJECT_HAL_CAMERASHOT_CONFIGFTBLSHOT_H_

