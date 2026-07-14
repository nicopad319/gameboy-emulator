#include "WRAM.h"

uint8_t WRAM::read(uint16_t address) {
    return _wram.at(address - 0xC000);
}

void WRAM::write(uint16_t address, uint8_t value) {
    _wram.at(address - 0xC000) = value;
}