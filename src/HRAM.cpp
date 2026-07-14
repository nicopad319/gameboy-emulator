#include "HRAM.h"

uint8_t HRAM::read(uint16_t address) {
    return _hram.at(address - 0xFF80);
}

void HRAM::write(uint16_t address, uint8_t value) {
    _hram.at(address - 0xFF80) = value;
}