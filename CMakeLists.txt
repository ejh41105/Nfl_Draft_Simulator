cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)
set(CMAKE_CXX_STANDARD 20)

add_executable(MockDraftProject
        Draft_Sim/main.cpp
        Draft_Sim/Team.cpp
        Draft_Sim/ScoringEngine.cpp
        Draft_Sim/LoadInfo.cpp
        Draft_Sim/SimDraft.cpp
)

target_include_directories(MockDraftProject PRIVATE Draft_Sim/)

configure_file(Draft_Sim/JSONS/Players.json JSONS/Players.json COPYONLY)
configure_file(Draft_Sim/JSONS/TeamConfig.json JSONS/TeamConfig.json COPYONLY)
configure_file(Draft_Sim/JSONS/DraftOrder.json JSONS/DraftOrder.json COPYONLY)