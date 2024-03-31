cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME linux-embedded-simple-utility)
project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_SYSROOT /usr/local/sysroot-glibc-linaro-2.25-2019.12-arm-linux-gnueabi)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_BUILD_TYPE Release)
endif ()

include_directories(include
        /usr/local/libX11target/include
        /usr/local/arm/tslib/include
        /usr/local/libX11target/include/freetype2
        )

file(GLOB_RECURSE ALL_SOURCES main.cpp "src/*.cpp" "src/*.c")
file(GLOB_RECURSE ALL_INCLUDES "include/*.hpp" "include/*.h")

set(ALL_SRCS ${ALL_SOURCES} ${ALL_INCLUDES})

add_executable(${TARGET_NAME} ${ALL_SRCS})

link_directories(/usr/local/libX11target/lib
        /usr/local/arm/tslib/lib
        /usr/local/sysroot-glibc-linaro-2.25-2019.12-arm-linux-gnueabi/lib)

find_library(FREETYPE_LIBRARY NAMES freetype freetype2 PATHS /usr/local/libX11target/lib)
find_library(TS_LIBRARY NAMES ts PATHS /usr/local/arm/tslib/lib)
find_library(PTHREAD_LIBRARY NAMES pthread)
find_library(JPEG_LIBRARY NAMES jpeg PATHS /usr/local/libX11target/lib)

target_link_libraries(${TARGET_NAME} ${FREETYPE_LIBRARY} ${TS_LIBRARY} ${PTHREAD_LIBRARY} ${JPEG_LIBRARY})
