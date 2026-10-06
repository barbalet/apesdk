
#ifndef BRZ_WORLD_H
#define BRZ_WORLD_H

#include "brz_types.h"
#include "brz_dsl.h"
#include <stddef.h>
#include <stdint.h>

struct BrzSettlement;

enum {
    BRZ_TAG_COAST   = 1u<<0,
    BRZ_TAG_FIELD   = 1u<<1,
    BRZ_TAG_FOREST  = 1u<<2,
    BRZ_TAG_CLAYPIT = 1u<<3,
    BRZ_TAG_MINE_CU = 1u<<4,
    BRZ_TAG_MINE_SN = 1u<<5,
    BRZ_TAG_FIRE    = 1u<<6
};

typedef struct {
    int w, h;
    uint16_t* tags;   /* [w*h] */
    uint8_t*  height; /* [w*h] heightmap sample in [0,255] */
    uint8_t   sea_level; /* waterline threshold in [0,255] */
    double*   res;    /* [w*h*res_n] */
    double*   cap;    /* [w*h*res_n] */
    double*   regen;  /* [res_n] */
} BrzWorld;

int  brz_world_init(BrzWorld* world, const ParsedConfig* cfg, int w, int h, size_t res_n);
void brz_world_free(BrzWorld* world);

void brz_world_step_regen(BrzWorld* world, size_t res_n);

uint16_t brz_world_tags_at(const BrzWorld* world, BrzPos p);
uint8_t  brz_world_height_at(const BrzWorld* world, BrzPos p);
double   brz_world_take(BrzWorld* world, BrzPos p, size_t res_n, int rid, double amt);
double   brz_world_peek(const BrzWorld* world, BrzPos p, size_t res_n, int rid);

BrzPos brz_world_find_nearest_tag(const BrzWorld* world, BrzPos from, uint16_t tag, int max_r);

void  brz_world_stamp_fields_around_settlements(BrzWorld* world, const struct BrzSettlement* setts, int sett_n, int radius);
char  brz_world_tile_glyph(const BrzWorld* world, int x, int y);

#endif
