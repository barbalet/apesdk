
/*
    brz_land.c

    This is a lightly-refactored version of land.c (Tom Barbalet, 1996-2024)
    turned into a reusable module for BRONZESIM.

    The logic is intentionally kept close to the original implementation:
      - same "genetics" PRNG
      - same 512x512 toroidal index math
      - same patching + smoothing passes

    The main difference is that state lives in a BrzLand struct rather than
    globals, and results are stored as uint8_t heights.
*/

#include "brz_land.h"

/* --- internal helpers (ported nearly 1:1) --- */

static int land_math_random(BrzLand* land) {
    int tmp0 = land->genetics[0];
    int tmp1 = land->genetics[1];
    int runIt = 1;

    land->genetics[0] = tmp1;
    int tempAnd7 = tmp0 & 7;

    if (tempAnd7 == 0) {
        land->genetics[0] = ( tmp1 ^ ( tmp0 >> 3 ) ^ 23141 );
        runIt = 0;
    }
    if (tempAnd7 == 3) {
        land->genetics[1] = ( tmp0 ^ ( tmp1 >> 1 ) ^ 53289 );
        runIt = 0;
    }
    if (tempAnd7 == 5) {
        land->genetics[1] = ( tmp1 ^ ( tmp0 >> 2 ) ^ 44550 );
        runIt = 0;
    }
    if (runIt == 1) {
        land->genetics[1] = ( tmp0 ^ ( tmp1 >> 1 ) );
    }
    return land->genetics[1];
}

static int land_wrap_index(int lx, int ly) {
    int converted_x = (lx + BRZ_LAND_DIM) & (BRZ_LAND_DIM - 1);
    int converted_y = (ly + BRZ_LAND_DIM) & (BRZ_LAND_DIM - 1);
    return (converted_x | (converted_y * BRZ_LAND_DIM));
}

static int land_topo_get(const BrzLand* land, int buffer, int lx, int ly) {
    return (int)land->topo[(buffer * BRZ_LAND_DIM * BRZ_LAND_DIM) + land_wrap_index(lx, ly)];
}

static void land_topo_set(BrzLand* land, int buffer, int lx, int ly, int value) {
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    land->topo[(buffer * BRZ_LAND_DIM * BRZ_LAND_DIM) + land_wrap_index(lx, ly)] = (uint8_t)value;
}

static void land_swap_buffers(BrzLand* land) {
    /* Copy buffer 0 to buffer 1 (as per original). */
    for (int i = 0; i < (BRZ_LAND_DIM * BRZ_LAND_DIM); i++) {
        land->topo[(BRZ_LAND_DIM * BRZ_LAND_DIM) + i] = land->topo[i];
    }
}

static void land_pack_flat(BrzLand* land) {
    for (int i = 0; i < (BRZ_LAND_DIM * BRZ_LAND_DIM); i++) {
        land->topo[i] = 128;
    }
}

static void land_round(BrzLand* land) {
    int local_tile_dimension = 1 << 9; /* 512 */
    int span_minor = 0;
    while (span_minor < 6) {
        int py = 0;
        while (py < local_tile_dimension) {
            int px = 0;
            while (px < local_tile_dimension) {
                int sum = 0;
                for (int ty = -1; ty < 2; ty++) {
                    for (int tx = -1; tx < 2; tx++) {
                        sum += land_topo_get(land, (span_minor & 1), px + tx, py + ty);
                    }
                }
                land_topo_set(land, (span_minor & 1) ^ 1, px, py, sum / 9);
                px += 1;
            }
            py += 1;
        }
        span_minor += 1;
    }
}

static void land_patch(BrzLand* land, int refine) {
    int local_tiles = 2;
    int span_minor = (64 >> ((refine & 7) ^ 7));
    int span_major = (1 << ((refine & 7) ^ 7));

    for (int tile_y = 0; tile_y < local_tiles; tile_y++) {
        for (int tile_x = 0; tile_x < local_tiles; tile_x++) {
            for (int py = 0; py < span_minor; py++) {
                for (int px = 0; px < span_minor; px++) {
                    int val1 = ((px << 2) + (py << 10));
                    int tseed = land_math_random(land);

                    for (int ty = 0; ty < 4; ty++) {
                        for (int tx = 0; tx < 4; tx++) {
                            int val2 = (tseed >> (tx | (ty << 2)));
                            int val3 = ((((val2 & 1) << 1) - 1) * 20);
                            val2 = (tx | (ty << 8));

                            for (int my = 0; my < span_major; my++) {
                                for (int mx = 0; mx < span_major; mx++) {
                                    int point = ((mx | (my << 8)) + (span_major * (val1 + val2)));
                                    int pointx = (point & 255);
                                    int pointy = (point >> 8);
                                    if ((refine & 2) != 0) {
                                        int pointx_tmp = pointx + pointy;
                                        pointy = pointx - pointy;
                                        pointx = pointx_tmp;
                                    }
                                    int base = land_topo_get(land, 0, pointx + (tile_x << 8), pointy + (tile_y << 8));
                                    land_topo_set(land, 0, pointx + (tile_x << 8), pointy + (tile_y << 8), base + val3);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/* --- public API --- */

void brz_land_seed(BrzLand* land, int r1, int r2) {
    if (!land) return;
    land->genetics[0] = r1;
    land->genetics[1] = r2;

    land->genetics[0] = (((land_math_random(land) & 255) << 8) | (land_math_random(land) & 255));
    land->genetics[1] = (((land_math_random(land) & 255) << 8) | (land_math_random(land) & 255));

    land_math_random(land);
    land_math_random(land);
    land_math_random(land);

    land->genetics[0] = (((land_math_random(land) & 255) << 8) | (land_math_random(land) & 255));
    land->genetics[1] = (((land_math_random(land) & 255) << 8) | (land_math_random(land) & 255));
}

void brz_land_generate(BrzLand* land) {
    if (!land) return;
    land_pack_flat(land);
    for (int refine = 0; refine < 7; refine++) {
        land_patch(land, refine);
        land_round(land);
        land_swap_buffers(land);
    }
}

uint8_t brz_land_height_at(const BrzLand* land, int x, int y) {
    if (!land) return 0;
    return land->topo[land_wrap_index(x, y)];
}
