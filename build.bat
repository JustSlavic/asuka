@echo off

SET COMMON_CL_FLAGS=/Fmwin32_main.map /MTd /nologo /GR- /Oi /Zi /EHa- /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505
SET COMMON_LINKER_FLAGS=/opt:ref
SET COMMON_MY_FLAGS=/DUNITY_BUILD=1 /DASUKA_DEBUG=1 /DASUKA_OS_WINDOWS=1 /DLITTLE_ENDIAN=1 /I../common /I../src /D_CRT_SECURE_NO_WARNINGS
SET COMMON_LIBS=User32.lib Xinput.lib Gdi32.lib Winmm.lib

IF NOT EXIST build mkdir build
REM pushd build

REM 32-bit build
REM cl %COMMON_CL_FLAGS% %COMMIN_MY_FLAGS% ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% /subsystem:windows,5.1 %COMMON_LIBS%

REM 64-bit build
REM noDLL build
REM cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /Femain ../src/win32_main.cpp /link %COMMON_LINKER_FLAGS% %COMMON_LIBS%

REM DLL build
DEL *.pdb 2>NUL

SET HOUR=%time:~0,2%
IF "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%
SET PDB_FILENAME=%date:~6,4%_%date:~3,2%_%date:~0,2%_%HOUR%_%time:~3,2%_%time:~6,2%_asuka.pdb

echo WAITING FOR PDB > lock.tmp
cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL_BUILD /Feasuka ../src/asuka.cpp      /LD  /link /PDB:%PDB_FILENAME% %COMMON_LINKER_FLAGS%
del lock.tmp
cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL_BUILD /Femain  ../src/win32_main.cpp      /link %COMMON_LINKER_FLAGS% %COMMON_LIBS%

cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL_BUILD /Feclient  ../web/client.cpp          /link Ws2_32.lib
cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL_BUILD /Feserver  ../web/server.cpp          /link Ws2_32.lib

REM popd
