#include <climits>
#include <function_patcher/function_patching.h>
#include <function_patcher/fpatching_defines.h>
#include <notifications/notifications.h>

#include <array>
#include "ffl_types.h"
#include "patches.h" // handles
#include "ffl_colors.h" // FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK
#if DEBUG
#include <string>
#include "utils/logger.h"
#endif

/// Title IDs for Mii Maker / Miiスタジオ (Mii Studio).
static constexpr std::array titleIdsMiiStudio = std::to_array<uint64_t>({
    0x000500101004A100ULL, // USA
    // TODO: Add more regions. See if binaries are identical.
});

/*
// these functions get RGB colors for the buttons:
// 0x02085BE8 - paramColorGetSkin
// 0x02085C30 - paramColorGetHair
// 0x02085C78 - paramColorGetEye
// 0x02085CC0 - paramColorGetGlass
// 0x02085D08 - paramColorGetLip
DECL_FUNCTION(uint32_t, paramColorGetHair, uint32_t index, uint32_t param2);
uint32_t my_paramColorGetHair(uint32_t index, uint32_t param2) {
    const uint32_t realIndex = 0;
    return real_paramColorGetHair(realIndex, param2);
}
*/

/// This function is called in the selector and when
/// the edit/delete buttons are pressed. If it returns
/// false, the Mii is not considered a "user Mii"
/// and it won't prevent us from editing it.
DECL_FUNCTION(bool, isAccountMii, void* param_1);
bool my_isAccountMii(void* /* param_1 */) {
    return false; // li r3,0; blr
}

bool gHasShownEditWarning = false;

/*
/// This function is called in the editor when selecting a
/// category, and chooses the currently selected color button.
/// If it's skipped, no buttons are selected.
/// This is also what causes the underlying
/// crash at 02003b5c when an OOB color is passed.
/// @param param_2 This contains the color index.
DECL_FUNCTION(void, FUN_02003b48, int param_1, int param_2);
void my_FUN_02003b48(int param_1, int param_2) {
    // TODO: DOES NOT account for OOB faceline colors.
    if ((param_2 & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) != 0) {
        if (!gHasShownEditWarning) {
            gHasShownEditWarning = true;
            NotificationModule_AddErrorNotification("Editing extra colors is not supported right now. Sorry about that :(");
        }
        return; // skip this function, do not mark any color as selected
    }
    real_FUN_02003b48(param_1, param_2);
}
*/
DECL_FUNCTION(int, FUN_020cdd1c, FFLiCharInfo* pInfo, uint32_t type);
int my_FUN_020cdd1c(FFLiCharInfo* pInfo, uint32_t type) {
    if (type == 0 && // select color for faceline
        pInfo->faceline.color > 5 // Ver3FacelineColor_Max
    ) {
        return INT_MAX; // all bits set
    }
    // otherwise, pass through
    return real_FUN_020cdd1c(pInfo, type);
}

/// This function is called in the editor when loading a tab
/// for a specific part. It selects the button for the color/part.
/// This is what causes the underlying crash at
/// 02003b5c when an OOB color is passed.
/// If this is skipped, the buttons aren't selected.
DECL_FUNCTION(void, FUN_020d02d8, void* param_1, int type, int index);
void my_FUN_020d02d8(void* param_1, int type, int index) {
#if DEBUG
    const std::string message = "select type "+std::to_string(type)+", index "+std::to_string(index);
    NotificationModule_AddInfoNotification(message.c_str());
    DEBUG_FUNCTION_LINE_VERBOSE("%s", message.c_str());
#endif
    // is this using an out of bounds common color?
    // (or faceline color, set to INT_MAX)
    if ((index & FFLI_NN_MII_COMMON_COLOR_ENABLE_MASK) != 0) {
        if (!gHasShownEditWarning) {
            gHasShownEditWarning = true;
            NotificationModule_AddErrorNotification("Adding new colors on Wii U is unsupported right now. Sorry about that :(");
        }
        return; // skip this function, do not mark any color as selected
    }
    // otherwise mark button as pressed, like usual
    real_FUN_020d02d8(param_1, type, index);
}

const std::array
functionReplacementsForMiiStudio = std::to_array<function_replacement_data_t>({

    /*
    REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
        paramColorGetHair,
        titleIdsMiiStudio.data(), titleIdsMiiStudio.size(),
        "ffl_app.rpx",
        0x00085C30,
        0, 0xFFFF   // version range
    ),
    */
    REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
        isAccountMii,
        titleIdsMiiStudio.data(), titleIdsMiiStudio.size(),
        "ffl_app.rpx",
        0x000791F8,
        50, 50 // version v50
    ),
    /*
    REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
        FUN_020cdd1c,
        titleIdsMiiStudio.data(), titleIdsMiiStudio.size(),
        "ffl_app.rpx",
        0x000CDD1C,
        50, 50
    ),
    */
    REPLACE_FUNCTION_OF_EXECUTABLE_BY_ADDRESS_WITH_VERSION(
        FUN_020d02d8,
        titleIdsMiiStudio.data(), titleIdsMiiStudio.size(),
        "ffl_app.rpx",
        0x000D02D8,
        50, 50
    )

});

void addPatchesMiiStudio() {

    for (function_replacement_data_t pReplacement
        : functionReplacementsForMiiStudio) {
        PatchedFunctionHandle handle;
        if (auto st = FunctionPatcher_AddFunctionPatch(&pReplacement, &handle, nullptr);
                    st == FUNCTION_PATCHER_RESULT_SUCCESS) {
            // Set the new handle in the global array.
            gHandles[gHandleIndex++] = handle;
        }
    }

}
