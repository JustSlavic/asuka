@echo off

mkdir build
pushd build
cl /Zi /Feasuka ../src/main.cpp User32.lib Gdi32.lib
popd
