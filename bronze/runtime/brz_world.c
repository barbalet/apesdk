
#include "brz_world.h"
#include "brz_kinds.h"
#include "brz_util.h"
#include "brz_settlement.h"
#include "brz_land.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static double* tile_ptr(BrzWorld* w, int x, int y, size_t res_n){
    return &w->res[(size_t)(y*w->w + x) * res_n];
}
static double* tile_cap_ptr(BrzWorld* w, int x, int y, size_t res_n){
    return &w->cap[(size_t)(y*w->w + x) * res_n];
}

static uint16_t tag_for_habitat(const char* habitat){
    if(!habitat) return 0;
    if(brz_streq(habitat,"coast")) return BRZ_TAG_COAST;
    if(brz_streq(habitat,"field")) return BRZ_TAG_FIELD;
    if(brz_streq(habitat,"forest")) return BRZ_TAG_FOREST;
    if(brz_streq(habitat,"claypit")) return BRZ_TAG_CLAYPIT;
    if(brz_streq(habitat,"mine_copper")) return BRZ_TAG_MINE_CU;
    if(brz_streq(habitat,"mine_tin")) return BRZ_TAG_MINE_SN;
    if(brz_streq(habitat,"fire")) return BRZ_TAG_FIRE;
    return 0;
}

int brz_world_init(BrzWorld* world, const ParsedConfig* cfg, int w, int h, size_t res_n)
{
    memset(world, 0, sizeof(*world));
    world->w = w; world->h = h;
    world->tags  = (uint16_t*)calloc((size_t)w*h, sizeof(uint16_t));
    world->height= (uint8_t*)calloc((size_t)w*h, sizeof(uint8_t));
    world->res   = (double*)calloc((size_t)w*h*res_n, sizeof(double));
    world->cap   = (double*)calloc((size_t)w*h*res_n, sizeof(double));
    world->regen = (double*)calloc(res_n, sizeof(double));
    if(!world->tags || !world->height || !world->res || !world->cap || !world->regen) return 1;

    /* sea level: default 128, override with param "sea_level" if present */
    uint8_t sea = 128;
    for(size_t i=0;i<cfg->params.len;i++){
        const ParamDef* p = (const ParamDef*)brz_vec_cat(&cfg->params, i);
        if(p->key && brz_streq(p->key, "sea_level") && !p->has_svalue){
            int iv = (int)(p->value);
            if(iv < 0) iv = 0;
            if(iv > 255) iv = 255;
            sea = (uint8_t)iv;
            break;
        }
    }
    world->sea_level = sea;

    /* Build a deterministic fractal heightmap (512x512), then sample it to the
       requested world size.

       Seeds: use cfg->seed when provided, else fall back to a fixed constant to
       preserve determinism.
    */
    BrzLand land;
    uint32_t s = (cfg->seed ? (uint32_t)cfg->seed : 0xC0FFEEu);
    int r1 = (int)(s & 0xFFFFu);
    int r2 = (int)((s >> 16) & 0xFFFFu);
    brz_land_seed(&land, r1, r2);
    brz_land_generate(&land);

    /* regen: read <resname>_renew params if present, else 0.01 */
    for(size_t rid=0; rid<res_n; rid++){
        const char* rn = kind_table_name(&cfg->resource_kinds, (int)rid);
        char key[128];
        snprintf(key, sizeof(key), "%s_renew", rn?rn:"");
        /* cfg params are in cfg->params array; easiest is to use small helper: */
        const ResourceDef* def = brz_resource_find(cfg,rn);
        double v = def ? def->renew : 0.01;
        for(size_t i=0;i<cfg->params.len;i++){
            const ParamDef* p = (const ParamDef*)brz_vec_cat(&cfg->params, i);
            if(p->key && brz_streq(p->key, key) && !p->has_svalue){
                v = p->value;
                break;
            }
        }
        world->regen[rid] = v;
    }

    /* tags, heights, and initial resources/caps */
    for(int y=0;y<h;y++){
        for(int x=0;x<w;x++){
            uint16_t t = 0;

            /* Sample from the 512x512 fractal map. */
            int sx = (int)((int64_t)x * BRZ_LAND_DIM / (w>0?w:1));
            int sy = (int)((int64_t)y * BRZ_LAND_DIM / (h>0?h:1));
            uint8_t height = brz_land_height_at(&land, sx, sy);
            world->height[y*w+x] = height;

            if(height < sea){
                /* Below waterline -> water tile. */
                t |= BRZ_TAG_COAST;
            } else {
                /* Above waterline -> land tile; choose a coarse biome tag.
                   These are intentionally simple heuristics; the DSL-driven
                   sim can evolve more sophisticated interpretations later.
                */
                uint8_t dh = (uint8_t)(height - sea);
                if(dh < 40) t |= BRZ_TAG_FIELD;      /* lowlands */
                else if(dh < 140) t |= BRZ_TAG_FOREST; /* midlands */
                /* highlands: leave as default '^' */
            }

            /* scatter clay pits */
            if(!(t & BRZ_TAG_COAST)){
                if(((x*73856093u) ^ (y*19349663u) ^ s) % 97u == 0u) t |= BRZ_TAG_CLAYPIT;
                /* scatter mines */
                if(((x*83492791u) ^ (y*2654435761u) ^ s) % 173u == 0u) t |= BRZ_TAG_MINE_CU;
                if(((x*2654435761u) ^ (y*83492791u) ^ s) % 199u == 0u) t |= BRZ_TAG_MINE_SN;
            }

            world->tags[y*w+x] = t;

            double* r = tile_ptr(world,x,y,res_n);
            double* c = tile_cap_ptr(world,x,y,res_n);
            for(size_t rid=0; rid<res_n; rid++){
                const char* rn = kind_table_name(&cfg->resource_kinds, (int)rid);
                const ResourceDef* def=brz_resource_find(cfg,rn);
                uint16_t need = tag_for_habitat(def ? def->habitat : NULL);
                double cap = def ? def->capacity : 10.0;
                if(need && (t & need)) cap = 100.0;
                if(need && !(t & need)) cap = 0.0;
                c[rid] = cap;
                r[rid] = cap * 0.5; /* start half full */
            }
        }
    }

    return 0;
}

void brz_world_free(BrzWorld* world){
    if(!world) return;
    free(world->tags);
    free(world->height);
    free(world->res);
    free(world->cap);
    free(world->regen);
    memset(world,0,sizeof(*world));
}

void brz_world_step_regen(BrzWorld* world, size_t res_n){
    const int W=world->w, H=world->h;
    for(int y=0;y<H;y++){
        for(int x=0;x<W;x++){
            double* r = tile_ptr(world,x,y,res_n);
            double* c = tile_cap_ptr(world,x,y,res_n);
            for(size_t rid=0; rid<res_n; rid++){
                double cap = c[rid];
                double add = cap * world->regen[rid];
                r[rid] += add;
                if(r[rid] > cap) r[rid] = cap;
                if(r[rid] < 0) r[rid] = 0;
            }
        }
    }
}

uint16_t brz_world_tags_at(const BrzWorld* world, BrzPos p){
    if(p.x<0||p.y<0||p.x>=world->w||p.y>=world->h) return 0;
    return world->tags[p.y*world->w+p.x];
}

uint8_t brz_world_height_at(const BrzWorld* world, BrzPos p){
    if(!world) return 0;
    if(p.x<0||p.y<0||p.x>=world->w||p.y>=world->h) return 0;
    return world->height[p.y*world->w+p.x];
}

double brz_world_take(BrzWorld* world, BrzPos p, size_t res_n, int rid, double amt){
    if(p.x<0||p.y<0||p.x>=world->w||p.y>=world->h) return 0;
    double* r = &world->res[(size_t)(p.y*world->w+p.x)*res_n + (size_t)rid];
    if(*r < 0) *r = 0;
    double t = (*r < amt) ? *r : amt;
    *r -= t;
    return t;
}

double brz_world_peek(const BrzWorld* world, BrzPos p, size_t res_n, int rid){
    if(p.x<0||p.y<0||p.x>=world->w||p.y>=world->h) return 0;
    return world->res[(size_t)(p.y*world->w+p.x)*res_n + (size_t)rid];
}

BrzPos brz_world_find_nearest_tag(const BrzWorld* world, BrzPos from, uint16_t tag, int max_r){
    /* simple expanding square search */
    BrzPos best = from;
    int W=world->w,H=world->h;
    if(from.x<0) from.x=0;
    if(from.y<0) from.y=0;
    if(from.x>=W) from.x=W-1;
    if(from.y>=H) from.y=H-1;
    for(int r=0; r<=max_r; r++){
        int x0 = from.x - r, x1 = from.x + r;
        int y0 = from.y - r, y1 = from.y + r;
        for(int y=y0; y<=y1; y++){
            for(int x=x0; x<=x1; x++){
                if(x<0||y<0||x>=W||y>=H) continue;
                if(world->tags[y*W+x] & tag){
                    best.x=x; best.y=y;
                    return best;
                }
            }
        }
    }
    return best;
}

void brz_world_stamp_fields_around_settlements(BrzWorld* world, const struct BrzSettlement* setts, int sett_n, int radius){
    for(int si=0; si<sett_n; si++){
        BrzPos c = setts[si].pos;
        for(int dy=-radius; dy<=radius; dy++){
            for(int dx=-radius; dx<=radius; dx++){
                int x=c.x+dx, y=c.y+dy;
                if(x<0||y<0||x>=world->w||y>=world->h) continue;
                if(dx*dx+dy*dy > radius*radius) continue;
                /* don't overwrite coast */
                if(world->tags[y*world->w+x] & BRZ_TAG_COAST) continue;
                world->tags[y*world->w+x] |= BRZ_TAG_FIELD;
            }
        }
    }
}

char brz_world_tile_glyph(const BrzWorld* world, int x, int y){
    uint16_t t = world->tags[y*world->w+x];
    if(t & BRZ_TAG_COAST) return '~';
    if(t & BRZ_TAG_FIELD) return ',';
    if(t & BRZ_TAG_CLAYPIT) return 'c';
    if(t & (BRZ_TAG_MINE_CU|BRZ_TAG_MINE_SN)) return 'm';
    return '^';
}
