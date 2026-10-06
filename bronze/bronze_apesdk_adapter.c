#include "bronze_apesdk_adapter.h"

#include <string.h>

static ScenarioTick ape_tick(const void* context)
{ (void)context; return ((ScenarioTick)land_date()*1440u)+(ScenarioTick)land_time(); }
static ScenarioActorId ape_id(const void* context)
{ return ((const BronzeApeActor*)context)->id; }
static ScenarioPosition ape_position(const void* context)
{ simulated_being* being=((const BronzeApeActor*)context)->being; return (ScenarioPosition){being_location_x(being),being_location_y(being)}; }
static ScenarioPlaceId ape_place_id(const void* context)
{ return ((const BronzeApePlace*)context)->id; }
static ScenarioPosition ape_place_position(const void* context)
{ return ((const BronzeApePlace*)context)->position; }
static const char* ape_place_name(const void* context)
{ return ((const BronzeApePlace*)context)->name; }
void bronze_ape_world_view(ScenarioWorldPort* port)
{ port->context=0; port->tick=ape_tick; }
void bronze_ape_actor_view(BronzeApeActor* actor, ScenarioActorPort* port)
{ port->context=actor; port->id=ape_id; port->position=ape_position; }
void bronze_ape_place_view(BronzeApePlace* place, ScenarioPlacePort* port)
{ port->context=place; port->id=ape_place_id; port->position=ape_place_position; port->name=ape_place_name; }
void bronze_ape_sidecar_init(BronzeApeSidecar* sidecar)
{ if(!sidecar) return; *sidecar=(BronzeApeSidecar){0,0,{0}}; }
double bronze_ape_sidecar_add(BronzeApeSidecar* sidecar, int resource, double amount)
{ if(!sidecar || resource<0 || resource>=BRONZE_APE_RESOURCE_COUNT) return 0; sidecar->resources[resource]+=amount; if(sidecar->resources[resource]<0) sidecar->resources[resource]=0; return sidecar->resources[resource]; }
ScenarioTick bronze_ape_day_for_cycle(ScenarioTick ape_cycle)
{ return ape_cycle / BRONZE_APE_CYCLES_PER_DAY; }

static const char* action_name(BronzeApeAction action)
{
    static const char* names[]={"move","gather","craft","deposit","eat","rest"};
    return action>=BRONZE_APE_MOVE && action<=BRONZE_APE_REST ? names[action] : "invalid";
}

ScenarioResult bronze_ape_action_run(const ScenarioWorldPort* world,
                                     const ScenarioActorPort* actor,
                                     BronzeApeSidecar* sidecar,
                                     BronzeApeSettlement* settlement,
                                     BronzeApeAction action, int resource,
                                     double amount, ScenarioEvent* event)
{
    ScenarioResult result=SCENARIO_RESULT_INVALID;
    double completed=0;
    ScenarioTick tick=0;
    ScenarioActorId actor_id=SCENARIO_ID_NONE;
    ScenarioPlaceId place_id=SCENARIO_ID_NONE;
    if(world && world->tick) tick=world->tick(world->context);
    if(actor && actor->id) actor_id=actor->id(actor->context);
    if(settlement) place_id=settlement->place.id;
    if(!world || !actor || !sidecar || !settlement || actor_id==SCENARIO_ID_NONE || amount<0)
        goto done;
    if(action==BRONZE_APE_MOVE) { result=SCENARIO_RESULT_COMPLETED; completed=1; }
    else if(action==BRONZE_APE_GATHER && resource>=0 && resource<BRONZE_APE_RESOURCE_COUNT) {
        sidecar->resources[resource]+=amount; result=SCENARIO_RESULT_COMPLETED; completed=amount;
    } else if(action==BRONZE_APE_CRAFT) {
        if(sidecar->resources[BRONZE_APE_COPPER]>=amount && sidecar->resources[BRONZE_APE_TIN]>=amount && sidecar->resources[BRONZE_APE_CHARCOAL]>=amount) {
            sidecar->resources[BRONZE_APE_COPPER]-=amount; sidecar->resources[BRONZE_APE_TIN]-=amount;
            sidecar->resources[BRONZE_APE_CHARCOAL]-=amount; result=SCENARIO_RESULT_COMPLETED; completed=amount;
        } else result=SCENARIO_RESULT_UNAVAILABLE;
    } else if(action==BRONZE_APE_DEPOSIT && resource>=0 && resource<BRONZE_APE_RESOURCE_COUNT) {
        completed=sidecar->resources[resource]<amount ? sidecar->resources[resource] : amount;
        sidecar->resources[resource]-=completed; settlement->resources[resource]+=completed;
        result=completed==amount ? SCENARIO_RESULT_COMPLETED : SCENARIO_RESULT_UNAVAILABLE;
    } else if(action==BRONZE_APE_EAT && (resource==BRONZE_APE_GRAIN || resource==BRONZE_APE_FISH)) {
        completed=sidecar->resources[resource]<amount ? sidecar->resources[resource] : amount;
        sidecar->resources[resource]-=completed; sidecar->hunger-=completed;
        if(sidecar->hunger<0) sidecar->hunger=0;
        result=completed==amount ? SCENARIO_RESULT_COMPLETED : SCENARIO_RESULT_UNAVAILABLE;
    } else if(action==BRONZE_APE_REST) {
        completed=sidecar->fatigue<amount ? sidecar->fatigue : amount;
        sidecar->fatigue-=completed; result=completed==amount ? SCENARIO_RESULT_COMPLETED : SCENARIO_RESULT_UNAVAILABLE;
    }
done:
    scenario_event_init(event,tick,actor_id,SCENARIO_ID_NONE,place_id,action_name(action),amount,completed,result);
    return result;
}

void bronze_ape_snapshot_capture(ScenarioActorId actor_id, const BronzeApeSidecar* sidecar,
                                 const BronzeApeSettlement* settlement, BronzeApeSnapshot* snapshot)
{
    if(!snapshot) return;
    memset(snapshot,0,sizeof(*snapshot)); snapshot->version=BRONZE_APE_STATE_VERSION; snapshot->actor_id=actor_id;
    if(sidecar) { snapshot->hunger=sidecar->hunger; snapshot->fatigue=sidecar->fatigue; memcpy(snapshot->actor_resources,sidecar->resources,sizeof(snapshot->actor_resources)); }
    if(settlement) memcpy(snapshot->settlement_resources,settlement->resources,sizeof(snapshot->settlement_resources));
}
int bronze_ape_snapshot_apply(const BronzeApeSnapshot* snapshot, ScenarioActorId actor_id,
                              BronzeApeSidecar* sidecar, BronzeApeSettlement* settlement)
{
    if(!snapshot || !sidecar || !settlement || snapshot->version!=BRONZE_APE_STATE_VERSION || snapshot->actor_id!=actor_id) return 0;
    sidecar->hunger=snapshot->hunger; sidecar->fatigue=snapshot->fatigue;
    memcpy(sidecar->resources,snapshot->actor_resources,sizeof(sidecar->resources));
    memcpy(settlement->resources,snapshot->settlement_resources,sizeof(settlement->resources)); return 1;
}
int bronze_ape_snapshot_equal(const BronzeApeSnapshot* left, const BronzeApeSnapshot* right)
{ return left && right && memcmp(left,right,sizeof(*left))==0; }
int bronze_ape_snapshot_write(FILE* stream, const BronzeApeSnapshot* snapshot)
{ return stream && snapshot && snapshot->version==BRONZE_APE_STATE_VERSION && fwrite(snapshot,sizeof(*snapshot),1,stream)==1; }
int bronze_ape_snapshot_read(FILE* stream, BronzeApeSnapshot* snapshot)
{ return stream && snapshot && fread(snapshot,sizeof(*snapshot),1,stream)==1 && snapshot->version==BRONZE_APE_STATE_VERSION; }

typedef struct { uint32_t version; ScenarioTick tick; ScenarioActorId actor_id, counterpart_id; ScenarioPlaceId place_id; uint32_t action; double requested, completed; ScenarioResult result; } BronzeApeEventDisk;
static uint32_t event_action(const char* action) { int i; static const char* names[]={"move","gather","craft","deposit","eat","rest","invalid"}; for(i=0;i<7;i++) if(action && strcmp(action,names[i])==0) return (uint32_t)i; return 6; }
static const char* event_action_name(uint32_t action) { static const char* names[]={"move","gather","craft","deposit","eat","rest","invalid"}; return action<7 ? names[action] : names[6]; }
int bronze_ape_events_write(FILE* stream, const ScenarioEvent* events, size_t count)
{
    size_t i; uint32_t disk_count=(uint32_t)count;
    if(!stream || (!events && count) || fwrite(&disk_count,sizeof(disk_count),1,stream)!=1) return 0;
    for(i=0;i<count;i++){ BronzeApeEventDisk disk={SCENARIO_RUNTIME_VERSION,events[i].tick,events[i].actor_id,events[i].counterpart_id,events[i].place_id,event_action(events[i].action_id),events[i].requested_amount,events[i].completed_amount,events[i].result}; if(fwrite(&disk,sizeof(disk),1,stream)!=1) return 0; } return 1;
}
int bronze_ape_events_read(FILE* stream, ScenarioEvent* events, size_t capacity, size_t* count)
{
    uint32_t disk_count; size_t i; if(count) *count=0;
    if(!stream || !count || fread(&disk_count,sizeof(disk_count),1,stream)!=1 || disk_count>capacity) return 0;
    for(i=0;i<disk_count;i++){ BronzeApeEventDisk disk; if(fread(&disk,sizeof(disk),1,stream)!=1 || disk.version!=SCENARIO_RUNTIME_VERSION) return 0; scenario_event_init(&events[i],disk.tick,disk.actor_id,disk.counterpart_id,disk.place_id,event_action_name(disk.action),disk.requested,disk.completed,disk.result); } *count=disk_count; return 1;
}
