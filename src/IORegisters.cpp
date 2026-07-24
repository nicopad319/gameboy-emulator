#include "IORegisters.h"
#include <iostream>

uint8_t IORegisters::read(uint16_t address) {
    return _ioRegisters.at(address - 0xFF00);
}

void IORegisters::write(uint16_t address, uint8_t value) {
    
    _ioRegisters.at(address - 0xFF00) = value;

    // Serial output capture (for test ROMs)
    if (address == 0xFF02 && value == 0x81) {
        char c = static_cast<char>(read(0xFF01));   // the byte to "send"
        std::cerr << c;                             // print to stderr (keeps stdout log clean)
    }
}