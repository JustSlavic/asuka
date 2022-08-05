@echo off

SET DLL_BUILD=/DASUKA_DLL_BUILD=1
SET INCLUDE_TEXTURES=/DIN_CODE_TEXTURES=0
SET UI_EDITOR=/DUI_EDITOR_ENABLED=1

SET BUILD_SETTINGS=%DLL_BUILD% %INCLUDE_TEXTURES% %UI_EDITOR%

SET WARNINGS=/W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4702
SET COMMON_CL_FLAGS=/std:c++17 /MTd /nologo /GR- /Oi /Zi /EHa-
SET COMMON_LINKER_FLAGS=/opt:ref /incremental:no
SET COMMON_MY_FLAGS=/DUNITY_BUILD=1 %BUILD_SETTINGS% /DASUKA_DEBUG=1 /DASUKA_OS_WINDOWS=1 /DBYTE_ORDER=1234 /I../common /I../src /D_CRT_SECURE_NO_WARNINGS
SET COMMON_LIBS=User32.lib Xinput.lib Gdi32.lib Winmm.lib

IF NOT EXIST build mkdir build
REM pushd build

REM ASUKA
DEL *.pdb 2>NUL

SET HOUR=%time:~0,2%
IF "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%

REM ASUKA
rem SET PDB_FILENAME=%date:~6,4%_%date:~3,2%_%date:~0,2%_%HOUR%_%time:~3,2%_%time:~6,2%_asuka.pdb
rem echo WAITING FOR PDB > lock.tmp
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL=1 /Fmasuka.map /Feasuka ../src/asuka.cpp /LD /link /PDB:%PDB_FILENAME% %COMMON_LINKER_FLAGS%
rem del lock.tmp

REM Wenjie
rem SET PDB_FILENAME=%date:~6,4%_%date:~3,2%_%date:~0,2%_%HOUR%_%time:~3,2%_%time:~6,2%_wenjie.pdb
rem echo WAITING FOR PDB > lock.tmp
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL=1 /Fmwenjie.map /Fewenjie ../wenjie/code/wenjie.cpp /LD /link /PDB:%PDB_FILENAME% %COMMON_LINKER_FLAGS%
rem del lock.tmp

REM Platform code
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /Femain /Fmwin32_main.map ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% %COMMON_LIBS%

REM Tests
cl /nologo /std:c++17 /EHa- %WARNINGS% /fsanitize=address /Zi /EHsc /Fetests ../tests/main.cpp /I../common /DASUKA_OS_WINDOWS=1 /D_CRT_SECURE_NO_WARNINGS /DUNITY_BUILD=1


REM XAudio2
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /I../common /Fexaudio2_demo ../xaudio2/xaudio2_main.cpp /link /PDB:xaudio2.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% Xaudio2.lib


REM OpenGL
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /I../common /Feopengl_demo ../opengl/opengl.cpp /link /PDB:opengl.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% opengl32.lib

REM D3D11
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /I../common /Fed3d11_demo ../d3d11/d3d11.cpp /link /PDB:d3d11.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% D3D11.lib D3DCompiler.lib

REM D3D12
rem cl %COMMON_CL_FLAGS% /DASUKA_DEBUG=1 /DUNITY_BUILD=1 /DASUKA_OS_WINDOWS=1 /I../common /Fed3d12_demo ../d3d12/d3d12.cpp /link /PDB:d3d12.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS% D3D12.lib DXGI.lib


REM Malloc Visualizer
rem cl /nologo /I../common /Fevisualizer ../malloc_proxy/visualizer.cpp /link /PDB:visualizer.pdb %COMMON_LINKER_FLAGS% User32.lib
rem cl /nologo /I../common /Feproxy ../malloc_proxy/proxy.cpp /LD /link /PDB:proxy.pdb %COMMON_LINKER_FLAGS% User32.lib
rem cl /nologo /MDd /DASUKA_DEBUG=1 /DUNITY_BUILD=1 /DASUKA_OS_WINDOWS=1 /I../common /Feclient_program ../malloc_proxy/client_program.cpp /link /PDB:client_program.pdb %COMMON_LINKER_FLAGS%


REM NOCRT TEST BUILD
rem cl /Zi /nologo /Gm- /GR- /EHa- /O2 /GS- /Gs9999999 /I../common ../nocrt/win32_nocrt.cpp /Fenocrt /Fmnocrt.map /link /subsystem:windows /nodefaultlib kernel32.lib User32.lib


REM LUA INTEGRATION
rem Actually have to build Lua for Windows (kinda sucks because it requires Make, and it's pain to intall Make on Windows)
rem cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DEBUG=1 /I../../lua-5.4.4/src /Felua_test ../lua/main.cpp /link /PDB:lua.pdb %COMMON_LINKER_FLAGS% %COMMON_LIBS%


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
