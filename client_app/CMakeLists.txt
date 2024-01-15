cmake_minimum_required(VERSION 3.24)
project(client_app C)

set(CMAKE_C_STANDARD 17)

add_executable(client_app
        main.c)

include_directories(
        "../logger"
        "../models"
        "../parsing"
        "../connection")
target_link_libraries(client_app
        logger
        models
        parsing
        connection)

# Setup sanitizers
add_sanitizers(client_app)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(client_app)