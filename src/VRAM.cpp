#include "VRAM.h"
#include "PPU.h" // Include the PPU header for access to PPU class

VRAM::VRAM(PPU* ppu) : _ppu(ppu) {
    // Constructor to initialize the PPU pointer
    // This allows VRAM to check the PPU's current mode for locking logic
}

uint8_t VRAM::read(uint16_t address) {
    // --- VRAM LOCKING LOGIC ---
    if (_ppu->getCurrentMode() == 3) {
        return 0xFF; // CPU reads 0xFF when VRAM is locked during Mode 3
    }
    return _vram.at(address - 0x8000);
}

void VRAM::write(uint16_t address, uint8_t value) {
    if (_ppu->getCurrentMode() == 3) {
        return; // CPU cannot write to VRAM when it is locked during Mode 3
    }
    _vram.at(address - 0x8000) = value;
}