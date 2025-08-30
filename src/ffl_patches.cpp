#include "ffl_patches.h"

#ifdef __WIIU__
#include "notifications/notifications.h"
#include "utils/logger.h"
    #ifdef DEBUG
    #include <cstdio>
    #endif
#endif

#include "ffl_types.h"

/// Red constant color for testing.
static constexpr FFLColor cColorRed       { 1.0f, 0.0f, 0.0f, 1.0f };

/// nn::mii::detail::EyeWhiteColor, nn::mii::detail::TeethColor
static constexpr FFLColor cColorWhite     { 1.0f, 1.0f, 1.0f, 1.0f };
/// nn::mii::detail::EyeShadowColor (Cyan)
static constexpr FFLColor cColorEyeShadow { 0.0f, 1.0f, 1.0f, 1.0f };
// While ffl_patches.h DEFINES functions, we need to DECLARE functions here.

DECL_FUNCTION(const void*, FFLiGetHairColor, int colorIndex);
// real_ pointer will be written by FunctionPatcher.
const void* my_FFLiGetHairColor(int /*colorIndex*/) {
    // Example: Ignore colorIndex and force RED to see if it's hooked.
    // Return address of our static Vec4 in plugin .data.
    return reinterpret_cast<const void*>(&cColorRed);
    // return real_FFLiGetHairColor(colorIndex);
}

DECL_FUNCTION(int, FFLiVerifyCharInfoWithReason, void* info, int nameCheck);
int my_FFLiVerifyCharInfoWithReason(void* info, int nameCheck) {
    int result = real_FFLiVerifyCharInfoWithReason(info, nameCheck);
#ifdef __WIIU__
    if (result != 0 &&
        result != 21) { // this is encountered when inputting null
                        // which a lot of games like to do for some reason
        char log[96];
        snprintf(log, sizeof(log), "charinfo verify fail: %d", result);
        DEBUG_FUNCTION_LINE_INFO("%s", log);
        NotificationModule_AddErrorNotification(log);
    }
#endif
    // return 0; // FFLI_VERIFY_REASON_OK


    return result;
}

/**
 * @brief Calculates the maximum Base64 encoded length for a given number of bytes.
 * @details Base64 expands every 3 bytes into 4 characters, plus padding.
 * Add 1 extra for null terminator.
 * @param n Number of input bytes.
 * @return Maximum required output size in bytes.
 */
#define BASE64_ENCODED_SIZE(n) ((((n) + 2) / 3) * 4 + 1)

/**
 * @brief Encodes a binary buffer into Base64 text.
 * @param input Pointer to raw input bytes.
 * @param len Number of bytes in input.
 * @param output Pointer to destination buffer (must be at least BASE64_ENCODED_SIZE(len)).
 */
/*
static void base64_encode(const uint8_t* input, size_t len, char* output) {
    static const char cBase64Alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    size_t outIndex = 0;
    size_t i = 0;

    while (i + 2 < len) {
        // Take 3 bytes and split into 4 groups of 6 bits.
        int32_t triple = (input[i] << 16) | (input[i + 1] << 8) | input[i + 2];
        output[outIndex++] = cBase64Alphabet[(triple >> 18) & 0x3F];
        output[outIndex++] = cBase64Alphabet[(triple >> 12) & 0x3F];
        output[outIndex++] = cBase64Alphabet[(triple >> 6)  & 0x3F];
        output[outIndex++] = cBase64Alphabet[triple & 0x3F];
        i += 3;
    }

    // Handle remaining 1 or 2 bytes with padding.
    if (i < len) {
        int32_t triple = input[i] << 16;
        if (i + 1 < len) {
            triple |= input[i + 1] << 8;
        }

        output[outIndex++] = cBase64Alphabet[(triple >> 18) & 0x3F];
        output[outIndex++] = cBase64Alphabet[(triple >> 12) & 0x3F];

        if (i + 1 < len) {
            output[outIndex++] = cBase64Alphabet[(triple >> 6) & 0x3F];
            output[outIndex++] = '=';
        } else {
            output[outIndex++] = '=';
            output[outIndex++] = '=';
        }
    }

    // Null terminate.
    output[outIndex] = '\0';
}
*/

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
void my_FFLiInitModulateEye(void* pParam, int /*colorGB*/, int /*colorR*/, const void* pTexture) {
    //if ((colorGB & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) == 0) {
    //    return real_FFLiInitModulateEye(pParam, colorGB, colorR, pTexture);
    //}

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
    param.pColorB = &cColorRed;
}

DECL_FUNCTION(void, FFLiInitModulateMouth, void* pParam, int color, const void* pTexture);
// real_ pointer will be written by FunctionPatcher.
void my_FFLiInitModulateMouth(void* pParam, int /*color*/, const void* pTexture) {
    //if ((color & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) == 0) {
    //    return real_FFLiInitModulateMouth(pParam, color, pTexture);
    //}

    FFLModulateParam& param = *reinterpret_cast<FFLModulateParam*>(pParam);
    param.mode = FFL_MODULATE_MODE_RGB_LAYERED;
    param.type = FFL_MODULATE_TYPE_MOUTH;
    param.pGX2Texture = pTexture;

    // Color B, aka: nn::mii::detail::TeethColor
    // https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiColor.cpp#L319
    param.pColorB = &cColorWhite;

    // Color R, from the common color table.
    param.pColorR = &cColorRed;

    // Color G, from: nn::mii::detail::UpperLipColorTable
    param.pColorG = &cColorEyeShadow; // Cyan for testing.
}
