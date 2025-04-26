#ifndef GBA
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

uint32_t nextXoshiro(void);
void setXoshiroSeed(uint32_t seed);
void setXoshiroState(uint32_t seed);
#ifdef __cplusplus
}
#endif
#endif
