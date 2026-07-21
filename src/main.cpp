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

void test16BitIncDec() {
    TestSystem sys;
    sys._cpu.setFlagZ(true);
    sys._cpu.setFlagN(true);
    sys._cpu.setFlagH(true);
    sys._cpu.setFlagC(true);

    sys._cpu.setHL(0xFFFF);
    int cycles = sys._cpu.execute(0x23);
    check("INC wraparound correctly", sys._cpu.getHL(), 0x0000);
    check("INC doesn't touch flags", sys._cpu.getFlagZ(), true);
    check("INC doesn't touch flags", sys._cpu.getFlagN(), true);
    check("INC doesn't touch flags", sys._cpu.getFlagH(), true);
    check("INC doesn't touch flags", sys._cpu.getFlagC(), true);
    check("Cycles is correct", cycles, 8);

    sys._cpu.reset();
    sys._cpu.setFlagZ(true);
    sys._cpu.setFlagN(true);
    sys._cpu.setFlagH(true);
    sys._cpu.setFlagC(true);
    sys._cpu.setBC(0x0001);
    cycles = sys._cpu.execute(0x0B);
    check("DEC decrements correctly", sys._cpu.getBC(), 0x0000);
    check("INC doesn't touch flags", sys._cpu.getFlagZ(), true);
    check("INC doesn't touch flags", sys._cpu.getFlagN(), true);
    check("INC doesn't touch flags", sys._cpu.getFlagH(), true);
    check("INC doesn't touch flags", sys._cpu.getFlagC(), true);
    check("Cycles is correct", cycles, 8);
}


void testAdd16() {
    TestSystem sys;
    sys._cpu.setFlagZ(true);
    sys._cpu.setHL(0x0FFF);
    sys._cpu.setBC(0x0001);
    int cycles = sys._cpu.execute(0x09);
    check("C1: Bit 11 half carry works", sys._cpu.getHL(), 0x1000);
    check("C1 FlagH", sys._cpu.getFlagH(), true);
    check("C1 FlagC", sys._cpu.getFlagC(), false);
    check("add16 doesn't touch Z", sys._cpu.getFlagZ(), true);

    sys._cpu.reset();
    sys._cpu.setFlagZ(false);
    sys._cpu.setHL(0xFFFF);
    sys._cpu.setBC(0x0001);
    cycles = sys._cpu.execute(0x09);
    check("C2: bit 15 carry", sys._cpu.getHL(), 0x0000);
    check("C2: flagC", sys._cpu.getFlagC(), true);
    check("C2: flagH", sys._cpu.getFlagH(), true);
    check("flagZ untouched", sys._cpu.getFlagZ(), false);
}

void testAddSPr8() {
    TestSystem sys;
    // Case 1: positive offset, no carries
    sys._bus.write(0xC500, 0x01);        // offset = +1
    sys._cpu.setSP(0xC000);
    sys._cpu.setPC(0xC500);
    int cycles = sys._cpu.execute(0xE8);
    check("c1 SP+1 result", sys._cpu.getSP(), 0xC001);
    check("c1 Z", sys._cpu.getFlagZ(), false);
    check("c1 N", sys._cpu.getFlagN(), false);
    check("c1 H", sys._cpu.getFlagH(), false);
    check("c1 C", sys._cpu.getFlagC(), false);
    check("c1 cycles", cycles, 16);

    // Case 2: NEGATIVE offset — proves signed interpretation
    sys._cpu.reset();
    sys._bus.write(0xC500, 0xFF);        // 0xFF as int8 = -1
    sys._cpu.setSP(0xC000);
    sys._cpu.setPC(0xC500);
    cycles = sys._cpu.execute(0xE8);
    check("c2 signed -1 gives 0xBFFF", sys._cpu.getSP(), 0xBFFF);  // NOT 0xC0FF
    check("c2 Z", sys._cpu.getFlagZ(), false);
    check("c2 N", sys._cpu.getFlagN(), false);
    check("c2 H", sys._cpu.getFlagH(), false);   // 0x0 + 0xF = 0xF, no carry
    check("c2 C", sys._cpu.getFlagC(), false);   // 0x00 + 0xFF = 0xFF, no carry

    // Case 3: sets H (bit-3 carry on low byte)
    sys._cpu.reset();
    sys._bus.write(0xC500, 0x01);
    sys._cpu.setSP(0xC00F);
    sys._cpu.setPC(0xC500);
    cycles = sys._cpu.execute(0xE8);
    check("c3 result 0xC010", sys._cpu.getSP(), 0xC010);
    check("c3 H set", sys._cpu.getFlagH(), true);   // 0xF + 0x1 = 0x10
    check("c3 C clear", sys._cpu.getFlagC(), false);

    // Case 4: sets H and C (bit-7 carry on low byte)
    sys._cpu.reset();
    sys._bus.write(0xC500, 0x01);
    sys._cpu.setSP(0xC0FF);
    sys._cpu.setPC(0xC500);
    cycles = sys._cpu.execute(0xE8);
    check("c4 result 0xC100", sys._cpu.getSP(), 0xC100);
    check("c4 H set", sys._cpu.getFlagH(), true);
    check("c4 C set", sys._cpu.getFlagC(), true);

    // Case 5: LD HL, SP+r8 (0xF8) — stores to HL, leaves SP alone, 12 cycles
    sys._cpu.reset();
    sys._bus.write(0xC500, 0x01);
    sys._cpu.setSP(0xC000);
    sys._cpu.setPC(0xC500);
    cycles = sys._cpu.execute(0xF8);
    check("c5 HL = SP+1", sys._cpu.getHL(), 0xC001);
    check("c5 SP unchanged", sys._cpu.getSP(), 0xC000);   // the distinguishing check
    check("c5 cycles", cycles, 12);
}

void testRotates() {
    TestSystem sys;

    // --- RLCA (0x07): rotate left circular ---
    sys._cpu.setA(0x80);              // 1000 0000
    sys._cpu.execute(0x07);
    check("RLCA 0x80 -> 0x01", sys._cpu.getA(), 0x01);   // bit7 wraps to bit0
    check("RLCA C = old bit7", sys._cpu.getFlagC(), true);
    check("RLCA Z forced 0", sys._cpu.getFlagZ(), false);
    check("RLCA N", sys._cpu.getFlagN(), false);
    check("RLCA H", sys._cpu.getFlagH(), false);

    // --- RRCA (0x0F): rotate right circular ---
    sys._cpu.reset();
    sys._cpu.setA(0x01);             // 0000 0001
    sys._cpu.execute(0x0F);
    check("RRCA 0x01 -> 0x80", sys._cpu.getA(), 0x80);   // bit0 wraps to bit7
    check("RRCA C = old bit0", sys._cpu.getFlagC(), true);
    check("RRCA Z forced 0", sys._cpu.getFlagZ(), false);

    // --- RLA (0x17): rotate left THROUGH carry ---
    // Key case: result is 0x00 but Z must STILL be 0, and proves old-carry fills bit0
    sys._cpu.reset();
    sys._cpu.setA(0x80);
    sys._cpu.setFlagC(false);        // old carry = 0 fills bit 0
    sys._cpu.execute(0x17);
    check("RLA 0x80,C=0 -> 0x00", sys._cpu.getA(), 0x00);
    check("RLA C = old bit7", sys._cpu.getFlagC(), true);
    check("RLA Z forced 0 even on zero result", sys._cpu.getFlagZ(), false);

    // Discriminator: same A, carry SET -> old carry (1) fills bit 0 -> different result
    sys._cpu.reset();
    sys._cpu.setA(0x80);
    sys._cpu.setFlagC(true);         // old carry = 1 fills bit 0
    sys._cpu.execute(0x17);
    check("RLA 0x80,C=1 -> 0x01 (carry routed in)", sys._cpu.getA(), 0x01);
    check("RLA C = old bit7", sys._cpu.getFlagC(), true);

    // --- RRA (0x1F): rotate right THROUGH carry ---
    sys._cpu.reset();
    sys._cpu.setA(0x01);
    sys._cpu.setFlagC(false);        // old carry = 0 fills bit 7
    sys._cpu.execute(0x1F);
    check("RRA 0x01,C=0 -> 0x00", sys._cpu.getA(), 0x00);
    check("RRA C = old bit0", sys._cpu.getFlagC(), true);
    check("RRA Z forced 0", sys._cpu.getFlagZ(), false);

    sys._cpu.reset();
    sys._cpu.setA(0x00);
    sys._cpu.setFlagC(true);         // old carry = 1 fills bit 7
    sys._cpu.execute(0x1F);
    check("RRA 0x00,C=1 -> 0x80 (carry routed in)", sys._cpu.getA(), 0x80);
    check("RRA C = old bit0 (0)", sys._cpu.getFlagC(), false);

    // --- Prove RLCA (circular) IGNORES carry-in, unlike RLA ---
    sys._cpu.reset();
    sys._cpu.setA(0x40);             // 0100 0000, bit7 = 0
    sys._cpu.setFlagC(true);         // carry set...
    sys._cpu.execute(0x07);          // RLCA
    check("RLCA ignores carry-in -> 0x80", sys._cpu.getA(), 0x80);  // carry-in irrelevant
    check("RLCA C = old bit7 (0)", sys._cpu.getFlagC(), false);
}

void testControlFlow() {
    TestSystem sys;

    // --- CALL -> RET round trip (the key integration test) ---
    sys._cpu.setSP(0xC100);
    // Put a CALL 0x1234 at 0xC000: opcode 0xCD, then addr bytes 0x34, 0x12
    sys._bus.write(0xC000, 0xCD);
    sys._bus.write(0xC001, 0x34);
    sys._bus.write(0xC002, 0x12);
    sys._cpu.setPC(0xC000);

    // Step through the CALL via the real fetch path (execute the fetched opcode)
    int cycles = sys._cpu.step();          // fetches 0xCD, runs CALL
    check("CALL jumps to target", sys._cpu.getPC(), 0x1234);
    check("CALL pushed return addr (SP-2)", sys._cpu.getSP(), 0xC0FE);
    check("CALL cycles", cycles, 24);

    // Now RET should bring PC back to 0xC003 (the instruction after the CALL)
    cycles = sys._cpu.execute(0xC9);       // RET
    check("RET returns to after CALL", sys._cpu.getPC(), 0xC003);
    check("RET restored SP", sys._cpu.getSP(), 0xC100);
    check("RET cycles", cycles, 16);

    // --- Conditional JP: taken vs not-taken ---
    // JP NZ taken (Z clear)
    sys._cpu.reset();
    sys._bus.write(0xC000, 0x00); sys._bus.write(0xC001, 0x20);  // addr 0x2000
    sys._cpu.setPC(0xC000);
    sys._cpu.setFlagZ(false);
    cycles = sys._cpu.execute(0xC2);       // JP NZ, a16 (operands at PC)
    check("JP NZ taken -> jumps", sys._cpu.getPC(), 0x2000);
    check("JP NZ taken cycles", cycles, 16);

    // JP NZ not taken (Z set) — PC should advance past operand only (to 0xC002), no jump
    sys._cpu.reset();
    sys._bus.write(0xC000, 0x00); sys._bus.write(0xC001, 0x20);
    sys._cpu.setPC(0xC000);
    sys._cpu.setFlagZ(true);
    cycles = sys._cpu.execute(0xC2);
    check("JP NZ not taken -> no jump", sys._cpu.getPC(), 0xC002);  // advanced past 2 operand bytes
    check("JP NZ not taken cycles", cycles, 12);

    // --- JR signed offset: forward and backward ---
    // JR +5 forward
    sys._cpu.reset();
    sys._bus.write(0xC000, 0x05);          // offset +5
    sys._cpu.setPC(0xC000);
    cycles = sys._cpu.execute(0x18);       // JR r8
    // after fetching offset, PC = 0xC001, then +5 = 0xC006
    check("JR +5 forward", sys._cpu.getPC(), 0xC006);

    // JR -1 backward (0xFF)
    sys._cpu.reset();
    sys._bus.write(0xC000, 0xFF);          // offset -1
    sys._cpu.setPC(0xC000);
    cycles = sys._cpu.execute(0x18);
    // after fetch, PC = 0xC001, then -1 = 0xC000
    check("JR -1 backward", sys._cpu.getPC(), 0xC000);

    // --- JP HL: 4 cycles, uses HL directly ---
    sys._cpu.reset();
    sys._cpu.setHL(0x4321);
    cycles = sys._cpu.execute(0xE9);
    check("JP HL jumps to HL value", sys._cpu.getPC(), 0x4321);
    check("JP HL cycles", cycles, 4);
}

void testDaa() {
    TestSystem sys;

    // Case 1: BCD 9 + 1 = 10  (low nibble > 9, add 6)
    sys._cpu.setA(0x09);
    sys._cpu.setB(0x01);
    sys._cpu.execute(0x80);          // ADD A,B -> 0x0A, N=0
    sys._cpu.execute(0x27);          // DAA
    check("DAA c1: 9+1 = BCD 10", sys._cpu.getA(), 0x10);
    check("DAA c1 Z", sys._cpu.getFlagZ(), false);
    check("DAA c1 H cleared", sys._cpu.getFlagH(), false);
    check("DAA c1 C", sys._cpu.getFlagC(), false);
    check("DAA c1 N unchanged", sys._cpu.getFlagN(), false);

    // Case 2: BCD 8 + 8 = 16  (half-carry triggers +6)
    sys._cpu.reset();
    sys._cpu.setA(0x08);
    sys._cpu.setB(0x08);
    sys._cpu.execute(0x80);          // ADD -> 0x10, H=1
    sys._cpu.execute(0x27);
    check("DAA c2: 8+8 = BCD 16", sys._cpu.getA(), 0x16);
    check("DAA c2 C", sys._cpu.getFlagC(), false);

    // Case 3: BCD 90 + 10 = 100 -> "00" + carry
    sys._cpu.reset();
    sys._cpu.setA(0x90);
    sys._cpu.setB(0x10);
    sys._cpu.execute(0x80);          // ADD -> 0xA0
    sys._cpu.execute(0x27);
    check("DAA c3: 90+10 = BCD 00", sys._cpu.getA(), 0x00);
    check("DAA c3 Z set", sys._cpu.getFlagZ(), true);
    check("DAA c3 C set (decimal carry)", sys._cpu.getFlagC(), true);

    // Case 4: BCD 10 - 1 = 09  (subtract path, H triggers -6)
    sys._cpu.reset();
    sys._cpu.setA(0x10);
    sys._cpu.setB(0x01);
    sys._cpu.execute(0x90);          // SUB A,B -> 0x0F, N=1, H=1
    sys._cpu.execute(0x27);
    check("DAA c4: 10-1 = BCD 09", sys._cpu.getA(), 0x09);
    check("DAA c4 N still set", sys._cpu.getFlagN(), true);
    check("DAA c4 C", sys._cpu.getFlagC(), false);
}

void testCB() {
    TestSystem sys;

    // Helper pattern: put CB opcode byte at PC, execute(0xCB) fetches it.

    // --- RLC B (0x00): rotate left circular ---
    sys._cpu.setB(0x80);
    sys._bus.write(0xC000, 0x00);
    sys._cpu.setPC(0xC000);
    int cycles = sys._cpu.execute(0xCB);
    check("RLC B: 0x80 -> 0x01", sys._cpu.getB(), 0x01);
    check("RLC B C flag", sys._cpu.getFlagC(), true);
    check("RLC B cycles", cycles, 8);

    // --- CB rotates SET Z (unlike accumulator RLCA) ---
    sys._cpu.reset();
    sys._cpu.setB(0x00);
    sys._bus.write(0xC000, 0x00);        // RLC B
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("RLC B on 0x00 sets Z=1 (CB differs from RLCA)", sys._cpu.getFlagZ(), true);

    // --- Dispatch: same op, different register (RLC C = 0x01) ---
    sys._cpu.reset();
    sys._cpu.setC(0x80);
    sys._bus.write(0xC000, 0x01);        // RLC C
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("RLC C routes to C register", sys._cpu.getC(), 0x01);

    // --- SRA vs SRL discriminator on 0x80 ---
    sys._cpu.reset();
    sys._cpu.setB(0x80);
    sys._bus.write(0xC000, 0x28);        // SRA B (op 5 << 3 | reg 0 = 0x28)
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("SRA B: 0x80 -> 0xC0 (bit7 preserved)", sys._cpu.getB(), 0xC0);

    sys._cpu.reset();
    sys._cpu.setB(0x80);
    sys._bus.write(0xC000, 0x38);        // SRL B (op 7 << 3 | reg 0 = 0x38)
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("SRL B: 0x80 -> 0x40 (bit7 zeroed)", sys._cpu.getB(), 0x40);

    // --- SWAP B (0x30) ---
    sys._cpu.reset();
    sys._cpu.setB(0xAB);
    sys._bus.write(0xC000, 0x30);        // SWAP B (op 6 << 3 | reg 0)
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("SWAP B: 0xAB -> 0xBA", sys._cpu.getB(), 0xBA);
    check("SWAP B clears C", sys._cpu.getFlagC(), false);

    // --- BIT: both Z directions ---
    // BIT 7, B (0x40 | 7<<3 | 0 = 0x78) on B=0x80 -> bit set -> Z=0
    sys._cpu.reset();
    sys._cpu.setB(0x80);
    sys._bus.write(0xC000, 0x78);        // BIT 7, B
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("BIT 7,B (bit set) -> Z=0", sys._cpu.getFlagZ(), false);
    check("BIT sets H=1", sys._cpu.getFlagH(), true);

    // BIT 0, B (0x40 | 0<<3 | 0 = 0x40) on B=0x80 -> bit clear -> Z=1
    sys._cpu.reset();
    sys._cpu.setB(0x80);
    sys._bus.write(0xC000, 0x40);        // BIT 0, B
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("BIT 0,B (bit clear) -> Z=1", sys._cpu.getFlagZ(), true);

    // --- RES 7, B (0x80 | 7<<3 | 0 = 0xB8) on 0xFF -> 0x7F ---
    sys._cpu.reset();
    sys._cpu.setB(0xFF);
    sys._bus.write(0xC000, 0xB8);        // RES 7, B
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("RES 7,B: 0xFF -> 0x7F", sys._cpu.getB(), 0x7F);

    // --- SET 3, B (0xC0 | 3<<3 | 0 = 0xD8) on 0x00 -> 0x08 ---
    sys._cpu.reset();
    sys._cpu.setB(0x00);
    sys._bus.write(0xC000, 0xD8);        // SET 3, B
    sys._cpu.setPC(0xC000);
    sys._cpu.execute(0xCB);
    check("SET 3,B: 0x00 -> 0x08", sys._cpu.getB(), 0x08);

    // --- (HL) target: RLC (HL) = 0x06, and check 16 cycles ---
    sys._cpu.reset();
    sys._cpu.setHL(0xC500);
    sys._bus.write(0xC500, 0x80);        // value in memory
    sys._bus.write(0xC000, 0x06);        // RLC (HL)
    sys._cpu.setPC(0xC000);
    cycles = sys._cpu.execute(0xCB);
    check("RLC (HL): mem 0x80 -> 0x01", sys._bus.read(0xC500), 0x01);
    check("RLC (HL) is 16 cycles", cycles, 16);
}

void runGameboyDoctor(const std::string& romPath) {
    GameBoy gb;
    gb.loadROM(romPath);
    for (int i = 0; i < 9000000; i++) {   // bounded — cpu_instrs sub-tests finish well within this
        gb.logState();          // need a way to call CPU's logState via GameBoy
        gb.step();
    }
}

// int main() {
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
    // testAdc();
    // testSbc();
    // test16BitIncDec();
    // testAdd16();
    // testAddSPr8();
    // testRotates();
    // testControlFlow();
    // testDaa();
    // testCB();
//     return 0;
// }

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: gbemu <rom-path>\n";
        return 1;
    }
    runGameboyDoctor(argv[1]);
    return 0;
}