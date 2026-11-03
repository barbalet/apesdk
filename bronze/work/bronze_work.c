#include "bronze_work.h"
#include <math.h>

uint64_t bronze_native_tick(void)
{ return (uint64_t)land_date()*TIME_DAY_MINUTES+land_time(); }
int bronze_work_duration(double days, uint64_t* ticks)
{
    double value=ceil(days*TIME_DAY_MINUTES);
    if(!ticks || !isfinite(days) || days<0 || !isfinite(value) || value>=0x1p64) return 0;
    *ticks=value<1?1:(uint64_t)value; return 1;
}
double bronze_clock_days(const BronzeClock* c)
{ return c?(double)c->elapsed/TIME_DAY_MINUTES:0; }
BronzeNeeds bronze_needs_read(simulated_being* b)
{
    BronzeNeeds n={0};
    if(!b) return n;
    n.energy=being_energy(b); n.wakefulness=being_awake(b); n.state=being_state(b);
    n.hunger=being_drive(b,DRIVE_HUNGER)/(double)DRIVES_MAX;
    /* Energy deficit and native hunger drive both matter to scenario rules. */
    double deficit=1.0-(double)n.energy/(double)BEING_FULL;
    if(deficit>n.hunger) n.hunger=deficit;
    if(n.hunger>1) n.hunger=1;
    n.fatigue=being_drive(b,DRIVE_FATIGUE)/(double)DRIVES_MAX;
    n.social=being_drive(b,DRIVE_SOCIAL)/(double)DRIVES_MAX;
    n.sex=being_drive(b,DRIVE_SEX)/(double)DRIVES_MAX;
    return n;
}
BronzeWorkReason bronze_needs_reason(const BronzeNeeds* n)
{
    if(!n) return BRONZE_WORK_DEAD;
    if(n->energy<=BEING_DEAD) return BRONZE_WORK_DEAD;
    if(n->energy<=BEING_HUNGRY || n->hunger>=0.90 ||
       (n->state&(BEING_STATE_HUNGRY|BEING_STATE_SWIMMING|BEING_STATE_EATING))) return BRONZE_WORK_SURVIVAL;
    if(n->wakefulness!=FULLY_AWAKE || n->fatigue>=0.90) return BRONZE_WORK_SLEEP;
    if(n->social>=0.90 || n->sex>=0.90 ||
       (n->state&(BEING_STATE_GROOMING|BEING_STATE_SUCKLING|BEING_STATE_SHOWFORCE|BEING_STATE_ATTACK|BEING_STATE_SPEAKING|BEING_STATE_SHOUTING)))
        return BRONZE_WORK_OBLIGATION;
    return BRONZE_WORK_READY;
}
void bronze_work_hold_position(simulated_being* b)
{
    BronzeActorRegistry* r=bronze_actors_current(); BronzeClock* c=bronze_actors_clock(r);
    BronzeActor* a=bronze_actors_for_being(r,b);
    if(!a || !c || !c->open || c->tick!=bronze_native_tick() || !a->remaining_ticks ||
       !a->work.sampled || a->work.sampled_tick!=c->tick ||
       a->work.initial_reason!=BRONZE_WORK_READY ||
       a->work.x!=being_location_x(b) || a->work.y!=being_location_y(b)) return;
    BronzeNeeds n=bronze_needs_read(b);
    if(bronze_needs_reason(&n)!=BRONZE_WORK_READY) return;
    being_set_speed(b,0);
    being_set_state(b,(being_state_type)(being_state(b)&~BEING_STATE_MOVING));
}
const char* bronze_work_reason_name(BronzeWorkReason r)
{
    static const char* names[]={"available","dead","survival","sleep","biological/social obligation","native movement","no intention","clock discontinuity"};
    return r>=BRONZE_WORK_READY && r<=BRONZE_WORK_CLOCK?names[r]:"invalid";
}
const char* bronze_work_phase_name(BronzeWorkPhase p)
{
    static const char* names[]={"idle","blocked","deferred","started","interrupted","resumed","completed","running"};
    return p>=BRONZE_WORK_IDLE && p<=BRONZE_WORK_RUNNING?names[p]:"invalid";
}
static void record(BronzeActor* a, BronzeWorkReason reason, BronzeWorkPhase phase, uint64_t tick)
{
    if(a->work.reason!=reason || a->work.phase!=phase) {
        a->work.reason=reason; a->work.phase=phase;
        a->work.transition_tick=tick; a->work.transitions++;
        if(reason!=BRONZE_WORK_READY && reason!=BRONZE_WORK_NO_INTENT) a->work.last_block_reason=reason;
        if(phase==BRONZE_WORK_INTERRUPTED) a->work.interruptions++;
    }
}
int bronze_work_request(BronzeActorRegistry* r, BronzeActorId id, uint32_t task, uint64_t ticks)
{
    BronzeActor* a=bronze_actors_find(r,id);
    if(!a || !bronze_actors_resolve(r,id) || task==BRONZE_KIND_NONE || !ticks || a->remaining_ticks) return 0;
    a->task=task; a->remaining_ticks=ticks; a->work.ever_ran=a->work.running=0;
    return 1;
}
void bronze_work_begin(simulated_group* group)
{
    BronzeActorRegistry* r=bronze_actors_current();
    BronzeClock* c=bronze_actors_clock(r);
    uint64_t now=bronze_native_tick();
    n_uint i;
    if(!c || !group) return;
    if(now<c->tick) {
        c->open=0;
        for(i=0;i<group->num && i<LARGE_SIM;i++) {
            BronzeActor* a=bronze_actors_for_being(r,&group->beings[i]);
            if(a) {
                a->work.eligible=0; a->work.running=0;
                record(a,BRONZE_WORK_CLOCK,BRONZE_WORK_DEFERRED,now);
            }
        }
        return;
    }
    if(now==c->tick) return; /* No replay/catch-up, including pause. */
    c->tick=now; c->elapsed=now-c->origin; c->open=1;
    for(i=0;i<group->num && i<LARGE_SIM;i++) {
        simulated_being* b=&group->beings[i];
        BronzeActor* a=bronze_actors_for_being(r,b);
        if(!a) continue;
        BronzeNeeds n=bronze_needs_read(b);
        a->work.sampled=1; a->work.sampled_tick=now;
        a->work.initial_reason=bronze_needs_reason(&n);
        a->work.x=being_location_x(b); a->work.y=being_location_y(b);
        a->work.eligible=a->work.consumed=0;
    }
}
static void consume(BronzeActor* a, simulated_being* b, uint64_t tick)
{
    /* A one-tick remainder can resume and complete in the same interval. */
    if(a->remaining_ticks && !a->work.running && a->work.ever_ran) a->work.resumptions++;
    BronzeWorkPhase phase=a->work.running?BRONZE_WORK_RUNNING:
        a->work.ever_ran?BRONZE_WORK_RESUMED:BRONZE_WORK_STARTED;
    a->work.consumed=1; a->work.units++; a->work.running=a->work.ever_ran=1;
    if (!a->work.resting) being_energy_delta(b,-1); /* Real native cost; no second fatigue/energy store. */
    if(!a->remaining_ticks || --a->remaining_ticks==0) {
        a->work.resting=0;
        phase=BRONZE_WORK_COMPLETED; a->task=BRONZE_KIND_NONE; a->work.running=0;
    }
    record(a,BRONZE_WORK_READY,phase,tick);
}
void bronze_work_end(simulated_group* group)
{
    BronzeActorRegistry* r=bronze_actors_current(); BronzeClock* c=bronze_actors_clock(r);
    n_uint i;
    if(!c || !c->open || c->tick!=bronze_native_tick() || !group) return;
    c->open=0;
    for(i=0;i<group->num && i<LARGE_SIM;i++) {
        simulated_being* b=&group->beings[i]; BronzeActor* a=bronze_actors_for_being(r,b);
        if(!a || !a->work.sampled || a->work.sampled_tick!=c->tick) continue; /* newborn waits */
        BronzeNeeds n=bronze_needs_read(b);
        BronzeWorkReason reason=bronze_needs_reason(&n);
        if(a->work.initial_reason!=BRONZE_WORK_READY &&
           (reason==BRONZE_WORK_READY || a->work.initial_reason<reason)) reason=a->work.initial_reason;
        if(reason==BRONZE_WORK_READY && (being_speed(b) || a->work.x!=being_location_x(b) || a->work.y!=being_location_y(b)))
            reason=BRONZE_WORK_NATIVE_MOVEMENT;
        a->work.eligible=reason==BRONZE_WORK_READY;
        if(reason!=BRONZE_WORK_READY) {
            BronzeWorkPhase phase=a->work.running?BRONZE_WORK_INTERRUPTED:
                reason==BRONZE_WORK_NATIVE_MOVEMENT?BRONZE_WORK_DEFERRED:BRONZE_WORK_BLOCKED;
            a->work.running=0; record(a,reason,phase,c->tick);
        } else if(a->remaining_ticks) consume(a,b,c->tick);
        else if(!a->work.running) record(a,BRONZE_WORK_NO_INTENT,BRONZE_WORK_IDLE,c->tick);
    }
}
int bronze_work_claim(simulated_being* b)
{
    BronzeActorRegistry* r=bronze_actors_current(); BronzeClock* c=bronze_actors_clock(r);
    BronzeActor* a=bronze_actors_for_being(r,b);
    if(!a || !c || c->open || c->tick!=bronze_native_tick() || !a->work.sampled ||
       a->work.sampled_tick!=c->tick || !a->work.eligible || a->work.consumed) return 0;
    consume(a,b,c->tick); return 1;
}
