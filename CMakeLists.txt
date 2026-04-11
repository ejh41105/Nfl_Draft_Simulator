cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(MockDraftProject
        "Draft_Sim/BackEnd/src/main.cpp"
        "Draft_Sim/BackEnd/src/Team.cpp"
        "Draft_Sim/BackEnd/src/ScoringEngine.cpp"
        "Draft_Sim/BackEnd/src/LoadInfo.cpp"
        "Draft_Sim/BackEnd/src/SimDraft.cpp"
)

target_include_directories(MockDraftProject PRIVATE
        "Draft_Sim/BackEnd/Include"
)

configure_file("Draft_Sim/BackEnd/Data/JSONS/Players.json" JSONS/Players.json COPYONLY)
configure_file("Draft_Sim/BackEnd/Data/JSONS/TeamConfig.json" JSONS/TeamConfig.json COPYONLY)
configure_file("Draft_Sim/BackEnd/Data/JSONS/DraftOrder.json" JSONS/DraftOrder.json COPYONLY)