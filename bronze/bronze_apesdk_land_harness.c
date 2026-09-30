#include "bronze_apesdk_adapter.h"
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

enum { TRACE_LEN=7 };

static void run_slice(ScenarioEvent trace[TRACE_LEN])
{
    ScenarioWorldPort world;
    ScenarioActorPort actor_port;
    BronzeApeActor actor;
    BronzeApeSidecar sidecar;
    BronzeApeSettlement settlement;
    simulated_group *group;
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

    bronze_ape_sidecar_init(&sidecar);
    settlement=(BronzeApeSettlement){{7,{320,-80},"River settlement"},{0}};
    sidecar.hunger=2; sidecar.fatigue=2;
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_MOVE,-1,1,&trace[0])==SCENARIO_RESULT_COMPLETED);
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_GATHER,BRONZE_APE_GRAIN,2,&trace[1])==SCENARIO_RESULT_COMPLETED);
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_GATHER,BRONZE_APE_FISH,1,&trace[2])==SCENARIO_RESULT_COMPLETED);
    bronze_ape_sidecar_add(&sidecar,BRONZE_APE_COPPER,1);
    bronze_ape_sidecar_add(&sidecar,BRONZE_APE_TIN,1);
    bronze_ape_sidecar_add(&sidecar,BRONZE_APE_CHARCOAL,1);
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_CRAFT,-1,1,&trace[3])==SCENARIO_RESULT_COMPLETED);
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_DEPOSIT,BRONZE_APE_GRAIN,1,&trace[4])==SCENARIO_RESULT_COMPLETED);
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_EAT,BRONZE_APE_FISH,1,&trace[5])==SCENARIO_RESULT_COMPLETED);
    assert(bronze_ape_action_run(&world,&actor_port,&sidecar,&settlement,BRONZE_APE_REST,-1,1,&trace[6])==SCENARIO_RESULT_COMPLETED);
    assert(settlement.resources[BRONZE_APE_GRAIN]==1);
    assert(sidecar.hunger==1 && sidecar.resources[BRONZE_APE_FISH]==0 && sidecar.fatigue==1);
    assert(strcmp(trace[3].action_id,"craft")==0 && trace[3].completed_amount==1);
    assert(trace[0].tick==trace[6].tick && trace[0].actor_id==23);
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
