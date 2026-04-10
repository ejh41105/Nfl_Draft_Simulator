cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(MockDraftProject
        "Draft_Sim/Back End/src/main.cpp"
        "Draft_Sim/Back End/src/Team.cpp"
        "Draft_Sim/Back End/src/ScoringEngine.cpp"
        "Draft_Sim/Back End/src/LoadInfo.cpp"
        "Draft_Sim/Back End/src/SimDraft.cpp"
)

target_include_directories(MockDraftProject PRIVATE
        "Draft_Sim/Back End/Include"
)

configure_file("Draft_Sim/Back End/Data/JSONS/Players.json" JSONS/Players.json COPYONLY)
configure_file("Draft_Sim/Back End/Data/JSONS/TeamConfig.json" JSONS/TeamConfig.json COPYONLY)
configure_file("Draft_Sim/Back End/Data/JSONS/DraftOrder.json" JSONS/DraftOrder.json COPYONLY)