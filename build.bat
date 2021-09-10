@echo off

set COMMON_FLAGS=/Fmwin32_main.map /MT /nologo /GR- /Oi /Zi /EHa- /Feasuka /W4 /WX /wd4201 /wd4100 /wd4189 /D_CRT_SECURE_NO_WARNINGS
set COMMON_LINKER_FLAGS=/opt:ref
set COMMON_MY_FLAGS=/DUNITY_BUILD /DASUKA_DEBUG /DASUKA_OS_WINDOWS /I../common
set COMMON_LIBS=User32.lib Xinput.lib Gdi32.lib

IF NOT EXIST build mkdir build
pushd build

REM 32-bit build
REM cl %COMMON_FLAGS% %COMMIN_MY_FLAGS% ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% /subsystem:windows,5.1 %COMMON_LIBS%

REM 64-bit build
cl %COMMON_FLAGS% %COMMON_MY_FLAGS% ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% %COMMON_LIBS%

popd
