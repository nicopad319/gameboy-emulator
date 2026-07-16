#include "GameBoy.h"

GameBoy::GameBoy()
        : _vram(&_ppu), // Initialize VRAM with a pointer to PPU
          _oam(&_ppu), // Initialize OAM with a pointer to PPU
          _bus(&_cartridge, &_wram, &_ioRegisters, &_vram, &_hram, &_ieRegister, &_oam) // Initialize Bus with pointers to all components
        {
            //empty body
        }


bool GameBoy::loadROM(const std::string& path) {
    return _cartridge.load(path); // Load the ROM into the cartridge
}