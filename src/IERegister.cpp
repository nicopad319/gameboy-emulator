#include "IERegister.h"

uint8_t IERegister::read() {
    return _ieRegister.at(0); // Since there is only one byte, we always return the first element
}

void IERegister::write(uint8_t value) {
    _ieRegister.at(0) = value;
}