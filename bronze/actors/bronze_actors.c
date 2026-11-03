#include "bronze_actors.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct BronzeActorRegistry {
    simulated_group* group; /* Runtime binding, not a world/actor identity. */
    BronzeActor records[LARGE_SIM];
    BronzeActor* slots[LARGE_SIM];
    BronzeActorId next_id;
    uint64_t generation;
    size_t count;
    int native_bound;
    BronzeClock clock;
    BronzeLandscape landscape;
    BronzeProfessionPolicy professions, pending_professions;
    BronzeProgram *program, *pending_program;
    BronzeActorId first_world_id;
    BronzeReleaseClaim release;
    void* release_context;
};
static BronzeActorRegistry* active;
static BronzeActorRegistry* direct_world;

static void dispose(BronzeActorRegistry* r, BronzeActor* actor)
{
    size_t i;
    if(!actor->id) return;
    for(i=0;i<actor->reservation_count;i++)
        if(r->release) r->release(r->release_context,actor->id,&actor->reservations[i]);
    free(actor->skills); free(actor->resources); free(actor->items);
    free(actor->reservations); free(actor->history);
    memset(actor,0,sizeof(*actor));
}
BronzeActorRegistry* bronze_actors_create(void)
{
    BronzeActorRegistry* r=calloc(1,sizeof(*r));
    if(r) {
        r->next_id=1;
        r->landscape.policy=bronze_land_defaults();
    }
    return r;
}
void bronze_actors_destroy(BronzeActorRegistry* r)
{
    size_t i;
    if(!r) return;
    if(active==r) active=NULL;
    if(direct_world==r) direct_world=NULL;
    for(i=0;i<LARGE_SIM;i++) dispose(r,&r->records[i]);
    bronze_program_release(r->program);
    bronze_program_release(r->pending_program);
    free(r);
}
static int is_current(const BronzeActorRegistry* r)
{
    return r && r->native_bound && (active ? active==r : direct_world==r);
}
static int slot_of(const BronzeActorRegistry* r, const simulated_being* being)
{
    size_t i;
    if(!is_current(r) || !r->group || !being) return -1;
    /* Equality search avoids undefined subtraction of unrelated pointers. */
    for(i=0;i<LARGE_SIM;i++) if(being==&r->group->beings[i]) return (int)i;
    return -1;
}
BronzeActor* bronze_actors_for_being(BronzeActorRegistry* r, const simulated_being* being)
{
    int index=slot_of(r,being);
    return index<0?NULL:r->slots[index];
}
BronzeActor* bronze_actors_find(BronzeActorRegistry* r, BronzeActorId id)
{
    size_t i;
    if(!r || !id) return NULL;
    for(i=0;i<LARGE_SIM;i++) if(r->records[i].id==id) return &r->records[i];
    return NULL;
}
simulated_being* bronze_actors_resolve(BronzeActorRegistry* r, BronzeActorId id)
{
    size_t i;
    BronzeActor* actor=bronze_actors_find(r,id);
    if(!actor || !is_current(r) || !r->group) return NULL;
    for(i=0;i<LARGE_SIM;i++) if(r->slots[i]==actor) return &r->group->beings[i];
    return NULL;
}
static int add(BronzeActorRegistry* r, simulated_being* being)
{
    size_t i; int slot=slot_of(r,being);
    if(slot<0) return 0;
    if(r->slots[slot]) return 1; /* Birth notifications are idempotent. */
    if(!r->next_id || r->next_id==UINT64_MAX) return 0;
    for(i=0;i<LARGE_SIM;i++) if(!r->records[i].id) {
        BronzeActor* actor=&r->records[i];
        actor->id=r->next_id++; actor->vocation=BRONZE_KIND_NONE;
        actor->task=BRONZE_KIND_NONE;
        if (r->program)
            actor->execution.rng = ((uint64_t)r->program->config->seed << 32) ^
                                   (actor->id - r->first_world_id);
        r->slots[slot]=actor; r->count++; return 1;
    }
    return 0;
}
void bronze_actors_invalidate(BronzeActorRegistry* r)
{
    size_t i;
    if(!r) return;
    /* A reset may follow native array replacement. Retire physical bindings
       before any claim-release callback can observe the new population. */
    r->native_bound=0; r->group=NULL;
    for(i=0;i<LARGE_SIM;i++) dispose(r,&r->records[i]);
    memset(r->slots,0,sizeof(r->slots)); r->count=0; r->generation++;
    memset(&r->clock,0,sizeof(r->clock));
}
int bronze_actors_reset(BronzeActorRegistry* r, simulated_group* group)
{
    size_t i;
    if(!r) return 0;
    bronze_actors_invalidate(r);
    if(!group || group->num>LARGE_SIM || r->next_id>UINT64_MAX-group->num-1 ||
       (active ? active!=r : direct_world!=r)) return 0;
    r->group=group; r->native_bound=1;
    r->professions=r->pending_professions;
    BronzeProgram *program = bronze_program_retain(r->pending_program);
    bronze_program_release(r->program);
    r->program = program;
    r->first_world_id = r->next_id;
    r->clock.origin=r->clock.tick=(uint64_t)land_date()*TIME_DAY_MINUTES+land_time();
    for(i=0;i<group->num;i++) if(!add(r,&group->beings[i])) return 0;
    return 1;
}
void bronze_actors_release_handler(BronzeActorRegistry* r, BronzeReleaseClaim handler, void* context)
{ if(r) { r->release=handler; r->release_context=context; } }
size_t bronze_actors_count(const BronzeActorRegistry* r) { return r?r->count:0; }
uint64_t bronze_actors_generation(const BronzeActorRegistry* r) { return r?r->generation:0; }
BronzeLandscape* bronze_actors_landscape(BronzeActorRegistry* r) { return r?&r->landscape:NULL; }
BronzeClock* bronze_actors_clock(BronzeActorRegistry* r) { return r?&r->clock:NULL; }
BronzeActorId bronze_actors_next_id(const BronzeActorRegistry* r) { return r?r->next_id:0; }
int bronze_actors_bind_order(BronzeActorRegistry* r, simulated_group* group, const BronzeActorId* ids, size_t count)
{
    BronzeActor* ordered[LARGE_SIM]; size_t i,j;
    if(!r || (active ? active!=r : direct_world!=r) || !group || !ids || count!=r->count || count!=group->num || count>LARGE_SIM) return 0;
    for(i=0;i<count;i++) {
        ordered[i]=bronze_actors_find(r,ids[i]);
        if(!ordered[i]) return 0;
        for(j=0;j<i;j++) if(ordered[j]==ordered[i]) return 0;
    }
    memset(r->slots,0,sizeof(r->slots));
    memcpy(r->slots,ordered,count*sizeof(*ordered)); r->group=group; r->native_bound=1; return 1;
}
int bronze_actors_rebind(BronzeActorRegistry* r, simulated_group* group)
{
    size_t i;
    if(!r) return 0;
    r->native_bound=0;
    if((active ? active!=r : direct_world!=r) || !group || r->count!=group->num) return 0;
    for(i=0;i<group->num;i++) if(!r->slots[i]) return 0;
    r->group=group; r->native_bound=1; return 1;
}
void bronze_actors_activate(BronzeActorRegistry* r)
{
    if(active==r) return;
    if(active) active->native_bound=0;
    else if(direct_world) direct_world->native_bound=0;
    active=r;
    if(r) r->native_bound=0; /* A paired native restore/reset must finish first. */
}
BronzeActorRegistry* bronze_actors_current(void)
{
    return active ? active : direct_world;
}
void bronze_actors_close_default(void) { bronze_actors_destroy(direct_world); }
int bronze_actors_world_reset(simulated_group* group)
{
    if(!active && !direct_world) direct_world=bronze_actors_create();
    return bronze_actors_reset(bronze_actors_current(),group);
}
int bronze_actors_birth(simulated_group* group, simulated_being* born)
{
    BronzeActorRegistry* r=bronze_actors_current();
    if(!r || r->group!=group) return 0;
    int slot=slot_of(r,born);
    if(slot<0 || (n_uint)slot>group->num) return 0;
    if((n_uint)slot<group->num) return r->slots[slot]!=NULL;
    return group->num<group->max ? add(r,born) : 0;
}
void bronze_actors_death(simulated_group* group, simulated_being* dead)
{
    BronzeActorRegistry* r=bronze_actors_current(); size_t i;
    BronzeActor* actor=r && r->group==group?bronze_actors_for_being(r,dead):NULL;
    if(!actor) return;
    BronzeActorId id=actor->id; int slot=slot_of(r,dead);
    r->slots[slot]=NULL;
    for(i=0;i<LARGE_SIM;i++) {
        if(r->records[i].mentor==id) r->records[i].mentor=BRONZE_ACTOR_NONE;
        if(r->records[i].apprentice==id) r->records[i].apprentice=BRONZE_ACTOR_NONE;
    }
    dispose(r,actor); r->count--;
}
void bronze_actors_relocated(simulated_group* group, simulated_being* from, simulated_being* to)
{
    BronzeActorRegistry* r=bronze_actors_current();
    if(!r || r->group!=group || from==to) return;
    int source=slot_of(r,from), target=slot_of(r,to);
    if(source<0 || target<0) return;
    r->slots[target]=r->slots[source]; r->slots[source]=NULL;
}


void bronze_actors_stage_professions(BronzeActorRegistry* registry, const BronzeProfessionPolicy* policy)
{
    if (!registry) return;
    if (policy && policy->count <= BRONZE_VOCATION_LIMIT) registry->pending_professions = *policy;
    else memset(&registry->pending_professions, 0, sizeof(registry->pending_professions));
}

const BronzeProfessionPolicy* bronze_actors_professions(const BronzeActorRegistry* registry)
{
    return registry ? &registry->professions : NULL;
}

static BronzeSkill* skill_for(const BronzeActor* actor, uint32_t vocation)
{
    for (size_t i = 0; i < actor->skill_count; i++)
        if (actor->skills[i].kind == vocation) return &actor->skills[i];
    return NULL;
}

double bronze_actor_skill(const BronzeActor* actor, uint32_t vocation)
{
    const BronzeSkill* skill = skill_for(actor, vocation);
    return skill ? skill->value : 0;
}

int bronze_actor_set_job(BronzeActor* actor, uint32_t vocation, uint64_t workplace,
                         unsigned skill_start, uint64_t tick)
{
    if (actor->vocation == vocation && actor->workplace == workplace) return 1;
    if (vocation != BRONZE_KIND_NONE && !skill_for(actor, vocation)) {
        BronzeSkill* skills = realloc(actor->skills, (actor->skill_count + 1) * sizeof(*skills));
        if (!skills) return 0;
        actor->skills = skills;
        skills[actor->skill_count++] = (BronzeSkill){vocation, skill_start};
    }
    if (actor->vocation != vocation) {
        enum { HISTORY_LIMIT = 32 };
        if (actor->history_count >= HISTORY_LIMIT) {
            memmove(actor->history, actor->history + 1, (HISTORY_LIMIT - 1) * sizeof(*actor->history));
            actor->history_count = HISTORY_LIMIT - 1;
        }
        BronzeAssignment* history = realloc(actor->history, (actor->history_count + 1) * sizeof(*history));
        if (history) {
            actor->history = history;
            history[actor->history_count++] = (BronzeAssignment){tick, actor->vocation, vocation};
        }
    }
    if (actor->execution.depth || actor->execution.operation) {
        actor->execution.depth = 0;
        actor->execution.operation = NULL;
        actor->execution.target = 0;
        actor->execution.status = BRONZE_TASK_CANCELLED;
        actor->execution.retry_at = tick + TIME_HOUR_MINUTES;
    } else {
        actor->execution.name = actor->execution.action = NULL;
        actor->execution.line = 0;
        actor->execution.target = 0;
        actor->execution.status = BRONZE_TASK_IDLE;
        actor->execution.retry_at = 0;
    }
    actor->vocation = vocation;
    actor->workplace = workplace;
    actor->task = BRONZE_KIND_NONE;
    actor->instruction = 0;
    actor->remaining_ticks = 0;
    actor->work.running = actor->work.ever_ran = actor->work.resting = 0;
    return 1;
}

void bronze_actor_practice(BronzeActor* actor, const BronzeProfessionPolicy* policy)
{
    BronzeSkill* skill = skill_for(actor, actor->vocation);
    if (!skill) return;
    skill->value += policy->skill_gain;
    if (skill->value > policy->skill_max) skill->value = policy->skill_max;
}


void bronze_actors_stage_program(BronzeActorRegistry* registry, BronzeProgram* program)
{
    if (!registry) return;
    bronze_program_retain(program);
    bronze_program_release(registry->pending_program);
    registry->pending_program = program;
}

const BronzeProgram* bronze_actors_program(const BronzeActorRegistry* registry)
{
    return registry ? registry->program : NULL;
}
