#ifndef BRONZE_LAND_POLICY_H
#define BRONZE_LAND_POLICY_H

enum { BRONZE_RESOURCE_LIMIT = 64, BRONZE_ITEM_LIMIT = 64 };

typedef struct {
    unsigned field_slope, field_coast, field_moisture;
    unsigned woodland_slope, woodland_moisture;
    unsigned ore_relief, tin_height;
    unsigned town_capacity, town_initial_grain, patch_initial_per_cell;
    unsigned carry_capacity, storage_capacity;
    unsigned household_capacity, ration_daily;
    double food_spoilage;
} BronzeLandPolicy;

static inline BronzeLandPolicy bronze_land_defaults(void)
{
    return (BronzeLandPolicy){6, 48, 64, 18, 45, 12, 205, 128, 0, 0, 32, 512, 0, 0, 0};
}
#endif
