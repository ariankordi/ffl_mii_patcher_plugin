#pragma once

#include <function_patcher/fpatching_defines.h>
#include "utils/SignatureScanner.h"

/// Shortcut I'm using for making function_replacement_data_t types statically.
/// REPLACE_FUNCTION_VIA_ADDRESS -> ... REPLACE_FUNCTION_EX (Please see fpatching_defines.h if you confused)
#define DEFINE_REPLACE_FUNC(func) \
    static /*constexpr*/ function_replacement_data_t replacement_##func = REPLACE_FUNCTION_VIA_ADDRESS(func, 0, 0)
    // Addresses are set to 0 because we will change them later.
    // Not const because we may modify the addresses and reuse these.

extern "C" {
    // real_ pointer will be written by FunctionPatcher.
    /// https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiColor.cpp#L186
    inline DECL_FUNCTION(void*, FFLiGetHairColor, int colorIndex);
    /// https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/detail/FFLiCharInfo.cpp#L28
    inline DECL_FUNCTION(int, FFLiVerifyCharInfoWithReason, void* info, /*BOOL*/int nameCheck);
    /// https://github.com/ariankordi/ffl/blob/0fe8e687dac5963000e3214a2c54d9219c99d63f/src/FFLiMiiData.cpp#L146
    inline DECL_FUNCTION(void, FFLiMiiDataCore2CharInfo, void* dst, const void* src, char16_t* creatorName, /*BOOL*/int birthday);
    /// https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiMiiDataCore.cpp#L32
    // inline DECL_FUNCTION(void, FFLiStoreData_SwapEndian, void* self);
}

DEFINE_REPLACE_FUNC(FFLiGetHairColor);
DEFINE_REPLACE_FUNC(FFLiVerifyCharInfoWithReason);
DEFINE_REPLACE_FUNC(FFLiMiiDataCore2CharInfo);
// DEFINE_REPLACE_FUNC(FFLiStoreData_SwapEndian);

static constexpr SignatureSet cSignatureSetFFL = {
    .defs = {
        // Verifies Mii data.
        {
            .name = "FFLiVerifyCharInfoWithReason",
            .pHookInfo = &replacement_FFLiVerifyCharInfoWithReason,
            .words = {
                { 0x80BE0008, 0xFFFFFFFF }, // faceline color offset in FFLiCharInfo
                { 0x38600000, 0xFFFFFFFF }, // min = 0
                { 0x38800005, 0xFFFFFFFF }, // max = 5
                { 0x48000001, 0xFC000003 }  // bl FFLiRange<int>
            },
            .wordCount = 4,
            .resolveMode = SignatureResolveMode::FunctionStart,
            .branchWordIndex = 0
        },

        // Unpacks Mii data.
        {
            .name = "FFLiMiiDataCore2CharInfo",
            .pHookInfo = &replacement_FFLiMiiDataCore2CharInfo,
            .words = {
                { 0x55287F3E, 0xFFFFFFFF } // rlwinm r8,r9,0xf,0x1c,0x1f
            },
            .wordCount = 1,
            .resolveMode = SignatureResolveMode::FunctionStart,
            .branchWordIndex = 0
        },

        /*
        {
            .name = "FFLiStoreData_SwapEndian",
            .pHookInfo = &replacement_FFLiStoreData_SwapEndian,
            .words = {
                { 0xA07F005C, 0xFFFFFFFF },
                { 0x48000001, 0xFC000003 },
                { 0xA01F005E, 0xFFFFFFFF },
                { 0xB07F005C, 0xFFFFFFFF }
            },
            .wordCount = 4,
            .resolveMode = SignatureResolveMode::FunctionStart,
            .branchWordIndex = 0
        },
        // Indicator for the current gamma type.
        // Most all titles use sRGB, but these system
        // titles are using linear: men.rpx, (applets >) frd.rpx, inf.rpx
        {
            .name = "s_ContainerType",
            .pHookInfo = nullptr,
            .words = {
                { 0x4BFFFF91, 0xFFFFFFFF }, // bl InitializeColorContainerIfUninitialized
                { 0x3D801002, 0xFFFF0000 }, // lis r12,0x10??
                { 0x818C4370, 0xFFFF0000 }, // lwz r12,-0x????(r12) => s_ContainerType
                { 0x1C0C0370, 0xFFFFFFFF }, // mulli r0,12,0x370
            },
            .wordCount = 4,
            .resolveMode = SignatureResolveMode::Direct,
            .branchWordIndex = 2
        },
        */

        // Color getter functions.
        {
            .name = "FFLiGetHairColor via FFLiInitModulateShapeHair",
            .pHookInfo = &replacement_FFLiGetHairColor,
            .words = {
                // Opcode to match, then mask.
                { 0x38000004, 0xFFFFFFFF }, // li r0,0x4
                { 0x93FE0000, 0xFFFFFFFF }, // stw r31,0(r30)
                { 0x7C832378, 0xFFFFFFFF }, // or r4,r4,r4
                { 0x901E0004, 0xFFFFFFFF }, // stw r0,0x4(r30)
                // BL: opcode=18 (bits 31..26), AA=0 (bit1), LK=1 (bit0). Mask off LI.
                { 0x48000001, 0xFC000003 } // (top 6 bits + AA/LK) must match
            },
            .wordCount = 5,
            .resolveMode = SignatureResolveMode::BranchTarget,
            .branchWordIndex = 4,
            // .lastWordMask = 0xFC000003,   // top 6 bits + AA + LK must match
        },

        { // Hair color
            .name = "FFLiGetBeardColor", .pHookInfo = &replacement_FFLiGetHairColor,
            .words = {
                { 0x38000001, 0xFFFFFFFF }, // Same as hair, but 04 changed to 01
                { 0x93FE0000, 0xFFFFFFFF }, { 0x7C832378, 0xFFFFFFFF },
                { 0x901E0004, 0xFFFFFFFF }, { 0x48000001, 0xFC000003 }
            },
            .wordCount = 5, .resolveMode = SignatureResolveMode::BranchTarget, .branchWordIndex = 4,
        },
        // NOTE: Eyebrow and mustache are technically using
        // the SrgbFetch variants, meaning they ALWAYS NEED TO USE sRGB
        { // Hair color
            .name = "FFLiGetSrgbFetchEyebrowColor", .pHookInfo = &replacement_FFLiGetHairColor,
            .words = {
                { 0x3800000b, 0xFFFFFFFF }, // Same as hair, but 04 changed to 0b
                { 0x919E0000, 0xFFFFFFFF }, // Store in r12 instead of r31
                { 0x7C832378, 0xFFFFFFFF }, { 0x901E0004, 0xFFFFFFFF }, { 0x48000001, 0xFC000003 }
            },
            .wordCount = 5, .resolveMode = SignatureResolveMode::BranchTarget, .branchWordIndex = 4,
        },
        { // Hair color
            .name = "FFLiGetSrgbFetchMustacheColor", .pHookInfo = &replacement_FFLiGetHairColor,
            .words = {
                { 0x38000009, 0xFFFFFFFF }, // Same as mustache, but 09 instead of 0b
                { 0x919E0000, 0xFFFFFFFF }, { 0x7C832378, 0xFFFFFFFF },
                { 0x901E0004, 0xFFFFFFFF }, { 0x48000001, 0xFC000003 }
            },
            .wordCount = 5, .resolveMode = SignatureResolveMode::BranchTarget, .branchWordIndex = 4,
        },
        { // Hair color
            .name = "FFLiGetSrgbFetchBeardColor", .pHookInfo = &replacement_FFLiGetHairColor,
            .words = {
                { 0x38000010, 0xFFFFFFFF }, // Same as mustache, but 10 instead of 0b
                { 0x919E0000, 0xFFFFFFFF }, { 0x7C832378, 0xFFFFFFFF },
                { 0x901E0004, 0xFFFFFFFF }, { 0x48000001, 0xFC000003 }
            },
            .wordCount = 5, .resolveMode = SignatureResolveMode::BranchTarget, .branchWordIndex = 4,
        },

    },
    .count = 7
};
