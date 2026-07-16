cmake_minimum_required(VERSION 3.20)
project(GameboyEmulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gbemu
    src/main.cpp
    src/Bus.cpp
    src/WRAM.cpp
    src/HRAM.cpp
    src/VRAM.cpp
    src/OAM.cpp
    src/IORegisters.cpp
    src/IERegister.cpp
    src/Cartridge.cpp
    src/PPU.cpp
    src/GameBoy.cpp
)

target_include_directories(gbemu PRIVATE include/gb)