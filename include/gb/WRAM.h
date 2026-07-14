#pragma once
#include <cstdint>
#include <array>

class WRAM {
public:
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

private:
    std::array<uint8_t, 0x2000> _wram; //8kb of ram (0xDFFF - 0xC000 + 1 = 0x2000)
};
