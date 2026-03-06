cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)

set(CMAKE_CXX_STANDARD 26)

include_directories(Draft_Sim)

add_executable(MockDraftProject
        Draft_Sim/main.cpp
        Player.cpp

)