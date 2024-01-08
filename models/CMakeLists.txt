cmake_minimum_required(VERSION 3.24)
project(models C)

set(CMAKE_C_STANDARD 17)

add_library(models
        models.h models.c
        models_serialization.h models_serialization.c)

find_package(CJSON)

include_directories(
        "../logger" ${CJSON_INCLUDE_DIRS})
target_link_libraries(
        logger
        ${CJSON_LIBRARY})

# Setup sanitizers
add_sanitizers(server_app)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(models)