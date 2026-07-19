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

#include <iostream>
#include "Cartridge.h"
#include "PPU.h"
#include "WRAM.h"
#include "IORegisters.h"
#include "VRAM.h"
#include "HRAM.h"
#include "IERegister.h"
#include "OAM.h"
#include "CPU.h"
#include "Bus.h"
#include "Bus.h"


bool check(const std::string& label, int actual, int expected) {
    if (actual == expected) {
        std::cout << "PASS: " << label << std::endl;
        return true;
    } else {
        std::cout << "FAIL: " << label << " (got 0x" << actual << ", expected 0x" << expected << ")." << std::endl;
        return false;
    }
}


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


// int main(int argc, char* argv[]) {
//     // argc = number of arguments (including the program name itself)
//     // argv[0] = program name, argv[1] = first real argument
//     if (argc < 2) {
//         std::cerr << "Usage: gbemu <rom-path>" << std::endl;
//         return 1;
//     }
//     std::string romPath = argv[1];
//     GameBoy gb;
//     if (!gb.loadROM(romPath)) {
//         std::cerr << "Failed to load ROM: " << romPath << std::endl;
//         return 1;
//     }
//     try {
//         std::cout << std::hex
//           << "0x0100: " << static_cast<int>(gb.read(0x0100)) << "\n"
//           << "0x0101: " << static_cast<int>(gb.read(0x0101)) << "\n"
//           << "0x0104: " << static_cast<int>(gb.read(0x0104)) << std::endl;
//         int cycles = gb.step();
//         std::cout << "stepped OK, cycles = " << std::dec << cycles << std::endl;
//     } catch (const std::exception& e) {
//         std::cout << "caught: " << e.what() << std::endl;
//     }
//     return 0;
// }


struct TestSystem {
    PPU _ppu;
    Cartridge _cartridge;
    WRAM _wram;
    HRAM _hram;
    IORegisters _ioRegisters;
    IERegister _ieRegister;
    VRAM _vram;
    OAM _oam;
    Bus _bus;
    CPU _cpu;

    TestSystem()          // ← inside the braces
        : _vram(&_ppu),
          _oam(&_ppu),
          _bus(&_cartridge, &_wram, &_ioRegisters, &_vram, &_hram, &_ieRegister, &_oam),
          _cpu(_bus)
    {}
};                        // ← struct ends after the constructor

void testRegisterLoads() {
    TestSystem sys;
    sys._cpu.setC(0x42);
    int cycles = sys._cpu.execute(0x41);
    check("LD B,C copies C into B", sys._cpu.getB(), 0x42);   // the actual effect
    check("LD B,C takes 4 cycles", cycles, 4);                 // the timing
    sys._cpu.reset();
    sys._cpu.setA(0x42);
    cycles = sys._cpu.execute(0x57);
    check("LD D,A copies A into D", sys._cpu.getD(), 0x42);
    check("LD D,A takes 4 cycles", cycles, 4);
    sys._cpu.reset();
    sys._cpu.setE(0x42);
    cycles = sys._cpu.execute(0x6B);
    check("LD L,E copies E into L", sys._cpu.getL(), 0x42);
    check("LD L,E takes 4 cycles", cycles, 4);
}

void test16BitLoads() {
    TestSystem sys;
    sys._bus.write16(0xC000, 0x1234);
    sys._cpu.setPC(0xC000);

    int cycles = sys._cpu.execute(0x01);
    check("16 bit loads work", sys._cpu.getBC(), 0x1234);
    check("16 bit loads takes 12 cycles", cycles, 12);
    check("PC advanced past operands", sys._cpu.getPC(), 0xC002);
    sys._cpu.reset();
    sys._bus.write16(0xC100, 0xBEEF);
    sys._cpu.setPC(0xC100);
    cycles = sys._cpu.execute(0x31);
    check("LD SP,d16 loads value", sys._cpu.getSP(), 0xBEEF);
    check("LD SP,d16 takes 12 cycles", cycles, 12);
    check("PC advanced by 2", sys._cpu.getPC(), 0xC102);
}

void testHLLoads() {
    TestSystem sys;
    sys._cpu.setHL(0xC000); // point HL at writable WRAM
    sys._cpu.setB(0x99); // value to store
    int cycles = sys._cpu.execute(0x70); // LD (HL), B — stores B to memory[0xC000]
    check("LD (HL) B took 8 cycles", cycles, 8);

    cycles = sys._cpu.execute(0x4E); // LD C, (HL) — reads memory[0xC000] into C
    check("LD C,(HL) round-trips value", sys._cpu.getC(), 0x99);
    check("LD C,(HL) takes 8 cycles", cycles, 8);
}

void testPushPop() {
    TestSystem sys;
    sys._cpu.setSP(0xC100);           //stack in writable WRAM
    sys._cpu.setBC(0xBEEF);
    sys._cpu.setAF(0x43FF);

    int cycles = sys._cpu.execute(0xC5);   //PUSH BC
    check("PUSH BC takes 16 cycles", cycles, 16);
    check("SP decremented by 2", sys._cpu.getSP(), 0xC0FE);   //0xC100 - 2

    cycles = sys._cpu.execute(0xD1);       //POP DE  (different pair)
    check("POP DE takes 12 cycles", cycles, 12);
    check("value round-trips BC->stack->DE", sys._cpu.getDE(), 0xBEEF);
    check("SP restored", sys._cpu.getSP(), 0xC100);

    cycles = sys._cpu.execute(0xF5); //PUSH AF
    sys._cpu.execute(0xF1);
    check("AF round-trips with masking", sys._cpu.getAF(), 0x43F0);
}

void testAdd() {
    TestSystem sys;
    // Case 1: 0xFF + 0x01 = 0x00, all flags set except N
    sys._cpu.setA(0xFF);
    sys._cpu.setB(0x01);
    int cycles = sys._cpu.execute(0x80); 
    check("c1 A + b correct", sys._cpu.getA(), 0x00);
    check("c1 flagZ correct", sys._cpu.getFlagZ(), true);
    check("c1 flagN correct", sys._cpu.getFlagN(), false);
    check("c1 flagH correct", sys._cpu.getFlagH(), true);
    check("c1 flagC correct", sys._cpu.getFlagC(), true);
    check("c1 A + b cycles", cycles, 4);
    // Case 2: 0x08 + 0x08 = 0x10, H set, C clear
    sys._cpu.reset();
    sys._cpu.setA(0x08);
    sys._cpu.setB(0x08);
    cycles = sys._cpu.execute(0x80); 
    check("c2 A + b correct", sys._cpu.getA(), 0x10);
    check("c2 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c2 flagN correct", sys._cpu.getFlagN(), false);
    check("c2 flagH correct", sys._cpu.getFlagH(), true);
    check("c2 flagC correct", sys._cpu.getFlagC(), false);
    // Case 3: 0x12 + 0x34 = 0x46, all clear
    sys._cpu.reset();
    sys._cpu.setA(0x12);
    sys._cpu.setB(0x34);
    cycles = sys._cpu.execute(0x80); 
    check("c3 A + b correct", sys._cpu.getA(), 0x46);
    check("c3 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c3 flagN correct", sys._cpu.getFlagN(), false);
    check("c3 flagH correct", sys._cpu.getFlagH(), false);
    check("c3 flagC correct", sys._cpu.getFlagC(), false);
}

void testSub() {
    TestSystem sys;
    // Case 1: 0x10 - 0x01 = 0x0F, flags: 0110
    sys._cpu.setA(0x10);
    sys._cpu.setB(0x01);
    int cycles = sys._cpu.execute(0x90); 
    check("c1 A - b correct", sys._cpu.getA(), 0x0F);
    check("c1 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c1 flagN correct", sys._cpu.getFlagN(), true);
    check("c1 flagH correct", sys._cpu.getFlagH(), true);
    check("c1 flagC correct", sys._cpu.getFlagC(), false);
    check("c1 A - b cycles", cycles, 4);
    // Case 2: 0x20 - 0x20 = 0x00, flags: 1100
    sys._cpu.reset();
    sys._cpu.setA(0x20);
    sys._cpu.setB(0x20);
    cycles = sys._cpu.execute(0x90); 
    check("c2 A - b correct", sys._cpu.getA(), 0x00);
    check("c2 flagZ correct", sys._cpu.getFlagZ(), true);
    check("c2 flagN correct", sys._cpu.getFlagN(), true);
    check("c2 flagH correct", sys._cpu.getFlagH(), false);
    check("c2 flagC correct", sys._cpu.getFlagC(), false);
    // Case 3: 0x10 + 0x20 = 0xF0, flags: 0111
    sys._cpu.reset();
    sys._cpu.setA(0x10);
    sys._cpu.setB(0x20);
    cycles = sys._cpu.execute(0x90); 
    check("c3 A - b correct", sys._cpu.getA(), 0xF0);
    check("c3 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c3 flagN correct", sys._cpu.getFlagN(), true);
    check("c3 flagH correct", sys._cpu.getFlagH(), false);
    check("c3 flagC correct", sys._cpu.getFlagC(), true);
}

void testCp() {
    TestSystem sys;
    sys._cpu.setA(0x10);
    sys._cpu.setB(0x20);
    int cycles = sys._cpu.execute(0xB8); 
    check("cp leaves a unchanged", sys._cpu.getA(), 0x10);
    check("flagZ correct", sys._cpu.getFlagZ(), false);
    check("flagN correct", sys._cpu.getFlagN(), true);
    check("flagH correct", sys._cpu.getFlagH(), false);
    check("flagC correct", sys._cpu.getFlagC(), true);
    check("correct amount of cycles", cycles, 4);
}

void testAnd() {
    TestSystem sys;
    sys._cpu.setA(0x0F);
    sys._cpu.setB(0xF0);
    int cycles = sys._cpu.execute(0xA0);
    check("and result is correct", sys._cpu.getA(), 0x00);
    check("flagZ correct", sys._cpu.getFlagZ(), true);
    check("flagN correct", sys._cpu.getFlagN(), false);
    check("flagH correct", sys._cpu.getFlagH(), true);
    check("flagC correct", sys._cpu.getFlagC(), false);
    check("correct amount of cycles", cycles, 4);
}

void testXor() {
    TestSystem sys;
    sys._cpu.setA(0xFF);
    sys._cpu.setB(0xFF);
    int cycles = sys._cpu.execute(0xA8);
    check("xor result is correct", sys._cpu.getA(), 0x00);
    check("flagZ correct", sys._cpu.getFlagZ(), true);
    check("flagN correct", sys._cpu.getFlagN(), false);
    check("flagH correct", sys._cpu.getFlagH(), false);
    check("flagC correct", sys._cpu.getFlagC(), false);
    check("correct amount of cycles", cycles, 4);
    check("A becomes 0", sys._cpu.getA(), 0);
}
void testOr() {
    TestSystem sys;
    sys._cpu.setA(0x0F);
    sys._cpu.setB(0xF0);
    int cycles = sys._cpu.execute(0xB0);
    check("or result is correct", sys._cpu.getA(), 0xFF);
    check("flagZ correct", sys._cpu.getFlagZ(), false);
    check("flagN correct", sys._cpu.getFlagN(), false);
    check("flagH correct", sys._cpu.getFlagH(), false);
    check("flagC correct", sys._cpu.getFlagC(), false);
    check("correct amount of cycles", cycles, 4);
}

void testIncDec() {
    TestSystem sys;
    // C stays set
    sys._cpu.setA(0x05);
    sys._cpu.setFlagC(true);       // C = 1 before
    sys._cpu.execute(0x3C);        // INC A
    check("INC leaves C set", sys._cpu.getFlagC(), true);   // still 1

    // C stays clear
    sys._cpu.reset();              // (reset clears flags)
    sys._cpu.setA(0x05);
    sys._cpu.setFlagC(false);      // C = 0 before
    sys._cpu.execute(0x3C);
    check("INC leaves C clear", sys._cpu.getFlagC(), false); // still 0
    sys._cpu.reset();
    sys._cpu.setA(0xFF);
    sys._cpu.execute(0x3C);
    check("c1 Wrap around case", sys._cpu.getA(), 0x00);
    check("c1 FlagZ correct", sys._cpu.getFlagZ(), true);
    check("c1 FlagN correct", sys._cpu.getFlagN(), false);
    check("c1 FlagH correct", sys._cpu.getFlagH(), true);
    sys._cpu.reset();
    sys._cpu.setA(0x00);
    sys._cpu.execute(0x3D);
    check("c2 Wrap around case", sys._cpu.getA(), 0xFF);
    check("c2 FlagZ correct", sys._cpu.getFlagZ(), false);
    check("c2 FlagN correct", sys._cpu.getFlagN(), true);
    check("c2 FlagH correct", sys._cpu.getFlagH(), true);

    sys._cpu.reset();
    sys._cpu.setA(0xFF);
    sys._cpu.setFlagC(false);      // C cleared
    sys._cpu.execute(0x3C);        // INC A → 0x00, overflow!
    check("INC 0xFF leaves C untouched (not set by overflow)", sys._cpu.getFlagC(), false);
}

void testAdc() {
    TestSystem sys;
    sys._cpu.setA(0x0F);
    sys._cpu.setB(0x00);
    sys._cpu.setFlagC(true);
    int cycles = sys._cpu.execute(0x88); 
    check("c1 A + b + carry correct", sys._cpu.getA(), 0x10);
    check("c1 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c1 flagN correct", sys._cpu.getFlagN(), false);
    check("c1 flagH correct", sys._cpu.getFlagH(), true);
    check("c1 flagC correct", sys._cpu.getFlagC(), false);
    check("c1 cycles", cycles, 4);
    
    sys._cpu.reset();
    sys._cpu.setA(0xFF);
    sys._cpu.setB(0x00);
    sys._cpu.setFlagC(true);
    cycles = sys._cpu.execute(0x88); 
    check("c2 A + b correct", sys._cpu.getA(), 0x00);
    check("c2 flagZ correct", sys._cpu.getFlagZ(), true);
    check("c2 flagN correct", sys._cpu.getFlagN(), false);
    check("c2 flagH correct", sys._cpu.getFlagH(), true);
    check("c2 flagC correct", sys._cpu.getFlagC(), true);
    
    sys._cpu.reset();
    cycles = sys._cpu.execute(0x8E); 
    check("HL cycles correct", cycles, 8);
}

void testSbc() {
    TestSystem sys;
    sys._cpu.setA(0x00);
    sys._cpu.setB(0x00);
    sys._cpu.setFlagC(true);
    int cycles = sys._cpu.execute(0x98); 
    check("c1 A + b + carry correct", sys._cpu.getA(), 0xFF);
    check("c1 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c1 flagN correct", sys._cpu.getFlagN(), true);
    check("c1 flagH correct", sys._cpu.getFlagH(), true);
    check("c1 flagC correct", sys._cpu.getFlagC(), true);
    check("c1 cycles", cycles, 4);
    
    sys._cpu.reset();
    sys._cpu.setA(0x10);
    sys._cpu.setB(0x00);
    sys._cpu.setFlagC(true);
    cycles = sys._cpu.execute(0x98); 
    check("c2 A + b correct", sys._cpu.getA(), 0x0F);
    check("c2 flagZ correct", sys._cpu.getFlagZ(), false);
    check("c2 flagN correct", sys._cpu.getFlagN(), true);
    check("c2 flagH correct", sys._cpu.getFlagH(), true);
    check("c2 flagC correct", sys._cpu.getFlagC(), false);

    sys._cpu.reset();
    cycles = sys._cpu.execute(0x9E); 
    check("HL cycles correct", cycles, 8);
}

int main() {
    // testRegisterLoads();
    // testHLLoads();
    // test16BitLoads();
    // testPushPop();
    // testAdd();
    // testSub();
    // testCp();
    // testAdd();
    // testOr();
    // testXor();
    // testIncDec();
    testAdc();
    testSbc();
    return 0;
}

