#pragma once
#include <cstdint>
#include <array>

class PPU; // Forward declaration of PPU class

class VRAM {
public:
    VRAM(PPU* ppu); // Constructor to initialize the PPU pointer
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

    uint8_t readRaw(uint16_t address) const;   // unlocked read for the PPU's own use

private:
    std::array<uint8_t, 0x2000> _vram{}; //8kb of ram (0x9FFF - 0x8000 + 1 = 0x2000)
    PPU* _ppu = nullptr;
};
