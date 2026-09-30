#include "bronze_apesdk_adapter.h"
#include "bronze_apesdk_compiled.h"
#include "dsl/brz_parser.h"
#include "../sim/sim.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* The harness deliberately uses ApeSDK without a graphical front end. */
n_int draw_error(n_constant_string error_text, n_constant_string location,
                 n_int line_number)
{
    (void)error_text;
    (void)location;
    (void)line_number;
    return 0;
}

enum { TRACE_LEN=14 };

static void run_slice(ScenarioEvent trace[TRACE_LEN])
{
    ScenarioWorldPort world;
    ScenarioActorPort actor_port;
    BronzeApeActor actor;
    BronzeApeSidecar sidecar;
    BronzeApeSettlement settlement;
    simulated_group *group;
    ParsedConfig config;
    int count=0;
    bronze_ape_world_view(&world);
    assert(sim_init(KIND_START_UP,0x42u,MAP_AREA,0)!=0);
    sim_cycle(); /* ApeSDK creates the initial group on this first cycle. */
    group=sim_group();
    assert(group!=0 && group->num>0);
    actor=(BronzeApeActor){&group->beings[0],23};
    bronze_ape_actor_view(&actor,&actor_port);
    assert(actor_port.id(actor_port.context)==23);
    assert(actor_port.position(actor_port.context).x==being_location_x(actor.being));
    assert(actor_port.position(actor_port.context).y==being_location_y(actor.being));

    brz_cfg_init(&config);
    assert(brz_parse_file(BRONZE_APE_CONTENT_PATH,&config));
    assert(brz_cfg_compile(&config,stderr));
    bronze_ape_sidecar_init(&sidecar);
    settlement=(BronzeApeSettlement){{7,{320,-80},"River settlement"},{0}};
    sidecar.hunger=2; sidecar.fatigue=2;
    count+=bronze_ape_execute_compiled_task(&config,"farmer","work",&world,&actor_port,&sidecar,&settlement,&trace[count],TRACE_LEN-count);
    count+=bronze_ape_execute_compiled_task(&config,"fisher","work",&world,&actor_port,&sidecar,&settlement,&trace[count],TRACE_LEN-count);
    count+=bronze_ape_execute_compiled_task(&config,"smith","work",&world,&actor_port,&sidecar,&settlement,&trace[count],TRACE_LEN-count);
    assert(count==TRACE_LEN);
    assert(settlement.resources[BRONZE_APE_GRAIN]==1);
    assert(sidecar.hunger==1);
    assert(sidecar.resources[BRONZE_APE_FISH]==0);
    assert(sidecar.fatigue==1);
    assert(strcmp(trace[12].action_id,"craft")==0 && trace[12].completed_amount==1);
    assert(trace[0].tick==trace[TRACE_LEN-1].tick && trace[0].actor_id==23);
    brz_cfg_free(&config);
    sim_close();
}

int main(void)
{
    n_byte2 seed[2]={17,29};
    ScenarioWorldPort world;
    ScenarioPlacePort place_port;
    BronzeApePlace place={7,{320,-80},"River settlement"};
    BronzeApeSidecar sidecar;
    ScenarioEvent first[TRACE_LEN], second[TRACE_LEN];
    int i;
    land_load_state(42,0,seed); bronze_ape_world_view(&world);
    assert(world.tick(world.context)==((ScenarioTick)42*1440u));
    land_cycle();
    assert(world.tick(world.context)==((ScenarioTick)42*1440u)+1u);
    bronze_ape_place_view(&place,&place_port);
    assert(place_port.id(place_port.context)==7);
    assert(place_port.position(place_port.context).x==320);
    assert(place_port.position(place_port.context).y==-80);
    assert(strcmp(place_port.name(place_port.context),"River settlement")==0);

    run_slice(first);
    run_slice(second);
    for(i=0;i<TRACE_LEN;i++) assert(scenario_event_equivalent(&first[i],&second[i]));
    bronze_ape_sidecar_init(&sidecar);
    assert(bronze_ape_sidecar_add(&sidecar,BRONZE_APE_GRAIN,2)==2);
    assert(bronze_ape_sidecar_add(&sidecar,BRONZE_APE_FISH,1)==1);
    assert(bronze_ape_sidecar_add(&sidecar,BRONZE_APE_COPPER,1)==1);
    puts("Bronze ApeSDK land harness passed."); return 0;
}
