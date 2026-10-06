
#include "brz_settlement.h"
#include "brz_util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int brz_settlements_alloc(BrzSettlement** out, int n, size_t res_n, size_t item_n){
    *out = (BrzSettlement*)calloc((size_t)n, sizeof(BrzSettlement));
    if(!*out) return 1;
    for(int i=0;i<n;i++){
        snprintf((*out)[i].name, sizeof((*out)[i].name), "Settlement%d", i+1);
        (*out)[i].res_inv = (double*)calloc(res_n, sizeof(double));
        (*out)[i].item_inv = (double*)calloc(item_n, sizeof(double));
        if(!(*out)[i].res_inv || !(*out)[i].item_inv) return 1;
    }
    return 0;
}

void brz_settlements_place(BrzSettlement* s, int n, int w, int h, unsigned seed){
    BrzRng rng; brz_rng_seed(&rng, seed?seed:0xC0FFEEu);
    for(int i=0;i<n;i++){
        /* avoid coast band by y>=h/5 */
        int x = brz_rng_range(&rng, 2, w-3);
        int y = brz_rng_range(&rng, h/5 + 2, h-3);
        /* simple spacing: n tries */
        for(int tries=0; tries<50; tries++){
            int ok=1;
            for(int j=0;j<i;j++){
                int d = abs(x - s[j].pos.x) + abs(y - s[j].pos.y);
                if(d < (w+h)/10) { ok=0; break; }
            }
            if(ok) break;
            x = brz_rng_range(&rng, 2, w-3);
            y = brz_rng_range(&rng, h/5 + 2, h-3);
        }
        s[i].pos.x = x; s[i].pos.y = y;
    }
}

void brz_settlements_begin_day(BrzSettlement* s, int n){
    (void)s; (void)n;
    /* placeholder for future daily accounting */
}

void brz_settlements_free(BrzSettlement* s, int n){
    if(!s) return;
    for(int i=0;i<n;i++){
        free(s[i].res_inv);
        free(s[i].item_inv);
    }
    free(s);
}

int brz_find_nearest_settlement(const BrzSettlement* s, int n, BrzPos p){
    int best=-1;
    int bestd=1<<30;
    for(int i=0;i<n;i++){
        int d = abs(p.x - s[i].pos.x) + abs(p.y - s[i].pos.y);
        if(d < bestd){ bestd=d; best=i; }
    }
    return best;
}

/* Simple scarcity pricing: 1.0 at target, rises to ~5 when scarce, drops to ~0.2 when abundant */
static double scarcity_price(double inv, double target){
    if(target <= 0.0) target = 1.0;
    double ratio = inv / target;
    if(ratio < 0.001) ratio = 0.001;
    double p = 1.0;
    if(ratio < 1.0) p = 1.0 + (1.0 - ratio) * 4.0;
    else p = 1.0 / (1.0 + (ratio - 1.0));
    if(p < 0.2) p = 0.2;
    if(p > 5.0) p = 5.0;
    return p;
}

double brz_settlement_price_res(const BrzSettlement* s, const ParsedConfig* cfg, int rid){
    const char* name=cfg ? kind_table_name(&cfg->resource_kinds,rid) : NULL;
    const ResourceDef* def=cfg ? brz_resource_find(cfg,name) : NULL;
    double target=def ? def->market_target : (double)(s->population > 0 ? s->population : 50);
    if(target<=0) target=(double)(s->population > 0 ? s->population : 50);
    return scarcity_price(s->res_inv[rid], target);
}

double brz_settlement_price_item(const BrzSettlement* s, int iid){
    double target = (double)(s->population > 0 ? s->population/4 : 10);
    if(target < 5) target = 5;
    return scarcity_price(s->item_inv[iid], target);
}
