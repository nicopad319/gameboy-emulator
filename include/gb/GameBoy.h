#pragma once
#include "Bus.h"
#include "Cartridge.h"
#include "HRAM.h"
#include "IERegister.h"
#include "IORegisters.h"
#include "OAM.h"
#include "PPU.h"
#include "VRAM.h"
#include "WRAM.h"
#include "CPU.h"
#include <string>
#include <cstdint>

class GameBoy {
public:
    GameBoy(); //constructs everything (is the root) so doesn't need to take in any parameters
    bool loadROM(const std::string& path);

    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

    int step();

    void logState();

private:
    PPU _ppu; //depends on nothing (is a stub) must be declared first
    Cartridge _cartridge;
    WRAM _wram;
    HRAM _hram;
    IORegisters _ioRegisters;
    IERegister _ieRegister;
    VRAM _vram; //depends on PPU so must be declared after it
    OAM _oam; //also depends on PPU
    Bus _bus; //depends on everything so must be declared last
    CPU _cpu; //holds a bus so must come after bus
};