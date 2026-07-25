#include <Arduino.h>
#include <TFT_eSPI.h>
#include "GameBoy.h"
#include "tetris_rom.h"

TFT_eSPI tft = TFT_eSPI();
GameBoy gb;

// Same DMG green palette the desktop SDL build uses, converted to RGB565 via
// TFT_eSPI's own color565() helper instead of hand-computing the bit-packed
// values -- less error-prone, and it's the exact same source colors, so a
// wrong palette isn't a new class of bug to chase here.
uint16_t palette[4];

// Landscape display is 320x240. The GB screen (160x144) has a different
// aspect ratio, so height is the limiting dimension: scale by 240/144 to
// fill the screen height exactly without cropping, and center the result
// horizontally (thin black bars left/right instead of a stretched image).
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;
constexpr int GB_W = 160;
constexpr int GB_H = 144;
constexpr int SCALED_H = SCREEN_H;
constexpr int SCALED_W = (GB_W * SCALED_H) / GB_H;   // 266
constexpr int OFFSET_X = (SCREEN_W - SCALED_W) / 2;   // 27

// No full-frame buffer at all -- pushImage() doesn't need the whole 266x240
// image in memory at once, and a single 127KB heap allocation is a real
// failure point on a memory-constrained target (it just crashed one). One
// row at a time, held in a small stack buffer, sidesteps that entirely.

struct ButtonPin {
    Joypad::Button button;
    int pin;
};

const ButtonPin buttonPins[] = {
    { Joypad::Button::Up,     13 },
    { Joypad::Button::Down,   14 },
    { Joypad::Button::Left,   27 },
    { Joypad::Button::Right,  26 },
    { Joypad::Button::A,      25 },
    { Joypad::Button::B,      33 },
    { Joypad::Button::Start,  32 },
    { Joypad::Button::Select, 21 },
};

void setupButtons() {
    for (const auto& bp : buttonPins) {
        pinMode(bp.pin, INPUT_PULLUP);
    }
}

void pollButtons() {
    for (const auto& bp : buttonPins) {
        // INPUT_PULLUP: idle = HIGH, pressed = LOW (button shorts pin to GND)
        bool pressed = (digitalRead(bp.pin) == LOW);
        gb.setButton(bp.button, pressed);
    }
}

void pushFrame() {
    const auto& fb = gb.getFramebuffer();
    uint16_t rowBuffer[SCALED_W];   // 266 * 2 bytes = 532 bytes, stack-safe
    // Nearest-neighbor upscale: for each destination pixel, find which
    // source GB pixel it maps to and copy its (palette-converted) color.
    for (int y = 0; y < SCALED_H; y++) {
        int srcY = y * GB_H / SCALED_H;
        for (int x = 0; x < SCALED_W; x++) {
            int srcX = x * GB_W / SCALED_W;
            rowBuffer[x] = palette[fb[srcY * GB_W + srcX] & 0x03];
        }
        tft.pushImage(OFFSET_X, y, SCALED_W, 1, rowBuffer);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    palette[0] = tft.color565(155, 188, 15);
    palette[1] = tft.color565(139, 172, 15);
    palette[2] = tft.color565(48, 98, 48);
    palette[3] = tft.color565(15, 56, 15);

    setupButtons();

    bool loaded = gb.loadFromMemory(tetris_rom, tetris_rom_len);
    Serial.print("Tetris loaded from flash: ");
    Serial.println(loaded ? "OK" : "FAILED");
}

void loop() {
    pollButtons();
    gb.runFrame();
    pushFrame();
    // No frame-rate pacing yet -- runs as fast as the chip allows for now.
    // If it's visibly too fast or too slow, that's the next thing to fix,
    // not something to guess at preemptively.
}
