cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_LOADBENCH_SOURCES
        LoadBench.cpp)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_executable(LoadBench ${BIN_LOADBENCH_SOURCES})
    target_link_libraries(LoadBench PUBLIC
            engine_serialization
            library_lod
            library_lod_formats)
    target_check_style(LoadBench)
endif()
