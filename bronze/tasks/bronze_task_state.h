#ifndef BRONZE_TASK_STATE_H
#define BRONZE_TASK_STATE_H
#include "bronze_program.h"

typedef enum {
    BRONZE_TASK_IDLE, BRONZE_TASK_RUNNING, BRONZE_TASK_COMPLETE,
    BRONZE_TASK_NO_RULE, BRONZE_TASK_TRAVEL, BRONZE_TASK_STOCK,
    BRONZE_TASK_UNSUPPORTED, BRONZE_TASK_CANCELLED, BRONZE_TASK_TARGET
} BronzeTaskStatus;

typedef struct {
    const BrzVec *statements;
    size_t index;
} BronzeTaskFrame;

typedef struct {
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
