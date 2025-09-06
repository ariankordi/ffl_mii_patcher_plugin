#include "ffl_patches.h"

#ifdef __WIIU__
#include "notifications/notifications.h"
#include "utils/logger.h"
    #ifdef DEBUG
    #include <cstdio>
    #endif
#endif

#include "ffl_types.h"
//#include "utils/base64enc.h"

/// Red constant color for testing.
static constexpr FFLColor cColorRed       { 1.0f, 0.0f, 0.0f, 1.0f };

/// nn::mii::detail::EyeWhiteColor, nn::mii::detail::TeethColor
static constexpr FFLColor cColorWhite     { 1.0f, 1.0f, 1.0f, 1.0f };
/// nn::mii::detail::EyeShadowColor (Cyan)
static constexpr FFLColor cColorEyeShadow { 0.0f, 1.0f, 1.0f, 1.0f };
// While ffl_patches.h DEFINES functions, we need to DECLARE functions here.

#include "ffl_colors.h"

DECL_FUNCTION(const void*, FFLiGetHairColor, int colorIndex);
// real_ pointer will be written by FunctionPatcher.
const void* my_FFLiGetHairColor(int colorIndex) {
    if ((colorIndex & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) == 0) {
        return real_FFLiGetHairColor(colorIndex);
    }
    // return reinterpret_cast<const void*>(&cColorRed);

    const int i = colorIndex & FFLI_NN_MII_COMMON_COLOR_MASK;
    return reinterpret_cast<const void*>(&nnmiiCommonColors[i][1]);
}

DECL_FUNCTION(const void*, FFLiGetGlassColor, int colorIndex);
const void* my_FFLiGetGlassColor(int colorIndex) {
    if ((colorIndex & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) == 0) {
        return real_FFLiGetGlassColor(colorIndex);
    }
    const int i = colorIndex & FFLI_NN_MII_COMMON_COLOR_MASK;
    return reinterpret_cast<const void*>(&nnmiiCommonColors[i][1]);
}

DECL_FUNCTION(const void*, FFLiGetFacelineColor, int colorIndex);
const void* my_FFLiGetFacelineColor(int colorIndex) {
    // Get sRGB color for now.
    return reinterpret_cast<const void*>(&nnmiiFacelineColors[colorIndex][1]);
    // return real_FFLiGetHairColor(colorIndex);
}

DECL_FUNCTION(int, FFLiVerifyCharInfoWithReason, void* pInfo, int nameCheck);
int my_FFLiVerifyCharInfoWithReason(void* pInfo, int nameCheck) {

    FFLiCharInfo prevInfo;
    memcpy(&prevInfo, pInfo, sizeof(FFLiCharInfo));

    FFLiCharInfo& info = *reinterpret_cast<FFLiCharInfo*>(pInfo);
    // use placeholder colors to spoof verification process
    info.hair.color = 4; // gray
    info.eye.color = 1; // gray
    info.eyebrow.color = 4; // gray
    info.glass.color = 5; // dark gray

    info.faceline.color = 0; // white
    info.beard.color = 1; // gray
    info.mouth.color = 0;

    int result = real_FFLiVerifyCharInfoWithReason(pInfo, nameCheck);
#ifdef __WIIU__
    if (result != 0 &&
        result != 21 && // this is encountered when inputting null
                        // which a lot of games like to do for some reason
        result < static_cast<int>(FFLiVerifyReasonStrings.size()) // Maximum.
    ) {
        char log[96];
        const char* str = FFLiVerifyReasonStrings.data()[result];
        snprintf(log, sizeof(log), "charinfo verify fail: %d (%s)", result, str);

        DEBUG_FUNCTION_LINE_INFO("%s", log);
        NotificationModule_AddErrorNotification(log);
    }
#endif
    // return 0; // FFLI_VERIFY_REASON_OK

    memcpy(pInfo, &prevInfo, sizeof(FFLiCharInfo));
/*
    char base64[BASE64_ENCODED_SIZE(sizeof(FFLiCharInfo))];
    base64_encode(static_cast<const uint8_t*>(pInfo), sizeof(FFLiCharInfo), base64);
    DEBUG_FUNCTION_LINE_VERBOSE("charinfo after verify: %s", base64);
*/
    return result;
}

DECL_FUNCTION(void, FFLiMiiDataCore2CharInfo, void* dst, const void* src, char16_t* creatorName, int birthday);
void my_FFLiMiiDataCore2CharInfo(void* dst, const void* src, char16_t* creatorName, int birthday) {
#ifdef __WIIU__
/*
    if (creatorName !== nullptr) { // official
        //uint8_t official[92];
        //memcpy(official, src, 92);
        //real_FFLiStoreData_SwapEndian(official);
        char base64[BASE64_ENCODED_SIZE(92)];
        base64_encode(static_cast<const uint8_t*>(base64), 92, base64);

        char log[192];
        snprintf(log, sizeof(log), "mii input: %s", base64);
        DEBUG_FUNCTION_LINE_INFO("%s", log);
        NotificationModule_AddInfoNotification(log);
    }
*/
#endif

    real_FFLiMiiDataCore2CharInfo(dst, src, creatorName, birthday);
}

//void my_FFLiStoreData_SwapEndian(void *self) {
//    return real_FFLiStoreData_SwapEndian(self);
//}

DECL_FUNCTION(void, FFLiInitModulateEye, void* pParam, int colorGB, int colorR, const void* pTexture);
// real_ pointer will be written by FunctionPatcher.
void my_FFLiInitModulateEye(void* pParam, int colorGB, int colorR, const void* pTexture) {
    if ((colorGB & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) == 0) {
        return real_FFLiInitModulateEye(pParam, colorGB, colorR, pTexture);
    }

    FFLModulateParam& param = *reinterpret_cast<FFLModulateParam*>(pParam);
    param.mode = FFL_MODULATE_MODE_RGB_LAYERED;
    param.type = FFL_MODULATE_TYPE_EYE;
    param.pGX2Texture = pTexture;

    // Color G, aka: nn::mii::detail::EyeWhiteColor (the sclera)
    // https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiColor.cpp#L281
    param.pColorG = &cColorWhite;
    // Color R, aka: nn::mii::detail::EyeShadowColor
    // While there are three eye shadow colors: https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiColor.cpp#L275
    // ... only index 2 is used: https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiColor.cpp#L496
    param.pColorR = &cColorEyeShadow;

    // Color B is the inner eye color.
    // (Use colorGB for eye color index.)
    // param.pColorB = &cColorRed;
    const int i = colorGB & FFLI_NN_MII_COMMON_COLOR_MASK;
    // DEBUG_FUNCTION_LINE_INFO("get eye color: %u, float: %f %f %f\n", i, nnmiiCommonColors[i][1].r, nnmiiCommonColors[i][1].g, nnmiiCommonColors[i][1].b);
    param.pColorB = &nnmiiCommonColors[i][1];
}

DECL_FUNCTION(void, FFLiInitModulateMouth, void* pParam, int color, const void* pTexture);
// real_ pointer will be written by FunctionPatcher.
void my_FFLiInitModulateMouth(void* pParam, int color, const void* pTexture) {
    if ((color & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) == 0) {
        return real_FFLiInitModulateMouth(pParam, color, pTexture);
    }

    FFLModulateParam& param = *reinterpret_cast<FFLModulateParam*>(pParam);
    param.mode = FFL_MODULATE_MODE_RGB_LAYERED;
    param.type = FFL_MODULATE_TYPE_MOUTH;
    param.pGX2Texture = pTexture;

    // Color B, aka: nn::mii::detail::TeethColor
    // https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiColor.cpp#L319
    param.pColorB = &cColorWhite;

    // Color R, from the common color table.
    const int i = color & FFLI_NN_MII_COMMON_COLOR_MASK;
    param.pColorR = &nnmiiCommonColors[i][1];
    // param.pColorR = &cColorRed;

    // Color G, from: nn::mii::detail::UpperLipColorTable
    param.pColorG = &cColorEyeShadow; // Cyan for testing.
}
