#include <iostream>
#include "GameBoy.h"

// int main() {
//     GameBoy gameboy; // Create an instance of the GameBoy class

//     if (!gameboy.loadROM("C:/Users/padil/Documents/gameboy-emulator/test_roms/01-special.gb")) {
//         std::cout << "ROM failed to load!\n";
//         return 1;
//     }

//     std::cout << std::hex << static_cast<int>(gameboy.read(0x0147)) << std::endl; // print the cartridge type from the loaded ROM

//     std::cout << std::hex << static_cast<int>(gameboy.read(0x0134)) << std::endl; // print the first byte of the title from the loaded ROM

//     std::cout << std::hex << static_cast<int>(gameboy.read(0x0104)) << std::endl; // print the first byte of the Nintendo logo from the loaded ROM

//     gameboy.write(0xC000, 0x12); // write a value to an IO register

//     std::cout << std::hex << static_cast<int>(gameboy.read(0xC000)) << std::endl; // read the value back from the IO register and print it

// }

// #include <iostream>
// #include "Cartridge.h"
// #include "PPU.h"
// #include "WRAM.h"
// #include "IORegisters.h"
// #include "VRAM.h"
// #include "HRAM.h"
// #include "IERegister.h"
// #include "OAM.h"
// #include "CPU.h"
// #include "Bus.h"


// bool check(const std::string& label, uint16_t actual, uint16_t expected) {
//     if (actual == expected) {
//         std::cout << "PASS: " << label << std::endl;
//         return true;
//     } else {
//         std::cout << "FAIL: " << label << " (got 0x" << actual << ", expected 0x" << expected << ")." << std::endl;
//         return false;
//     }
// }


// int main() {
//     PPU _ppu;
//     Cartridge cart;
//     WRAM wram;
//     IORegisters io;
//     VRAM vram(&_ppu);
//     HRAM hram;
//     IERegister ie;
//     OAM oam(&_ppu);
//     Bus bus(&cart, &wram, &io, &vram, &hram, &ie, &oam);
//     CPU cpu(bus);

//     check("AF default value", cpu.getAF(), 0x01B0);
//     check("BC default value", cpu.getBC(), 0x0013);
//     check("DE default value", cpu.getDE(), 0x00D8);
//     check("HL default value", cpu.getHL(), 0x014D);

//     cpu.setAF(0x12FF);
//     check("_f bits 0-3 are truly 0", cpu.getF(), 0xF0);
//     cpu.reset();


//     cpu.setF(0x00);
//     cpu.setFlagZ(true);
//     check("FlagZ is true", cpu.getFlagZ(), true);
//     check("FlagC is untouched", cpu.getFlagC(), false);
//     cpu.reset();

//     cpu.setBC(0xABCD);
//     check("B is expected value", cpu.getB(), 0xAB);
//     check("C is expected val", cpu.getC(), 0xCD);
// }


int main(int argc, char* argv[]) {
    // argc = number of arguments (including the program name itself)
    // argv[0] = program name, argv[1] = first real argument
    if (argc < 2) {
        std::cerr << "Usage: gbemu <rom-path>" << std::endl;
        return 1;
    }
    std::string romPath = argv[1];
    GameBoy gb;
    if (!gb.loadROM(romPath)) {
        std::cerr << "Failed to load ROM: " << romPath << std::endl;
        return 1;
    }
    try {
        std::cout << std::hex
          << "0x0100: " << static_cast<int>(gb.read(0x0100)) << "\n"
          << "0x0101: " << static_cast<int>(gb.read(0x0101)) << "\n"
          << "0x0104: " << static_cast<int>(gb.read(0x0104)) << std::endl;
        int cycles = gb.step();
        std::cout << "stepped OK, cycles = " << std::dec << cycles << std::endl;
    } catch (const std::exception& e) {
        std::cout << "caught: " << e.what() << std::endl;
    }
    return 0;
}
