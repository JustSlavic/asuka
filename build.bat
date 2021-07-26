@echo off

mkdir build
pushd build
cl /Zi /Feasuka /DUNITY_BUILD /I../common ../src/win32_main.cpp User32.lib Xinput.lib Gdi32.lib
popd
