
#ifndef BRZ_TYPES_H
#define BRZ_TYPES_H
#include <stdint.h>

typedef struct { int x, y; } BrzPos;

static inline int brz_abs_i(int v){ return v<0?-v:v; }
static inline int brz_clamp_i(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }

static inline BrzPos brz_step_toward(BrzPos from, BrzPos to){
    int dx = (to.x>from.x) - (to.x<from.x);
    int dy = (to.y>from.y) - (to.y<from.y);
    BrzPos p = { from.x + dx, from.y + dy };
    return p;
}

static inline int brz_dist_manhattan(BrzPos a, BrzPos b){
    return brz_abs_i(a.x-b.x) + brz_abs_i(a.y-b.y);
}

#endif
