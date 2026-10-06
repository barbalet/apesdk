#ifndef APESDK_BRONZE_ADAPTER_H
#define APESDK_BRONZE_ADAPTER_H

#include "../../entity/entity.h"

/* This is intentionally self-contained: BronzeSim's richer port interfaces
   can bind here without requiring apeSDK core headers to know Bronze types. */
typedef struct { n_int x, y; } bronze_ape_position;
typedef struct {
    simulated_being *being;
} bronze_ape_actor;

typedef struct {
    n_int (*terrain_height)(n_int x, n_int y);
    n_byte (*is_water)(n_int x, n_int y);
    n_byte4 (*date)(void);
    n_byte4 (*time)(void);
} bronze_ape_world;

void bronze_ape_actor_init(bronze_ape_actor *actor, simulated_being *being);
bronze_ape_position bronze_ape_actor_position(const bronze_ape_actor *actor);
void bronze_ape_actor_set_position(bronze_ape_actor *actor, bronze_ape_position position);
n_int bronze_ape_actor_energy(const bronze_ape_actor *actor);
n_byte bronze_ape_actor_fatigue(const bronze_ape_actor *actor);
void bronze_ape_world_init(bronze_ape_world *world);

#endif
