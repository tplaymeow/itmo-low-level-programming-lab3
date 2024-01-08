file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.c *.h)
add_custom_target(
        format
        COMMAND clang-format
        -style=LLVM
        -i
        ${ALL_SOURCE_FILES})