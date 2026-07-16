#pragma once
#include <cstdint>
#include <array>

class PPU; // Forward declaration of PPU class

class OAM {
public:
    OAM(PPU* ppu); // Constructor to initialize the PPU pointer
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

private:
    std::array<uint8_t, 0x00A0> _oam; //160 bytes of ram (0xFE9F - 0xFE00 + 1 = 0x00A0)
    PPU* _ppu = nullptr;
};
