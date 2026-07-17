#include "CPU.h"
#include "Bus.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

CPU::CPU(Bus& bus) : _bus(bus) {
    reset();
}
void CPU::reset() {
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

//8 bit getter functions
uint8_t CPU::getA() const {
    return _a;
}
uint8_t CPU::getF() const {
    return _f;
}
uint8_t CPU::getB() const {
    return _b;
}
uint8_t CPU::getC() const {
    return _c;
}
uint8_t CPU::getD() const {
    return _d;
}
uint8_t CPU::getE() const {
    return _e;
}
uint8_t CPU::getH() const {
    return _h;
}
uint8_t CPU::getL() const {
    return _l;
}

//16 bit getter functions
uint16_t CPU::getAF() const {
    return (static_cast<uint16_t>(_a) << 8) | _f;
}
uint16_t CPU::getBC() const {
    return (static_cast<uint16_t>(_b) << 8) | _c;
}
uint16_t CPU::getDE() const {
    return (static_cast<uint16_t>(_d) << 8) | _e;
}
uint16_t CPU::getHL() const {
    return (static_cast<uint16_t>(_h) << 8) | _l;
}
uint16_t CPU::getSP() const {
    return _sp;
}
uint16_t CPU::getPC() const {
    return _pc;
}

//8 bit setters
void CPU::setA(uint8_t value) {
    _a = value;
}
void CPU::setF(uint8_t value) {
    _f = value;
}
void CPU::setB(uint8_t value) {
    _b = value;
}
void CPU::setC(uint8_t value) {
    _c = value;
}
void CPU::setD(uint8_t value) {
    _d = value;
}
void CPU::setE(uint8_t value) {
    _e = value;
}
void CPU::setH(uint8_t value) {
    _h = value;
}
void CPU::setL(uint8_t value) {
    _l = value;
}

//16 bit setters
void CPU::setAF(uint16_t value) {
    _a = static_cast<uint8_t>(value >> 8);
    _f = static_cast<uint8_t>(value & 0xF0); //keeps bits 0-3 of F strictly to 0
}
void CPU::setBC(uint16_t value) {
    _b = static_cast<uint8_t>(value >> 8);
    _c = static_cast<uint8_t>(value); 
}
void CPU::setDE(uint16_t value) {
    _d = static_cast<uint8_t>(value >> 8);
    _e = static_cast<uint8_t>(value); 
}
void CPU::setHL(uint16_t value) {
    _h = static_cast<uint8_t>(value >> 8);
    _l = static_cast<uint8_t>(value);
}
void CPU::setSP(uint16_t value) {
    _sp = value;
}
void CPU::setPC(uint16_t value) {
    _pc = value;
}

//flag accessors
bool CPU::getFlagZ() const {
    return ((_f & FLAG_Z) != 0);
}
bool CPU::getFlagN() const {
    return ((_f & FLAG_N) != 0);
}
bool CPU::getFlagH() const {
    return ((_f & FLAG_H) != 0);
}
bool CPU::getFlagC() const {
    return ((_f & FLAG_C) != 0);
}

void CPU::setFlagZ(bool value) {
    _f = value ? (_f | FLAG_Z) : (_f & ~FLAG_Z); //? is ternary operator: condition ? expression_if_true : expression_if_false;
}
void CPU::setFlagN(bool value) {
    _f = value ? (_f | FLAG_N) : (_f & ~FLAG_N);
}
void CPU::setFlagH(bool value) {
    _f = value ? (_f | FLAG_H) : (_f & ~FLAG_H);
}
void CPU::setFlagC(bool value) {
    _f = value ? (_f | FLAG_C) : (_f & ~FLAG_C);
}

//opcode logic
uint8_t CPU::fetchByte() {
    uint8_t byte = _bus.read(_pc);
    _pc++;
    return byte;
}

int CPU::execute(uint8_t opcode) {
    switch (opcode) {
        case 0x00: return 4; 
        default: 
            std::cerr << "Unimplemented opcode 0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(opcode)
            << " at PC 0x" << static_cast<int>(_pc - 1)
            << std::endl;
            throw std::runtime_error("Unimplemented opcode");
    }
}
int CPU::step() {
    uint8_t opcode = fetchByte();
    return execute(opcode);
}



