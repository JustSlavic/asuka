@echo off

SET COMMON_CL_FLAGS=/std:c++17 /MTd /nologo /GR- /O2 /Zi /EHa- /W4 /WX /wd4201 /wd4100 /wd4189 /wd4505 /wd4702
SET COMMON_LINKER_FLAGS=/opt:ref /incremental:no
SET COMMON_MY_FLAGS=/DUNITY_BUILD=1 /DASUKA_DEBUG=1 /DASUKA_OS_WINDOWS=1 /I../common /I../src /D_CRT_SECURE_NO_WARNINGS
SET COMMON_LIBS=User32.lib

cl %COMMON_CL_FLAGS% %COMMON_MY_FLAGS% /DASUKA_DLL_BUILD /Fepng_to_c png_to_c.cpp /link %COMMON_LINKER_FLAGS% %COMMON_LIBS%
