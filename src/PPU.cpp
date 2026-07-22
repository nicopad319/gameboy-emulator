#include "PPU.h"
#include "VRAM.h"

uint8_t PPU::getCurrentMode() {
    return static_cast<uint8_t>(_mode);
}

uint8_t PPU::getLY() {
    return static_cast<uint8_t>(_ly);
}

uint8_t PPU::tick(int cycles) {
    uint8_t result = 0;
    _dotCounter += cycles;
    switch (_mode) {
        case Mode::OamScan: 
        if (_dotCounter >= 80) {
            _mode = Mode::Drawing;
        } 
        break;

        case Mode::Drawing:
        if (_dotCounter >= 252) {
            _mode = Mode::HBlank;
            if ((_lcdc & 0x80) && _vram) {   // only when LCD is on and memory is connected
                renderScanline();
            }
        }
        break;

        case Mode::HBlank: 
        if (_dotCounter >= 456) {
            _dotCounter -= 456;
            _ly++;
            if (_ly < 144) {
                _mode = Mode::OamScan;
            }
            else if (_ly == 144) {
                _mode = Mode::VBlank;
                result |= 0x01;
            }
        }
        break;

        case Mode::VBlank: 
        if (_dotCounter >= 456) {
            _dotCounter -=456;
            _ly++;
            if (_ly == 154) {
                _ly = 0;
                _mode = Mode::OamScan;
            }
        }
        break;

        default: break;
    }
    return result;
}

void PPU::decodeTileRow(uint8_t lowPlane, uint8_t highPlane, uint8_t outPixels[8]) {
    for (int x = 0; x < 8; ++x) {
        int bitpos = 7 - x;                       // leftmost pixel is bit 7
        uint8_t lo = (lowPlane  >> bitpos) & 1;
        uint8_t hi = (highPlane >> bitpos) & 1;
        outPixels[x] = static_cast<uint8_t>((hi << 1) | lo);
    }
}

uint8_t PPU::readRegister(uint16_t addr) const {
    switch (addr) {
        case 0xFF40: return _lcdc;
        case 0xFF41: return _stat;                       // TODO: compose mode + coincidence bits later
        case 0xFF42: return _scy;
        case 0xFF43: return _scx;
        case 0xFF44: return static_cast<uint8_t>(_ly);   // LY is produced by the PPU
        case 0xFF45: return _lyc;
        case 0xFF47: return _bgp;
        default:     return 0xFF;                        // other PPU regs not wired yet
    }
}

void PPU::writeRegister(uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0xFF40: _lcdc = value; break;
        case 0xFF41: _stat = value; break;   // TODO: bits 0-2 are read-only on hardware
        case 0xFF42: _scy  = value; break;
        case 0xFF43: _scx  = value; break;
        case 0xFF44: /* LY read-only — writes ignored */ break;
        case 0xFF45: _lyc  = value; break;
        case 0xFF47: _bgp  = value; break;
        default: break;
    }
}

void PPU::renderScanline() {
    // LCDC selects which tilemap and which tile-data addressing mode to use.
    uint16_t tileMapBase = (_lcdc & 0x08) ? 0x9C00 : 0x9800;   // bit 3
    bool     unsignedTiles = (_lcdc & 0x10) != 0;              // bit 4

    // Map this screen line to a background-space Y, wrapping at 256 (uint8_t does it for free).
    uint8_t bgY      = static_cast<uint8_t>(_ly + _scy);
    uint8_t tileRow  = bgY / 8;
    uint8_t pixelRow = bgY % 8;

    for (int x = 0; x < 160; ++x) {
        uint8_t bgX      = static_cast<uint8_t>(x + _scx);   // wraps at 256
        uint8_t tileCol  = bgX / 8;
        uint8_t pixelCol = bgX % 8;

        // 1. Look up which tile sits at this map cell.
        uint16_t mapAddr   = static_cast<uint16_t>(tileMapBase + tileRow * 32 + tileCol);
        uint8_t  tileIndex = _vram->readRaw(mapAddr);

        // 2. Turn that index into a tile-data address (the two addressing modes).
        uint16_t tileAddr;
        if (unsignedTiles) {
            tileAddr = static_cast<uint16_t>(0x8000 + tileIndex * 16);
        } else {
            tileAddr = static_cast<uint16_t>(0x9000 + static_cast<int8_t>(tileIndex) * 16);
        }

        // 3. Fetch and decode the one row of that tile we need.
        uint8_t low  = _vram->readRaw(static_cast<uint16_t>(tileAddr + pixelRow * 2));
        uint8_t high = _vram->readRaw(static_cast<uint16_t>(tileAddr + pixelRow * 2 + 1));
        uint8_t rowPixels[8];
        decodeTileRow(low, high, rowPixels);
        uint8_t colorIndex = rowPixels[pixelCol];

        // 4. Map the 2-bit index through the background palette.
        uint8_t shade = (_bgp >> (colorIndex * 2)) & 0x03;

        // 5. Store into the framebuffer.
        _framebuffer[static_cast<size_t>(_ly) * 160 + x] = shade;
    }
}

bool PPU::isVramLocked() const {
    return (_lcdc & 0x80) && _mode == Mode::Drawing;   // LCD on AND actively drawing
}
