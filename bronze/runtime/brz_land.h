
#ifndef BRZ_LAND_H
#define BRZ_LAND_H

/*
    brz_land.*

    Fractal landscape generator adapted from land.c.

    Produces a deterministic 512x512 heightmap with values in [0,255].
    The simulation samples this map (with wrapping) to derive a height/land/water
    classification for the world.

    Notes:
      - The generator is intentionally tiny and deterministic.
      - Heights wrap toroidally (x,y modulo 512) like the original.
*/

#include <stdint.h>

enum { BRZ_LAND_DIM = 512 };

typedef struct {
    int genetics[2];
    /* Double-buffer like the original (2 * 512 * 512). Only buffer 0 is public. */
    uint8_t topo[2 * BRZ_LAND_DIM * BRZ_LAND_DIM];
} BrzLand;

/* Seed the PRNG / genetics. The same seeds will always produce the same land. */
void   brz_land_seed(BrzLand* land, int r1, int r2);

/* Generate the full 512x512 heightmap into land->topo (buffer 0). */
void   brz_land_generate(BrzLand* land);

/* Sample height with wrapping; returns [0,255]. */
uint8_t brz_land_height_at(const BrzLand* land, int x, int y);

#endif /* BRZ_LAND_H */
