#include "bronze_work.h"
#include "../hybrid/bronze_hybrid.h"
#include "../../shared.h"
#include "../../gui/gui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static n_byte2 seed[2]={17,29};
static void healthy(simulated_being* b)
{
    being_energy_delta(b,BEING_FULL-being_energy(b));
    memset(being_drives(b),0,DRIVES);
    being_set_state(b,BEING_STATE_AWAKE); being_set_speed(b,0);
}
static void step(simulated_group* g)
{ land_cycle(); bronze_work_begin(g); bronze_work_end(g); }

static void policy(void)
{
    uint64_t duration=99;
    assert(bronze_work_duration(1,&duration) && duration==TIME_DAY_MINUTES);
    assert(bronze_work_duration(0,&duration) && duration==1);
    assert(bronze_work_duration(0.001,&duration) && duration==2);
    assert(!bronze_work_duration(NAN,&duration) && !bronze_work_duration(INFINITY,&duration));
    assert(!bronze_work_duration(-1,&duration) && !bronze_work_duration(0x1p64,&duration));
    assert(!bronze_work_duration(1,NULL));
    assert(sim_init(KIND_START_UP,42,MAP_AREA,0)); sim_cycle();
    simulated_group* g=sim_group(); simulated_being* b=&g->beings[0];
    /* Stable dry location for native sleep, not a water-driven wake override. */
    n_uint i; for(i=0;i<MAP_AREA;i++) if(land_topography()[i]>200) break;
    assert(i<MAP_AREA);
    n_byte2 pos[2]={MAPSPACE_TO_APESPACE(i%MAP_DIMENSION),MAPSPACE_TO_APESPACE(i/MAP_DIMENSION)};
    being_set_location(b,pos);
    land_load_state(42,720,seed); assert(bronze_actors_world_reset(g));
    BronzeActorRegistry* r=bronze_actors_current(); BronzeActor* a=bronze_actors_for_being(r,b);
    BronzeClock* clock=bronze_actors_clock(r); uint64_t origin=bronze_native_tick();
    assert(clock->origin==origin && !bronze_clock_days(clock));
    healthy(b);
    assert(bronze_work_request(r,a->id,7,4));
    assert(!bronze_work_request(r,a->id,8,2));
    step(g); assert(a->remaining_ticks==3 && a->work.phase==BRONZE_WORK_STARTED);
    assert(a->work.units==1 && being_energy(b)==BEING_FULL-1);
    assert(clock->elapsed==1 && bronze_clock_days(clock)==1.0/TIME_DAY_MINUTES);
    bronze_work_begin(g); bronze_work_end(g);
    assert(a->remaining_ticks==3 && !bronze_work_claim(b)); /* same tick budget */
    being_energy_delta(b,BEING_HUNGRY-being_energy(b));
    step(g); assert(a->remaining_ticks==3 && a->work.phase==BRONZE_WORK_INTERRUPTED);
    assert(a->work.reason==BRONZE_WORK_SURVIVAL);
    /* Even eating/recovery during native actions cannot authorize work in a
       tick which began with an urgent survival need. */
    land_cycle(); bronze_work_begin(g); healthy(b); bronze_work_end(g);
    assert(a->remaining_ticks==3 && a->work.reason==BRONZE_WORK_SURVIVAL);
    healthy(b); step(g); assert(a->remaining_ticks==2 && a->work.phase==BRONZE_WORK_RESUMED);
    assert(a->work.interruptions==1 && a->work.resumptions==1 && a->work.last_block_reason==BRONZE_WORK_SURVIVAL);
    being_drives(b)[DRIVE_FATIGUE]=255; step(g);
    assert(a->remaining_ticks==2 && a->work.reason==BRONZE_WORK_SLEEP);
    healthy(b); being_drives(b)[DRIVE_SOCIAL]=255; step(g);
    assert(a->remaining_ticks==2 && a->work.reason==BRONZE_WORK_OBLIGATION);
    healthy(b); being_set_state(b,BEING_STATE_SUCKLING); step(g);
    assert(a->work.reason==BRONZE_WORK_OBLIGATION);
    healthy(b); being_set_speed(b,10); step(g);
    assert(a->remaining_ticks==2 && a->work.reason==BRONZE_WORK_NATIVE_MOVEMENT);
    assert(a->work.phase==BRONZE_WORK_DEFERRED);
    healthy(b); land_cycle(); bronze_work_begin(g);
    pos[0]++; being_set_location(b,pos); bronze_work_end(g);
    assert(a->remaining_ticks==2 && a->work.reason==BRONZE_WORK_NATIVE_MOVEMENT);
    healthy(b); land_load_state(43,0,seed); step(g);
    assert(a->remaining_ticks==2 && a->work.reason==BRONZE_WORK_SLEEP);
    /* Discontinuous time never replays missed work: at most one unit. */
    land_load_state(43,720,seed); healthy(b); step(g);
    assert(a->remaining_ticks==1 && a->work.units==3);
    uint64_t resumptions=a->work.resumptions;
    being_drives(b)[DRIVE_FATIGUE]=255; step(g); healthy(b);
    step(g); assert(!a->remaining_ticks && a->task==BRONZE_KIND_NONE);
    assert(a->work.resumptions==resumptions+1); /* resumed and completed */
    assert(a->work.phase==BRONZE_WORK_COMPLETED && a->work.units==4);
    assert(!bronze_work_claim(b));
    resumptions=a->work.resumptions;
    healthy(b); step(g); n_int energy=being_energy(b);
    assert(bronze_work_claim(b) && !bronze_work_claim(b));
    assert(being_energy(b)==energy-1 && a->work.units==5);
    assert(a->work.resumptions==resumptions); /* separate one-shot claim */
    uint64_t units=a->work.units;
    land_load_state(42,721,seed); bronze_work_begin(g); bronze_work_end(g);
    assert(!bronze_work_claim(b) && a->work.units==units);
    assert(a->work.reason==BRONZE_WORK_CLOCK);
    land_load_state(43,1439,seed); bronze_work_begin(g); bronze_work_end(g);
    step(g); assert(land_date()==44 && land_time()==0);
    assert(clock->elapsed==bronze_native_tick()-origin);
    healthy(b); being_energy_delta(b,-being_energy(b));
    BronzeNeeds n=bronze_needs_read(b); assert(bronze_needs_reason(&n)==BRONZE_WORK_DEAD);
    healthy(b); being_drives(b)[DRIVE_HUNGER]=128; being_drives(b)[DRIVE_SEX]=64;
    n=bronze_needs_read(b); assert(n.hunger==128/255.0 && n.sex==64/255.0);
    assert(!strcmp(bronze_work_reason_name(BRONZE_WORK_SLEEP),"sleep"));
    sim_close();
}
static void native_intention(void)
{
    assert(sim_init(KIND_START_UP,42,MAP_AREA,0)); sim_cycle();
    simulated_group* g=sim_group(); simulated_being* b=&g->beings[0];
    n_uint i; for(i=0;i<MAP_AREA;i++) if(land_topography()[i]>200) break;
    assert(i<MAP_AREA);
    n_byte2 pos[2]={MAPSPACE_TO_APESPACE(i%MAP_DIMENSION),MAPSPACE_TO_APESPACE(i/MAP_DIMENSION)};
    being_set_location(b,pos); healthy(b); being_set_speed(b,10);
    land_load_state(42,720,seed); g->num=1; assert(bronze_actors_world_reset(g));
    BronzeActorRegistry* r=bronze_actors_current(); BronzeActor* a=bronze_actors_for_being(r,b);
    assert(bronze_work_request(r,a->id,9,3));
    int native_eating=0;
    for(i=1;i<=32 && a->remaining_ticks;i++) {
        uint64_t before=a->work.units;
        n_int x=being_location_x(b), y=being_location_y(b);
        sim_cycle();
        assert(a->work.units<=before+1 && a->remaining_ticks==3-a->work.units);
        if(a->work.units>before) {
            assert(being_location_x(b)==x && being_location_y(b)==y);
            assert(!being_speed(b)); /* Single locomotion owner on work ticks. */
        }
        if(being_state(b)&BEING_STATE_EATING) native_eating=1;
        assert(bronze_actors_clock(r)->elapsed==i);
    }
    assert(!a->remaining_ticks && a->work.phase==BRONZE_WORK_COMPLETED);
    assert(native_eating && a->work.interruptions && a->work.resumptions);
    sim_cycle(); assert(a->work.units==3);
    sim_close();
}
static uint64_t run_render_schedule(int extra_windows)
{
    shared_session* s=shared_session_create(42); assert(s);
    assert(shared_session_init(s,NUM_CONTROL,42)==NUM_CONTROL);
    shared_session_cycle(s,1,NUM_CONTROL);
    simulated_group* g=sim_group();
    n_uint dry; for(dry=0;dry<MAP_AREA;dry++) if(land_topography()[dry]>200) break;
    assert(dry<MAP_AREA);
    n_byte2 pos[2]={MAPSPACE_TO_APESPACE(dry%MAP_DIMENSION),MAPSPACE_TO_APESPACE(dry/MAP_DIMENSION)};
    being_set_location(&g->beings[0],pos); healthy(&g->beings[0]);
    land_load_state(42,720,seed); g->num=1; assert(bronze_actors_world_reset(g));
    BronzeActorRegistry* r=bronze_actors_current(); BronzeActor* a=bronze_actors_for_being(r,&g->beings[0]);
    assert(a && bronze_work_request(r,a->id,3,100));
    shared_session_menu(s,NA_MENU_PAUSE); assert(shared_session_pause_state(s));
    uint64_t tick=bronze_native_tick(), elapsed=bronze_actors_clock(r)->elapsed, units=a->work.units;
    for(int k=2;k<5;k++) {
        shared_session_cycle(s,k,NUM_CONTROL);
        shared_session_cycle(s,k,NUM_VIEW); shared_session_cycle(s,k,NUM_TERRAIN);
        assert(shared_session_draw(s,NUM_VIEW,512,512,0));
    }
    assert(bronze_native_tick()==tick && bronze_actors_clock(r)->elapsed==elapsed && a->work.units==units);
    shared_session_menu(s,NA_MENU_PAUSE); assert(!shared_session_pause_state(s));
    for(int k=5;k<12;k++) {
        shared_session_cycle(s,k,NUM_CONTROL);
        if(extra_windows) for(int j=0;j<3;j++) {
            shared_session_cycle(s,k,NUM_VIEW); shared_session_cycle(s,k,NUM_TERRAIN);
            assert(shared_session_draw(s,NUM_VIEW,512,512,0));
            assert(shared_session_draw(s,NUM_TERRAIN,512,512,0));
        }
    }
    assert(bronze_native_tick()==tick+7);
    assert(a->work.units>units && a->work.units<=units+7 && a->remaining_ticks==100-a->work.units);
    uint64_t result=bronze_native_tick()*257+a->remaining_ticks*17+a->work.units;
    /* Include actual bodies: render scheduling must not perturb native work. */
    for(n_uint i=0;i<g->num;i++) result=result*33+being_location_x(&g->beings[i])+being_energy(&g->beings[i]);
    shared_session_destroy(s); shared_close(); return result;
}
void bronze_test_work(void)
{
    policy();
    native_intention();
    assert(run_render_schedule(0)==run_render_schedule(1));
    puts("Native clock, needs precedence, bounded work, interruption/resumption, pause and render-rate tests passed.");
}
