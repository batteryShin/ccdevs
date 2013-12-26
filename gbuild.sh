#!/bin/bash
ndk-build clean
ndk-build NDK_DEBUG=1 && ant debug && adb wait-for-devices && adb install -r bin/MainActivity-debug.apk

