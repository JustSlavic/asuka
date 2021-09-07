@echo off

IF NOT EXIST build mkdir build
pushd build
cl /Fmwin32_main.map /MT /nologo /GR- /Oi /Zi /EHa- /Feasuka /W4 /WX /wd4201 /wd4100 /wd4189 /D_CRT_SECURE_NO_WARNINGS /DUNITY_BUILD /DASUKA_DEBUG /DASUKA_OS_WINDOWS /I../common ../src/win32_main.cpp /link /opt:ref /subsystem:windows,5.1 User32.lib Xinput.lib Gdi32.lib
popd
