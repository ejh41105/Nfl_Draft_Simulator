cmake_minimum_required(VERSION 3.20)
project(DraftSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${CMAKE_SOURCE_DIR}/Back End/Include")

add_executable(DraftSim
    "Back End/src/main.cpp"
    "Back End/src/LoadInfo.cpp"
    "Back End/src/ScoringEngine.cpp"
    "Back End/src/SimDraft.cpp"
    "Back End/src/Team.cpp"
)