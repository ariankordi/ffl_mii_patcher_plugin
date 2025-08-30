#ifdef DEBUG
#include <stdint.h>
#include <whb/log_cafe.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>

BOOL moduleLogInit = false;
BOOL cafeLogInit   = false;
BOOL udpLogInit    = false;
#endif // DEBUG

void initLogging() {
#ifdef DEBUG
    if (!(moduleLogInit = WHBLogModuleInit())) {
        cafeLogInit = WHBLogCafeInit();
        udpLogInit  = WHBLogUdpInit();
    }
#endif // DEBUG
}

void deinitLogging() {
#ifdef DEBUG
    if (moduleLogInit) {
        WHBLogModuleDeinit();
        moduleLogInit = false;
    }
    if (cafeLogInit) {
        WHBLogCafeDeinit();
        cafeLogInit = false;
    }
    if (udpLogInit) {
        WHBLogUdpDeinit();
        udpLogInit = false;
    }
#endif // DEBUG
}