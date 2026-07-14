cmake_minimum_required(VERSION 3.20)
project(GameboyEmulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gbemu src/main.cpp src/Bus.cpp)
target_include_directories(gbemu PRIVATE include)