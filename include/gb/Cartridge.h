#pragma once
#include <cstdint>
#include <vector>
#include <string>

class Cartridge {
public:
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);
    bool load(const std::string& path); // Load the cartridge data from a file

private:
    std::vector<uint8_t> _rom; // Vector to hold the ROM data loaded from the cartridge file
    std::vector<uint8_t> _ram; // Vector to hold the RAM data (if any) for the cartridge
    uint8_t _cartridgeType = 0; // Type of cartridge (e.g., ROM only, MBC1, MBC2, etc.) initialized to 0 (ROM only) by default
    std::string _title; // Title of game (extracted from bytes 0x0134–0x0143 (ASCII) in the cartridge header)

    enum class Mapper { None, MBC1, MBC3 };
    uint32_t currentRomBank() const;
    uint32_t currentRamBank() const;

    Mapper  _mapper      = Mapper::None;
    bool    _ramEnabled  = false;
    uint8_t _romBankReg  = 1;   // raw value written to 0x2000-0x3FFF
    uint8_t _ramBankReg  = 0;   // raw value written to 0x4000-0x5FFF
    uint8_t _mode        = 0;   // MBC1 banking mode (0x6000-0x7FFF)
};