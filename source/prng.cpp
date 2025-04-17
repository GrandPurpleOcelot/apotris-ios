#include "def.h"

bool sampleEntropy = true;

/**
 * By sampling a hashed user input every VBlank the PRNG is seeded effectively
 * even without a save. Whisky's hash is sufficiently different for the seed.
 */
void SampleEntropy() {
    // If a game has ended begin sampling input
    if (ended)
        sampleEntropy = true;
    // Don't sample input during user games or fixed PRNG tasks like replays
    if (!sampleEntropy || replaying || multiplayer)
        return;
#ifdef GBA
    // Read keys directly on GBA to capture potential simultaneous key inputs
    u32 input = REG_KEYINPUT ^ 0x3ff;
#else
    // Non-GBA doesn't have a direct register to read -- it should be OK
    uint32_t input = key_first() ^ 0x3ff;
#endif
    // Hash the input with the last seed
    savefile->seed = (int)whisky2(input, savefile->seed);
    // Seed first 32 bits of state as Xoshiro128++ authors suggest
    randSetSeed(savefile->seed);
}
#ifndef GBA
void randSetSeed(uint32_t seed) { setXoshiroSeed(seed); }
uint32_t randNext() { return nextXoshiro(); }
void randSetFullState(uint32_t seed) { setXoshiroState(seed); }
#endif
