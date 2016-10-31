cmake_minimum_required(VERSION 3.2)
project(CacheLab)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    cachelab.c
    cachelab.h
    csim.c
    test-trans.c
    tracegen.c
    trans.c)

add_executable(CacheLab ${SOURCE_FILES})