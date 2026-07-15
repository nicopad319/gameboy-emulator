#include "Cartridge.h"
#include <fstream>
#include <iostream>

bool Cartridge::load(const std::string& path) {
    // open binary stream, size to the file, read bytes into _rom, then parse header for cartridge type and title


    // 0. clear any existing data in _rom and reset cartridge type and title (so function is cleanly recallable)
    _rom.clear();
    _cartridgeType = 0;
    _title.clear();
    //_ram.clear(); uncomment once implemented

    // 1. open the file in binary mode
    std::ifstream file(path, std::ios::binary); //std::ios::binary to read the file as binary data
    if (!file.is_open()) {
        return false; // failed to open file
    }

    // 2. determine the size of the file and resize _rom accordingly
    file.seekg(0, std::ios::end); // move to the end of the file
    std::streamsize size = file.tellg(); // get the current position (which is the size of the file)
    file.seekg(0, std::ios::beg); // move back to the beginning

    if (size < 0x0148) { // ROM needs to be at least 0x0148 bytes long to contain the cartridge type and title (0x0000 - 0x0147)
        return false; // invalid ROM size
    }

    _rom.resize(size); // resize the vector to hold the file data


    // 3. read the file data into the vector
    file.read(reinterpret_cast<char*>(_rom.data()), size); // read the file data into the vector]
    if (file.gcount() != size) { // check if the number of bytes read matches the expected size
        return false; // failed to read the entire file
    }
    file.close();

    // 4. parse the cartridge header to determine the cartridge type and title
    
    _cartridgeType = _rom[0x0147]; // cartridge type is at 0x0147
    for (int i = 0; i < 16; i++) { // title is at 0x0134–0x0143 (16 byte range)
        uint8_t byte = _rom[0x0134 + i];
        if (byte == 0x00) { // null terminator (0x00) indicates end of title
            break;
        }
        _title += static_cast<char>(byte); // append each byte to the title string
    }

    // 4.5 verify that the cartridge type is supported (for now, only support 0x00)
    if (_cartridgeType != 0x00) { // unsupported cartridge type for now
        std::cerr << "Unsupported cartridge type: 0x" << std::hex << static_cast<int>(_cartridgeType) << std::endl;
        return false;
    }

    return true; // successfully loaded and parsed the cartridge
}


uint8_t Cartridge::read(uint16_t address) {
    if (address < 0x8000) { // ROM area (0x0000 - 0x7FFF)
        if (address < _rom.size()) { // check if the address is within the bounds of the loaded ROM
            return _rom.at(address); // read from the ROM vector
        } else {
            return 0xFF; // return 0xFF for out-of-bounds reads (real hardware behavior)
        }
    } 

    else { //RAM area (0xA000 - 0xBFFF)
        if (_ram.empty()) {
            return 0xFF; // real hardware behavior is to return 0xFF if RAM is not present
        }
        //TODO: implement RAM banking and MBC support for cartridges that have RAM
        return 0xFF; // for now, return 0xFF for any RAM reads (since we don't have RAM implemented yet)
    }
}

void Cartridge::write(uint16_t address, uint8_t value) {
    if (address < 0x8000) { // ROM area (0x0000 - 0x7FFF)
        // writing to ROM is not allowed, ignore the write
        return;
    } 
    else { //RAM area (0xA000 - 0xBFFF)
        if (!_ram.empty()) {
            _ram.at(address - 0xA000) = value; // write to the RAM vector (if present)
        }
    }
}