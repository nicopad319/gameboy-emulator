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
    _ime = false;
    _halted = false;
    _imeScheduled = false;
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

uint16_t CPU::fetchWord() {
    uint8_t lo = fetchByte();
    uint8_t hi = fetchByte();
    return (hi << 8) | lo;
}

void CPU::push(uint16_t value) {
    uint8_t high = (value >> 8) & 0xFF;   // high byte of value
    uint8_t low  = value & 0xFF;          // low byte of value
    setSP(getSP() - 1);
    _bus.write(getSP(), high);            // high byte at higher address
    setSP(getSP() - 1);
    _bus.write(getSP(), low);             // low byte at lower address
}

uint16_t CPU::pop() {
    uint8_t low  = _bus.read(getSP());
    setSP(getSP() + 1);
    uint8_t high = _bus.read(getSP());
    setSP(getSP() + 1);
    return (high << 8) | low;
}

//arithmetic operations
void CPU::add8(uint8_t value) {
    uint8_t a = getA(); //store original a
    int result = a + value; //wide type (sees full sum)
    
    setFlagZ((result & 0xFF) == 0);
    setFlagN(false); //always cleared for addition
    setFlagH((a & 0x0F) + (value & 0x0F) > 0x0F);
    setFlagC(result > 0xFF);

    setA(result & 0xFF); //set result after all flag calculations
}

uint8_t CPU::subFlags(uint8_t value) {   // computes A - value, sets flags, returns result
    uint8_t a = getA();
    int result = a - value;
    setFlagZ((result & 0xFF) == 0);
    setFlagN(true);
    setFlagH((a & 0x0F) < (value & 0x0F));
    setFlagC(a < value);
    return result & 0xFF;
}

void CPU::sub8(uint8_t value) {
    setA(subFlags(value));      // store the result
}

void CPU::cp8(uint8_t value) {
    subFlags(value);            // discard the result, keep only the flags
}

void CPU::and8(uint8_t value) {
    uint8_t a = getA();
    uint8_t result = a & value;
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(true); 
    setFlagC(false);
    setA(result);
}

void CPU::or8(uint8_t value) {
    uint8_t a = getA();
    uint8_t result = a | value;
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(false);
    setA(result);
}

void CPU::xor8(uint8_t value) {
    uint8_t a = getA();
    uint8_t result = a ^ value;
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(false);
    setA(result);
}

uint8_t CPU::inc8(uint8_t value) {
    uint8_t result = value + 1;
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH((value & 0x0F) == 0x0F);
    //flagC is untouched
    return result;
}

uint8_t CPU::dec8(uint8_t value) {
    uint8_t result = value - 1;
    setFlagZ(result == 0);
    setFlagN(true);
    setFlagH((value & 0x0F) == 0x00);
    //flagC untouched
    return result;
}

void CPU::adc8(uint8_t value) {
    uint8_t a = getA(); 
    int carry = getFlagC() ? 1 : 0; //1 if carry flag is set 0 if not
    int result = a + value + carry; //wide type (sees full sum)
    
    setFlagZ((result & 0xFF) == 0);
    setFlagN(false); //always cleared for addition
    setFlagH((a & 0x0F) + (value & 0x0F) + carry > 0x0F);
    setFlagC(result > 0xFF);

    setA(result & 0xFF); //set result after all flag calculations
}

void CPU::sbc8(uint8_t value) {
    uint8_t a = getA();
    int carry = getFlagC() ? 1 : 0;
    int result = a - value - carry;
    setFlagZ((result & 0xFF) == 0);
    setFlagN(true);
    setFlagH((a & 0x0F) < (value & 0x0F) + carry);
    setFlagC(a < value + carry);
    setA(result & 0xFF);
}

void CPU::add16(uint16_t value) {
    uint16_t hl = getHL();
    int result = hl + value; //wide enough to see bit-15 carry
    //FlagZ untouched
    setFlagN(false);
    setFlagH((hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF);
    setFlagC(result > 0xFFFF);
    setHL(result & 0xFFFF);
}

uint16_t CPU::addSPr8() {
    uint8_t raw = fetchByte();                      // unsigned, for flags
    int8_t offset = static_cast<int8_t>(raw);       // signed, for the result
    uint16_t sp = getSP();
    setFlagZ(false);
    setFlagN(false);
    setFlagH((sp & 0x0F) + (raw & 0x0F) > 0x0F);    // use raw (unsigned) for flags
    setFlagC((sp & 0xFF) + (raw & 0xFF) > 0xFF);
    return sp + offset;                             // use offset (signed) for result
}

int CPU::execute(uint8_t opcode) {
    switch (opcode) {
        case 0x00: return 4; //NOP
        case 0x01: setBC(fetchWord()); return 12;
        case 0x02: _bus.write(getBC(), getA()); return 8;
        case 0x03: setBC(getBC() + 1); return 8;
        case 0x04: setB(inc8(getB())); return 4;
        case 0x05: setB(dec8(getB())); return 4;
        case 0x06: setB(fetchByte()); return 8;
        case 0x07: { //RLCA
            uint8_t bit7 = (getA() >> 7) & 1;
            setA((getA() << 1) | bit7);
            setFlagC(bit7);
            setFlagZ(false);
            setFlagN(false);
            setFlagH(false);
            return 4;
        }
        case 0x08: {
            uint16_t addr = fetchWord(); //safer to call fetchWord first b/c C++ has no specified order for parameters
            _bus.write16(addr, getSP()); //(fetchWord could affect SP if it was called in the same line)
            return 20;
        }
        case 0x09: add16(getBC()); return 8;
        case 0x0A: setA(_bus.read(getBC())); return 8;
        case 0x0B: setBC(getBC() - 1); return 8;
        case 0x0C: setC(inc8(getC())); return 4;
        case 0x0D: setC(dec8(getC())); return 4;
        case 0x0E: setC(fetchByte()); return 8;
        case 0x0F: { //RRCA
            uint8_t bit0 = getA() & 1;
            setA((getA() >> 1) | (bit0 << 7));
            setFlagC(bit0);
            setFlagZ(false);
            setFlagN(false);
            setFlagH(false);
            return 4;
        }
        case 0x10: fetchByte(); _halted = true; return 4;   // TODO: real STOP behavior (M5); consumes next byte
        case 0x11: setDE(fetchWord()); return 12;
        case 0x12: _bus.write(getDE(), getA()); return 8;
        case 0x13: setDE(getDE() + 1); return 8;
        case 0x14: setD(inc8(getD())); return 4;
        case 0x15: setD(dec8(getD())); return 4;
        case 0x16: setD(fetchByte()); return 8;
        case 0x17: { //RLA
            uint8_t oldCarry = getFlagC();
            uint8_t bit7 = (getA() >> 7) & 1;
            setA((getA() << 1) | oldCarry);
            setFlagC(bit7); 
            setFlagZ(false);
            setFlagN(false);
            setFlagH(false);
            return 4;
        }
        case 0x18: {
            int8_t offset = static_cast<int8_t>(fetchByte());
            setPC(getPC() + offset);
            return 12;
        }
        case 0x19: add16(getDE()); return 8;
        case 0x1A: setA(_bus.read(getDE())); return 8;
        case 0x1B: setDE(getDE() - 1); return 8;
        case 0x1C: setE(inc8(getE())); return 4;
        case 0x1D: setE(dec8(getE())); return 4;
        case 0x1E: setE(fetchByte()); return 8;
        case 0x1F: {
            uint8_t oldCarry = getFlagC();
            uint8_t bit0 = getA() & 1;
            setA((getA() >> 1) | (oldCarry << 7));
            setFlagC(bit0); 
            setFlagZ(false);
            setFlagN(false);
            setFlagH(false);
            return 4;
        }
        case 0x20: {  // JR NZ, r8
            int8_t offset = static_cast<int8_t>(fetchByte());   // ALWAYS fetch
            if (!getFlagZ()) {
                setPC(getPC() + offset);
                return 12;   // taken
            }
            return 8;        // not taken
        }
        case 0x21: setHL(fetchWord()); return 12;
        case 0x22: {  // LD (HL+), A
            _bus.write(getHL(), getA());   // write at CURRENT HL
            setHL(getHL() + 1);            // THEN increment HL
            return 8;
        }
        case 0x23: setHL(getHL() + 1); return 8;
        case 0x24: setH(inc8(getH())); return 4;
        case 0x25: setH(dec8(getH())); return 4;
        case 0x26: setH(fetchByte()); return 8;
        case 0x27: {  // DAA
            int a = getA(); //work in int
            int correction = 0;
            bool setC = false;

            if (!getFlagN()) {   // after an ADD
                if (getFlagH() || (a & 0x0F) > 0x09) {
                    correction |= 0x06;
                }
                if (getFlagC() || a > 0x99) {
                    correction |= 0x60;
                    setC = true;
                }
                a += correction;
            } else {             // after a SUB
                if (getFlagH()) {
                    correction |= 0x06;
                }
                if (getFlagC()) {
                    correction |= 0x60;
                }
                a -= correction;
                setC = getFlagC();   // subtract keeps the existing carry
            }

            setA(static_cast<uint8_t>(a & 0xFF));   // explicit narrowing at the store
            setFlagZ((a & 0xFF) == 0);
            setFlagH(false);        // H always cleared
            setFlagC(setC);
            // N unchanged
            return 4;
        }
        case 0x28: {  // JR Z, r8
            int8_t offset = static_cast<int8_t>(fetchByte());   // ALWAYS fetch
            if (getFlagZ()) {
                setPC(getPC() + offset);
                return 12;   // taken
            }
            return 8;        // not taken
        }
        case 0x29: add16(getHL()); return 8;
        case 0x2A: {
            setA(_bus.read(getHL()));
            setHL(getHL() + 1);
            return 8;
        }
        case 0x2B: setHL(getHL() - 1); return 8;
        case 0x2C: setL(inc8(getL())); return 4;
        case 0x2D: setL(dec8(getL())); return 4;
        case 0x2E: setL(fetchByte()); return 8;
        case 0x2F: {
            setA(~getA());
            setFlagN(true);
            setFlagH(true);
            return 4;
        }
        case 0x30: {  // JR NC, r8
            int8_t offset = static_cast<int8_t>(fetchByte());   // ALWAYS fetch
            if (!getFlagC()) {
                setPC(getPC() + offset);
                return 12;   // taken
            }
            return 8;        // not taken
        }
        case 0x31: setSP(fetchWord()); return 12;
        case 0x32: {  // LD (HL-), A
            _bus.write(getHL(), getA());   // write at CURRENT HL
            setHL(getHL() - 1);            // THEN subtract HL
            return 8;
        }
        case 0x33: setSP(getSP() + 1); return 8;
        case 0x34: {
            uint8_t v = _bus.read(getHL());
            _bus.write(getHL(), inc8(v));
            return 12;
        }
        case 0x35: {
            uint8_t v = _bus.read(getHL());
            _bus.write(getHL(), dec8(v));
            return 12;
        }
        case 0x36: {
            uint8_t value = fetchByte();
            _bus.write(getHL(), value);
            return 12;
        }
        case 0x37: {
            setFlagN(false);
            setFlagH(false);
            setFlagC(true);
            return 4;
        }
        case 0x38: {  // JR C, r8
            int8_t offset = static_cast<int8_t>(fetchByte());   // ALWAYS fetch
            if (getFlagC()) {
                setPC(getPC() + offset);
                return 12;   // taken
            }
            return 8;        // not taken
        }
        case 0x39: add16(getSP()); return 8;
        case 0x3A: {
            setA(_bus.read(getHL()));
            setHL(getHL() - 1);
            return 8;
        }
        case 0x3B: setSP(getSP() - 1); return 8;
        case 0x3C: setA(inc8(getA())); return 4;
        case 0x3D: setA(dec8(getA())); return 4;
        case 0x3E: setA(fetchByte()); return 8;
        case 0x3F: {
            setFlagN(false);
            setFlagH(false);
            setFlagC(!(getFlagC()));
            return 4;
        }
        //8 bit loads
        case 0x40: setB(getB()); return 4;
        case 0x41: setB(getC()); return 4;
        case 0x42: setB(getD()); return 4;
        case 0x43: setB(getE()); return 4;
        case 0x44: setB(getH()); return 4;
        case 0x45: setB(getL()); return 4;
        case 0x46: setB(_bus.read(getHL())); return 8;
        case 0x47: setB(getA()); return 4;
        case 0x48: setC(getB()); return 4; 
        case 0x49: setC(getC()); return 4;
        case 0x4A: setC(getD()); return 4;
        case 0x4B: setC(getE()); return 4;
        case 0x4C: setC(getH()); return 4;
        case 0x4D: setC(getL()); return 4;
        case 0x4E: setC(_bus.read(getHL())); return 8;
        case 0x4F: setC(getA()); return 4;
        case 0x50: setD(getB()); return 4; 
        case 0x51: setD(getC()); return 4;
        case 0x52: setD(getD()); return 4;
        case 0x53: setD(getE()); return 4;
        case 0x54: setD(getH()); return 4;
        case 0x55: setD(getL()); return 4;
        case 0x56: setD(_bus.read(getHL())); return 8;
        case 0x57: setD(getA()); return 4;
        case 0x58: setE(getB()); return 4;
        case 0x59: setE(getC()); return 4;
        case 0x5A: setE(getD()); return 4;
        case 0x5B: setE(getE()); return 4;
        case 0x5C: setE(getH()); return 4;
        case 0x5D: setE(getL()); return 4;
        case 0x5E: setE(_bus.read(getHL())); return 8;
        case 0x5F: setE(getA()); return 4;
        case 0x60: setH(getB()); return 4;
        case 0x61: setH(getC()); return 4;
        case 0x62: setH(getD()); return 4;
        case 0x63: setH(getE()); return 4;
        case 0x64: setH(getH()); return 4;
        case 0x65: setH(getL()); return 4;
        case 0x66: setH(_bus.read(getHL())); return 8;
        case 0x67: setH(getA()); return 4;
        case 0x68: setL(getB()); return 4;
        case 0x69: setL(getC()); return 4;
        case 0x6A: setL(getD()); return 4;
        case 0x6B: setL(getE()); return 4;
        case 0x6C: setL(getH()); return 4;
        case 0x6D: setL(getL()); return 4;
        case 0x6E: setL(_bus.read(getHL())); return 8;
        case 0x6F: setL(getA()); return 4;
        case 0x70: _bus.write(getHL(), getB()); return 8;
        case 0x71: _bus.write(getHL(), getC()); return 8;
        case 0x72: _bus.write(getHL(), getD()); return 8;
        case 0x73: _bus.write(getHL(), getE()); return 8;
        case 0x74: _bus.write(getHL(), getH()); return 8;
        case 0x75: _bus.write(getHL(), getL()); return 8;
        case 0x76: _halted = true; return 4;   // TODO: real HALT behavior needs interrupts (M5)
        case 0x77: _bus.write(getHL(), getA()); return 8;
        case 0x78: setA(getB()); return 4;
        case 0x79: setA(getC()); return 4;
        case 0x7A: setA(getD()); return 4;
        case 0x7B: setA(getE()); return 4;
        case 0x7C: setA(getH()); return 4;
        case 0x7D: setA(getL()); return 4;
        case 0x7E: setA(_bus.read(getHL())); return 8;
        case 0x7F: setA(getA()); return 4;
        //arithmetic add operations
        case 0x80: add8(getB()); return 4;
        case 0x81: add8(getC()); return 4;
        case 0x82: add8(getD()); return 4;
        case 0x83: add8(getE()); return 4;
        case 0x84: add8(getH()); return 4;
        case 0x85: add8(getL()); return 4;
        case 0x86: add8(_bus.read(getHL())); return 8;
        case 0x87: add8(getA()); return 4;
        case 0x88: adc8(getB()); return 4;
        case 0x89: adc8(getC()); return 4;
        case 0x8A: adc8(getD()); return 4;
        case 0x8B: adc8(getE()); return 4;
        case 0x8C: adc8(getH()); return 4;
        case 0x8D: adc8(getL()); return 4;
        case 0x8E: adc8(_bus.read(getHL())); return 8;
        case 0x8F: adc8(getA()); return 4;
        //sub8 operations
        case 0x90: sub8(getB()); return 4;
        case 0x91: sub8(getC()); return 4;
        case 0x92: sub8(getD()); return 4;
        case 0x93: sub8(getE()); return 4;
        case 0x94: sub8(getH()); return 4;
        case 0x95: sub8(getL()); return 4;
        case 0x96: sub8(_bus.read(getHL())); return 8;
        case 0x97: sub8(getA()); return 4;
        case 0x98: sbc8(getB()); return 4;
        case 0x99: sbc8(getC()); return 4;
        case 0x9A: sbc8(getD()); return 4;
        case 0x9B: sbc8(getE()); return 4;
        case 0x9C: sbc8(getH()); return 4;
        case 0x9D: sbc8(getL()); return 4;
        case 0x9E: sbc8(_bus.read(getHL())); return 8;
        case 0x9F: sbc8(getA()); return 4;
        case 0xA0: and8(getB()); return 4;
        case 0xA1: and8(getC()); return 4;
        case 0xA2: and8(getD()); return 4;
        case 0xA3: and8(getE()); return 4;
        case 0xA4: and8(getH()); return 4;
        case 0xA5: and8(getL()); return 4;
        case 0xA6: and8(_bus.read(getHL())); return 8;
        case 0xA7: and8(getA()); return 4;
        case 0xA8: xor8(getB()); return 4;
        case 0xA9: xor8(getC()); return 4;
        case 0xAA: xor8(getD()); return 4;
        case 0xAB: xor8(getE()); return 4;
        case 0xAC: xor8(getH()); return 4;
        case 0xAD: xor8(getL()); return 4;
        case 0xAE: xor8(_bus.read(getHL())); return 8;
        case 0xAF: xor8(getA()); return 4;
        case 0xB0: or8(getB()); return 4;
        case 0xB1: or8(getC()); return 4;
        case 0xB2: or8(getD()); return 4;
        case 0xB3: or8(getE()); return 4;
        case 0xB4: or8(getH()); return 4;
        case 0xB5: or8(getL()); return 4;
        case 0xB6: or8(_bus.read(getHL())); return 8;
        case 0xB7: or8(getA()); return 4;
        case 0xB8: cp8(getB()); return 4;
        case 0xB9: cp8(getC()); return 4;
        case 0xBA: cp8(getD()); return 4;
        case 0xBB: cp8(getE()); return 4;
        case 0xBC: cp8(getH()); return 4;
        case 0xBD: cp8(getL()); return 4;
        case 0xBE: cp8(_bus.read(getHL())); return 8;
        case 0xBF: cp8(getA()); return 4;
        case 0xC0: {  // RET NZ
            if (!getFlagZ()) {
                setPC(pop());
                return 20;   // taken
            }
            return 8;        // not taken
        }
        case 0xC1: setBC(pop()); return 12;
        case 0xC2: {  // JP NZ, a16
            uint16_t addr = fetchWord();      // ALWAYS fetch — PC must advance past operand
            if (!getFlagZ()) {                // NZ = Z clear
                setPC(addr);
                return 16;                    // branch taken
            }
            return 12;                        // branch not taken
        }
        case 0xC3: setPC(fetchWord()); return 16;
        case 0xC4: {  // CALL NZ, a16
            uint16_t addr = fetchWord();    // ALWAYS fetch
            if (!getFlagZ()) {
                push(getPC());
                setPC(addr);
                return 24;   // taken
            }
            return 12;        // not taken
        }
        case 0xC5: push(getBC()); return 16;
        case 0xC6: add8(fetchByte()); return 8;
        case 0xC7: push(getPC()); setPC(0x0000); return 16;
        case 0xC8: {  // RET Z
            if (getFlagZ()) {
                setPC(pop());
                return 20;   // taken
            }
            return 8;        // not taken
        }
        case 0xC9: setPC(pop()); return 16;
        case 0xCA: {  // JP NZ, a16
            uint16_t addr = fetchWord();      // ALWAYS fetch — PC must advance past operand
            if (getFlagZ()) {                 // Z = Z set
                setPC(addr);
                return 16;                    // branch taken
            }
            return 12;                        // branch not taken
        }
        case 0xCB: return executeCB(fetchByte());
        case 0xCC: {  // CALL Z, a16
            uint16_t addr = fetchWord();    // ALWAYS fetch
            if (getFlagZ()) {
                push(getPC());
                setPC(addr);
                return 24;   // taken
            }
            return 12;        // not taken
        }
        case 0xCD: {
            uint16_t addr = fetchWord();    // fetch target — PC now points at next instruction
            push(getPC());                  // push return address (the next-instruction PC)
            setPC(addr);                    // jump to target
            return 24;
        }
        case 0xCE: adc8(fetchByte()); return 8;
        case 0xCF: push(getPC()); setPC(0x0008); return 16;
        case 0xD0: {  // RET NC
            if (!getFlagC()) {
                setPC(pop());
                return 20;   // taken
            }
            return 8;        // not taken
        }
        case 0xD1: setDE(pop()); return 12;
        case 0xD2: {  // JP NZ, a16
            uint16_t addr = fetchWord();      // ALWAYS fetch — PC must advance past operand
            if (!getFlagC()) {                 // NC = C clear
                setPC(addr);
                return 16;                    // branch taken
            }
            return 12;                        // branch not taken
        }
        case 0xD4: {  // CALL NC, a16
            uint16_t addr = fetchWord();    // ALWAYS fetch
            if (!getFlagC()) {
                push(getPC());
                setPC(addr);
                return 24;   // taken
            }
            return 12;        // not taken
        }
        case 0xD5: push(getDE()); return 16;
        case 0xD6: sub8(fetchByte()); return 8;
        case 0xD7: push(getPC()); setPC(0x0010); return 16;
        case 0xD8: {  // RET C
            if (getFlagC()) {
                setPC(pop());
                return 20;   // taken
            }
            return 8;        // not taken
        }
        case 0xD9: _ime = true; setPC(pop()); return 16;
        case 0xDA: {  // JP NZ, a16
            uint16_t addr = fetchWord();      // ALWAYS fetch — PC must advance past operand
            if (getFlagC()) {                 // C = C set
                setPC(addr);
                return 16;                    // branch taken
            }
            return 12;                        // branch not taken
        }
        case 0xDC: {  // CALL C, a16
            uint16_t addr = fetchWord();    // ALWAYS fetch
            if (getFlagC()) {
                push(getPC());
                setPC(addr);
                return 24;   // taken
            }
            return 12;        // not taken
        }
        case 0xDE: sbc8(fetchByte()); return 8;
        case 0xDF: push(getPC()); setPC(0x0018); return 16;
        case 0xE0: {
            uint8_t offset = fetchByte();
            _bus.write(0xFF00 + offset, getA());
            return 12;
        }
        case 0xE1: setHL(pop()); return 12;
        case 0xE2: _bus.write(0xFF00 + getC(), getA()); return 8;
        case 0xE5: push(getHL()); return 16;
        case 0xE6: and8(fetchByte()); return 8;
        case 0xE7: push(getPC()); setPC(0x0020); return 16;
        case 0xE8: setSP(addSPr8()); return 16;
        case 0xE9: setPC(getHL()); return 4;
        case 0xEA: {
            uint16_t addr = fetchWord();
            _bus.write(addr, getA());
            return 16;
        }
        case 0xEE: xor8(fetchByte()); return 8;
        case 0xEF: push(getPC()); setPC(0x0028); return 16;
        case 0xF0: {
            uint8_t offset = fetchByte();
            setA(_bus.read(0xFF00 + offset));
            return 12;
        }
        case 0xF1: setAF(pop()); return 12;
        case 0xF2: setA(_bus.read(0xFF00 + getC())); return 8;
        case 0xF3: _ime = false; return 4;
        case 0xF5: push(getAF()); return 16;
        case 0xF6: or8(fetchByte()); return 8;
        case 0xF7: push(getPC()); setPC(0x0030); return 16;
        case 0xF8: setHL(addSPr8()); return 12; 
        case 0xF9: setSP(getHL()); return 8;
        case 0xFA: {
            uint16_t addr = fetchWord();
            setA(_bus.read(addr));
            return 16;
        }
        case 0xFB: _imeScheduled = true; return 4;
        case 0xFE: cp8(fetchByte()); return 8;
        case 0xFF: push(getPC()); setPC(0x0038); return 16;
        
        default: 
            std::cerr << "Unimplemented opcode 0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(opcode)
            << " at PC 0x" << static_cast<int>(_pc - 1)
            << std::endl;
            throw std::runtime_error("Unimplemented opcode");
    }
}

uint8_t CPU::getReg(uint8_t index) {
    switch (index) {
        case 0: return getB();
        case 1: return getC();
        case 2: return getD();
        case 3: return getE();
        case 4: return getH();
        case 5: return getL();
        case 6: return _bus.read(getHL());   // (HL) — memory, not a register
        case 7: return getA();
        default: 
            std::cerr << "Unreachable register" << std::endl;
            throw std::runtime_error("Unreachable register");
    }
}

void CPU::setReg(uint8_t index, uint8_t value) {
    switch (index) {
        case 0: setB(value); break;
        case 1: setC(value); break;
        case 2: setD(value); break;
        case 3: setE(value); break;
        case 4: setH(value); break;
        case 5: setL(value); break;
        case 6: _bus.write(getHL(), value); break;   // write to (HL)
        case 7: setA(value); break;
        default:
            std::cerr << "Unreachable register" << std::endl;
            throw std::runtime_error("Unreachable register");
    }
}

int CPU::executeCB(uint8_t opcode) {
    uint8_t reg   = opcode & 0x07;        // target register index
    uint8_t value = getReg(reg);          // read target once
    bool isHL     = (reg == 6);           // (HL) target costs more cycles

    if (opcode < 0x40) {
        // rotate/shift ops — operation selector is bits 5-3
        uint8_t result;
        switch ((opcode >> 3) & 0x07) {
            case 0: result = rlc(value);  break;
            case 1: result = rrc(value);  break;
            case 2: result = rl(value);   break;
            case 3: result = rr(value);   break;
            case 4: result = sla(value);  break;
            case 5: result = sra(value);  break;
            case 6: result = swap(value); break;
            case 7: result = srl(value);  break;
            default:
                throw std::runtime_error("unreachable CB rotate/shift op");
        }
        setReg(reg, result);
        return isHL ? 16 : 8;
    }
    else if (opcode < 0x80) {
        // BIT b, r
        uint8_t bit = (opcode >> 3) & 0x07;
        // ... test bit `bit` of `value`, set flags, NO writeback ...
        setFlagZ((value & (1 << bit)) == 0);
        setFlagN(false);
        setFlagH(true);
        //dont touch flag C
        return isHL ? 12 : 8;
    }
    else if (opcode < 0xC0) {
        // RES b, r — clear bit
        uint8_t bit = (opcode >> 3) & 0x07;
        setReg(reg, value & ~(1 << bit));
        return isHL ? 16 : 8;
    }
    else {
        // SET b, r — set bit
        uint8_t bit = (opcode >> 3) & 0x07;
        setReg(reg, value | (1 << bit));
        return isHL ? 16 : 8;
    }
}

uint8_t CPU::rlc(uint8_t value) {
    uint8_t bit7 = (value >> 7) & 1;        // bit 7 of the INPUT
    uint8_t result = (value << 1) | bit7;   // rotate left, wrap bit7 to bit0
    setFlagZ(result == 0);                  // CB version: Z from result
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit7);                         // C = old bit 7
    return result;                          // return the rotated BYTE
}

uint8_t CPU::rrc(uint8_t value) {
    uint8_t bit0 = value & 1;       
    uint8_t result = (value >> 1) | (bit0 << 7);  
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit0);                         
    return result;                          
}

uint8_t CPU::rl(uint8_t value) {
    uint8_t oldCarry = getFlagC();
    uint8_t bit7 = (value >> 7) & 1;
    uint8_t result = (value << 1) | oldCarry;
    setFlagZ(result == 0);                  // CB version: Z from result
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit7);                         // C = old bit 7
    return result; 
}

uint8_t CPU::rr(uint8_t value) {
    uint8_t oldCarry = getFlagC();
    uint8_t bit0 = value & 1;
    uint8_t result = (value >> 1) | (oldCarry << 7);
    setFlagZ(result == 0);                  // CB version: Z from result
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit0); 
    return result;
}

uint8_t CPU::sla(uint8_t value) {
    uint8_t bit7 = (value >> 7) & 1;
    uint8_t result = value << 1;        // bit 0 becomes 0
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit7);
    return result;
    // Z from result, N=0, H=0, C=bit7
}

uint8_t CPU::sra(uint8_t value) {
    uint8_t bit0 = value & 1;
    uint8_t result = (value >> 1) | (value & 0x80);   // top bit kept
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit0);
    return result;
    // Z from result, N=0, H=0, C=bit0
}

uint8_t CPU::swap(uint8_t value) {
    uint8_t result = (value << 4) | (value >> 4);   // high<->low nibble
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(false);
    return result;
    // Z from result, N=0, H=0, C=0   (swap CLEARS carry)
}

uint8_t CPU::srl(uint8_t value) {
    uint8_t bit0 = value & 1;
    uint8_t result = value >> 1;        // bit 7 becomes 0
    setFlagZ(result == 0);
    setFlagN(false);
    setFlagH(false);
    setFlagC(bit0);
    return result;
    // Z from result, N=0, H=0, C=bit0
}

void CPU::logState() {
    char buf[128];
    std::snprintf(buf, sizeof(buf),
        "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
        getA(), getF(), getB(), getC(), getD(), getE(), getH(), getL(),
        getSP(), getPC(),
        _bus.read(getPC()),
        _bus.read(getPC() + 1),
        _bus.read(getPC() + 2),
        _bus.read(getPC() + 3));
    std::cout << buf;
}

bool CPU::handleInterrupts() {
    if (!_ime) {
        return false;
    }
    uint8_t pending = _bus.read(0xFF0F) & _bus.read(0xFFFF);
    if (pending == 0) {
        return false;
    }
    for (int i = 0; i < 5; i++) {              // 5 interrupts, bits 0-4
        if (pending & (1 << i)) {              // is bit i set? (mask, not i itself)
            _ime = false;                      // disable further interrupts
            uint8_t currentIF = _bus.read(0xFF0F);
            _bus.write(0xFF0F, currentIF & ~(1 << i));   // clear this interrupt's request bit
            push(getPC());                     // save return address
            setPC(static_cast<uint16_t>(0x0040 + i * 8));             // jump to vector
            return true;                       // serviced one — stop (priority: first match wins)
        }
    }
    return false;   // unreachable (pending != 0 guarantees a bit is found), but satisfies return type
}

bool CPU::interruptPending() {
    return (_bus.read(0xFF0F) & _bus.read(0xFFFF) & 0x1F) != 0;
}

int CPU::step() {
    if (interruptPending()) {
        _halted = false;   // HALT wakes on any pending interrupt, regardless of IME
    }

    if (_halted) {
        return 4;   // still asleep — nothing changed, don't log
    }

    if (_loggingEnabled) logState();

    int interruptCycles = 0;
    if (handleInterrupts()) {
        interruptCycles = 20;
    }

    bool willEnableIME = _imeScheduled;
    _imeScheduled = false;

    uint8_t opcode = fetchByte();
    int cycles = execute(opcode);

    if (willEnableIME) _ime = true;
    return interruptCycles + cycles;
}


void CPU::enableLogging() {
    _loggingEnabled = true;
}