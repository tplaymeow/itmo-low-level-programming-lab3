cmake_minimum_required(VERSION 3.24)
project(database C)

set(CMAKE_C_STANDARD 17)

include_directories(
        "../logger"
        "../paging")
target_link_libraries(
        logger
        paging)

add_library(database
        database_attribute.h
        database_create_table_request.h database_create_table_request.c
        database_attribute_type.h
        database_attributes.h database_attributes.c
        database.h database.c
        database_table.h database_table.c
        database_insert_row_request.h database_insert_row_request.c
        database_attribute_value.h
        database_row.h database_row.c
        database_where.h database_where.c
        database_attribute_values.h database_attribute_values.c
        database_join.h database_join.c)

# Setup sanitizers
add_sanitizers(database)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(database)