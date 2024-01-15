cmake_minimum_required(VERSION 3.24)
project(connection C)

set(CMAKE_C_STANDARD 17)

add_library(connection
        connection.h connection.c)

include_directories(
        "../logger")
target_link_libraries(
        logger)

# Setup sanitizers
add_sanitizers(connection)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(connection)