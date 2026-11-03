#ifndef BRONZE_TASK_STATE_H
#define BRONZE_TASK_STATE_H
#include "bronze_program.h"

typedef enum {
    BRONZE_TASK_IDLE, BRONZE_TASK_RUNNING, BRONZE_TASK_COMPLETE,
    BRONZE_TASK_NO_RULE, BRONZE_TASK_TRAVEL, BRONZE_TASK_STOCK,
    BRONZE_TASK_UNSUPPORTED, BRONZE_TASK_CANCELLED, BRONZE_TASK_TARGET,
    BRONZE_TASK_ROUTE_BLOCKED, BRONZE_TASK_ROUTE_PAUSED, BRONZE_TASK_ARRIVED,
    BRONZE_TASK_CAPACITY, BRONZE_TASK_RATION, BRONZE_TASK_TOOL, BRONZE_TASK_SKILL
} BronzeTaskStatus;

typedef struct {
    const BrzVec *statements;
    size_t index;
} BronzeTaskFrame;

enum { BRONZE_ROUTE_LIMIT = 256 };
typedef struct {
    uint64_t tick;
    uint32_t from, cells[BRONZE_ROUTE_LIMIT];
    unsigned length, next, stalled, attempts;
} BronzeRoute;

typedef struct {
    BronzeRoute route;
    uint64_t claim_place, claim_until;
    unsigned claim_kind;
    double claim_amount;
    const RecipeDef *craft_recipe;
    uint64_t craft_town;
    unsigned craft_batches;
    uint64_t rng, retry_at, prepared_tick, target;
    uint32_t task_index;
    unsigned depth;
    BronzeTaskFrame frames[65];
    const OpDef *operation;
    const char *name, *action;
    int line;
    BronzeTaskStatus status;
} BronzeTaskState;
#endif
