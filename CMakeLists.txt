cmake_minimum_required(VERSION 3.20)
project(MockDraftProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Crow dependencies
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

if(APPLE)
    list(APPEND EXTRA_INCLUDE_DIRS "/opt/homebrew/include")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Boost REQUIRED COMPONENTS system)
endif()

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
        ${EXTRA_INCLUDE_DIRS}
)

target_link_libraries(MockDraftProject PRIVATE Threads::Threads ZLIB::ZLIB)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(MockDraftProject PRIVATE CROW_USE_BOOST)
    target_link_libraries(MockDraftProject PRIVATE Boost::system)
endif()

message(STATUS "Configuring ${PROJECT_NAME} for ${CMAKE_SYSTEM_NAME}")
