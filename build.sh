#!/bin/bash

CXX_STANDARD=17

DEBUG_BUILD=1
UNITY_BUILD=1
DLL_BUILD=0
INCLUDE_TEXTURES=0
UI_EDITOR=1
WARNINGS="-Wall -Werror -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function"

BUILD_SETTING="-DASUKA_DEBUG=$DEBUG_BUILD -DUNITY_BUILD=$UNITY_BUILD -DASUKA_DLL_BUILD=$DLL_BUILD -DIN_CODE_TEXTURES=$INCLUDE_TEXTURES -DUI_EDITOR_ENABLED=$UI_EDITOR -std=c++$CXX_STANDARD"

mkdir -p build

g++ src/linux_main.cpp -o build/main $WARNINGS -g3 $BUILD_SETTING -DASUKA_OS_LINUX=1 -Icommon -Isrc -lX11 -lasound -ldl
echo "build/main"

if [[ "$DLL_BUILD" == 1 ]]; then
    g++ -shared -o build/asuka.so -fPIC src/asuka.cpp $WARNINGS -g3 $BUILD_SETTING -DASUKA_DLL=1 -DASUKA_OS_LINUX=1 -Icommon -Isrc -lX11 -lasound
    echo "build/asuka.so"
fi
