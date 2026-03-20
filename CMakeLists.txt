cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)
set(CMAKE_CXX_STANDARD 20)

add_executable(MockDraftProject
        Draft_Sim/main.cpp
        Draft_Sim/Team.cpp
        Draft_Sim/ScoringEngine.cpp
        Draft_Sim/LoadInfo.cpp
        Draft_sim/SimDraft.cpp
)

target_include_directories(MockDraftProject PRIVATE Draft_Sim/)

file(COPY Draft_Sim/JSONS DESTINATION ${CMAKE_BINARY_DIR})