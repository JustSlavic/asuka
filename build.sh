#!/bin/bash

[[ -d "build" ]] || mkdir build

g++ src/linux_main.cpp -o build/main -g3 -DUNITY_BUILD -DASUKA_DEBUG -DASUKA_OS_LINUX=1 -Icommon -Isrc -lX11
