cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(CORE_RANDOM_SOURCES
        Random.cpp
        RandomEnums.cpp)

set(CORE_RANDOM_HEADERS
        Random.h
        RandomEnums.h)

add_library(core_random STATIC ${CORE_RANDOM_SOURCES} ${CORE_RANDOM_HEADERS})
target_link_libraries(core_random PUBLIC library_random library_serialization)
target_check_style(core_random)
