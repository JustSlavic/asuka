#!/bin/bash

[[ -d "build" ]] || mkdir build

# g++ src/linux_main.cpp -o build/main -g3 -DUNITY_BUILD -DASUKA_DEBUG -DASUKA_OS_LINUX=1 -DSOUND_ALSA=0 -Icommon -Isrc -lX11 -lasound
# g++ web/linux_server.cpp -o build/server -DUNITY_BUILD -DASUKA_OS_LINUX -Icommon -Isrc


gcc debugger/linux_debugger.cpp -o build/debugger -DUNITY_BUILD -DASUKA_OS_LINUX -Icommon -Isrc
gcc debugger/debuggee.cpp -o build/debuggee -ggdb3 -static
# gcc debugger/debuggee.s -o build/debuggee -ggdb3 -static -nostdlib

