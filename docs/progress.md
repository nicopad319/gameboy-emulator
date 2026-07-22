Session 0 (7/12/26): 
Set up environment (CMake and everything else)

Session 1 (7/13/26):
implemented bus and 6/7 components. Saving cartridge for tomorrow since it's the meaty one.

Session 2 (7/14/26): 
finished all M2 components incl. Cartridge file I/O; next: injection seam + AccessSource enum + GameBoy wiring + smoke test

Session 3 (7/15/26):
Milestone 2 done! Memory Bus complete. Bus is handed addresses from the CPU and delegates each one to its respective component to figure out what to do with them. WRAM, HRAM, IORegisters, IERegister were all easy and straightforward, just read and write functions. VRAM and OAM were a bit trickier to plan out since they have locking mechanisms based on PPU mode that we haven't fully implemented yet. Cartridge was the most complex, I had to learn file I/O to allow it to load ROMs and it also had it's own read and write functions. I created a PPU stub to allow for basic testing/functionality. GameBoy was the constructor class that is basically the root of the program.

Bugs I ran into and fixed: 
Nested duplicate gameboy-emulator folders: Certain calls weren't working because my directories were all jumbled, I fixed it by calling dir /s /b in the terminal and figuring out the different file paths.

MBC 1 Catch (BIG ONE): Once I completed the memory bus skeleton, i wrote a basic program in main that loaded a ROM and read and returned certain bytes to test if my program was working (things like the first byte in the nintendo logo, the cartridge type etc). Essentially, when I first wrote my mem bus I only allowed for cartridge type bytes of 0x00 (no MBC bank in the ROM) out of simplicity, if it was any other byte my program would return false and return and a message explaining why it failed. Anyway, i got a test rom from Blarggs test roms and tried it on my program, and that error kept getting called and i couldn't figure out why because I had thought the test rom i chose had no MBC bank. Until I went through the file using powershell and found the byte i was looking for and it turned out to be 0x01 (MBC1). So i edited my code to allow for MBC 1 as well and then the test went through and my program was working successfully.

Why Bus is a "dumb router" — separation of concerns, so hardware logic lives in components, not the router.

Why base-address subtraction lives in the components, not Bus — so Bus never needs to know any component's internal storage layout (encapsulation)

The VRAM/OAM access-lock decision — you say they were "trickier to plan," but the actual decision was: keep the PPU-mode check inside the component (consistent with component-owned rules) rather than in Bus, and you worked through the DMA nuance (that DMA is genuinely different because access depends on who's requesting, not just mode). That reasoning is worth a sentence — it's one of your strongest "I weighed a real tradeoff" stories

Dependency injection + composition root — GameBoy owns everything by value and injects pointers via constructors; you chose constructor injection over setters specifically to avoid half-constructed states and eliminate null checks. That's a deliberate, defensible choice worth recording.

New Skills:
binary-mode std::ifstream, the reinterpret_cast<char*> idiom, std::vector vs std::array tradeoff, forward declarations to break circular dependencies, constructor initializer lists, and multi-file CMake builds.

Deferred/TODO: Real MBC1 banking, AccessSource to prevent the PPU from locking itself out during rendering.
per-register IORegisters behavior, and the "should VRAM/OAM live inside PPU?" question to revisit at M6

Session 3.5 (7/15/26):
Productive day. Finished milestone 2, learned hex and binary and bitwise operation for good this time, learned the structure of the CPU (read architecture.md), and implemented CPU.h.

Session 4 (7/16/26):
Finished wiring the CPU, ran a second smoke test and implemented a test function (check) to make sure everything works. Implemented the very first opcode (NOP) and a default in the switch. 

Session 5 (7/17/26): 
Short session today. implemented 8 bit load opcodes, and started writing a TestSystem to individually test opcodes/

Session 6 (7/18/26):
Implemented TestSystem struct and a testRegisterLoads() method and a testHLLoads() method. 8-bit loads are all done and tested. onto 16-bit loads

6.5: did 16 bit loads, and implemented arithmetic logic including ADD, SUB, CP, AND, OR, XOR. roughly 25% done with opcodes. testing as I go to make sure that it passes blargg's test roms hopefully first try. green flags all around so far. will try to be done with opcodes either tomorrow or two days from now.

Session 7 (7/19/26):
implemented and tested about 100 opcodes today. i think 33 or 34 left in the main branch, then onto the 256 more in the Prefix CB branch. will hopefully be done with the CPU either tomorrow or tuesday.

Session 8 (7/20/26):
Finished all ~500 opcodes including the CB block; built a Gameboy Doctor validation harness; passed every runnable Blargg cpu_instrs sub-test against real hardware (test 2 isn't runnable yet (wait till milestone 5)).

BIG BUG I FIXED (uninitialized memory): While running Blargg's cpu_instrs, Gameboy Doctor showed my emulator's memory diverging from the reference, a WRAM byte read as 0xCC where it should've been 0x00. Every unit test I'd written wrote a value before reading it back, so all my read/write functions were provably correct but they never exercised reading an unwritten cell. The bug: in C++, a default-initialized std::array isn't zeroed, its cells hold indeterminate leftover memory (random garbage). Real programs assume RAM starts zeroed at power-on, so when the test ROM read a WRAM location it hadn't written, my emulator handed back garbage instead of 0x00. 

I originially had std::array<uint8_t, 0x2000> _wram; (no {}).
Fix: value-initialize the arrays with {} (e.g. std::array<uint8_t, 0x2000> _wram{};), which zeroes every cell. Applied to all memory components. Lesson: uninitialized memory is garbage, not zero so anything that might be read before written must be explicitly initialized, and write-before-read tests can't catch it.

Session 9 (7/21/26):
Implemented Interrupts and timing. had a couple tedious bugs, took me a couple hours to fix:

Bug 1 — the dispatch was silently unlogged
CPU::step() had handleInterrupts() return early (return 20) before the line that logs CPU state. So whenever an interrupt actually fired, that step produced zero log lines instead of one. Every line after that point was off by one relative to the reference, which looks exactly like a garbled trace even though the interrupt itself dispatched correctly. A missing log line and a wrong CPU state produce the identical symptom. You have to check whether you're comparing the same moment in time before you conclude the logic is wrong.

Bug 2 — dispatch and the ISR's first instruction are one atomic unit
Fixing #1 exposed a second layer: real hardware (and the reference trace) never shows the interrupt vector address (e.g. 0x0050) as its own step. I confirmed this by grepping the entire reference log for every possible vector address — zero hits, ever. Interrupt dispatch (push PC, jump to vector) takes 5 M-cycles, and the last of those cycles doubles as the opcode fetch for the ISR's first instruction — there's no independently observable boundary between "just jumped" and "fetched the first ISR opcode." So the fix was structural: instead of dispatch being an early-return, it falls through into the normal fetch/execute path within the same step() call, sharing one log line for both.

Bug 3 — cycle count dropped the dispatch cost
Once dispatch stopped short-circuiting, the 20-cycle cost of the dispatch itself was no longer being returned from step() — only the ISR's first instruction's cycles were. This wouldn't fail cpu_instrs (it doesn't check timing), but it would silently desync the Timer (and later, PPU) from the CPU by 20 cycles every single interrupt. Caught it by reasoning about where the return value goes, not from a failing test

Bug 4 — HALT never woke up when IME was disabled
Real hardware conflates two different conditions that look similar: "should HALT stop sleeping" (IE & IF != 0, independent of IME) versus "should the CPU actually service an interrupt" (also requires IME=1). My handleInterrupts() checked IME first and bailed immediately if it was off — which meant if a test did DI then HALT, the CPU could never clear _halted, because the only place that ever cleared it was gated behind the same IME check. On real hardware, HALT with IME=0 still wakes up when an interrupt becomes pending; it just resumes normal execution at the next instruction instead of jumping to a vector. Fixed by splitting the concerns: an interruptPending() check independent of IME drives waking, while the existing IME-gated logic still controls whether a jump actually happens.

Bug 5 — logging every idle tick instead of once
Last one, and the subtlest: after #4, the CPU correctly stayed halted for ~1015 4-cycle ticks (matching the exact cycle count for TIMA to overflow) and then woke up right on schedule — verified by literally counting how many times that log line appeared in mine (1015) versus the reference (1). The behavior was already correct; only the logging was wrong. Real hardware doesn't perform 1015 separate instruction-fetch attempts while halted — it does zero, then one, when it wakes. My code was logging on every early-return tick. Fix: move the halted early-return before the log call, so idle ticks produce no line, and only the tick where something actually changes gets logged.

