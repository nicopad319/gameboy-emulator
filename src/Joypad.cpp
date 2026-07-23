#include "Joypad.h"

void Joypad::setButton(Button b, bool pressed) {
    _pressed[static_cast<int>(b)] = pressed;
}

void Joypad::write(uint8_t value) {
    _select = value & 0x30;   // only bits 4-5 are writable
}

uint8_t Joypad::read() const {
    uint8_t lower = 0x0F;     // bits 0-3, 1 = released
    if (!(_select & 0x10)) {  // bit 4 low -> directions selected
        if (_pressed[static_cast<int>(Button::Right)]) lower &= ~0x01;
        if (_pressed[static_cast<int>(Button::Left)])  lower &= ~0x02;
        if (_pressed[static_cast<int>(Button::Up)])    lower &= ~0x04;
        if (_pressed[static_cast<int>(Button::Down)])  lower &= ~0x08;
    }
    if (!(_select & 0x20)) {  // bit 5 low -> actions selected
        if (_pressed[static_cast<int>(Button::A)])      lower &= ~0x01;
        if (_pressed[static_cast<int>(Button::B)])      lower &= ~0x02;
        if (_pressed[static_cast<int>(Button::Select)]) lower &= ~0x04;
        if (_pressed[static_cast<int>(Button::Start)])  lower &= ~0x08;
    }
    return 0xC0 | (_select & 0x30) | lower;   // bits 7-6 read as 1
}