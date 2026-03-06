cmake_minimum_required(VERSION 4.0)
project(MockDraftProject)

set(CMAKE_CXX_STANDARD 26)

add_executable(MockDraftProject main.cpp
        Player.h
        Team.h)
