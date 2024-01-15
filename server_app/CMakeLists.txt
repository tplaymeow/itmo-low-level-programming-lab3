cmake_minimum_required(VERSION 3.24)
project(server_app C)

set(CMAKE_C_STANDARD 17)

add_executable(server_app
        main.c
        handlers.h handlers.c)

include_directories(
        "../logger"
        "../connection"
        "../models"
        "../paging"
        "../database")
target_link_libraries(server_app
        logger
        connection
        models
        paging
        database)

# Setup sanitizers
add_sanitizers(server_app)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(server_app)