#ifndef BRONZE_HYBRID_H
#define BRONZE_HYBRID_H
#include "../../entity/entity.h"
#include "bronze_land_policy.h"
#include <stdint.h>

enum { BRONZE_PATCH_LIMIT = 32, BRONZE_SITE_LIMIT = 10, BRONZE_TOWN_LIMIT = 2,
       BRONZE_PLACE_LIMIT = BRONZE_SITE_LIMIT + 2 * BRONZE_TOWN_LIMIT };
typedef struct { n_int x, y; } bronze_land_cell;
typedef struct { double amount, capacity, reserved, daily_renewal; } BronzePatchStock;

typedef enum { BRONZE_TOWN, BRONZE_FIELD, BRONZE_COPPER_MINE,
               BRONZE_TIN_MINE, BRONZE_WOODLAND } bronze_site_kind;
typedef struct {
    n_int x, y; /* ApeSDK low-resolution map coordinates */
    bronze_site_kind kind;
    uint64_t place_id;
    double stock, capacity; /* Totals across scenario resources. */
    BronzePatchStock resources[BRONZE_RESOURCE_LIMIT];
    n_uint area;
    bronze_land_cell cells[BRONZE_PATCH_LIMIT];
} bronze_site;
typedef struct {
    double grain, copper, tin, charcoal, tools;
    n_uint work_events;
} bronze_economy;
typedef enum {
    BRONZE_PLACE_HOME, BRONZE_PLACE_STORAGE, BRONZE_PLACE_SERVICE, BRONZE_PLACE_PRODUCTION
} BronzePlaceKind;

typedef struct {
    uint64_t id, owner;
    BronzePlaceKind kind;
    bronze_land_cell location;
} BronzePlace;

typedef struct {
    uint64_t home, storage, service;
    unsigned residents, capacity;
    bronze_economy stores;
    double resources[BRONZE_RESOURCE_LIMIT], reserved_resources[BRONZE_RESOURCE_LIMIT];
    double items[BRONZE_ITEM_LIMIT], reserved_items[BRONZE_ITEM_LIMIT];
    /* Native map walk distances plus one, with zero for unreachable cells. */
    uint32_t distance[MAP_AREA];
} BronzeTownship;

typedef void (*bronze_plot)(n_int x, n_int y, n_byte color, void *context);
typedef struct {
    BronzeLandPolicy policy;
    bronze_site sites[BRONZE_SITE_LIMIT];
    BronzePlace places[BRONZE_PLACE_LIMIT];
    BronzeTownship towns[BRONZE_TOWN_LIMIT];
    double consumed[BRONZE_RESOURCE_LIMIT], lost[BRONZE_RESOURCE_LIMIT], spoiled[BRONZE_RESOURCE_LIMIT];
    double lost_items[BRONZE_ITEM_LIMIT];
    unsigned place_count, town_count;
    uint64_t next_place_id, next_profession_tick;
    n_uint site_count, last_day;
    uint64_t last_tick;
} BronzeLandscape;
void bronze_hybrid_reset(void);
void bronze_hybrid_cycle(simulated_group *group);
void bronze_hybrid_draw(n_int dimension, bronze_plot plot, void *context);
void bronze_hybrid_draw_hud(n_byte4 *rgba, n_int width, n_int height);
const bronze_site *bronze_hybrid_sites(n_uint *count);
const BronzePlace *bronze_hybrid_place(uint64_t id);
bronze_economy bronze_hybrid_economy(void);
#endif
