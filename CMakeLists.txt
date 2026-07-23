cmake_minimum_required(VERSION 3.20)
project(GameboyEmulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.9
)
FetchContent_MakeAvailable(SDL2)

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
    src/CPU.cpp
    src/Timer.cpp
    src/Joypad.cpp
)

target_include_directories(gbemu PRIVATE include/gb)

if (MSVC)
    target_compile_options(gbemu PRIVATE /W4 /WX)
endif()

target_link_libraries(gbemu PRIVATE SDL2::SDL2 SDL2::SDL2main)

# Copy SDL2.dll next to the exe so it runs without PATH fiddling (Windows)
add_custom_command(TARGET gbemu POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:gbemu>")