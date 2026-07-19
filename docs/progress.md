Session 0 (7/12/26): 
Set up environment (CMake and everything else) (helped by Claude)

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