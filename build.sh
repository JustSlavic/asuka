#!/bin/bash

g++ src/linux_main.cpp -o main -g3 -DUNITY_BUILD -DASUKA_DEBUG -DASUKA_OS_WINDOWS -Icommon -lX11
