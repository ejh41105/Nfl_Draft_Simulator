cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)

set(CMAKE_CXX_STANDARD 26)

include_directories(Draft_Sim)

add_executable(MockDraftProject
        Draft_Sim/main.cpp
        Draft_Sim/Team.cpp
        Draft_Sim/ScoringEngine.h
        Draft_Sim/ScoringEngine.cpp
        Draft_Sim/RNG.h

)