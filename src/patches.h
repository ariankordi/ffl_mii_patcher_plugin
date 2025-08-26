#include "coreinit/dynload.h"

/// Uses the SignatureScanner to scan a module and apply patches to FFL functions.
bool scanSingleModuleForPatchFFL(OSDynLoad_NotifyData& module);

void initPatchHandles();
void deinitPatchHandles();
