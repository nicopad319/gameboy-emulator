#pragma once
#include <cstdint>

class Timer {
public:
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

    bool tick(int cycles);

private:
    uint8_t _div = 0;
    uint8_t _tima = 0;
    uint8_t _tma = 0;
    uint8_t _tac = 0;
    int _divCounter = 0;
    int _timaCounter = 0;
};