@echo off

SET DLL_BUILD=/DASUKA_DLL_BUILD=1
SET INCLUDE_TEXTURES=/DIN_CODE_TEXTURES=0
SET UI_EDITOR=/DUI_EDITOR_ENABLED=1
SET MALLOCATOR=/DASUKA_MALLOCATOR=1

SET BUILD_SETTINGS=%DLL_BUILD% %INCLUDE_TEXTURES% %UI_EDITOR% %MALLOCATOR%

SET COMMON_CL_FLAGS=/std:c++17 /MTd /nologo /GR- /Oi /Zi /EHa- /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4702
SET COMMON_LINKER_FLAGS=/opt:ref /incremental:no
SET COMMON_MY_FLAGS=/DUNITY_BUILD=1 %BUILD_SETTINGS% /DASUKA_DEBUG=1 /DASUKA_OS_WINDOWS=1 /DLITTLE_ENDIAN=1 /I../common /I../src /D_CRT_SECURE_NO_WARNINGS
SET COMMON_LIBS=User32.lib Xinput.lib Gdi32.lib Winmm.lib

IF NOT EXIST build mkdir build
REM pushd build

REM 32-bit build
REM cl %COMMON_CL_FLAGS% %COMMIN_MY_FLAGS% ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% /subsystem:windows,5.1 %COMMON_LIBS%


REM ASUKA
DEL *.pdb 2>NUL

SET HOUR=%time:~0,2%
IF "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%
SET PDB_FILENAME=%date:~6,4%_%date:~3,2%_%date:~0,2%_%HOUR%_%time:~3,2%_%time:~6,2%_asuka.pdb

echo WAITING FOR PDB > lock.tmp
cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL=1 /Fmasuka.map /Feasuka ../src/asuka.cpp /LD /link /PDB:%PDB_FILENAME% %COMMON_LINKER_FLAGS%
del lock.tmp
cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /Femain /Fmwin32_main.map ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% %COMMON_LIBS%


REM XAudio2
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /I../common /Fexaudio2_demo ../xaudio2/xaudio2_main.cpp /link /PDB:xaudio2.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% Xaudio2.lib


REM OpenGL
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /I../common /Feopengl_demo ../opengl/opengl.cpp /link /PDB:opengl.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% opengl32.lib

rem REM D3D11
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /I../common /Fed3d11_demo ../d3d11/d3d11.cpp /link /PDB:d3d11.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% D3D11.lib D3DCompiler.lib

rem REM D3D12
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /DUNITY_BUILD=1 /DASUKA_OS_WINDOWS=1 /I../common /Fed3d12_demo ../d3d12/d3d12.cpp /link /PDB:d3d12.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% D3D12.lib DXGI.lib


REM NOCRT TEST BUILD
rem cl /Zi /nologo /Gm- /GR- /EHa- /O2 /GS- /Gs9999999 /I../common ../nocrt/win32_nocrt.cpp /Fenocrt /Fmnocrt.map /link /subsystem:windows /nodefaultlib kernel32.lib User32.lib


REM COMPILER THINGS
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /DUNITY_BUILD=1 /DASUKA_OS_WINDOWS=1 /DLITTLE_ENDIAN=1 /D_CRT_SECURE_NO_WARNINGS /I../common /Ferei ../rei/main.cpp          /link


REM NEURAL NETWORKS
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DEBUG=1 /I../common /Feneural ../neural/neural.cpp /link /PDB:neural.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS%


REM EDITOR
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /I../common /Feeditor ../editor/editor.cpp /link /PDB:d3d11.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS%


REM CLIENT-SERVER THINGS
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /Feclient  ../web/client.cpp          /link Ws2_32.lib
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /Feserver  ../web/server.cpp          /link Ws2_32.lib


REM DEBUGGER THINGS
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /Fedebugger ../debugger/debugger.cpp /link /PDB:debugger.pdb
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /O2 /Fedebuggee ../debugger/debuggee.cpp /link /PDB:debuggee.pdb


REM popd
