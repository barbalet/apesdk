
#ifndef BRZ_SETTLEMENT_H
#define BRZ_SETTLEMENT_H

#include "brz_types.h"
#include "brz_dsl.h"
#include <stddef.h>

typedef struct BrzSettlement {
    char name[64];
    BrzPos pos;
    int population;
    double* res_inv;   /* [res_n] */
    double* item_inv;  /* [item_n] */
} BrzSettlement;

int  brz_settlements_alloc(BrzSettlement** out, int n, size_t res_n, size_t item_n);
void brz_settlements_place(BrzSettlement* s, int n, int w, int h, unsigned seed);
void brz_settlements_begin_day(BrzSettlement* s, int n);
void brz_settlements_free(BrzSettlement* s, int n);

int   brz_find_nearest_settlement(const BrzSettlement* s, int n, BrzPos p);
double brz_settlement_price_res(const BrzSettlement* s, const ParsedConfig* cfg, int rid);
double brz_settlement_price_item(const BrzSettlement* s, int iid);

#endif
