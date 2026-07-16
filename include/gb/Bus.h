#include <cstdint>
// Forward declarations of classes used in the Bus class (they don't exist yet)
class Cartridge;
class WRAM;
class IORegisters;
class VRAM;
class HRAM;
class IERegister;
class OAM;

class Bus {
public:
        Bus(Cartridge* cartridge, 
            WRAM* wram,
            IORegisters* ioRegisters,
            VRAM* vram,
            HRAM* hram,
            IERegister* ieRegister,
            OAM* oam);

    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

    uint16_t read16(uint16_t address); // built from two uint8_t read() calls internally
    void write16(uint16_t address, uint16_t value); // built from two uint8_t write() calls internally


private:
    Cartridge* _cartridge;
    WRAM* _wram;
    IORegisters* _ioRegisters;
    VRAM* _vram;
    HRAM* _hram;
    IERegister* _ieRegister;
    OAM* _oam;
    uint64_t _cycles; //uint64_t to avoid overflow when counting cycles, potentially billions of cycles in a long-running emulation
};