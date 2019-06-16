#!/usr/bin/env bash
# -------------------------------------------------------------------------------
# This script builds  set environment variables
# -------------------------------------------------------------------------------
set -e # exit on error

# select if you want to build 32-bit (i686), 64-bit (x86_64), or both
case "$MSYSTEM" in
  MINGW32)
    ARCH=mingw-w64-i686
    PREFIX=/mingw32
    MSYS2_ARCH=i686
    ;;
  MINGW64)
    ARCH=mingw-w64-x86_64
    PREFIX=/mingw64
    MSYS2_ARCH=x86_64
    ;;
  *)
    printf "Unsupported mode! Choose MinGW64 (32 or 64 bit)"
    exit 1
    #ARCH={mingw-w64-i686,mingw-w64-x86_64}
	#ARCH=mingw-w64-x86_64
    # TODO: need to check compilation for both architectures
    ;;
esac
