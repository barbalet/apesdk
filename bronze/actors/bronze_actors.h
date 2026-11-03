#ifndef BRONZE_ACTORS_H
#define BRONZE_ACTORS_H
#include "../../universe/universe.h"
#include <stdint.h>
#include <stddef.h>
#include "../work/bronze_work_types.h"

/* IDs are scoped to the owning registry/world, never native slots or names. */
typedef uint64_t BronzeActorId;
#define BRONZE_ACTOR_NONE UINT64_C(0)
#define BRONZE_KIND_NONE UINT32_MAX

typedef struct { uint32_t kind; double value; } BronzeSkill;
typedef struct { uint32_t kind; double amount; } BronzeStock;
typedef struct { uint64_t place; uint32_t kind; double amount; } BronzeReservation;
typedef struct { uint64_t tick; uint32_t from_vocation, to_vocation; } BronzeAssignment;
typedef struct {
    BronzeActorId id;
    uint32_t vocation, task, instruction;
    uint64_t home, workplace, remaining_ticks;
    BronzeActorId mentor, apprentice;
    BronzeSkill* skills; size_t skill_count;
    BronzeStock* resources; size_t resource_count;
    BronzeStock* items; size_t item_count;
    BronzeReservation* reservations; size_t reservation_count;
    BronzeAssignment* history; size_t history_count;
    BronzeWorkState work;
} BronzeActor;
typedef struct BronzeActorRegistry BronzeActorRegistry;
typedef void (*BronzeReleaseClaim)(void* context, BronzeActorId actor, const BronzeReservation* claim);

BronzeActorRegistry* bronze_actors_create(void);
void bronze_actors_destroy(BronzeActorRegistry* registry);
void bronze_actors_invalidate(BronzeActorRegistry* registry);
int bronze_actors_reset(BronzeActorRegistry* registry, simulated_group* group);
void bronze_actors_release_handler(BronzeActorRegistry*, BronzeReleaseClaim, void*);
BronzeActor* bronze_actors_find(BronzeActorRegistry*, BronzeActorId);
BronzeActor* bronze_actors_for_being(BronzeActorRegistry*, const simulated_being*);
simulated_being* bronze_actors_resolve(BronzeActorRegistry*, BronzeActorId);
size_t bronze_actors_count(const BronzeActorRegistry*);
uint64_t bronze_actors_generation(const BronzeActorRegistry*);
BronzeClock* bronze_actors_clock(BronzeActorRegistry*);
BronzeActorId bronze_actors_next_id(const BronzeActorRegistry*);
/* Explicit permutation for hosts that reorder a complete native population.
   The ordered IDs must match the resulting native array; validate before use. */
int bronze_actors_bind_order(BronzeActorRegistry*, simulated_group*, const BronzeActorId*, size_t);
int bronze_actors_rebind(BronzeActorRegistry*, simulated_group*);

/* Active-world routing only; the owner is shared_core_context, or the direct
   headless-engine fallback. All use is at serialized native lifecycle points. */
void bronze_actors_activate(BronzeActorRegistry* registry);
BronzeActorRegistry* bronze_actors_current(void);
void bronze_actors_close_default(void);
int bronze_actors_world_reset(simulated_group* group);
int bronze_actors_birth(simulated_group* group, simulated_being* born);
void bronze_actors_death(simulated_group* group, simulated_being* dead);
void bronze_actors_relocated(simulated_group* group, simulated_being* from, simulated_being* to);
#endif
