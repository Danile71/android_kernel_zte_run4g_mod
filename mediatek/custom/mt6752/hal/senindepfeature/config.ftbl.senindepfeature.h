#ifndef _MTK_CUSTOM_PROJECT_HAL_SENINDEPFEATURE_CONFIGFTBL_H_
#define _MTK_CUSTOM_PROJECT_HAL_SENINDEPFEATURE_CONFIGFTBL_H_
#if 1

/*******************************************************************************
 *
 ******************************************************************************/
#define CUSTOM_SENINDEPFEATURE   "senindepfeature"
FTABLE_DEFINITION(CUSTOM_SENINDEPFEATURE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
    //==========================================================================

#if 1
    //  3DNR ON/OFF
#if (1 == NR3D_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_3DNR_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::ON), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_3DNR_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif
#endif

    //==========================================================================

#if 1
    //  STEREO REFOCUS ON/OFF
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_REFOCUS_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif

    //==========================================================================

#if 1
    //  STEREO DEPTHAF ON/OFF
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_DEPTHAF_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif

    //==========================================================================

#if 1
    //  STEREO DISTANCE ON/OFF
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_DISTANCE_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
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

