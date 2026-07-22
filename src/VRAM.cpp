#include "VRAM.h"
#include "PPU.h" // Include the PPU header for access to PPU class

VRAM::VRAM(PPU* ppu) : _ppu(ppu) {
    // Constructor to initialize the PPU pointer
    // This allows VRAM to check the PPU's current mode for locking logic
}

uint8_t VRAM::read(uint16_t address) {
    if (_ppu->isVramLocked()) {
        return 0xFF;
    }
    return _vram.at(address - 0x8000);
}

void VRAM::write(uint16_t address, uint8_t value) {
    if (_ppu->isVramLocked()) {
        return;
    }
    _vram.at(address - 0x8000) = value;
}

uint8_t VRAM::readRaw(uint16_t address) const {
    return _vram.at(address - 0x8000);
}