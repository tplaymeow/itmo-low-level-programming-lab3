cmake_minimum_required(VERSION 3.24)
project(utils C)

set(CMAKE_C_STANDARD 17)

add_library(utils
        math_utils.h math_utils.c)

# Setup sanitizers
add_sanitizers(utils)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(utils)