#ifdef BRONZE_DISK_SAVE
#include "../save/bronze_save.h"
#endif
#include "bronze_actors.h"
#include "../../entity/entity.h"
#include "../../shared.h"
#include "../../gui/gui.h"
#include "../adapter/bronze_apesdk_adapter.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ui/bronze_workspace_render.h"
#include "../scenario/bronze_scenario.h"
#include "../tasks/bronze_tasks.h"
#include "../work/bronze_work.h"

#include "test_settlement_render.inc"

static void awake_status_text(void)
{
    n_string_block description;
    memset(description, 'X', sizeof(description));
    being_state_description(BEING_STATE_AWAKE, description);
    assert(!strcmp(description, "Awake"));
    memset(description, 'X', sizeof(description));
    being_state_description(BEING_STATE_ASLEEP, description);
    assert(!strcmp(description, "Sleeping"));
    memset(description, 'X', sizeof(description));
    being_state_description(BEING_STATE_AWAKE | BEING_STATE_HUNGRY, description);
    assert(!strcmp(description, "Hungry"));
    puts("Awake, sleeping and compound state description tests passed.");
}

static void terrain_interpolation(void)
{
    n_byte *heights = malloc(MAP_AREA);
    assert(heights);
    memset(heights, 180, MAP_AREA);
    heights[10 * MAP_DIMENSION + 10] = 180;
    heights[10 * MAP_DIMENSION + 11] = 200;
    heights[11 * MAP_DIMENSION + 10] = 220;
    heights[11 * MAP_DIMENSION + 11] = 240;
    BronzeRenderLand land = {0};
    land.heights = heights;
    land.tide = 40;
    BronzeRenderSample corners[4] = {
        bronze_render_sample(&land, 10, 10, 0), bronze_render_sample(&land, 11, 10, 0),
        bronze_render_sample(&land, 10, 11, 0), bronze_render_sample(&land, 11, 11, 0)
    };
    BronzeRenderSample center = bronze_render_sample(&land, 10.5, 10.5, 0);
    assert(fabs(center.height - 210 * 0.155) < 1e-9);
    assert(center.r == (int)lround((corners[0].r + corners[1].r + corners[2].r + corners[3].r) / 4.0));
    assert(center.g == (int)lround((corners[0].g + corners[1].g + corners[2].g + corners[3].g) / 4.0));
    assert(center.b == (int)lround((corners[0].b + corners[1].b + corners[2].b + corners[3].b) / 4.0));
    assert(fabs(bronze_render_sample(&land, 10.25, 10.75, 0).height - 215 * 0.155) < 1e-9);
    const double epsilon = 1e-7;
    for (unsigned axis = 0; axis < 2; ++axis) {
        for (unsigned seam = 0; seam < 2; ++seam) {
            double edge = seam ? MAP_DIMENSION : 11;
            double x = axis ? 10.37 : edge;
            double y = axis ? edge : 10.37;
            BronzeRenderSample left = bronze_render_sample(&land, x - (axis ? 0 : epsilon),
                                                           y - (axis ? epsilon : 0), 0);
            BronzeRenderSample right = bronze_render_sample(&land, x + (axis ? 0 : epsilon),
                                                            y + (axis ? epsilon : 0), 0);
            assert(fabs(left.height - right.height) < 1e-4);
            assert(abs(left.r - right.r) <= 1 && abs(left.g - right.g) <= 1 && abs(left.b - right.b) <= 1);
        }
    }
    BronzeRenderSample wrapped = bronze_render_sample(&land, 10.5 - MAP_DIMENSION, 10.5, 0);
    assert(fabs(wrapped.height - center.height) < 1e-9 && wrapped.r == center.r);
    /* A sloping shoreline may have a dry center and a submerged corner. */
    land.tide = TIDE_MAX;
    memset(heights, land.tide + 20, MAP_AREA);
    assert(bronze_render_tree_ground(&land, 10, 10));
    heights[11 * MAP_DIMENSION + 11] = land.tide - 1;
    assert(!bronze_render_tree_ground(&land, 10, 10));
    assert(!bronze_tree_allowed(&land, 10, 10, 0));
    heights[11 * MAP_DIMENSION + 11] = land.tide + BRONZE_RENDER_BEACH_HEIGHT;
    assert(!bronze_render_tree_ground(&land, 10, 10));
    heights[11 * MAP_DIMENSION + 11] += 1;
    assert(bronze_render_tree_ground(&land, 10, 10));
    land.tide = TIDE_MAX;
    memset(heights, TIDE_MAX + 1, MAP_AREA);
    BronzeRenderSample beach = bronze_render_sample(&land, 10.5, 10.5, 0);
    BronzeRenderSample beach_map = bronze_render_sample(&land, 10.5, 10.5, 1);
    assert(beach.r > beach.g && beach.g > beach.b);
    assert(beach.height == beach_map.height && beach.r == beach_map.r);
    assert(!bronze_render_tree_ground(&land, -1, -1));
    memset(heights, land.tide - 1, MAP_AREA);
    assert(!bronze_tree_allowed(&land, 10, 10, 0));
    BronzeRenderSample seabed = bronze_render_sample(&land, 10.5, 10.5, 0);
    assert(fabs(seabed.height - (land.tide - 1) * 0.155) < 1e-9);
    assert(seabed.r > seabed.g && seabed.g > seabed.b);
    BronzeRenderSample shallow = bronze_render_water(seabed, seabed.height + 0.1, 10.5, 10.5);
    BronzeRenderSample deep = bronze_render_water(seabed, seabed.height + 10, 10.5, 10.5);
    assert(shallow.height == seabed.height && deep.height == seabed.height);
    assert(shallow.r > deep.r && shallow.g > deep.g);
    /* Deep water must preserve visible bottom contrast, not converge to an
     * opaque sea color. This also applies to the overhead map. */
    BronzeRenderSample dark_bottom = {seabed.height, 40, 50, 30};
    BronzeRenderSample light_bottom = {seabed.height, 200, 190, 160};
    BronzeRenderSample dark_water = bronze_render_water(dark_bottom,seabed.height+30,10.5,10.5);
    BronzeRenderSample light_water = bronze_render_water(light_bottom,seabed.height+30,10.5,10.5);
    assert(light_water.r-dark_water.r >= 70 && light_water.g-dark_water.g >= 60);
    /* A neutral seabed must acquire a visible blue surface tint. */
    BronzeRenderSample neutral = {seabed.height,160,160,160};
    BronzeRenderSample blue_water = bronze_render_water(neutral,seabed.height+10,10.5,10.5);
    assert(blue_water.b>blue_water.g && blue_water.g>blue_water.r);
    assert(blue_water.r<neutral.r && blue_water.b<neutral.b);
    BronzeRenderSample dry = bronze_render_water(seabed,seabed.height-1,10.5,10.5);
    assert(dry.r==seabed.r && dry.g==seabed.g && dry.b==seabed.b);
    land.tide += 10;
    BronzeRenderSample raised_tide = bronze_render_sample(&land, 10.5, 10.5, 0);
    assert(raised_tide.height == seabed.height && raised_tide.r == seabed.r
           && raised_tide.g == seabed.g && raised_tide.b == seabed.b);
    puts("Sandy seabed, translucent water and tide-independent ground tests passed.");
    free(heights);
    puts("Bronze bilinear height, color, cell-edge and wrapped-seam tests passed.");
}

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
    uint64_t inspected_building=bronze_hybrid_settlement()->buildings[0].id;
    BronzeSettlementInspection settlement_a=shared_session_bronze_settlement_inspection(a,inspected_building | (UINT64_C(1)<<63),5);
    assert(settlement_a.building_id==inspected_building && settlement_a.township_id);

    const BronzePlace* home=bronze_hybrid_place(landscape_a->towns[0].home);
    n_byte2 location[2]={MAPSPACE_TO_APESPACE(home->location.x), MAPSPACE_TO_APESPACE(home->location.y)};
    being_set_location(bronze_actors_resolve(ar,aid),location);
    bronze_hybrid_reset();
    char summary[512];
    shared_session_bronze_actor_summary(a,summary,sizeof(summary));
    assert(strstr(summary,"host") && strstr(summary,"skill 4/10") &&
           strstr(summary,"home #") && strstr(summary,"workplace #"));
    assert(strstr(summary,"Energy ") && strstr(summary,"Cargo empty") &&
           strstr(summary,"Town store #"));
    BronzeInspectorActor inspector = shared_session_bronze_inspector_actor(a);
    assert(inspector.actor_id == aid && inspector.workplace_id == actor_a->workplace);
    assert(strcmp(inspector.vocation, "host") == 0);
    assert(strcmp(inspector.cargo, "Empty") == 0 && strcmp(inspector.items, "Empty") == 0);
    assert(inspector.energy == being_energy(sim_group()->select) && inspector.energy_full == BEING_FULL);
    assert(strstr(inspector.site, "Township #") && inspector.exchange[0] == '\0');
    simulated_being *selected = sim_group()->select;
    sim_group()->select = NULL;
    BronzeInspectorActor empty_inspector = shared_session_bronze_inspector_actor(a);
    assert(empty_inspector.actor_id == 0 && empty_inspector.workplace_id == 0);
    assert(empty_inspector.vocation[0] == '\0' && empty_inspector.task[0] == '\0');
    assert(empty_inspector.cargo[0] == '\0' && empty_inspector.site[0] == '\0' &&
           empty_inspector.exchange_partner_id == 0);
    sim_group()->select = selected;
    assert(!strstr(summary, "Latest exchange"));
    actor_a->economic_action = BRZ_ACTION_TRADE;
    actor_a->economic_status = BRONZE_TASK_COMPLETE;
    actor_a->economic_partner = 42;
    shared_session_bronze_actor_summary(a, summary, sizeof(summary));
    assert(strstr(summary, "Trade · complete · partner #42"));
    inspector = shared_session_bronze_inspector_actor(a);
    assert(inspector.exchange_partner_id == 42 && strcmp(inspector.exchange, "Trade · complete") == 0);
    actor_a->economic_action = BRZ_ACTION_SERVE;
    actor_a->economic_status = BRONZE_TASK_PARTNER;
    actor_a->economic_partner = BRONZE_ACTOR_NONE;
    shared_session_bronze_actor_summary(a, summary, sizeof(summary));
    assert(strstr(summary, "Paid delivery · no eligible partner at the service entrance"));
    assert(!strstr(summary, "partner #"));
    inspector = shared_session_bronze_inspector_actor(a);
    assert(inspector.exchange_partner_id == 0 && strstr(inspector.exchange, "no eligible partner"));
    strcpy(jobs.vocations[0].name,"next_world");
    shared_session_bronze_professions(a,&jobs);
    shared_session_bronze_actor_summary(a,summary,sizeof(summary));
    assert(strstr(summary,"host") && !strstr(summary,"next_world"));
    n_int ax=being_location_x(bronze_actors_resolve(ar,aid)), ay=being_location_y(bronze_actors_resolve(ar,aid));
    simulated_being *native_a = bronze_actors_resolve(ar, aid);
    n_byte2 native_name = native_a->constant.name[0];
    assert(native_name);
    native_a->delta.velocity[9] = 71;
    native_a->changes.father_name[0] = 0x1234;
    native_a->changes.mother_name[1] = 0xabcd;
#ifdef BRAIN_ON
    native_a->braindata.brain[DOUBLE_BRAIN - 1] = 93;
#endif
    n_file *continuation = bronze_native_continuation_capture();
    assert(continuation);
    n_file truncated = *continuation;
    --truncated.size;
    native_a->delta.velocity[9] = 0;
    assert(!bronze_native_continuation_restore(&truncated));
    assert(native_a->delta.velocity[9] == 0);
    native_a->delta.location[0] ^= 1;
    assert(!bronze_native_continuation_restore(continuation));
    assert(native_a->delta.velocity[9] == 0);
    native_a->delta.location[0] ^= 1;
    assert(bronze_native_continuation_restore(continuation));
    io_file_free(&continuation);
    assert(shared_session_init(b,NUM_CONTROL,1337)==NUM_CONTROL);
    shared_session_cycle(b,1,NUM_CONTROL);
    BronzeActorId bid=shared_session_bronze_actor_id(b); assert(bid);
    BronzeActorRegistry* br=bronze_actors_current(); assert(ar!=br);
    n_uint site_count;
    const bronze_site* b_sites=bronze_hybrid_sites(&site_count);
    assert(site_count>0);
    for(n_uint i=0;i<site_count;i++) assert(b_sites[i].kind!=BRONZE_FIELD);
    assert(bronze_actors_landscape(br)->town_count==0);
    assert(!shared_session_bronze_settlement_inspection(b,inspected_building | (UINT64_C(1)<<63),5).township_id);
    BronzeSettlementInspection isolated=shared_session_bronze_settlement_inspection(a,inspected_building | (UINT64_C(1)<<63),5);
    /* a was deliberately rebuilt above; retired selections must not bind to b. */
    assert(!isolated.building_id);
    assert(shared_session_bronze_actor_id(b)==bid);

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
        native_a = bronze_actors_resolve(ar, aid);
        assert(native_a->delta.velocity[9] == 71);
        assert(native_a->constant.name[0] == native_name);
        assert(native_a->changes.father_name[0] == 0x1234);
        assert(native_a->changes.mother_name[1] == 0xabcd);
#ifdef BRAIN_ON
        assert(native_a->braindata.brain[DOUBLE_BRAIN - 1] == 93);
#endif
        assert(shared_session_bronze_actor_id(b)==bid && bronze_actors_current()==br);
        assert(bronze_actors_find(br,bid)->vocation==22);
        assert(bronze_hybrid_sites(NULL)==b_sites && bronze_hybrid_economy().grain==0);
        assert(being_location_x(bronze_actors_resolve(br,bid))==bx && being_location_y(bronze_actors_resolve(br,bid))==by);
    }
    assert(shared_session_bronze_actor_id(a)==aid);
#ifdef BRONZE_DISK_SAVE
    n_byte digest[BRONZE_SCENARIO_DIGEST_BYTES] = {24};
    /* The native remains counter can reach one above the ring size. */
    sim_group()->remains.count = NUMBER_OF_BODIES + 1;
    sim_group()->remains.location = 4;
    BronzeSettlementLayout *saved_layout = malloc(sizeof(*saved_layout));
    assert(saved_layout);
    *saved_layout = *bronze_hybrid_settlement();
    n_file *package = shared_session_bronze_save(a, digest);
    assert(package);
    assert(bronze_save_publish("combined.bronzesave", (BronzeSaveSection){package->data, package->location}));
    n_file *disk = io_file_new();
    assert(disk && io_disk_read(disk, "combined.bronzesave") == FILE_OKAY);
    io_file_free(&package);
    package = disk;
    bronze_actors_landscape(ar)->towns[0].stores.grain = 91;
    sim_group()->remains.count = 0;
    n_byte wrong_digest[BRONZE_SCENARIO_DIGEST_BYTES] = {25};
    assert(!shared_session_bronze_open(a, package->data, package->location, wrong_digest));
    assert(!shared_session_bronze_open(a, package->data, package->location - 1, digest));
    BronzeSaveSection native, sidecar;
    assert(bronze_save_read((BronzeSaveSection){package->data, package->location}, &native, &sidecar));
    n_byte *altered_native = malloc(native.length);
    n_byte *mismatched = malloc(package->location);
    assert(altered_native && mismatched);
    memcpy(altered_native, native.data, native.length);
    /* The continuation header sits after the native transfer. Change its
       version and rebuild the checksum so this reaches staged validation. */
    n_uint transfer_length = 0;
    for (unsigned i = 0; i < 4; ++i)
        transfer_length |= (n_uint)altered_native[i] << (8 * i);
    altered_native[8 + transfer_length] ^= 1;
    assert(bronze_save_write(mismatched, package->location,
                             (BronzeSaveSection){altered_native, native.length}, sidecar));
    assert(!shared_session_bronze_open(a, mismatched, package->location, digest));
    free(mismatched);
    free(altered_native);
    assert(bronze_actors_current() == ar);
    assert(bronze_hybrid_economy().grain == 91 && sim_group()->remains.count == 0);
    assert(shared_session_bronze_open(a, package->data, package->location, digest));
    ar = bronze_actors_current();
    assert(!memcmp(saved_layout, bronze_hybrid_settlement(), sizeof(*saved_layout)));
    free(saved_layout);
    assert(shared_session_bronze_actor_id(a) == aid);
    assert(bronze_hybrid_economy().grain == 7);
    assert(sim_group()->remains.count == NUMBER_OF_BODIES + 1 && sim_group()->remains.location == 4);
    assert(bronze_actors_resolve(ar, aid)->delta.velocity[9] == 71);
    n_file *restored_package = shared_session_bronze_save(a, digest);
    assert(restored_package);
    assert(restored_package->location == package->location);
    assert(!memcmp(restored_package->data, package->data, package->location));
    io_file_free(&restored_package);

    for (unsigned tick = 0; tick < 3; ++tick) sim_cycle();
    n_file *uninterrupted = shared_session_bronze_save(a, digest);
    assert(uninterrupted);
    assert(shared_session_bronze_open(a, package->data, package->location, digest));
    ar = bronze_actors_current();
    for (unsigned tick = 0; tick < 3; ++tick) sim_cycle();
    n_file *resumed = shared_session_bronze_save(a, digest);
    assert(resumed && resumed->location == uninterrupted->location);
    assert(!memcmp(resumed->data, uninterrupted->data, resumed->location));
    io_file_free(&uninterrupted);
    io_file_free(&resumed);
    io_file_free(&package);
    if (shared_session_pause_state(a)) shared_session_menu(a, NA_MENU_PAUSE);
#endif
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
    /* Workspace drawing and input must use the active world without advancing it. */
    enum { workspace_width = 80, workspace_height = 48 };
    n_byte4 overhead[workspace_width * workspace_height] = {0};
    n_byte4 terrain[workspace_width * workspace_height] = {0};
    n_byte4 repeat[workspace_width * workspace_height] = {0};
    BronzeWorkspacePoint center = shared_session_bronze_workspace_center(b, 1);
    n_uint before = (n_uint)land_date() * TIME_DAY_MINUTES + land_time();
    shared_session_bronze_workspace_draw(b, overhead, workspace_width, workspace_height, NUM_VIEW, center, 64, 0, 0);
    shared_session_bronze_workspace_draw(b, terrain, workspace_width, workspace_height, NUM_TERRAIN, center, 64, 0, 0);
    shared_session_bronze_workspace_draw(b, repeat, workspace_width, workspace_height, NUM_VIEW, center, 64, 0, 0);
    for (unsigned i = 0; i < sizeof(overhead); i += 4) {
        const n_byte *map_rgba = (const n_byte *)overhead + i;
        const n_byte *terrain_rgba = (const n_byte *)terrain + i;
        assert(map_rgba[3] == 255 && terrain_rgba[3] == 255);
    }
    assert(memcmp(overhead, terrain, sizeof(overhead)));
    assert(!memcmp(overhead, repeat, sizeof(overhead)));
    assert((n_uint)land_date() * TIME_DAY_MINUTES + land_time() == before);
    assert(!shared_session_bronze_workspace_step(b));
    assert(shared_session_menu(b, NA_MENU_PAUSE));
    assert(shared_session_bronze_workspace_step(b));
    assert(shared_session_pause_state(b));
    assert((n_uint)land_date() * TIME_DAY_MINUTES + land_time() == before + 1);
    puts("Bronze workspace rendering and single paused step passed.");
    remove("native-only.txt"); remove("bad-native.txt");
    shared_session_destroy(b); shared_close();
    puts("Independent shared-session registry, selection, reset and destruction tests passed.");
}
static void extended_scenario(void)
{
    BronzeScenario *scenario = bronze_scenario_create();
    assert(scenario && bronze_scenario_load(scenario, BRONZE_EXTENDED_CONTENT, "example_large.bronze"));
    assert(bronze_scenario_vocations(scenario) == 120);
    BronzeProgram *program = bronze_scenario_program(scenario);
    assert(program && program->config->resource_kinds.names.len == 19);
    const VocationDef *vocations = program->config->vocations.data;
    for (unsigned i = 0; i < 120; ++i) {
        assert(vocations[i].tasks.len == 9 && vocations[i].rules.len == 2);
        const TaskDef *daily = brz_voc_find_task((VocationDef *)&vocations[i], "__rule_daily_cycle");
        const TaskDef *crisis = brz_voc_find_task((VocationDef *)&vocations[i], "__rule_crisis");
        assert(daily && daily->stmts.len == 7);
        const StmtDef *daily_ops = daily->stmts.data;
        assert(!strcmp(daily_ops[0].as.op.a0, "grain") && !strcmp(daily_ops[3].as.op.a0, "cattle"));
        assert(daily_ops[6].kind == ST_CHANCE && daily_ops[6].as.chance.body.len == 3);
        assert(crisis && crisis->stmts.len == 1);
        const StmtDef *when = crisis->stmts.data;
        assert(when->kind == ST_WHEN && when->as.when_stmt.body.len == 6);
        assert(when->as.when_stmt.condition.kind[0] == BRZ_COND_RESOURCE);
        assert(when->as.when_stmt.condition.resource_id[0] == kind_table_find(&program->config->resource_kinds, "grain"));
    }
    shared_session *session = shared_session_create(bronze_scenario_seed(scenario));
    assert(session);
    shared_session_bronze_land_policy(session, bronze_scenario_land_policy(scenario));
    shared_session_bronze_professions(session, bronze_scenario_professions(scenario));
    shared_session_bronze_program(session, program);
    assert(shared_session_init(session, NUM_CONTROL, bronze_scenario_seed(scenario)) == NUM_CONTROL);
    if (shared_session_pause_state(session)) shared_session_menu(session, NA_MENU_PAUSE);
    for (unsigned tick = 0; tick < 4; ++tick) shared_session_cycle(session, tick, NUM_CONTROL);
    /* Exercise the exact timer-to-status path from the crash report while
     * the selected being is awake with no activity bits. */
    simulated_being *status_being = sim_group()->select;
    assert(status_being);
    n_byte2 prior_state = being_state(status_being);
    being_set_state(status_being, BEING_STATE_AWAKE);
    struct {
        n_string_block text;
        unsigned char guard[32];
    } output;
    memset(&output, 0xA5, sizeof(output));
    watch_control(sim_group(), being_get_select_name(sim_group()), status_being, output.text);
    assert(strstr(output.text, ": Awake\n"));
    for (unsigned i = 0; i < sizeof(output.guard); ++i) assert(output.guard[i] == 0xA5);
    sim_set_output(1);
    for (unsigned i = 0; i < 128; ++i) sim_update_output();
    sim_set_output(0);
    being_set_state(status_being, prior_state);
    puts("Awake native status output regression passed.");
    BronzeActorRegistry *registry = bronze_actors_current();
    assert(bronze_actors_professions(registry)->count == 120);
    const BronzeSettlementLayout *layout = bronze_hybrid_settlement();
    assert(layout && layout->version == 1 && layout->building_count);
    BronzeLandscape *layout_land = bronze_actors_landscape(registry);
    for (unsigned t = 0; t < layout_land->town_count; ++t) {
        const BronzeLayoutReport *report = &layout->reports[t];
        assert(report->owner == layout_land->towns[t].home);
        assert(report->requested_housing == layout_land->towns[t].capacity);
        assert(report->placed_housing + report->missing_housing == report->requested_housing);
        printf("Large-default township %llu: housing %u/%u, missing store %u, plaza %u, yards %u\n",
               (unsigned long long)report->owner, report->placed_housing, report->requested_housing,
               report->missing_store, report->missing_plaza, report->missing_work_yards);
        printf("Township paths: unreachable %u, limited %u; missing professional premises %u\n",
               report->unreachable_paths,report->limited_paths,report->missing_premises);
    }

    BronzeActor *actor = NULL;
    simulated_being *being = NULL;
    simulated_group *group = sim_group();
    for (n_uint i = 0; i < group->num; ++i) {
        BronzeActor *candidate = bronze_actors_for_being(registry, &group->beings[i]);
        if (candidate && candidate->job_status == BRONZE_JOB_ASSIGNED) {
            actor = candidate; being = &group->beings[i]; break;
        }
    }
    assert(actor && being);
    assert(bronze_actor_set_job(registry, actor, 119, actor->workplace, 17, 4));
    actor->job_status = BRONZE_JOB_ASSIGNED;
    sim_set_select(being);
    assert(!strcmp(shared_session_bronze_inspector_actor(session).vocation, "clan_role_120"));
    /* Isolate the crisis rule and check its inventory comparison in execution. */
    RuleDef *rules = ((VocationDef *)program->config->vocations.data)[119].rules.data;
    CompiledCondition daily_condition = rules[0].condition;
    rules[0].condition.kind[0] = BRZ_COND_FALSE;
    free(actor->resources);
    actor->resources = calloc(1, sizeof(*actor->resources));
    assert(actor->resources);
    actor->resource_count = 1;
    actor->resources[0].kind = (unsigned)kind_table_find(&program->config->resource_kinds, "grain");
    BronzeClock *clock = bronze_actors_clock(registry);
    clock->tick = bronze_native_tick();
    clock->open = 1;
    actor->work.initial_reason = BRONZE_WORK_READY;
    actor->execution.prepared_tick = UINT64_MAX;
    bronze_tasks_begin(group);
    assert(actor->execution.operation && actor->execution.operation->has_n0
           && actor->execution.operation->n0 == 2);
    uint64_t workplace = actor->workplace;
    assert(bronze_actor_set_job(registry, actor, 118, workplace, 17, 4));
    assert(bronze_actor_set_job(registry, actor, 119, workplace, 17, 4));
    actor->job_status = BRONZE_JOB_ASSIGNED;
    actor->resources[0].amount = 3;
    actor->work.initial_reason = BRONZE_WORK_READY;
    actor->execution.prepared_tick = UINT64_MAX;
    bronze_tasks_begin(group);
    assert(!actor->execution.operation && actor->execution.status == BRONZE_TASK_COMPLETE);
    clock->open = 0;
    rules[0].condition = daily_condition;
#ifdef BRONZE_DISK_SAVE
    unsigned char digest[32] = {120, 19};
    n_file *saved = shared_session_bronze_save(session, digest);
    assert(saved);
    assert(bronze_actor_set_job(registry, actor, 0, actor->workplace, 0, 5));
    assert(shared_session_bronze_open(session, saved->data, saved->location, digest));
    assert(!strcmp(shared_session_bronze_inspector_actor(session).vocation, "clan_role_120"));
    io_file_free(&saved);
#endif
    shared_session_destroy(session);
    bronze_scenario_destroy(scenario);
    shared_close();
    puts("Extended 120-vocation scenario, task sequences, resource conditions and high-role restore passed.");
}

#include "test_township_qualification.inc"

void bronze_test_work(void);
int main(int argc, char** argv)
{
    awake_status_text();
    terrain_interpolation();
    settlement_render_test();
    if(argc>1 && !strcmp(argv[1],"townships")) township_qualification();
    else if(argc>1 && !strcmp(argv[1],"extended")) extended_scenario();
    else if(argc>1 && !strcmp(argv[1],"sessions")) sessions();
    else if(argc>1 && !strcmp(argv[1],"work")) bronze_test_work();
    else lifecycle();
    return 0;
}
