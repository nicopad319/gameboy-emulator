#pragma once
#include <cstdint>


class Bus;

class CPU {
public:
    CPU(Bus& bus);

    void reset();
    // Getter methods for 8-bit registers
    uint8_t getA() const;
    uint8_t getF() const;
    uint8_t getB() const;
    uint8_t getC() const;
    uint8_t getD() const;
    uint8_t getE() const;
    uint8_t getH() const;
    uint8_t getL() const;
    // Getter methods for 16-bit registers
    uint16_t getAF() const;
    uint16_t getBC() const;
    uint16_t getDE() const;
    uint16_t getHL() const;
    uint16_t getSP() const;
    uint16_t getPC() const;

    // Setter methods for 8-bit registers
    void setA(uint8_t value);
    void setF(uint8_t value);
    void setB(uint8_t value);
    void setC(uint8_t value);
    void setD(uint8_t value);
    void setE(uint8_t value);
    void setH(uint8_t value);
    void setL(uint8_t value);

    // Setter methods for 16-bit registers
    void setAF(uint16_t value);
    void setBC(uint16_t value);
    void setDE(uint16_t value);
    void setHL(uint16_t value);
    void setSP(uint16_t value);
    void setPC(uint16_t value);

    // Flag manipulation methods
    bool getFlagZ() const;
    bool getFlagN() const;
    bool getFlagH() const;
    bool getFlagC() const;

    void setFlagZ(bool value);
    void setFlagN(bool value);
    void setFlagH(bool value);
    void setFlagC(bool value);

    int step();
    //moved from private to public for testing purposes
    int execute(uint8_t opcode);
    void push(uint16_t value);
    uint16_t pop(); 
    //arithmetic operations
    void add8(uint8_t value);

    uint8_t subFlags(uint8_t value); //actual sub function
    void sub8(uint8_t value); //wrapper that stores result
    void cp8(uint8_t value); //identical to sub but doesn't store result, so wrapper that doesn't store result

    void and8(uint8_t value);
    void or8(uint8_t value);
    void xor8(uint8_t value);

    uint8_t inc8(uint8_t value);
    uint8_t dec8(uint8_t value);

    void adc8(uint8_t value);
    void sbc8(uint8_t value);

    void add16(uint16_t value);

    uint16_t addSPr8();

private:
    uint8_t _a; // Accumulator register
    uint8_t _f; // Flags register

    uint8_t _b; // General-purpose register B
    uint8_t _c; // General-purpose register C

    uint8_t _d; // General-purpose register D
    uint8_t _e; // General-purpose register E

    uint8_t _h; // High byte of the HL register pair
    uint8_t _l; // Low byte of the HL register pair

    //dedicated 16-bit registers
    uint16_t _sp; // Stack Pointer
    uint16_t _pc; // Program Counter

    static constexpr uint8_t FLAG_Z = 0x80;
    static constexpr uint8_t FLAG_N = 0x40;
    static constexpr uint8_t FLAG_H = 0x20;
    static constexpr uint8_t FLAG_C = 0x10;

    Bus& _bus;

    //method needed for opcode logic
    uint8_t fetchByte(); 
    uint16_t fetchWord();

    bool _ime;
};