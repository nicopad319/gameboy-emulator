#pragma once
#include <cstdint>
#include <array>

class IERegister {
public:
    uint8_t read();
    void write(uint8_t value); //no address parameter needed since there is only one byte of memory at 0xFFFF

private:
    std::array<uint8_t, 1> _ieRegister{}; //1 byte of ram (0xFFFF - 0xFFFF + 1 = 1)
};
