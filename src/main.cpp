#include <cstdio>
#include <cstring>
#include <wups.h>
#include <function_patcher/function_patching.h>
#include <function_patcher/fpatching_defines.h>
#include <notifications/notifications.h>
#include <coreinit/dynload.h> // OSDynLoad_GetNumberOfRPLs
#include <coreinit/title.h> // OSGetTitleID

// Example logger from: https://github.com/wiiu-env/WiiUPluginSystem/blob/3b1133c9c9626e0b9a30bf890c3e2f66a7bcad51/plugins/example_plugin/src/utils/logger.c#L12
#include "utils/logger.h"
#include "patches.h"
#include "editor_patches.h"

// // ---------------------------------------------------------------
// //  Plugin Metadata
// // ---------------------------------------------------------------
WUPS_PLUGIN_NAME("FFL Mii Patcher Plugin");
WUPS_PLUGIN_VERSION("0.0.1");
WUPS_PLUGIN_AUTHOR("ariankordi");
WUPS_PLUGIN_LICENSE("GPLv3");

/// Called after FunctionPatcher_InitLibrary(), when the app's modules are loaded.
void scanAllModulesAndPatchFFL() {
    // Note: I don't actually know if there's a max amount of modules.
    constexpr int MAXIMUM_MODULES = 90;

    // DEBUG_FUNCTION_LINE_VERBOSE("OSDynLoad_GetNumberOfRPLs()");
    int numberOfRPLs = OSDynLoad_GetNumberOfRPLs();
    if (numberOfRPLs <= 0) {
        DEBUG_FUNCTION_LINE_WARN("No modules to scan.");
        return;
    }
    else if (numberOfRPLs > MAXIMUM_MODULES) {
        DEBUG_FUNCTION_LINE_ERR("Too many modules are loaded! Max: %d, loaded: %d", MAXIMUM_MODULES, numberOfRPLs);
        assert(false); // Should never happen.
        return;
    }

    /// List of modules to be populated.
    OSDynLoad_NotifyData modules[MAXIMUM_MODULES];

    DEBUG_FUNCTION_LINE_VERBOSE("OSDynLoad_GetRPLInfo(0, %d, %p)", numberOfRPLs, &modules);
    if (!OSDynLoad_GetRPLInfo(0, static_cast<uint32_t>(numberOfRPLs), modules)) {
        DEBUG_FUNCTION_LINE_ERR("OSDynLoad_GetRPLInfo failed");
        return;
    }

    for (OSDynLoad_NotifyData& module : modules) {
        // Only scan RPX executables that plausibly embed FFL.
        // This would soon have to support scanning RPLs as well,
        // in which case it'd be a good idea to skip OS libs (coreinit.rpl, gx2.rpl)

        // DEBUG_FUNCTION_LINE_VERBOSE("rpx: %s", m.name);
        //std::string_view name(module.name);
        //if (!name.ends_with(".rpx")) {
        size_t nameLen = strlen(module.name);
        bool isRPX = (nameLen >= 4) &&
                     module.name[nameLen - 1] == 'x' &&
                     module.name[nameLen - 2] == 'p' &&
                     module.name[nameLen - 3] == 'r' &&
                     module.name[nameLen - 4] == '.';
        if (!isRPX) {
            continue;
        }

        // Break out of entire loop if a single module was patched.
        // In 99% of cases, there's only one binary that contains FFL.
        // Even if it's in an RPL (NWF private Mii extension, jmargaris Unity MiiPlugin.rpl)
        if (scanSingleModuleForPatchFFL(module)) {
            break;
        }

#if DEBUG
        // Could not find function in single module.
        NotificationModule_AddInfoNotification("Could not find FFL functions to patch.");
#endif

        break; // Break to only process one module.
    }
}

/**
 * @brief Whether or not the Wii U Menu was ever loaded.
 * @details It turns out the ON_APPLICATION_START callback will get called
 * after entering the environment (title ID = Health and Safety Information)
 * but before entering the Wii U Menu, and scanning for RPLs will crash
 * in the in-between state, right after printing the RPL name (or checking ends_with).
 * Therefore, we need to only run that functionality after the Wii U Menu has loaded.
 */
bool gHasLoadedWiiUMenu = false;

/// Is the provided title ID matching the Wii U Menu for any region?
bool isTitleIDWiiUMenu(uint64_t titleID = OSGetTitleID()) {
    DEBUG_FUNCTION_LINE_VERBOSE("Title ID: %016llX", titleID);
    return titleID == 0x0005001010040000ULL ||
        titleID == 0x0005001010040100ULL ||
        titleID == 0x0005001010040200ULL;
}

/// Check if it's safe to scan modules before scanning to patch FFL.
void checkAndScanModules() {
    // Check if the Wii U Menu was never loaded.
    if (!gHasLoadedWiiUMenu &&
        // And that the current title ID is not the Wii U Menu.
        !(gHasLoadedWiiUMenu = isTitleIDWiiUMenu())
    ) {
        // Do not run until that case is true.
        DEBUG_FUNCTION_LINE_INFO("Title ID (%016llX) is not Wii U Menu and it has not loaded before. Skipping plugin to avoid a crash.", OSGetTitleID());
        return;
    }

    return scanAllModulesAndPatchFFL(); // Proceed.
}

/// Guard against calling FunctionPatcher if this stays false.
/// Not sure if this is actually needed.
static bool gFunctionPatcherInitialized = false;

// // ---------------------------------------------------------------
// //  Plugin Lifecycle
// // ---------------------------------------------------------------
INITIALIZE_PLUGIN() {
    initLogging();
    initPatchHandles();

    if (auto st = NotificationModule_InitLibrary(); st != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Notifications init failed: %s", NotificationModule_GetStatusStr(st));
    }
    if (auto st = FunctionPatcher_InitLibrary(); st != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("FunctionPatcher init failed: %s", FunctionPatcher_GetStatusStr(st));
    } else {
        gFunctionPatcherInitialized = true;
    }
}

DEINITIALIZE_PLUGIN() {
    deinitPatchHandles();
    FunctionPatcher_DeInitLibrary();
    NotificationModule_DeInitLibrary();

    deinitLogging();
}

ON_APPLICATION_START() {
    initLogging();

    DEBUG_FUNCTION_LINE_VERBOSE("FFL plugin ON_APPLICATION_START");
    if (!gFunctionPatcherInitialized) {
        return;
    }

    checkAndScanModules(); // Calls scanAllModulesAndPatchFFL.
}

ON_APPLICATION_ENDS() {
    deinitLogging();
}
