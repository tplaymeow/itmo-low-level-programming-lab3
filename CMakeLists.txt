cmake_minimum_required(VERSION 3.24)
project(low-level-programming-lab3 C)

set(CMAKE_C_STANDARD 17)

# Setup sanitizers
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers REQUIRED)

add_subdirectory(logger)
add_subdirectory(utils)
add_subdirectory(paging)
add_subdirectory(database)
add_subdirectory(models)
add_subdirectory(parsing)
add_subdirectory(connection)
add_subdirectory(server_app)
add_subdirectory(client_app)

include(cmake-additions/format.cmake)
