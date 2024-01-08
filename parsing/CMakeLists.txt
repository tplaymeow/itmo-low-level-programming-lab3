cmake_minimum_required(VERSION 3.24)
project(parsing C)

set(CMAKE_C_STANDARD 17)

# Setup Bison and Flex
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix bison
            RESULT_VARIABLE BREW_BISON
            OUTPUT_VARIABLE BREW_BISON_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
        message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
        set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
    endif ()

    execute_process(
            COMMAND brew --prefix flex
            RESULT_VARIABLE BREW_FLEX
            OUTPUT_VARIABLE BREW_FLEX_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
    endif ()
endif ()

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(PARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(LEXER_OUT "${PARSER_DIR}/sql_lexer.c")
set(PARSER_OUT "${PARSER_DIR}/sql_parser.c")

flex_target(LEXER "sql_lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/sql_lexer.h")
bison_target(PARSER "sql_parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/sql_parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_library(parsing
        "${PARSER_OUT}"
        "${LEXER_OUT}"
        parsing.h parsing.c)

include_directories(
        "../logger"
        "../models")
target_link_libraries(
        logger
        models)

# Setup sanitizers
add_sanitizers(parsing)

# Setup logging
include(../cmake-additions/filename.cmake)
define_file_info(parsing)