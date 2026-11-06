#ifndef BRONZE_HYBRID_H
#define BRONZE_HYBRID_H
#include "../../entity/entity.h"
#include "bronze_land_policy.h"
#include <stdint.h>

enum { BRONZE_PATCH_LIMIT = 32, BRONZE_SITE_LIMIT = 10, BRONZE_TOWN_LIMIT = 2,
       BRONZE_PLACE_LIMIT = BRONZE_SITE_LIMIT + 6 * BRONZE_TOWN_LIMIT };
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

/* Spatial records are Bronze-owned derived layout data (version 1).
 * IDs are scoped to the landscape; owner is the existing home place ID. */
enum { BRONZE_BUILDINGS_PER_TOWN = 40,
       BRONZE_BUILDING_LIMIT = BRONZE_TOWN_LIMIT * BRONZE_BUILDINGS_PER_TOWN,
       BRONZE_FOOTPRINT_LIMIT = 9, BRONZE_PATH_LIMIT = 128,
       BRONZE_PATH_CELL_LIMIT = 256, BRONZE_LAYOUT_RADIUS = 14,
       BRONZE_HOUSE_CAPACITY = 8, BRONZE_LAYOUT_SLOPE = 6 };
typedef enum { BRONZE_BUILDING_PLAZA, BRONZE_BUILDING_STORE,
               BRONZE_BUILDING_HOME, BRONZE_BUILDING_WORK_YARD } BronzeBuildingKind;
typedef enum { BRONZE_PREMISE_NONE, BRONZE_PREMISE_METAL, BRONZE_PREMISE_POTTERY,
               BRONZE_PREMISE_WOOD, BRONZE_PREMISE_FOOD } BronzePremiseKind;
typedef struct {
    uint64_t id, owner, compatibility_place;
    BronzeBuildingKind kind;
    BronzePremiseKind profession;
    unsigned area, capacity;
    bronze_land_cell cells[BRONZE_FOOTPRINT_LIMIT];
    bronze_land_cell entrance, work_position;
} BronzeBuilding;

/* Stable world-space bearing, derived from persisted identity. Keeping the
 * full rotated structure inside its reserved cells preserves save geometry. */
static inline unsigned bronze_building_bearing(const BronzeBuilding *building)
{
    uint64_t seed=building->id*UINT64_C(0x9e3779b97f4a7c15);
    seed^=seed>>29; seed*=UINT64_C(0xbf58476d1ce4e5b9); seed^=seed>>31;
    return (unsigned)((seed>>16)%4)*90+12+(unsigned)(seed%67);
}

typedef enum { BRONZE_PATH_FOOT, BRONZE_PATH_EARTH_ROAD } BronzePathSurface;
typedef struct {
    uint64_t id, owner, from_building, to_building, to_place;
    BronzePathSurface surface;
    unsigned width, count;
    bronze_land_cell cells[BRONZE_PATH_CELL_LIMIT];
} BronzePath;
typedef struct {
    uint64_t owner;
    unsigned requested_housing, placed_housing, missing_housing;
    unsigned missing_store, missing_plaza, missing_work_yards;
    unsigned missing_premises, unreachable_paths, limited_paths;
} BronzeLayoutReport;
typedef struct {
    unsigned version, building_count, path_count;
    BronzeBuilding buildings[BRONZE_BUILDING_LIMIT];
    BronzePath paths[BRONZE_PATH_LIMIT];
    BronzeLayoutReport reports[BRONZE_TOWN_LIMIT];
    /* Derived O(1) lookups, rebuilt with the immutable layout. */
    uint16_t building_at[MAP_AREA], entrance_at[MAP_AREA];
    n_byte solid[MAP_AREA], track[MAP_AREA], track_links[MAP_AREA];
} BronzeSettlementLayout;

typedef void (*bronze_plot)(n_int x, n_int y, n_byte color, void *context);
typedef struct {
    BronzeSettlementLayout settlement;
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
/* Deterministic, bounded layout generation. Does not move compatibility places,
 * change membership capacity, or change native terrain/movement. */
void bronze_townships_layout(BronzeLandscape *land, const n_byte *heights);
const BronzeSettlementLayout *bronze_hybrid_settlement(void);
int bronze_townships_validate_heights(const BronzeSettlementLayout *layout,const n_byte *heights);
void bronze_townships_index(BronzeSettlementLayout *layout);
void bronze_townships_paths(BronzeLandscape *land, const n_byte *heights);
/* Service jobs may use a specific premise; zero means unavailable. */
uint64_t bronze_township_premise(uint64_t home, unsigned vocation);
void bronze_townships_prepare(void);
void bronze_hybrid_reset(void);
void bronze_hybrid_cycle(simulated_group *group);
void bronze_hybrid_draw(n_int dimension, bronze_plot plot, void *context);
void bronze_hybrid_draw_hud(n_byte4 *rgba, n_int width, n_int height);
const bronze_site *bronze_hybrid_sites(n_uint *count);
const BronzePlace *bronze_hybrid_place(uint64_t id);
bronze_economy bronze_hybrid_economy(void);
#endif
