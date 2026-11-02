#ifndef BRONZE_HYBRID_H
#define BRONZE_HYBRID_H
#include "../../entity/entity.h"

typedef enum { BRONZE_TOWN, BRONZE_FIELD, BRONZE_COPPER_MINE,
               BRONZE_TIN_MINE, BRONZE_WOODLAND } bronze_site_kind;
typedef struct {
    n_int x, y; /* ApeSDK low-resolution map coordinates */
    bronze_site_kind kind;
    double stock, capacity;
} bronze_site;
typedef struct {
    double grain, copper, tin, charcoal, tools;
    n_uint work_events;
} bronze_economy;
typedef void (*bronze_plot)(n_int x, n_int y, n_byte color, void *context);
void bronze_hybrid_reset(void);
void bronze_hybrid_cycle(simulated_group *group);
void bronze_hybrid_draw(n_int dimension, bronze_plot plot, void *context);
void bronze_hybrid_draw_hud(n_byte4 *rgba, n_int width, n_int height);
const bronze_site *bronze_hybrid_sites(n_uint *count);
const bronze_economy *bronze_hybrid_economy(void);
#endif
