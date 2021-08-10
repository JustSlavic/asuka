#!/bin/bash

g++ src/win32_main.cpp -o main -DUNITY_BUILD -DASUKA_DEBUG -DASUKA_OS_WINDOWS -Icommon -lUser32 -lXInput -lGdi32
