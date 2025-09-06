# FFL Mii Patcher plugin for Aroma

This is a plugin I'm making to play with patching FFL (Cafe Face Library), a library statically linked into any title that uses Miis at all. Goal is unclear as of now.

(Most of the README, Makefile, main.cpp etc. are from [example_plugin_cpp](https://github.com/wiiu-env/WiiUPluginSystem/tree/3b1133c9c9626e0b9a30bf890c3e2f66a7bcad51/plugins/example_plugin_cpp))

## Installation

(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

## Usage

This plugin currently has no options, and is enabled for all titles. So it's possible that this plugin will cause a certain game to crash if it can't patch it properly. Use with caution.

As of now, the only obvious thing this will do is change all hair/beard/mustache/eyebrow colors to always be red in every game that uses FFL.

## Building

For building you need:

- [wut](https://github.com/devkitpro/wut)
- [WiiUPluginSystem](https://github.com/wiiu-env/WiiUPluginSystem)
- [WiiUModuleSystem](https://github.com/wiiu-env/WiiUModuleSystem)
- [libfunctionpatcher](https://github.com/wiiu-env/libfunctionpatcher)
- [libnotifications](https://github.com/wiiu-env/libnotifications/)

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

Then you should be able to compile via `make` (with no logging) or `make DEBUG=1` (with logging).

## Buildflags

### Logging

Building via `make` only logs errors (via OSReport). To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).
`make DEBUG=VERBOSE` Enables verbose information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) isn't present, it will fall back to UDP (port 4405) and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.
