#pragma once
#include <cstdint>
#include <array>


class VRAM;

class PPU {
public:
    uint8_t getCurrentMode();
    uint8_t tick(int cycles);
    uint8_t getLY();

    // outPixels[x] receives a 2-bit color index (0..3); x=0 is the leftmost pixel.
    static void decodeTileRow(uint8_t lowPlane, uint8_t highPlane, uint8_t outPixels[8]);

    uint8_t readRegister(uint16_t addr) const;
    void    writeRegister(uint16_t addr, uint8_t value);

    void connectVRAM(VRAM* vram) { _vram = vram; }
    const std::array<uint8_t, 160 * 144>& getFramebuffer() const { return _framebuffer; }

    bool isVramLocked() const;

private:
    int _dotCounter = 0;
    int _ly = 0;
    enum class Mode : uint8_t { HBlank = 0, VBlank = 1, OamScan = 2, Drawing = 3 };
    Mode _mode = Mode::OamScan;

    uint8_t _lcdc = 0;   // 0xFF40 — master control (tilemap/tiledata select, BG enable)
    uint8_t _stat = 0;   // 0xFF41 — status/interrupt-enable bits (simplified for now)
    uint8_t _scy  = 0;   // 0xFF42 — background scroll Y
    uint8_t _scx  = 0;   // 0xFF43 — background scroll X
    uint8_t _lyc  = 0;   // 0xFF45 — LY compare 
    uint8_t _bgp  = 0;   // 0xFF47 — background palette

    void renderScanline();

    VRAM* _vram = nullptr;
    std::array<uint8_t, 160 * 144> _framebuffer{};   // {} zero-initializes
};