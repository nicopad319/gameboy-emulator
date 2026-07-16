#include "OAM.h"
#include "PPU.h" // Include the PPU header for access to PPU class

OAM::OAM(PPU* ppu) : _ppu(ppu) {
    // Constructor to initialize the PPU pointer
    // This allows OAM to check the PPU's current mode for locking logic
}

uint8_t OAM::read(uint16_t address) {
    // --- OAM LOCKING LOGIC ---
    if (_ppu->getCurrentMode() == 2 || _ppu->getCurrentMode() == 3) {
        return 0xFF; // CPU reads 0xFF when OAM is locked during Mode 2 or 3
    }
    return _oam.at(address - 0xFE00);
}

void OAM::write(uint16_t address, uint8_t value) {
    if (_ppu->getCurrentMode() == 2 || _ppu->getCurrentMode() == 3) {
        return; // CPU cannot write to OAM when it is locked during Mode 2 or 3
    }
    _oam.at(address - 0xFE00) = value;
}