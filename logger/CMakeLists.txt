cmake_minimum_required(VERSION 3.24)
project(logger C)

set(CMAKE_C_STANDARD 11)

add_library(logger
        logger.h logger.c)

# Setup sanitizers
add_sanitizers(logger)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(logger)