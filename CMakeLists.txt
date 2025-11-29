cmake_minimum_required(VERSION 4.0)
project(bcg729)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(bcg729
        bcg729.c
        config.m4
        LICENSE
        php_bcg729.h
        README.md
        SECURITY.md)
