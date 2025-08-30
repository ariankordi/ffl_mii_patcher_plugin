#include <function_patcher/function_patching.h>
#include <function_patcher/fpatching_defines.h>
#include <coreinit/dynload.h>
#include <notifications/notifications.h>
#include <cstring>

#if DEBUG
#include <chrono> // Benchmarking
#include "utils/logger.h"
#endif

#include "utils/SignatureScanner.h"
#include "patches.h"
#include "ffl_patches.h" // cSignaturesFFL

static constexpr int MAX_PATCHED_HANDLES = 15;
/// A map of every patched function handle added.
static PatchedFunctionHandle gHandles[MAX_PATCHED_HANDLES];
static int gHandleIndex; ///< Current index for gHandles array.

static void applyPatchFromMatch(const SignatureMatch& match) {
    assert(match.pDef != nullptr);
    const SignatureDefinition& def = *match.pDef;
    assert(def.pHookInfo != nullptr);

    // Interpret hookInfo as a pointer to function_replacement_data_t.
    function_replacement_data_t* tmpl =
        reinterpret_cast<function_replacement_data_t*>(def.pHookInfo);
    function_replacement_data_t repl = *tmpl;

    // Set pointers in either the original or copy.
    repl.virtualAddr = static_cast<uint32_t>(match.effectiveAddress);
    // There's a chance only one of these have to be set...
    repl.physicalAddr = static_cast<uint32_t>(match.physicalAddress);

    PatchedFunctionHandle handle;
    if (auto st = FunctionPatcher_AddFunctionPatch(&repl, &handle, nullptr);
             st == FUNCTION_PATCHER_RESULT_SUCCESS) {
        // Set the new handle in the global array.
        gHandles[gHandleIndex++] = handle;

#if DEBUG
        char log[128];
        // TODO: Potentially make a test to ensure these are never null.
        assert(repl.ReplaceInRPL.function_name != nullptr);
        assert(def.name != nullptr);
        snprintf(log, sizeof(log), "Patched %s with my_%s at %08X",
            def.name,
            repl.ReplaceInRPL.function_name,
            match.effectiveAddress);
        DEBUG_FUNCTION_LINE("%s", log);
        NotificationModule_AddInfoNotification(log);
    } else {
        DEBUG_FUNCTION_LINE("Failed patch at %08X: %s",
            match.effectiveAddress, FunctionPatcher_GetStatusStr(st));
#endif
    }

}

/// Global SignatureScanner instance set up with FFL signature set.
static const SignatureScanner gSignatureScanner(cSignaturesFFL.data(),
    cSignaturesFFL.size());

bool scanSingleModuleForPatchFFL(OSDynLoad_NotifyData& module) {
    uint32_t textAddr = module.textAddr;
    uint32_t textSize = module.textSize;
    if (!textAddr || !textSize) {
        return false;
    }

    SignatureMatch matches[SIGSCAN_MAX_MATCHES];

#if DEBUG
    auto t0 = std::chrono::high_resolution_clock::now();
#endif

    // TODO:
    // - Smash 4: ~1450 ms
    // - Wii U Menu: ~250 ms
    // - Mii Maker: 100 ms
    // scan results NEED TO BE CACHED
    uint32_t found = gSignatureScanner.scanModule(textAddr,
        textSize, matches, SIGSCAN_MAX_MATCHES);

#if DEBUG
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    DEBUG_FUNCTION_LINE("scanner.scanModule(): %llu ms", ms);
#endif

    for (uint32_t m = 0; m < found; ++m) {
#if DEBUG
        char log[256];
        snprintf(log, sizeof(log), "Decoded \"%s\" effective=%08X physical=%08X", matches[m].pDef->name, matches[m].effectiveAddress, matches[m].physicalAddress);
        DEBUG_FUNCTION_LINE("%s", log);
#endif

        // Patch each resolved function entry, once per module.
        applyPatchFromMatch(matches[m]);
    }

    return true; // Break out of the loop.
}

void initPatchHandles() {
    memset(&gHandles, 0, sizeof(gHandles)); // Clear handle array before use.
}

void deinitPatchHandles() {
    // Remove all function patcher handles.
    for (int i = 0; i < gHandleIndex; i++) {
        FunctionPatcher_RemoveFunctionPatch(gHandles[i]);
    }
    gHandleIndex = 0;
}
