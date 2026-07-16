#include <iostream>
#include "GameBoy.h"

int main() {
    GameBoy gameboy; // Create an instance of the GameBoy class

    if (!gameboy.loadROM("C:/Users/padil/Documents/gameboy-emulator/test_roms/01-special.gb")) {
        std::cout << "ROM failed to load!\n";
        return 1;
    }

    std::cout << std::hex << static_cast<int>(gameboy.read(0x0147)) << std::endl; // print the cartridge type from the loaded ROM

    std::cout << std::hex << static_cast<int>(gameboy.read(0x0134)) << std::endl; // print the first byte of the title from the loaded ROM

    std::cout << std::hex << static_cast<int>(gameboy.read(0x0104)) << std::endl; // print the first byte of the Nintendo logo from the loaded ROM

    gameboy.write(0xC000, 0x12); // write a value to an IO register

    std::cout << std::hex << static_cast<int>(gameboy.read(0xC000)) << std::endl; // read the value back from the IO register and print it

}