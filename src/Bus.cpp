#include "Bus.h"
#include "Cartridge.h"
#include "WRAM.h"
#include "IORegisters.h"
#include "VRAM.h"
#include "HRAM.h"
#include "IERegister.h"
#include "OAM.h"


Bus::Bus(Cartridge* cartridge, 
           WRAM* wram,
           IORegisters* ioRegisters,
           VRAM* vram,
           HRAM* hram,
           IERegister* ieRegister,
           OAM* oam)
            : _cartridge(cartridge),
            _wram(wram),
            _ioRegisters(ioRegisters),
            _vram(vram),
            _hram(hram),
            _ieRegister(ieRegister),
            _oam(oam),
            _cycles(0) // Initialize cycles to 0
{
    // Constructor body can be empty since we are using an initializer list
}


uint8_t Bus::read(uint16_t address) {
    if (address < 0x8000) {
        return _cartridge->read(address);
    } else if (address < 0xA000) {
        return _vram->read(address);
    } else if (address < 0xC000) {
        return _cartridge->read(address);
    } else if (address < 0xE000) {
        return _wram->read(address);
    } else if (address < 0xFE00) { //echo RAM
        return _wram->read(address - 0x2000); //remap to WRAM
    } else if (address < 0xFEA0) {
        return _oam->read(address);
    } else if (address < 0xFF00) { //unusable memory
        return 0xFF;
    } else if (address < 0xFF80) {
        return _ioRegisters->read(address);
    } else if (address < 0xFFFF) {
        return _hram->read(address);
    } else {
        return _ieRegister->read();
    }
}

void Bus::write(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        _cartridge->write(address, value);
    } else if (address < 0xA000) {
        _vram->write(address, value);
    } else if (address < 0xC000) {
        _cartridge->write(address, value);
    } else if (address < 0xE000) {
        _wram->write(address, value);
    } else if (address < 0xFE00) { //echo RAM
        _wram->write(address - 0x2000, value); //remap to WRAM
    } else if (address < 0xFEA0) {
        _oam->write(address, value);
    } else if (address < 0xFF00) { //unusable memory
        return;
    } else if (address < 0xFF80) {
        _ioRegisters->write(address, value);
    } else if (address < 0xFFFF) {
        _hram->write(address, value);
    } else {
        _ieRegister->write(value);
    }
}

uint16_t Bus::read16(uint16_t address) {
    uint8_t lo = read(address);
    uint8_t hi = read(address + 1);
    return (hi << 8) | lo; //combine the two bytes into a 16-bit value
}

void Bus::write16(uint16_t address, uint16_t value) {
    uint8_t lo = value & 0xFF; //extract the low byte (mask with 0xFF)
    uint8_t hi = (value >> 8) & 0xFF; //extract the high byte (shift right by 8 bits and mask with 0xFF)
    write(address, lo);
    write(address + 1, hi);
}