#include "Cartridge.h"
#include <fstream>
#include <iostream>

bool Cartridge::load(const std::string& path) {
    // open binary stream, size to the file, read bytes into _rom, then parse header for cartridge type and title


    // 0. clear any existing data in _rom and reset cartridge type and title (so function is cleanly recallable)
    _rom.clear();
    _cartridgeType = 0;
    _title.clear();
    _rom.clear();
    _ram.clear();
    _cartridgeType = 0;
    _title.clear();
    _mapper = Mapper::None;
    _ramEnabled = false;
    _romBankReg = 1;
    _ramBankReg = 0;
    _mode = 0;

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
    switch (_cartridgeType) {
        case 0x00:
            _mapper = Mapper::None;
            break;
        case 0x01: case 0x02: case 0x03:
            _mapper = Mapper::MBC1;
            break;
        case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
            _mapper = Mapper::MBC3;
            break;
        default:
            std::cerr << "Unsupported cartridge type: 0x" << std::hex
                      << static_cast<int>(_cartridgeType) << std::endl;
            return false;
    }

    // allocate external RAM based on the RAM-size byte at 0x0149
    uint32_t ramSize = 0;
    switch (_rom[0x0149]) {
        case 0x02: ramSize = 8   * 1024; break;   // 1 bank
        case 0x03: ramSize = 32  * 1024; break;   // 4 banks
        case 0x04: ramSize = 128 * 1024; break;   // 16 banks
        case 0x05: ramSize = 64  * 1024; break;   // 8 banks
        default:   ramSize = 0;          break;
    }
    if (ramSize > 0) {
        _ram.assign(ramSize, 0);
    }

    return true; // successfully loaded and parsed the cartridge
}


uint8_t Cartridge::read(uint16_t address) {
    if (address < 0x4000) {                                   // fixed ROM bank 0
        return address < _rom.size() ? _rom[address] : 0xFF;
    }
    if (address < 0x8000) {                                   // switchable ROM bank
        uint32_t offset = currentRomBank() * 0x4000u + (address - 0x4000u);
        return offset < _rom.size() ? _rom[offset] : 0xFF;
    }
    // external RAM (0xA000-0xBFFF)
    if (_ramEnabled && !_ram.empty()) {
        uint32_t offset = currentRamBank() * 0x2000u + (address - 0xA000u);
        return offset < _ram.size() ? _ram[offset] : 0xFF;
    }
    return 0xFF;
}

void Cartridge::write(uint16_t address, uint8_t value) {
    // external RAM write (all mappers)
    if (address >= 0xA000 && address < 0xC000) {
        if (_ramEnabled && !_ram.empty()) {
            uint32_t offset = currentRamBank() * 0x2000u + (address - 0xA000u);
            if (offset < _ram.size()) _ram[offset] = value;
        }
        return;
    }

    if (_mapper == Mapper::None) return;   // ROM-only: ROM writes ignored

    // MBC control-register writes (ROM address range)
    if (address < 0x2000) {
        _ramEnabled = ((value & 0x0F) == 0x0A);
    } else if (address < 0x4000) {
        _romBankReg = value;               // helpers mask per-mapper
    } else if (address < 0x6000) {
        _ramBankReg = value;
    } else if (address < 0x8000) {
        _mode = value & 0x01;              // MBC1 only; MBC3 RTC-latch (ignored)
    }
}

uint32_t Cartridge::currentRomBank() const {
    switch (_mapper) {
        case Mapper::MBC3: {
            uint32_t b = _romBankReg & 0x7F;   // 7-bit bank
            return b == 0 ? 1 : b;             // bank 0 -> 1
        }
        case Mapper::MBC1: {
            uint32_t low = _romBankReg & 0x1F; // 5-bit
            if (low == 0) low = 1;             // 0 -> 1 quirk
            return ((_ramBankReg & 0x03) << 5) | low;
        }
        default:
            return 1;                          // ROM-only: 0x4000-0x7FFF is bank 1
    }
}

uint32_t Cartridge::currentRamBank() const {
    switch (_mapper) {
        case Mapper::MBC3: return _ramBankReg & 0x03;
        case Mapper::MBC1: return (_mode == 1) ? (_ramBankReg & 0x03) : 0;
        default:           return 0;
    }
}