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

CARTRIDGE

0x0134–0x0143: Title (the game's name in ASCII)

byte 0x0147 = CRITICAL BYTE, tells you what MBC chip it uses (if any) Ex. 0x00 = no MBC (32KB ROM only), 0x01 = MBC1, 0x03 = MBC1+RAM+battery, and so on.

0x0148: ROM size — encodes how many ROM banks exist (and thus total ROM size)

0x0149: RAM size — how much external (cartridge) RAM, if any


--CPU--
CPU's job is simply to fetch bytes from Bus and manipulate its internal registers
Think of registers as the CPU’s ultra-fast, local scratchpad memory

1. The 8-Bit General Purpose Registers (B, C, D, E, H, L)
These are used to store temporary variables, counters, and loop values during math or data-moving operations.The Pairs: The CPU can link them together to form 16-bit registers (BC, DE, HL).

The Special Pointer (HL): While BC and DE are mostly for data, HL stands for High/Low. It is specifically designed to act as a 16-bit memory pointer. Many Game Boy instructions tell the CPU to read or write data to the memory address currently held inside HL (often written as [HL] or (HL) in assembly).

2. The Accumulator (A): The A register is the star of the show. Almost all 8-bit arithmetic and logical operations must involve the accumulator. If you want to add two numbers, one of them has to be in A.The result of that addition is automatically saved right back into A.
Example instruction: ADD A, B means: "Take the value in A, add the value in B, and save the result into A.

3. The Flags Register (F): The F register doesn't hold data numbers. Instead, it acts as a status board. Whenever the CPU does a math calculation, logical operation, or bit shift, it updates individual bits (flags) in F to describe what just happened. Subsequent instructions (like conditional jumps) read these flags to make decisions (e.g., "Jump to this loop if the last subtraction resulted in zero"). On the Game Boy, only the top 4 bits of F are used:

Bit    Flag Name    Symbol  What it means
7      Zero Flag      Z     Set to 1 if the result of the last operation was exactly 0. Set to 0 otherwise.
6     Subtract Flag   N     Set to 1 if the last operation was a subtraction/decrease. Set to 0 if it was an addition/increase. (The CPU uses this to adjust decimal numbers later)
5   Half-Carry Flag   H     Set to 1 if an addition carried a value over from Bit 3 to Bit 4 (the lower 4 bits overflowed). Crucial for Binary Coded Decimal (BCD) math.
4      Carry Flag     C     Set to 1 if an addition overflowed past Bit 7 (e.g., 255 + 1), or if a subtraction required a borrow below 0. Also used in bit-shifting.
3-0     Unused        -     Always hardwired to 0

4. Special 16-Bit Registers (PC and SP): These two do not share 8-bit pairs because they have very specific, automated roles in hardware:
Program Counter (PC): This holds the 16-bit memory address of the next instruction the CPU needs to execute. Every time the CPU fetches an instruction byte from your memory bus, PC automatically increments (PC++) to point to the next byte.
Stack Pointer (SP): This points to a temporary storage area in the Game Boy's High RAM called the "Stack." When your CPU jumps to a function (a CALL instruction), it pushes the current PC address onto the stack so it knows how to return later. SP tracks the top of this memory pile.

Bit manipulation:

the mask is the bit position written in hex
Ex. the mask for bit 6 is (2^6 = 64) = 0x40.
the mask for bit 7 is (2^7 = 128) = 0x80.
inverse mask is just ~0x...

getting a bit -> & with the mask. Getting bit 7: f = f & 0x80;
setting a bit (force it to 1) -> | with the mask. Setting bit 7: f = f | 0x80;
clearing a bit (force it to 0) -> & with the inverted mask. Clearing bit 7: f = f & ~0x80;


Implementing opcodes. 
Starting with 8 bit loads (0x40 - 0x7F). 
Ex. Opcode at 0x41 = (LD B, C), 1, 4 --> load C onto B, 4 cycles --> case 0x41: setB(get(C)); return = 4;

PUSH/POP functions:
on the Sharp SM83 the stack grows downward. so SP decreases on push and increases on pop. and byte order is little endian so the low byte should go first.

PUSH architecture: 

so for PUSH BC:
setSP(getSP() - 1); //cant just do SP - 1 since it's a private variable
write(SP, hi byte);
setSP(getSP() - 1);
write(SP, lo byte);

POP architecture;
so POP BC:
lo byte = read(SP);
setSP(getSP() + 1);
hi byte = read(SP);
setSP(getSP() + 1);


arithmetic (flags and stuff)

Z (Zero) — set if the result is 0, cleared otherwise
N (Subtract) — set if the last op was a subtraction, cleared for addition. For ADD it's always cleared (0). It 
exists because the DAA instruction later needs to know whether the previous op was add or subtract

C (Carry) — set if the result overflowed past 8 bits, i.e. the true sum exceeded 0xFF. For A + r, carry occurs if A + r > 0xFF.

H (Half-carry) — set if there was a carry from bit 3 into bit 4 — i.e., the low nibbles overflowed. basically if the two lower nibbles are greater than 16 its set
