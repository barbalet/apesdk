#ifndef BRONZE_WORK_H
#define BRONZE_WORK_H
#include "../actors/bronze_actors.h"
#include "../../entity/entity.h"

/* Observations only: no second biological state is integrated or saved. */
typedef struct {
    n_int energy;
    n_byte wakefulness;
    n_byte2 state;
    double hunger, fatigue, social, sex;
} BronzeNeeds;
uint64_t bronze_native_tick(void);
/* Duration intentions use scenario days: ceil(days * TIME_DAY_MINUTES), min 1.
   Numeric DSL quantities are not implicitly interpreted as durations.
   Invalid/overflowing input is rejected rather than truncated. */
int bronze_work_duration(double days, uint64_t* ticks);
double bronze_clock_days(const BronzeClock* clock);
BronzeNeeds bronze_needs_read(simulated_being* being);
BronzeWorkReason bronze_needs_reason(const BronzeNeeds* needs);
const char* bronze_work_reason_name(BronzeWorkReason reason);
const char* bronze_work_phase_name(BronzeWorkPhase phase);
/* Submit a stationary duration intention; no movement or DSL dispatch here.
   Refuses replacement of an outstanding intention. */
int bronze_work_request(BronzeActorRegistry*, BronzeActorId, uint32_t task, uint64_t ticks);
/* Called only by sim_cycle, bracketing native biology and action execution. */
void bronze_work_begin(simulated_group* group);
/* After native awake biology/social decisions, before movement integration:
   a safe stationary intention may own locomotion for this interval. */
void bronze_work_hold_position(simulated_being* being);
void bronze_work_end(simulated_group* group);
/* Prototype production consumes the same one-unit budget as a task. */
int bronze_work_claim(simulated_being* being);
#ifdef CANTERBURY_SIM
/* Serialized city callback only. Sample with bronze_work_begin, explicitly
   nominate stationary work after city needs, then close without auto-dispatch. */
int bronze_work_canterbury_claim(simulated_being*, BronzeWorkReason);
void bronze_work_canterbury_close(void);
#endif
#endif
