#ifndef BRONZE_APESDK_ADAPTER_H
#define BRONZE_APESDK_ADAPTER_H

#include "scenario-runtime/include/scenario_runtime.h"
#include "../universe/universe.h"
#include <stdio.h>

typedef struct { simulated_being* being; ScenarioActorId id; } BronzeApeActor;
typedef struct {
    ScenarioPlaceId id;
    ScenarioPosition position;
    const char* name;
} BronzeApePlace;
typedef struct { double hunger, fatigue; double resources[6]; } BronzeApeSidecar;
typedef struct {
    BronzeApePlace place;
    double resources[6];
} BronzeApeSettlement;
enum { BRONZE_APE_GRAIN, BRONZE_APE_FISH, BRONZE_APE_WOOD, BRONZE_APE_COPPER, BRONZE_APE_TIN, BRONZE_APE_CHARCOAL, BRONZE_APE_RESOURCE_COUNT };
typedef enum {
    BRONZE_APE_MOVE, BRONZE_APE_GATHER, BRONZE_APE_CRAFT,
    BRONZE_APE_DEPOSIT, BRONZE_APE_EAT, BRONZE_APE_REST
} BronzeApeAction;
#define BRONZE_APE_CYCLES_PER_DAY 1440u
#define BRONZE_APE_STATE_VERSION 1u
typedef struct {
    uint32_t version;
    ScenarioActorId actor_id;
    double hunger, fatigue;
    double actor_resources[BRONZE_APE_RESOURCE_COUNT];
    double settlement_resources[BRONZE_APE_RESOURCE_COUNT];
} BronzeApeSnapshot;
void bronze_ape_world_view(ScenarioWorldPort* port);
void bronze_ape_actor_view(BronzeApeActor* actor, ScenarioActorPort* port);
void bronze_ape_place_view(BronzeApePlace* place, ScenarioPlacePort* port);
void bronze_ape_sidecar_init(BronzeApeSidecar* sidecar);
double bronze_ape_sidecar_add(BronzeApeSidecar* sidecar, int resource, double amount);
ScenarioTick bronze_ape_day_for_cycle(ScenarioTick ape_cycle);
ScenarioResult bronze_ape_action_run(const ScenarioWorldPort* world,
                                     const ScenarioActorPort* actor,
                                     BronzeApeSidecar* sidecar,
                                     BronzeApeSettlement* settlement,
                                     BronzeApeAction action, int resource,
                                     double amount, ScenarioEvent* event);
void bronze_ape_snapshot_capture(ScenarioActorId actor_id, const BronzeApeSidecar* sidecar,
                                 const BronzeApeSettlement* settlement, BronzeApeSnapshot* snapshot);
int bronze_ape_snapshot_apply(const BronzeApeSnapshot* snapshot, ScenarioActorId actor_id,
                              BronzeApeSidecar* sidecar, BronzeApeSettlement* settlement);
int bronze_ape_snapshot_equal(const BronzeApeSnapshot* left, const BronzeApeSnapshot* right);
int bronze_ape_snapshot_write(FILE* stream, const BronzeApeSnapshot* snapshot);
int bronze_ape_snapshot_read(FILE* stream, BronzeApeSnapshot* snapshot);
int bronze_ape_events_write(FILE* stream, const ScenarioEvent* events, size_t count);
int bronze_ape_events_read(FILE* stream, ScenarioEvent* events, size_t capacity, size_t* count);

#endif
