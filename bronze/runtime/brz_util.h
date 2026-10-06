#ifndef BRZ_UTIL_H
#define BRZ_UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool   brz_streq(const char* a, const char* b);
char*  brz_strdup(const char* s);
char*  brz_read_entire_file(const char* path, size_t* out_size);

/* simple xorshift rng (deterministic) */
typedef struct {
    uint32_t state;
} BrzRng;

void     brz_rng_seed(BrzRng* r, uint32_t seed);
uint32_t brz_rng_u32(BrzRng* r);
int      brz_rng_range(BrzRng* r, int lo, int hi); /* inclusive */

#endif /* BRZ_UTIL_H */
