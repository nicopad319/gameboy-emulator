#pragma once
#include <cstdint>
#include <array>

class HRAM {
public:
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

private:
    std::array<uint8_t, 0x7F> _hram{}; //127 bytes of ram (0xFFFE - 0xFF80 + 1 = 0x7F)
};
