Bus Class:
The Bus acts as the middle man between the CPU and the rest of the Game Boy hardware. The CPU never directly accesses memory or hardware components. Instead, every memory read or write goes through the Bus.

Ex:
If the CPU executes a read from address 0xFF44, it calls: bus.read(0xFF44);
The Bus checks the memory map:
FF00-FF7F → I/O Registers

Since 0xFF44 falls within that range, it forwards the request to the I/O registers (or later, the appropriate hardware such as the PPU): return ioRegisters[address - 0xFF00];
Subtracting the start of the memory region converts the Game Boy address into a zero-based array index.

Another ex: bus.write(0xC005, 42);

The Bus sees that 0xC005 belongs to WRAM (0xC000-0xDFFF) and stores the value in:

wram[address - 0xC000] = 42 --> wram[5] = 42

The Bus does not execute instructions or emulate hardware behavior. Its responsibility is simply to route memory accesses to the correct hardware component.

Main Idea: Bus answers the question of "who owns this address?"

Classes within Bus:

Certain "dumb storage" components (like ROM and WRAM) can sit inside Bus as member variables, but other components that must operate independently of the CPU (like VRAM), must be referenced within Bus (they need to be their own class). For consistency, it's best to just have all components (even ROM and WRAM) as their own wrapper classes.

read16 and write16 functions:
The sm83 (cpu) is 8 bit, so there's no true 16 bit operations done. Every 16 bit operation is actually two separate 8 bit actions. These two functions are convenience functions that do what the cpu would normally do (two 8 bit accesses).

The sm83 is little-endian, so it stores the least significant (lower) byte value at the lower address. So for a 16-byte read starting at address, the byte at address is the lower byte and the byte at address + 1 is the high byte (backwards from how you'd normally write it left to right)

The bit math for combining bytes (read16): you have two separate uint8_t values (lo and hi) and need one uint16_t. Simply adding them (hi + lo) would be wrong — worth thinking through why if you haven't already: what happens if hi = 0x01 and lo = 0x01? Addition gives you 0x02, which is nonsense — you've lost the fact that hi represented the upper 8 bits. Shifting hi left by 8 bits before combining (hi << 8) moves it into the correct bit positions, and OR-ing (|) with lo merges them without any overlap or loss, since after the shift, hi's bits and lo's bits occupy completely different bit positions. (explained by claude)

The inverse for write16: splitting a uint16_t back into its component bytes — the low byte is the value's low 8 bits (extracted via mask or truncating cast), the high byte is the value shifted right by 8 bits.

