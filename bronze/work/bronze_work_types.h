#ifndef BRONZE_WORK_TYPES_H
#define BRONZE_WORK_TYPES_H
#include <stdint.h>

typedef enum {
    BRONZE_WORK_READY, BRONZE_WORK_DEAD, BRONZE_WORK_SURVIVAL,
    BRONZE_WORK_SLEEP, BRONZE_WORK_OBLIGATION, BRONZE_WORK_NATIVE_MOVEMENT,
    BRONZE_WORK_NO_INTENT, BRONZE_WORK_CLOCK
} BronzeWorkReason;
typedef enum {
    BRONZE_WORK_IDLE, BRONZE_WORK_BLOCKED, BRONZE_WORK_DEFERRED,
    BRONZE_WORK_STARTED, BRONZE_WORK_INTERRUPTED, BRONZE_WORK_RESUMED,
    BRONZE_WORK_COMPLETED, BRONZE_WORK_RUNNING
} BronzeWorkPhase;
typedef struct {
    uint64_t origin, tick, elapsed;
    int open;
} BronzeClock;
typedef struct {
    BronzeWorkReason reason, initial_reason;
    BronzeWorkPhase phase;
    BronzeWorkReason last_block_reason;
    uint64_t sampled_tick, transition_tick, transitions, units;
    uint64_t interruptions, resumptions;
    long x, y;
    int sampled, eligible, consumed, running, ever_ran;
} BronzeWorkState;
#endif
