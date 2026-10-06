#include "brz_port.h"
#include "brz_world.h"
#include "brz_agent.h"
#include "brz_settlement.h"
#include <stdlib.h>
#include <string.h>

typedef struct { BrzWorld* world; size_t resource_count; } LocalWorld;
typedef struct { const BrzSettlement* settlements; int count; } LocalSettlements;

static uint16_t local_tags_at(void* context, BrzPos position)
{ return brz_world_tags_at(((LocalWorld*)context)->world,position); }
static double local_take(void* context, BrzPos position, int resource, double amount)
{ LocalWorld* world=(LocalWorld*)context; return brz_world_take(world->world,position,world->resource_count,resource,amount); }
static BrzPos local_nearest(void* context, BrzPos from, uint16_t tag, int radius)
{ return brz_world_find_nearest_tag(((LocalWorld*)context)->world,from,tag,radius); }
static void local_regen(void* context)
{ LocalWorld* world=(LocalWorld*)context; brz_world_step_regen(world->world,world->resource_count); }

void bronze_world_port_init(BronzeWorldPort* port, void* world, size_t resource_count)
{
    LocalWorld* local;
    if(!port) return;
    local=(LocalWorld*)malloc(sizeof(*local));
    if(!local){ memset(port,0,sizeof(*port)); return; }
    local->world=(BrzWorld*)world; local->resource_count=resource_count;
    port->context=local; port->tags_at=local_tags_at; port->take=local_take;
    port->nearest_tag=local_nearest; port->step_regen=local_regen;
}

void bronze_world_port_destroy(BronzeWorldPort* port)
{
    if(!port) return;
    free(port->context); memset(port,0,sizeof(*port));
}

static uint32_t local_actor_id(void* context) { return ((BrzAgent*)context)->id; }
static BrzPos local_actor_position(void* context) { return ((BrzAgent*)context)->pos; }
static void local_actor_set_position(void* context, BrzPos pos) { ((BrzAgent*)context)->pos=pos; }
static double local_actor_need(void* context, const char* name)
{ BrzAgent* actor=(BrzAgent*)context; return strcmp(name,"hunger")==0 ? actor->hunger : (strcmp(name,"fatigue")==0 ? actor->fatigue : 0.0); }

void bronze_actor_port_init(BronzeActorPort* port, void* agent)
{
    if(!port) return;
    port->context=agent; port->id=local_actor_id; port->position=local_actor_position;
    port->set_position=local_actor_set_position; port->need=local_actor_need;
}

static int local_settlement_nearest(void* context, BrzPos position)
{ LocalSettlements* settlements=(LocalSettlements*)context; return brz_find_nearest_settlement(settlements->settlements,settlements->count,position); }
void bronze_settlement_port_init(BronzeSettlementPort* port, const void* settlements, int count)
{
    LocalSettlements* local;
    if(!port) return;
    local=(LocalSettlements*)malloc(sizeof(*local));
    if(!local){ memset(port,0,sizeof(*port)); return; }
    local->settlements=(const BrzSettlement*)settlements; local->count=count;
    port->context=local; port->nearest=local_settlement_nearest;
}

void bronze_settlement_port_destroy(BronzeSettlementPort* port)
{
    if(!port) return;
    free(port->context); memset(port,0,sizeof(*port));
}

void bronze_event_emit(const BronzeEventSink* sink, BronzeEventKind kind,
                       uint32_t actor_id, int settlement_id, const char* subject,
                       double amount, int day)
{
    if(!sink || !sink->emit) return;
    BronzeEvent event;
    event.kind=kind; event.actor_id=actor_id; event.settlement_id=settlement_id;
    event.subject=subject; event.amount=amount; event.day=day;
    sink->emit(sink->context,&event);
}
