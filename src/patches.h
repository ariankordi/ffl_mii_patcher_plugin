#include <coreinit/dynload.h>
#include <function_patcher/fpatching_defines.h>

static constexpr int MAX_PATCHED_HANDLES = 15;
/// A map of every patched function handle added.
extern PatchedFunctionHandle gHandles[MAX_PATCHED_HANDLES];
extern int gHandleIndex; ///< Current index for gHandles array.

/// Uses the SignatureScanner to scan a module and apply patches to FFL functions.
bool scanSingleModuleForPatchFFL(OSDynLoad_NotifyData& module);

void initPatchHandles();
void deinitPatchHandles();
