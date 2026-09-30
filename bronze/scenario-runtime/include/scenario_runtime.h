#ifndef SCENARIO_RUNTIME_H
#define SCENARIO_RUNTIME_H

#include <stdint.h>

/* Public C99 contract for independent scenario simulation adapters. */
#define SCENARIO_RUNTIME_VERSION 1u
#define SCENARIO_ID_NONE UINT32_MAX

typedef uint64_t ScenarioTick;
typedef uint32_t ScenarioActorId;
typedef uint32_t ScenarioPlaceId;

typedef struct { int32_t x; int32_t y; } ScenarioPosition;

typedef enum {
    SCENARIO_RESULT_COMPLETED,
    SCENARIO_RESULT_UNAVAILABLE,
    SCENARIO_RESULT_DEFERRED,
    SCENARIO_RESULT_INVALID
} ScenarioResult;

typedef struct {
    uint32_t version;
    ScenarioTick tick;
    ScenarioActorId actor_id;
    ScenarioActorId counterpart_id;
    ScenarioPlaceId place_id;
    const char* action_id;
    double requested_amount;
    double completed_amount;
    ScenarioResult result;
} ScenarioEvent;

/* Read-only views. Their contexts remain owned by the local adapter. */
typedef struct {
    const void* context;
    ScenarioTick (*tick)(const void* context);
} ScenarioWorldPort;

typedef struct {
    const void* context;
    ScenarioActorId (*id)(const void* context);
    ScenarioPosition (*position)(const void* context);
} ScenarioActorPort;

typedef struct {
    const void* context;
    ScenarioPlaceId (*id)(const void* context);
    ScenarioPosition (*position)(const void* context);
    const char* (*name)(const void* context);
} ScenarioPlacePort;

typedef struct {
    ScenarioEvent events[2];
    ScenarioPosition recipient_position;
    ScenarioPlaceId recipient_place_id;
    int recipient_arrived;
    double delivered_amount;
} ScenarioServiceFixture;

void scenario_event_init(ScenarioEvent* event, ScenarioTick tick,
                         ScenarioActorId actor_id, ScenarioActorId counterpart_id,
                         ScenarioPlaceId place_id, const char* action_id,
                         double requested_amount, double completed_amount,
                         ScenarioResult result);
int scenario_event_equivalent(const ScenarioEvent* left, const ScenarioEvent* right);

/* Emits the portable travel + service vector without mutating either runtime. */
int scenario_service_fixture_run(const ScenarioWorldPort* world,
                                 const ScenarioActorPort* recipient,
                                 const ScenarioPlacePort* place,
                                 ScenarioActorId provider_id,
                                 const char* service_action,
                                 double requested_amount,
                                 ScenarioResult service_result,
                                 ScenarioServiceFixture* fixture);
int scenario_service_fixture_equivalent(const ScenarioServiceFixture* left,
                                        const ScenarioServiceFixture* right);

#endif
