#include "Timer.h"


uint8_t Timer::read(uint16_t address) {
    switch (address) {
        case 0xFF04: return _div;
        case 0xFF05: return _tima;
        case 0xFF06: return _tma;
        case 0xFF07: return _tac;
        default: return 0xFF;   // shouldn't happen if bus routes correctly
    }
}

void Timer::write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF04: _div = 0; _divCounter = 0; break;   // DIV: any write RESETS to 0 (value ignored!)
        case 0xFF05: _tima = value; break;
        case 0xFF06: _tma = value; break;
        case 0xFF07: _tac = value; break;
        default:
            break;
    }
}

bool Timer::tick(int cycles) {
    _divCounter += cycles;
    while (_divCounter >= 256) {
        _divCounter -= 256;
        _div++;   // wraps at 256 automatically (uint8_t)
    }

    if (!(_tac & 0x04)) return false;

    int period;
    switch (_tac & 0x03) {
        case 0: period = 1024; break;
        case 1: period = 16;   break;
        case 2: period = 64;   break;
        case 3: period = 256;  break;
        default: period = 256; break; //unreachable but it satisfies the compiler
    }
    
    _timaCounter += cycles;
    bool overflowed = false;
    while (_timaCounter >= period) {
        _timaCounter -= period;
        if (_tima == 0xFF) {          // about to overflow
            _tima = _tma;             // reload from TMA
            overflowed = true;        // request Timer interrupt
        } else {
            _tima++;
        }
    }
    return overflowed;
}