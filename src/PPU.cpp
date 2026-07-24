#include "PPU.h"
#include "VRAM.h"
#include "OAM.h"

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
                _windowLineCounter = 0; // window counter restarts each frame
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
        case 0xFF48: return _obp0;
        case 0xFF49: return _obp1;
        case 0xFF4A: return _wy;
        case 0xFF4B: return _wx;
        default:     return 0xFF;                  
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
        case 0xFF48: _obp0 = value; break;
        case 0xFF49: _obp1 = value; break;
        case 0xFF4A: _wy = value; break;
        case 0xFF4B: _wx = value; break;
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
    renderWindow(); //window sits on top of the background, but sprites sit on top of the window:
    renderSprites();
}

bool PPU::isVramLocked() const {
    // Deliberately disabled: our timing is instruction-granular with a fixed mode-3
    // length, so strict mode-3 locking drops legitimate writes. No target game relies
    // on VRAM being inaccessible during rendering, so we keep VRAM always accessible.
    return false;
}

bool PPU::isOamLocked() const {
    // Deliberately disabled: our timing is instruction-granular with a fixed mode-3
    // length, so strict mode-3 locking drops legitimate writes. No target game relies
    // on VRAM being inaccessible during rendering, so we keep VRAM always accessible.
    return false;
}

void PPU::renderSprites() {
    if (!(_lcdc & 0x02) || !_oam) return;         // OBJ disabled or no OAM connected

    int height = (_lcdc & 0x04) ? 16 : 8;         // LCDC bit 2: 8x16 mode

    // Draw high OAM indices first so index 0 ends up on top (simplified priority).
    for (int i = 39; i >= 0; --i) {
        uint16_t base = static_cast<uint16_t>(0xFE00 + i * 4);
        int spriteY   = _oam->readRaw(static_cast<uint16_t>(base + 0)) - 16;
        int spriteX   = _oam->readRaw(static_cast<uint16_t>(base + 1)) - 8;
        uint8_t tileIndex = _oam->readRaw(static_cast<uint16_t>(base + 2));
        uint8_t attrs     = _oam->readRaw(static_cast<uint16_t>(base + 3));

        if (_ly < spriteY || _ly >= spriteY + height) continue;   // not on this line

        bool yFlip = attrs & 0x40;
        bool xFlip = attrs & 0x20;
        uint8_t palette = (attrs & 0x10) ? _obp1 : _obp0;

        int row = _ly - spriteY;
        if (yFlip) row = height - 1 - row;

        uint8_t tile = tileIndex;
        if (height == 16) {                        // 8x16: bit 0 ignored, row picks top/bottom
            tile = tileIndex & 0xFE;
            if (row >= 8) { tile += 1; row -= 8; }
        }

        uint16_t addr = 0x8000 + tile * 16;        // sprites always use 0x8000 unsigned
        uint8_t low  = _vram->readRaw(static_cast<uint16_t>(addr + row * 2));
        uint8_t high = _vram->readRaw(static_cast<uint16_t>(addr + row * 2 + 1));

        for (int px = 0; px < 8; ++px) {
            int bit = xFlip ? px : (7 - px);
            uint8_t colorIndex = static_cast<uint8_t>((((high >> bit) & 1) << 1) | ((low >> bit) & 1));
            if (colorIndex == 0) continue;         // 0 = transparent for sprites

            int screenX = spriteX + px;
            if (screenX < 0 || screenX >= 160) continue;

            uint8_t shade = (palette >> (colorIndex * 2)) & 0x03;
            _framebuffer[_ly * 160 + screenX] = shade;
        }
    }
}

void PPU::renderWindow() {
    if (!(_lcdc & 0x20)) return;          // window disabled
    if (!(_lcdc & 0x01)) return;          // on DMG, window only shows if BG is on
    if (!_vram || _ly < _wy) return;      // window hasn't reached this line yet

    uint16_t winMapBase    = (_lcdc & 0x40) ? 0x9C00 : 0x9800;   // bit 6
    bool     unsignedTiles = (_lcdc & 0x10) != 0;                // bit 4 (shared with BG)

    uint8_t tileRow  = static_cast<uint8_t>(_windowLineCounter / 8);
    uint8_t pixelRow = static_cast<uint8_t>(_windowLineCounter % 8);

    for (int x = 0; x < 160; ++x) {
        int winX = x - (static_cast<int>(_wx) - 7);   // window's left edge is at WX-7
        if (winX < 0) continue;                       // left of the window: keep background

        uint8_t tileCol  = static_cast<uint8_t>(winX / 8);
        uint8_t pixelCol = static_cast<uint8_t>(winX % 8);

        uint16_t mapAddr   = static_cast<uint16_t>(winMapBase + tileRow * 32 + tileCol);
        uint8_t  tileIndex = _vram->readRaw(mapAddr);

        uint16_t tileAddr;
        if (unsignedTiles) tileAddr = static_cast<uint16_t>(0x8000 + tileIndex * 16);
        else               tileAddr = static_cast<uint16_t>(0x9000 + static_cast<int8_t>(tileIndex) * 16);

        uint8_t low  = _vram->readRaw(static_cast<uint16_t>(tileAddr + pixelRow * 2));
        uint8_t high = _vram->readRaw(static_cast<uint16_t>(tileAddr + pixelRow * 2 + 1));
        uint8_t rowPixels[8];
        decodeTileRow(low, high, rowPixels);
        uint8_t colorIndex = rowPixels[pixelCol];

        uint8_t shade = (_bgp >> (colorIndex * 2)) & 0x03;
        _framebuffer[static_cast<size_t>(_ly) * 160 + x] = shade;
    }

    _windowLineCounter++;   // advance only on lines where the window drew
}