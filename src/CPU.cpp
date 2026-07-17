#include "CPU.h"

CPU::CPU() {
    // load standard initial register values for an original Game Boy (DMG) -- from PanDocs
    _a = 0x01;
    _f = 0xB0;
    _b = 0x00;
    _c = 0x13;
    _d = 0x00;
    _e = 0xD8;
    _h = 0x01;
    _l = 0x4D;
    _sp = 0xFFFE; //top of HRAM
    _pc = 0x0100; //execution entry point on game cartridge
}

uint8_t CPU::getA() const {
    return _a;
}
uint8_t CPU::getF() const {
    return _f;
}
uint8_t CPU::get