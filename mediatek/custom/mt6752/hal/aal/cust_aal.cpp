
extern "C" {

// The LCM number which this project can support
int LCM_COUNT = 1;

// The gamma of LCM
// This value must be configured correctly.
// LcmGamma[0] is for LCM 0, LcmGamma[1] is for LCM 1, ... etc,
// if this project supports multiple LCM.
float LcmGamma[] = { 2.2 };


// --------------------------------------------------------------------------
//  Behavior configuration
// --------------------------------------------------------------------------

// The default brightness of auto backlight in [0, 255]
// Larger value means DARKER
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int BrightnessLevel[] = { 128 };

// Brightening speed of auto backlight in [0, 255]
// Larger value means faster
int BrighteningSpeedLevel = 128;

// Darkening speed of auto backlight in [0, 255]
// Larger value means faster
int DarkeningSpeedLevel = 13;

// The enhancement level of DRE in [0, 255]
// Larger value means stronger
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int ReadabilityLevel[] = { 128 };

// Strength of Content-adaptive backlight control
// In [0, 255]
// This function could intelligently reduce backlight to save power according to content.
// The larger SmartBacklightStrength value, the more power saving.
// However, the excessive large value may degrade image¡¦s brightness. 
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int SmartBacklightStrength[] = { 128 };

// Effective range of Content-adaptive backlight control
// In [0, 255]
// This parameter influences the image type which SmartBacklightStrength applied.
// Using small SmartBacklightRange value, SmartBacklightStrength only effects on bright image scene.
// SmartBacklightStrength effects on mid-bright image scene by using larger SmartBacklightStrength value.
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int SmartBacklightRange[] = { 128 };

// Minimum backlight value of AAL output
// In [0, 1023]
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int MinOutBL[] = { 0 };

}

