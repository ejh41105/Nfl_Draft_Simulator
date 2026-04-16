cmake_minimum_required(VERSION 3.20)
project(MockDraftProject)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Crow requires Threads
find_package(Threads REQUIRED)

# Fetch Crow at configure time
include(FetchContent)
FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.2.1.2
)
FetchContent_MakeAvailable(Crow)

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
)

target_link_libraries(MockDraftProject PRIVATE
        Threads::Threads
        Crow::Crow
)