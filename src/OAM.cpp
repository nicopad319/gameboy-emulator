#include "OAM.h"
#include "PPU.h" // Include the PPU header for access to PPU class

OAM::OAM(PPU* ppu) : _ppu(ppu) {
    // Constructor to initialize the PPU pointer
    // This allows OAM to check the PPU's current mode for locking logic
}

uint8_t OAM::read(uint16_t address) {
    if (_ppu->isOamLocked()) return 0xFF;
    return _oam.at(address - 0xFE00);
}

void OAM::write(uint16_t address, uint8_t value) {
    if (_ppu->isOamLocked()) return;
    _oam.at(address - 0xFE00) = value;
}

uint8_t OAM::readRaw(uint16_t address) const { return _oam.at(address - 0xFE00); }

void    OAM::writeRaw(uint16_t address, uint8_t value) { _oam.at(address - 0xFE00) = value; }

