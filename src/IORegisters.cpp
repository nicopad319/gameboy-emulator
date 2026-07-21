#include "IORegisters.h"

uint8_t IORegisters::read(uint16_t address) {
    if (address == 0xFF44) {
        return 0x90; //TODO: remove when PPU is implemented
    } 
    return _ioRegisters.at(address - 0xFF00);
}

void IORegisters::write(uint16_t address, uint8_t value) {
    _ioRegisters.at(address - 0xFF00) = value;
}