#include "bronze_actors.h"
#include "../../entity/entity.h"
#include "../../shared.h"
#include "../../gui/gui.h"
#include "../adapter/bronze_apesdk_adapter.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int claims, deaths, births;
static BronzeActorId newborn;
static BronzeActorRegistry* claims_owner;
static void release_claim(void* context, BronzeActorId actor, const BronzeReservation* claim)
{ assert(context==&claims && actor && claim->amount==2); assert(!bronze_actors_resolve(claims_owner,actor)); claims++; }
static void death_callback(simulated_being* being, void* context)
{ (void)context; assert(bronze_actors_for_being(bronze_actors_current(),being)); deaths++; }
static void birth_callback(simulated_being* being, simulated_being* mother, void* context)
{
    (void)mother; (void)context;
    BronzeActor* actor=bronze_actors_for_being(bronze_actors_current(),being);
    assert(actor); newborn=actor->id; births++;
}
static void allocate_claim(BronzeActor* actor)
{
    actor->reservations=calloc(1,sizeof(*actor->reservations)); assert(actor->reservations);
    actor->reservation_count=1; actor->reservations[0].amount=2;
}
static void lifecycle(void)
{
    simulated_group* group;
    BronzeActorRegistry* registry;
    BronzeActorId ids[LARGE_SIM];
    n_uint i;
    assert(sim_init(KIND_START_UP,42,MAP_AREA,0)); sim_cycle(); group=sim_group();
    registry=bronze_actors_current(); assert(bronze_actors_count(registry)==group->num);
    for(i=0;i<group->num;i++) assert(bronze_actors_for_being(registry,&group->beings[i])->id==i+1);
    assert(bronze_actors_next_id(registry)==group->num+1);
    /* Deliberately identical names, birth dates AND genetics: still distinct IDs. */
    memcpy(&group->beings[1],&group->beings[0],sizeof(simulated_being));
    memcpy(&group->beings[2],&group->beings[0],sizeof(simulated_being)); group->num=3;
    assert(bronze_actors_world_reset(group));
    for(i=0;i<3;i++) ids[i]=bronze_actors_for_being(registry,&group->beings[i])->id;
    assert(ids[0]!=ids[1] && ids[1]!=ids[2]);
    BronzeActor* first=bronze_actors_find(registry,ids[0]);
    BronzeActor* second=bronze_actors_find(registry,ids[1]);
    BronzeActor* third=bronze_actors_find(registry,ids[2]);
    assert(first->vocation==BRONZE_KIND_NONE && first->task==BRONZE_KIND_NONE);
    second->vocation=7; second->home=9; second->workplace=12; second->task=4;
    second->instruction=3; second->remaining_ticks=61; second->mentor=first->id;
    third->apprentice=first->id;
    second->skills=calloc(1,sizeof(*second->skills)); assert(second->skills);
    second->skill_count=1; second->skills[0].value=23;
    second->items=calloc(1,sizeof(*second->items)); assert(second->items);
    second->item_count=1; second->items[0].amount=5;
    second->history=calloc(1,sizeof(*second->history)); assert(second->history);
    second->history_count=1; second->history[0].to_vocation=7;
    second->resources=calloc(1,sizeof(*second->resources)); assert(second->resources);
    second->resource_count=1; second->resources[0].amount=17;
    claims_owner=registry;
    allocate_claim(first); bronze_actors_release_handler(registry,release_claim,&claims);
    group->ext_death=death_callback;
    being_energy_delta(&group->beings[0],-being_energy(&group->beings[0]));
    being_remove_loop2_struct* removal=being_remove_initial(group);
    loop_no_thread(group,NULL,being_remove_loop1,NULL);
    loop_no_thread(group,NULL,being_remove_loop2,removal);
    group->num=removal->count; memory_free((void**)&removal); being_remove_internal_clear();
    assert(deaths==1 && claims==1 && group->num==2 && bronze_actors_count(registry)==2);
    assert(!bronze_actors_find(registry,ids[0]) && !bronze_actors_resolve(registry,ids[0]));
    assert(bronze_actors_for_being(registry,&group->beings[0])==second);
    assert(bronze_actors_for_being(registry,&group->beings[1])==third);
    assert(second->vocation==7 && second->resources[0].amount==17 && second->instruction==3);
    assert(second->skills[0].value==23 && second->items[0].amount==5 && second->history[0].to_vocation==7);
    assert(second->remaining_ticks==61 && !second->mentor && !third->apprentice);
    assert(group->ext_death==death_callback);
    sim_set_select(&group->beings[1]); assert(bronze_actors_for_being(registry,group->select)==third);
    simulated_being* tmp=malloc(sizeof(*tmp)); assert(tmp);
    memcpy(tmp,&group->beings[0],sizeof(*tmp));
    memcpy(&group->beings[0],&group->beings[1],sizeof(*tmp));
    memcpy(&group->beings[1],tmp,sizeof(*tmp)); free(tmp);
    BronzeActorId order[2]={ids[2],ids[1]}, invalid[2]={ids[1],ids[1]};
    assert(!bronze_actors_bind_order(registry,group,invalid,2));
    BronzeActorId unknown[2]={ids[1],UINT64_MAX};
    assert(!bronze_actors_bind_order(registry,group,unknown,2));
    assert(bronze_actors_bind_order(registry,group,order,2));
    assert(bronze_actors_resolve(registry,ids[1])==&group->beings[1]);
    assert(bronze_actors_for_being(registry,&group->beings[1])->resources[0].amount==17);
    /* Death without an external callback still retires the record. */
    group->ext_death=NULL;
    being_energy_delta(&group->beings[0],-being_energy(&group->beings[0]));
    removal=being_remove_initial(group); loop_no_thread(group,NULL,being_remove_loop2,removal);
    group->num=removal->count; memory_free((void**)&removal); being_remove_internal_clear();
    assert(!bronze_actors_find(registry,ids[2]) && bronze_actors_count(registry)==1);
    allocate_claim(second);
    uint64_t generation=bronze_actors_generation(registry); BronzeActorId next=bronze_actors_next_id(registry);
    assert(bronze_actors_world_reset(group));
    assert(claims==2 && bronze_actors_generation(registry)==generation+1);
    assert(bronze_actors_for_being(registry,&group->beings[0])->id==next);
    assert(!bronze_actors_find(registry,ids[1]));
    BronzeActor* reset=bronze_actors_for_being(registry,&group->beings[0]);
    assert(!reset->resources && reset->vocation==BRONZE_KIND_NONE);
    BronzeApeActor adapter;
    assert(bronze_ape_actor_from_registry(&adapter,&group->beings[0]));
    assert(adapter.id==reset->id && adapter.being==&group->beings[0]);
    assert(!bronze_ape_actor_from_registry(&adapter,NULL) && adapter.id==SCENARIO_ID_NONE);
    allocate_claim(reset);
    group->num=LARGE_SIM+1;
    assert(!bronze_actors_world_reset(group) && bronze_actors_count(registry)==0);
    assert(claims==3 && !bronze_actors_resolve(registry,next));
    group->num=1; assert(bronze_actors_world_reset(group));
    allocate_claim(bronze_actors_for_being(registry,&group->beings[0]));
    sim_close(); assert(claims==4);

    /* Real native pregnancy/birth path, not a simulated registration call. */
    assert(sim_init(KIND_START_UP,42,MAP_AREA,0)); sim_cycle(); group=sim_group();
    for(i=0;i<group->num;i++) if(being_female(&group->beings[i])) break;
    assert(i<group->num);
    if(i) memcpy(&group->beings[0],&group->beings[i],sizeof(simulated_being));
    group->num=1; assert(bronze_actors_world_reset(group)); registry=bronze_actors_current();
    for(i=0;i<MAP_AREA;i++) if(land_topography()[i]>200) break; assert(i<MAP_AREA);
    n_byte2 location[2]={(n_byte2)MAPSPACE_TO_APESPACE(i%MAP_DIMENSION),(n_byte2)MAPSPACE_TO_APESPACE(i/MAP_DIMENSION)};
    being_set_location(&group->beings[0],location);
    n_byte2 seed[2]={17,29}; land_load_state(land_date(),720,seed);
    group->beings[0].changes.date_of_conception=land_date()-2;
    memcpy(being_fetal_genetics(&group->beings[0]),being_genetics(&group->beings[0]),sizeof(n_genetics)*CHROMOSOMES);
    group->beings[0].delta.awake=2; being_set_speed(&group->beings[0],0);
    group->ext_birth=birth_callback;
    being_cycle_awake(group,&group->beings[0]);
    assert(births==1 && newborn && group->num==2 && bronze_actors_count(registry)==2);
    assert(bronze_actors_resolve(registry,newborn)==&group->beings[1]);
    assert(group->ext_birth==birth_callback);
    assert(!bronze_actors_birth(group,&group->beings[group->num+1]));
    assert(bronze_actors_birth(group,&group->beings[1]) && bronze_actors_count(registry)==2);
    sim_close(); puts("Actor identity, real birth/death, compaction, reorder, claims and reset tests passed.");
}
static void sessions(void)
{
    shared_session* a=shared_session_create(42); shared_session* b=shared_session_create(1337);
    assert(a && b);
    assert(!shared_session_bronze_actor_id(NULL) && !shared_session_bronze_actor_id(a));
    BronzeLandPolicy policy=bronze_land_defaults();
    policy.field_moisture=255;
    shared_session_bronze_land_policy(b,policy);
    BronzeProfessionPolicy jobs={
        .count=1, .site_capacity=256, .retry_minutes=60, .skill_start=4, .skill_max=10,
        .vocations={{"host", 1, BRONZE_JOB_SERVICE}}
    };
    shared_session_bronze_professions(a,&jobs);
    assert(shared_session_init(a,NUM_CONTROL,42)==NUM_CONTROL);
    shared_session_cycle(a,1,NUM_CONTROL);
    BronzeActorId aid=shared_session_bronze_actor_id(a); assert(aid);
    BronzeActorRegistry* ar=bronze_actors_current();
    BronzeActor* actor_a=bronze_actors_find(ar,aid); assert(actor_a);
    BronzeLandscape* landscape_a=bronze_actors_landscape(ar);
    assert(landscape_a->town_count>0);
    const BronzePlace* home=bronze_hybrid_place(landscape_a->towns[0].home);
    n_byte2 location[2]={MAPSPACE_TO_APESPACE(home->location.x), MAPSPACE_TO_APESPACE(home->location.y)};
    being_set_location(bronze_actors_resolve(ar,aid),location);
    bronze_hybrid_reset();
    char summary[256];
    shared_session_bronze_actor_summary(a,summary,sizeof(summary));
    assert(strstr(summary,"host") && strstr(summary,"skill 4/10") &&
           strstr(summary,"home #") && strstr(summary,"workplace #"));
    strcpy(jobs.vocations[0].name,"next_world");
    shared_session_bronze_professions(a,&jobs);
    shared_session_bronze_actor_summary(a,summary,sizeof(summary));
    assert(strstr(summary,"host") && !strstr(summary,"next_world"));
    n_int ax=being_location_x(bronze_actors_resolve(ar,aid)), ay=being_location_y(bronze_actors_resolve(ar,aid));
    assert(shared_session_init(b,NUM_CONTROL,1337)==NUM_CONTROL);
    shared_session_cycle(b,1,NUM_CONTROL);
    BronzeActorId bid=shared_session_bronze_actor_id(b); assert(bid);
    BronzeActorRegistry* br=bronze_actors_current(); assert(ar!=br);
    n_uint site_count;
    const bronze_site* b_sites=bronze_hybrid_sites(&site_count);
    assert(site_count>0);
    for(n_uint i=0;i<site_count;i++) assert(b_sites[i].kind!=BRONZE_FIELD);
    assert(bronze_actors_landscape(br)->town_count==0);
    assert(bronze_actors_landscape(ar)->town_count>0);
    bronze_actors_landscape(ar)->towns[0].stores.grain=7;
    assert(!bronze_actors_resolve(ar,aid) && !bronze_actors_for_being(ar,sim_group()->select));
    BronzeActor* actor_b=bronze_actors_find(br,bid); assert(actor_b); actor_b->vocation=22;
    n_int bx=being_location_x(bronze_actors_resolve(br,bid)), by=being_location_y(bronze_actors_resolve(br,bid));
    for(int i=0;i<4;i++) {
        assert(shared_session_bronze_actor_id(a)==aid && bronze_actors_current()==ar);
        assert(bronze_actors_find(ar,aid)->vocation==0);
        assert(bronze_hybrid_sites(NULL)!=b_sites && bronze_hybrid_economy().grain==7);
        assert(!bronze_actors_resolve(br,bid));
        assert(being_location_x(bronze_actors_resolve(ar,aid))==ax && being_location_y(bronze_actors_resolve(ar,aid))==ay);
        assert(shared_session_bronze_actor_id(b)==bid && bronze_actors_current()==br);
        assert(bronze_actors_find(br,bid)->vocation==22);
        assert(bronze_hybrid_sites(NULL)==b_sites && bronze_hybrid_economy().grain==0);
        assert(being_location_x(bronze_actors_resolve(br,bid))==bx && being_location_y(bronze_actors_resolve(br,bid))==by);
    }
    assert(shared_session_bronze_actor_id(a)==aid);
    n_uint count=sim_group()->num;
    assert(count>1); sim_set_select(&sim_group()->beings[1]);
    BronzeActorId other=shared_session_bronze_actor_id(a); assert(other && other!=aid);
    assert(bronze_actors_resolve(ar,other)==sim_group()->select);
    assert(shared_session_new(a,42)==0); shared_session_cycle(a,2,NUM_CONTROL);
    assert(shared_session_bronze_actor_id(a)!=aid && !bronze_actors_find(ar,aid));
    assert(shared_session_bronze_actor_id(b)==bid && bronze_actors_find(br,bid)->vocation==22);
    shared_session_destroy(a); /* Inactive owner must not destroy B's sidecars. */
    assert(shared_session_bronze_actor_id(b)==bid && bronze_actors_find(br,bid)->vocation==22);
    /* Native-only file imports deliberately start fresh actor identities. */
    shared_session_saveFileName(b,"native-only.txt");
    assert(shared_session_openFileName(b,"native-only.txt",0)==1);
    BronzeActorId imported=shared_session_bronze_actor_id(b);
    assert(imported && imported!=bid && !bronze_actors_find(br,bid));
    assert(bronze_actors_find(br,imported)->vocation==BRONZE_KIND_NONE);
    BronzeActorId after_import=bronze_actors_next_id(br);
    sim_cycle(); /* Finish deferred load setup without a second identity reset. */
    assert(bronze_actors_next_id(br)==after_import);
    (void)shared_session_openFileName(b,"missing-native.txt",0); /* Legacy return code does not distinguish absence. */
    assert(shared_session_bronze_actor_id(b)==imported); /* No read, no mutation. */
    FILE* bad=fopen("bad-native.txt","wb"); assert(bad); fputs("not native",bad); fclose(bad);
    assert(!shared_session_openFileName(b,"bad-native.txt",0));
    assert(!shared_session_bronze_actor_id(b) && !bronze_actors_count(br));
    assert(!bronze_actors_find(br,imported));
    assert(shared_session_new(b,1337)==0);
    if(shared_session_pause_state(b)) shared_session_menu(b,NA_MENU_PAUSE);
    shared_session_cycle(b,3,NUM_CONTROL); assert(shared_session_bronze_actor_id(b));
    remove("native-only.txt"); remove("bad-native.txt");
    shared_session_destroy(b); shared_close();
    puts("Independent shared-session registry, selection, reset and destruction tests passed.");
}
void bronze_test_work(void);
int main(int argc, char** argv)
{
    if(argc>1 && !strcmp(argv[1],"sessions")) sessions();
    else if(argc>1 && !strcmp(argv[1],"work")) bronze_test_work();
    else lifecycle();
    return 0;
}
