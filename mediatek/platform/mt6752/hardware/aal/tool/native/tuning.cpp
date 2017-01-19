#define LOG_TAG "AAL-Tuning"

#include <cutils/xlog.h> 
#include <cstring>
#include <stdio.h>
#include "AALClient.h"

using namespace android;

#define NAME_BRIGHTNESS         "Brightness level"
#define NAME_BRIGHTENING_SPEED  "Brightening speed leve"
#define NAME_DARKENING_SPEED    "Darkening speed leve"
#define NAME_READABILITY        "Readability level"
#define NAME_SMART_BL_STRENGTH  "Smart backlight strength"
#define NAME_SMART_BL_RANGE     "Smart backlight range"


static void print_usages()
{
    printf(
        "aal-tune show\n"
        "    Show current configuration.\n"
        "\n"
        "aal-tune [Parameter] [LevelValue]\n"
        "\n"
        "Parameters:\n"
        "    Brightness / BR\n"
        "    DarkeningSpeed / DS\n"
        "    BrighteningSpeed / BS\n"
        "    Readability / RD\n"
        "    SmartBLStrength / SBS\n"
        "    SmartBLRange / SBR\n"
        "\n"
        "Level values should be in [0, 255]\n"
        "\n"
        "Example:\n"
        "# ./aaltune BS 170\n"
        );
}


int main(int argc, char *argv[])
{	
    const char *param = "";
    int32_t ret = 0;
    int32_t level = 0;
    const char *name;

    if (argc == 1) {
        print_usages();
        return 0;
    }

    param = argv[1];

    AALClient &client(AALClient::getInstance());
    
    if (strcmp(param, "show") == 0) {
        AALParameters params;
        ret = client.getParameters(&params);
        if (ret == 0) {
            printf(
                "%-30s: %3d\n%-30s: %3d\n%-30s: %3d\n%-30s: %3d\n%-30s: %3d\n-30s: %3d\n",
                NAME_BRIGHTNESS, params.brightnessLevel,
                NAME_DARKENING_SPEED, params.darkeningSpeedLevel,
                NAME_BRIGHTENING_SPEED, params.brighteningSpeedLevel,
                NAME_READABILITY, params.readabilityLevel,
                NAME_SMART_BL_STRENGTH, params.smartBacklightStrength,
                NAME_SMART_BL_RANGE, params.smartBacklightRange);
        } else {
            printf("Get AAL parameters failed.\n");
        }
        
        return 0;
    }

    if (argc < 3) {
        printf("Wrong argument.\n");
        print_usages();
        return 0;
    }
        
    level = atoi(argv[2]);
    name = "";
    
    if (!(0 <= level && level <= 255)) {
        printf("Level value should be in [0, 255]");
        ret = -1;
    } else if (strcmp(param, "Brightness") == 0 || strcmp(param, "BR") == 0) {
        ret = client.setBrightnessLevel(level);
        name = NAME_BRIGHTNESS;
    } else if (strcmp(param, "DarkeningSpeed") == 0 || strcmp(param, "DS") == 0) {
        ret = client.setDarkeningSpeedLevel(level);
        name = NAME_DARKENING_SPEED;
    } else if (strcmp(param, "BrighteningSpeed") == 0 || strcmp(param, "BS") == 0) {
        ret = client.setBrighteningSpeedLevel(level);
        name = NAME_BRIGHTENING_SPEED;
    } else if (strcmp(param, "Readability") == 0 || strcmp(param, "RD") == 0) {
        ret = client.setReadabilityLevel(level);
        name = NAME_READABILITY;
    } else if (strcmp(param, "SmartBLStrength") == 0 || strcmp(param, "SBS") == 0) {
        ret = client.setSmartBacklightStrength(level);
        name = NAME_SMART_BL_STRENGTH;
    } else if (strcmp(param, "SmartBLRange") == 0 || strcmp(param, "SBR") == 0) {
        ret = client.setSmartBacklightRange(level);
        name = NAME_SMART_BL_RANGE;
    } else {
        printf("Invalid parameter name: %s\n", param);
        ret = -1;
    }

    if (ret == 0) {
        printf("%s(%s) = %d OK", name, param, level);
        XLOGI("%s(%s) = %d", name, param, level);
    }
        
    return 0;
}
