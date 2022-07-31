cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

project(Asuka VERSION 0.1.0 LANGUAGES CXX)

option(ASUKA_UNITY_BUILD OFF)
if (ASUKA_UNITY_BUILD)
    set(ASUKA_UNITY_BUILD_VALUE 1)
else()
    set(ASUKA_UNITY_BUILD_VALUE 0)
endif()

option(ASUKA_DLL_BUILD "Build with engine as shared lib" OFF)
if (ASUKA_DLL_BUILD)
    set(ASUKA_DLL_BUILD_VALUE 1)
    set(ASUKA_LIB_TYPE SHARED)
else()
    set(ASUKA_DLL_BUILD_VALUE 0)
    set(ASUKA_LIB_TYPE STATIC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if (NOT ASUKA_UNITY_BUILD)
    add_library(AsukaLib ${ASUKA_LIB_TYPE}
        src/asuka.cpp
        src/sim_region.cpp
        src/world.cpp
        src/ui/ui.cpp
        src/ui/ui_editor.cpp
        src/ui/ui_element.cpp
        common/bmp.cpp
        common/crc.cpp
        common/png.cpp
        common/wav.cpp
        common/array.cpp
        common/code_location.cpp
        common/math/float.cpp
        common/math/vector2.cpp
        common/math/vector3.cpp
        common/math/vector4.cpp
        common/math/color.cpp
        common/math/matrix4.cpp
        common/os/file.cpp
        common/os/time.cpp
        common/os/memory.cpp
        common/os/linux/file.cpp
        common/os/linux/time.cpp
        common/os/linux/memory.cpp
        )

    target_include_directories(AsukaLib PUBLIC common src)
    target_compile_definitions(AsukaLib PUBLIC
        ASUKA_DEBUG=1
        UNITY_BUILD=${ASUKA_UNITY_BUILD_VALUE}
        ASUKA_DLL_BUILD=${ASUKA_DLL_BUILD_VALUE}
        IN_CODE_TEXTURES=0
        UI_EDITOR_ENABLED=1
        ASUKA_OS_LINUX=1
    )
    set_target_properties(AsukaLib PROPERTIES PREFIX "" OUTPUT_NAME "asuka")

    set(ASUKA_LIBRARIES AsukaLib)
endif(NOT ASUKA_UNITY_BUILD)

add_executable(${PROJECT_NAME} src/linux_main.cpp)

# find_package(${PROJECT_NAME} PATHS /usr/lib ${HOME}/libs) # Search of libraries

target_include_directories(${PROJECT_NAME} PUBLIC common src)
target_link_libraries(${PROJECT_NAME} PUBLIC ${ASUKA_LIBRARIES} X11 asound dl)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    ASUKA_DEBUG=1
    UNITY_BUILD=${ASUKA_UNITY_BUILD_VALUE}
    ASUKA_DLL_BUILD=${ASUKA_DLL_BUILD_VALUE}
    IN_CODE_TEXTURES=0
    UI_EDITOR_ENABLED=1
    ASUKA_OS_LINUX=1
)
