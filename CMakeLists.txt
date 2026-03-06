cmake_minimum_required(VERSION 3.10)
project(MockDraftProject)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake where headers are
include_directories(include)

# Tell CMake where source files are
add_executable(MockDraftProject
        src/main.cpp
)