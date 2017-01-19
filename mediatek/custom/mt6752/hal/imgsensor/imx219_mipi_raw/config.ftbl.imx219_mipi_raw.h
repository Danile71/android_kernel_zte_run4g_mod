//#ifndef _MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBL__H_
//#define _MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBL__H_
#if 1
//


/*******************************************************************************
 *
 ******************************************************************************/
FTABLE_DEFINITION(SENSOR_DRVNAME_IMX219_MIPI_RAW)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
    //==========================================================================
#if 1
    //  Scene Mode
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_SCENE_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::SCENE_MODE_AUTO), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::SCENE_MODE_AUTO,           
//                MtkCameraParameters::SCENE_MODE_NORMAL, 
                MtkCameraParameters::SCENE_MODE_PORTRAIT,       
                MtkCameraParameters::SCENE_MODE_LANDSCAPE, 
                MtkCameraParameters::SCENE_MODE_NIGHT,          
                MtkCameraParameters::SCENE_MODE_NIGHT_PORTRAIT, 
                MtkCameraParameters::SCENE_MODE_THEATRE,        
                MtkCameraParameters::SCENE_MODE_BEACH, 
                MtkCameraParameters::SCENE_MODE_SNOW,           
                MtkCameraParameters::SCENE_MODE_SUNSET, 
                MtkCameraParameters::SCENE_MODE_STEADYPHOTO,    
                MtkCameraParameters::SCENE_MODE_FIREWORKS, 
                MtkCameraParameters::SCENE_MODE_SPORTS,         
                MtkCameraParameters::SCENE_MODE_PARTY, 
                MtkCameraParameters::SCENE_MODE_CANDLELIGHT, 
                MtkCameraParameters::SCENE_MODE_HDR, 
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Effect
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_EFFECT), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::EFFECT_NONE), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::EFFECT_NONE,       
                MtkCameraParameters::EFFECT_MONO,   
                MtkCameraParameters::EFFECT_NEGATIVE,
                MtkCameraParameters::EFFECT_SEPIA,      
                MtkCameraParameters::EFFECT_AQUA,   
                MtkCameraParameters::EFFECT_WHITEBOARD, 
                MtkCameraParameters::EFFECT_BLACKBOARD, 
                #ifdef MTK_CAM_LOMO_SUPPORT
                MtkCameraParameters::EFFECT_POSTERIZE,
                MtkCameraParameters::EFFECT_NASHVILLE,
                MtkCameraParameters::EFFECT_HEFE,
                MtkCameraParameters::EFFECT_VALENCIA ,
                MtkCameraParameters::EFFECT_XPROII ,
                MtkCameraParameters::EFFECT_LOFI,
                MtkCameraParameters::EFFECT_SIERRA ,
//                MtkCameraParameters::EFFECT_KELVIN ,
                MtkCameraParameters::EFFECT_WALDEN ,
//                MtkCameraParameters::EFFECT_F1977 ,
                #endif
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Picture Size (Both width & height must be 16-aligned)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_PICTURE_SIZE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("2560x1920"), 
            ITEM_AS_VALUES_(
                "320x240",      "640x480",      "1024x768",     "1280x720",     "1280x768",     "1280x960", 
                "1600x1200",    "1920x1088",    "2048x1536",    "2560x1440",    "2560x1920",    "3264x2448",
                "3328x1872",    "2880x1728",    "3600x2160",    "4096x2304",    "4096x3072",    "4160x3120"
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Preview Size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_PREVIEW_SIZE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("640x480"), 
            ITEM_AS_VALUES_(
                "176x144",      "320x240",      "352x288",      "480x320",      "480x368", 
                "640x480",      "720x480",      "800x480",      "800x600",      "864x480", 
                "960x540",      "1280x720",     "1440x1080",    "1920x1080",    "1920x1088",
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Video Size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_SIZE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("640x480"), 
            ITEM_AS_VALUES_(
                "176x144",      "480x320",      "640x480", 
                "864x480",      "1280x720",     "1920x1080",
                "3840x2160", 
            )
        ), 
    )
#endif
    //==========================================================================
#ifdef MTK_SLOW_MOTION_VIDEO_SUPPORT
    //  High Speed Video Size
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HSVR_PRV_SIZE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("1280x720"), 
            ITEM_AS_VALUES_(
                "0x0",
                "1280x720",
                "0x0",
                "0x0",
            )
        ), 
    )

    //==========================================================================

    //  High Speed Video FPS
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HSVR_PRV_FPS), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("30"), 
            ITEM_AS_VALUES_(
                "0",
                "30",
                "0",
                "0",
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Preview Frame Rate Range
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_PREVIEW_FPS_RANGE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("30000,30000"), 
            ITEM_AS_USER_LIST_(
                "(15000,15000)", 
                "(24000,24000)", 
                "(30000,30000)",
#ifdef MTK_SLOW_MOTION_VIDEO_SUPPORT
                "(60000,60000)",
                "(120000,120000)",
#endif 
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Exposure Compensation
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_EXPOSURE_COMPENSATION), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"), 
            ITEM_AS_USER_LIST_(
                "-3",       //min exposure compensation index
                "3",        //max exposure compensation index
                "1.0",      //exposure compensation step; EV = step x index
            )
        ), 
        //......................................................................
        #if 1   //  SCENE HDR
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_HDR, 
            ITEM_AS_DEFAULT_("0"), 
            ITEM_AS_USER_LIST_(
                "0",        //min exposure compensation index
                "0",        //max exposure compensation index
                "1.0",      //exposure compensation step; EV = step x index
            )
        )
        #endif
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  Anti-banding (Flicker)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_ANTIBANDING), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::ANTIBANDING_OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::ANTIBANDING_OFF, 
                MtkCameraParameters::ANTIBANDING_50HZ, 
                MtkCameraParameters::ANTIBANDING_60HZ, 
                MtkCameraParameters::ANTIBANDING_AUTO, 
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Video Snapshot
#if (1 == VSS_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::TRUE), 
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
        ), 
    )
#endif
#endif
    //==========================================================================
#if 1
    //  Video Stabilization (EIS)
#if (1 == EIS_SUPPORTED)    
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_STABILIZATION), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(           
                MtkCameraParameters::TRUE            
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_STABILIZATION), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(            
                MtkCameraParameters::FALSE
            )
        ), 
    )
#endif
#endif
    //==========================================================================
#if 1
    //  Zoom
    FTABLE_CONFIG_AS_TYPE_OF_USER(
        KEY_AS_(MtkCameraParameters::KEY_ZOOM), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"),  //Zoom Index
            ITEM_AS_USER_LIST_(
                //Zoom Ratio
                "100", "114", "132", "151", "174", 
                "200", "229", "263", "303", "348", 
                "400", 
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  Zsd
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_ZSD_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF, 
                MtkCameraParameters::ON
            )
        ), 
    )
#endif
    //==========================================================================
#if 1
    //  (Shot) Capture Mode
    if(facing == 1)
    {
       FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
           KEY_AS_(MtkCameraParameters::KEY_CAPTURE_MODE), 
           SCENE_AS_DEFAULT_SCENE(
               ITEM_AS_DEFAULT_(MtkCameraParameters::CAPTURE_MODE_NORMAL), 
               ITEM_AS_VALUES_(
                   MtkCameraParameters::CAPTURE_MODE_NORMAL, 
                   MtkCameraParameters::CAPTURE_MODE_FACE_BEAUTY, 
                   MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT, 
                   MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT, 
                   MtkCameraParameters::CAPTURE_MODE_BEST_SHOT, 
                   MtkCameraParameters::CAPTURE_MODE_ASD_SHOT,
                   #ifdef MTK_MOTION_TRACK_SUPPORT
                   MtkCameraParameters::CAPTURE_MODE_MOTION_TRACK_SHOT,
                   #endif
               )
           ), 
       )
    }
    else
    {
    	  FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_CAPTURE_MODE), 
        SCENE_AS_DEFAULT_SCENE(
               ITEM_AS_DEFAULT_(MtkCameraParameters::CAPTURE_MODE_NORMAL), 
               ITEM_AS_VALUES_(
                   MtkCameraParameters::CAPTURE_MODE_NORMAL, 
                   MtkCameraParameters::CAPTURE_MODE_FACE_BEAUTY, 
                   MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT, 
                   MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT, 
                   MtkCameraParameters::CAPTURE_MODE_BEST_SHOT, 
                   #if (1 == MTK_CAM_AUTORAMA_SUPPORT)
                   MtkCameraParameters::CAPTURE_MODE_AUTO_PANORAMA_SHOT, 
                   #endif       
                   #if (1 == MTK_CAM_MAV_SUPPORT)
                   MtkCameraParameters::CAPTURE_MODE_MAV_SHOT,   
                   #endif	
                   MtkCameraParameters::CAPTURE_MODE_ASD_SHOT,
                   #ifdef MTK_MOTION_TRACK_SUPPORT
                   MtkCameraParameters::CAPTURE_MODE_MOTION_TRACK_SHOT,
                   #endif
               )
           ), 
       )
    }
#endif
    //==========================================================================
#if 1

    //	Video Hdr
    
#ifdef MTK_CAM_VHDR_SUPPORT
    
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_HDR), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF, 
                MtkCameraParameters::ON, 
            )
        ), 
    )
#else
    
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_HDR), 
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
    //  Video Face Beauty
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_FACE_BEAUTY), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(           
                MtkCameraParameters::TRUE            
            )
        ), 
    )
#endif
    //==========================================================================    
#if 1
    //	MFB
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_MFB_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF, 
                MtkCameraParameters::KEY_MFB_MODE_MFLL,
                MtkCameraParameters::KEY_MFB_MODE_AIS, 
            )
        ), 
    )
#endif
    //==========================================================================    
END_FTABLE_SCENE_INDEP()
//------------------------------------------------------------------------------
/*******************************************************************************
 *
 ******************************************************************************/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_DEP()
    //==========================================================================
#if 1
    //  Focus Mode
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FOCUS_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FOCUS_MODE_AUTO), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::FOCUS_MODE_AUTO,   
                MtkCameraParameters::FOCUS_MODE_MACRO, 
                MtkCameraParameters::FOCUS_MODE_INFINITY, 
                MtkCameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE, 
                MtkCameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO, 
                "manual",   "fullscan", 
            )
        ), 
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISO
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_ISO_SPEED), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("auto"), 
            ITEM_AS_VALUES_(
                "auto", 
            )
        ), 
        //......................................................................
        #if 1   //  SCENE AUTO
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_AUTO, 
            ITEM_AS_DEFAULT_("auto"), 
            ITEM_AS_VALUES_(
                "auto", "100", "200", "400", "800", "1600", 
            )
        )
        #endif
        //......................................................................
        #if 1   //  SCENE NORMAL
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_NORMAL, 
            ITEM_AS_DEFAULT_("auto"), 
            ITEM_AS_VALUES_(
                "auto", "100", "200", "400", "800", "1600", 
            )
        )
        #endif
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  White Balance.
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_WHITE_BALANCE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_AUTO), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT, 
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT, 
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT, 
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE, 
            )
        ), 
        //......................................................................
        #if 1   //  SCENE LANDSCAPE
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_LANDSCAPE, 
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_DAYLIGHT), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT, 
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT, 
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT, 
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE, 
            )
        )
        #endif
        //......................................................................
        #if 1   //  SCENE SUNSET
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_SUNSET, 
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_DAYLIGHT), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT, 
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT, 
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT, 
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE, 
            )
        )
        #endif
        //......................................................................
        #if 1   //  SCENE CANDLELIGHT
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_CANDLELIGHT, 
            ITEM_AS_DEFAULT_(MtkCameraParameters::WHITE_BALANCE_INCANDESCENT), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::WHITE_BALANCE_AUTO,            MtkCameraParameters::WHITE_BALANCE_INCANDESCENT, 
                MtkCameraParameters::WHITE_BALANCE_FLUORESCENT,     MtkCameraParameters::WHITE_BALANCE_WARM_FLUORESCENT, 
                MtkCameraParameters::WHITE_BALANCE_DAYLIGHT,        MtkCameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT, 
                MtkCameraParameters::WHITE_BALANCE_TWILIGHT,        MtkCameraParameters::WHITE_BALANCE_SHADE, 
            )
        )
        #endif
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Edge
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_EDGE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH, 
            )
        ), 
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Hue
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_HUE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH, 
            )
        ), 
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Saturation
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_SATURATION), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH, 
            )
        ), 
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Brightness
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_BRIGHTNESS), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH, 
            )
        ), 
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
#if 1
    //  ISP Contrast
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_CONTRAST), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::MIDDLE), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::LOW, MtkCameraParameters::MIDDLE, MtkCameraParameters::HIGH, 
            )
        ), 
        //......................................................................
        //......................................................................
    )
#endif
    //==========================================================================
END_FTABLE_SCENE_DEP()
//------------------------------------------------------------------------------
END_FTABLE_DEFINITION()


/*******************************************************************************
 *
 ******************************************************************************/
#endif
//#endif //_MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBL__H_

