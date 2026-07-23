#pragma once
#include <cstdint>

class Joypad {
public:
    enum class Button { Right, Left, Up, Down, A, B, Select, Start };

    void    setButton(Button b, bool pressed);  // the portable seam (SDL now, GPIO later)
    uint8_t read() const;                        // CPU reads 0xFF00
    void    write(uint8_t value);                // CPU writes select bits

private:
    bool    _pressed[8] = { false };  // indexed by Button; true = held
    uint8_t _select = 0x30;           // bits 4,5 (1 = that group NOT selected)
};