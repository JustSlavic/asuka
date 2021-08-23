@echo off

IF NOT EXIST build mkdir build
pushd build
cl /Zi /Feasuka /DUNITY_BUILD /DASUKA_DEBUG /DASUKA_OS_WINDOWS /I../common ../src/win32_main.cpp User32.lib Xinput.lib Gdi32.lib
popd
