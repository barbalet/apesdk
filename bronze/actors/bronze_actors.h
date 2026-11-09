#ifndef BRONZE_ACTORS_H
#define BRONZE_ACTORS_H
#include "../../universe/universe.h"
#include <stdint.h>
#include <stddef.h>
#include "../work/bronze_work_types.h"
#include "../hybrid/bronze_hybrid.h"
#include "bronze_profession_policy.h"
#include "../tasks/bronze_task_state.h"

/* IDs are scoped to the owning registry/world, never native slots or names. */
typedef uint64_t BronzeActorId;
#define BRONZE_ACTOR_NONE UINT64_C(0)
#define BRONZE_KIND_NONE UINT32_MAX

typedef struct { uint32_t kind; double value; } BronzeSkill;
typedef struct { uint32_t kind; double amount; } BronzeStock;
typedef struct { uint64_t place; uint32_t kind; double amount; } BronzeReservation;
typedef struct { uint64_t tick; uint32_t from_vocation, to_vocation; } BronzeAssignment;
typedef enum {
    BRONZE_JOB_DISABLED, BRONZE_JOB_ASSIGNED, BRONZE_JOB_CHILD, BRONZE_JOB_NO_HOME,
    BRONZE_JOB_NO_WORKPLACE, BRONZE_JOB_FULL, BRONZE_JOB_MEMORY
} BronzeJobStatus;
typedef struct {
    BronzeActorId id;
    uint32_t vocation, task, instruction;
    uint32_t occupation; /* Selected social role; does not grant task eligibility. */
    BronzeJobStatus job_status;
    uint64_t home, workplace, remaining_ticks;
    BronzeActorId mentor, apprentice;
    BronzeSkill* skills; size_t skill_count;
    BronzeStock* resources; size_t resource_count;
    BronzeStock* items; size_t item_count;
    BronzeReservation* reservations; size_t reservation_count;
    BronzeAssignment* history; size_t history_count;
    uint64_t household_home;
    double household_resources[BRONZE_RESOURCE_LIMIT], household_items[BRONZE_ITEM_LIMIT];
    n_byte4 ration_day;
    double ration_used;
    /* Diagnostic outcome and memory rate limits follow actor identity. */
    BronzeActorId economic_partner;
    uint64_t economic_memory_after, work_memory_after;
    BronzeTaskStatus economic_status;
    int economic_action;
    BronzeWorkState work;
    BronzeTaskState execution;
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
#ifdef CANTERBURY_SIM
size_t bronze_actors_canterbury_storage_bytes(void);
size_t bronze_actors_canterbury_checkpoint_bytes(size_t);
int bronze_actors_canterbury_checkpoint(BronzeActorRegistry *,void *,size_t);
int bronze_actors_canterbury_checkpoint_stage(BronzeActorRegistry *,const void *,size_t,simulated_group *);
size_t bronze_actors_canterbury_record_slot(const BronzeActorRegistry *,BronzeActorId);
/* Initialize zeroed caller storage; destroy clears sidecars but does not free it. */
BronzeActorRegistry *bronze_actors_canterbury_init_storage(void *,size_t);
int bronze_actors_canterbury_seed_ids(BronzeActorRegistry *,BronzeActorId);
/* Read staged serialization order without activating or binding a registry. */
BronzeActorId bronze_actors_canterbury_slot_id(const BronzeActorRegistry *,size_t);
#endif
uint64_t bronze_actors_generation(const BronzeActorRegistry*);
BronzeLandscape* bronze_actors_landscape(BronzeActorRegistry*);
void bronze_actors_stage_program(BronzeActorRegistry*, BronzeProgram*);
const BronzeProgram* bronze_actors_program(const BronzeActorRegistry*);
void bronze_actors_stage_professions(BronzeActorRegistry*, const BronzeProfessionPolicy*);
const BronzeProfessionPolicy* bronze_actors_professions(const BronzeActorRegistry*);
int bronze_actor_set_job(BronzeActorRegistry*, BronzeActor*, uint32_t vocation, uint64_t workplace, unsigned skill_start, uint64_t tick);
double bronze_actor_skill(const BronzeActor*, uint32_t vocation);
void bronze_actor_practice(BronzeActor*, const BronzeProfessionPolicy*);
BronzeClock* bronze_actors_clock(BronzeActorRegistry*);
BronzeActorId bronze_actors_next_id(const BronzeActorRegistry*);
enum { BRONZE_SCENARIO_DIGEST_BYTES = 32 };
/* The caller supplies the digest of the active scenario and pairs the payload
   with its native snapshot. Pass content length in n_file.location when
   staging. The staged registry is independent and remains unbound until
   the caller activates it and rebinds it to the matching native population. */
#ifdef BRONZE_SIDECAR_TEST
n_file *bronze_actors_capture_legacy_sidecar(BronzeActorRegistry *,const n_byte *);
#endif
n_file *bronze_actors_capture_sidecar(BronzeActorRegistry *, const n_byte *scenario_digest);
BronzeActorRegistry *bronze_actors_stage_sidecar(const BronzeActorRegistry *scenario,
                                                const n_file *, const n_byte *scenario_digest);
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
