#include "GameBoy.h"

GameBoy::GameBoy()
        : _vram(&_ppu), // Initialize VRAM with a pointer to PPU
          _oam(&_ppu), // Initialize OAM with a pointer to PPU
          _bus(&_cartridge, &_wram, &_ioRegisters, &_vram, &_hram, &_ieRegister, &_oam, &_timer, &_ppu), // Initialize Bus with pointers to all components
          _cpu(_bus)
        {
            _ppu.connectVRAM(&_vram);
        }


bool GameBoy::loadROM(const std::string& path) {
    return _cartridge.load(path); // Load the ROM into the cartridge
}

uint8_t GameBoy::read(uint16_t address) {
    return _bus.read(address);
}

void GameBoy::write(uint16_t address, uint8_t value) {
    _bus.write(address, value);
}

void GameBoy::logState() {
    _cpu.logState();
}

void GameBoy::requestInterrupt(int bit) {
    uint8_t current = _bus.read(0xFF0F);
    _bus.write(0xFF0F, current | (1 << bit));
}

int GameBoy::step() {
    int cycles = _cpu.step();
    if (_timer.tick(cycles)) {       // tick returns true on TIMA overflow
        requestInterrupt(2);         // Timer interrupt = bit 2
    }
    uint8_t ppuIrq = _ppu.tick(cycles);
    if (ppuIrq & 0x01) {
        requestInterrupt(0);   // VBlank
    }
    if (ppuIrq & 0x02) { 
        requestInterrupt(1);   // STAT (nothing sets this yet)
    }
    return cycles;
}

void GameBoy::enableCpuLogging() {
    _cpu.enableLogging();
}