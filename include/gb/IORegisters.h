#pragma once
#include <cstdint>
#include <array>

class IORegisters {
public:
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

private:
    std::array<uint8_t, 0x0080> _ioRegisters; //128 bytes of I/O registers (0xFF7F - 0xFF00 + 1 = 0x80)
};
