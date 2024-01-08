cmake_minimum_required(VERSION 3.24)
project(server_app C)

set(CMAKE_C_STANDARD 17)

add_executable(server_app
        main.c)

include_directories(
        "../logger")
target_link_libraries(server_app
        logger)

# Setup sanitizers
add_sanitizers(server_app)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(server_app)