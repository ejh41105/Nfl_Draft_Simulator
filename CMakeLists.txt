cmake_minimum_required(VERSION 3.20)
project(MockDraftProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Crow requires Threads
find_package(Threads REQUIRED)

add_executable(MockDraftProject
        "Draft_Sim/BackEnd/src/main.cpp"
        "Draft_Sim/BackEnd/src/DraftSession.cpp"
        "Draft_Sim/BackEnd/src/Team.cpp"
        "Draft_Sim/BackEnd/src/ScoringEngine.cpp"
        "Draft_Sim/BackEnd/src/LoadInfo.cpp"
        "Draft_Sim/BackEnd/src/SimDraft.cpp"
)

target_include_directories(MockDraftProject PRIVATE
        "Draft_Sim/BackEnd/Include"
        "/opt/homebrew/include"
)

target_link_libraries(MockDraftProject PRIVATE Threads::Threads)