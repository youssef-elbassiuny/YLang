cmake_minimum_required(VERSION 3.20)

project(compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

add_executable(ylang
        scr/ylang.cpp
)

target_include_directories(ylang PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(ylang PRIVATE
        ${LLVM_DEFINITIONS}
)

target_link_libraries(ylang PRIVATE
        LLVM
)