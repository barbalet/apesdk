/* Cycle 01 observable baseline: explicit scalar encoding, never struct padding. */
#include "../../universe/universe.h"
#include "../../entity/entity.h"
#ifdef BRONZE_SIM
#include "../hybrid/bronze_hybrid.h"
#include "../actors/bronze_actors.h"
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef BRONZE_SIM
/* Headless error sink, as in the native unit harness; no Bronze link dependency. */
static int errors;
n_int draw_error(n_constant_string message, n_constant_string location, n_int line)
{
    fprintf(stderr, "%s:%ld: %s\n", location, (long)line, message);
    errors++;
    return 0;
}
#endif

static uint64_t mix(uint64_t hash, uint64_t value)
{
    unsigned i;
    for (i = 0; i < 8; i++) { hash ^= value & 255; hash *= UINT64_C(1099511628211); value >>= 8; }
    return hash;
}
static void sample(unsigned seed, unsigned cycle)
{
    simulated_group *group = sim_group();
    uint64_t actors = UINT64_C(14695981039346656037);
    uint64_t terrain = actors, weather = actors, lightning = actors;
    n_uint i;
#ifdef BRONZE_SIM
    BronzeActorRegistry* registry=bronze_actors_current();
    assert(bronze_actors_count(registry)==group->num);
    for(i=0;i<group->num;i++) {
        BronzeActor* actor=bronze_actors_for_being(registry,&group->beings[i]);
        assert(actor && actor->id && bronze_actors_resolve(registry,actor->id)==&group->beings[i]);
    }
#endif
    for (i = 0; i < group->num; i++)
    {
        simulated_being *being = &group->beings[i];
        actors = mix(actors, being_location_x(being));
        actors = mix(actors, being_location_y(being));
        actors = mix(actors, being_energy(being));
        actors = mix(actors, being_state(being));
        actors = mix(actors, being_facing(being));
        actors = mix(actors, being_drive(being, DRIVE_FATIGUE));
    }
    for (i = 0; i < MAP_AREA; i++)
    {
        terrain = mix(terrain, land_topography()[i]);
        weather = mix(weather, (uint64_t)(int64_t)land_weather(0)[i]);
        lightning = mix(lightning, land_weather_lightning(0)[i]);
    }
    printf("BASE native %u %u %u %u %lu %zu %016llx %016llx %016llx\n",
           seed, cycle, (unsigned)land_date(), (unsigned)land_time(),
           (unsigned long)group->num, sizeof(simulated_being),
           (unsigned long long)actors, (unsigned long long)terrain, (unsigned long long)weather);
    printf("BASE lightning %u %u %016llx\n", seed, cycle, (unsigned long long)lightning);
#ifdef BRONZE_SIM
    {
        n_uint count;
        const bronze_site *sites = bronze_hybrid_sites(&count);
        const bronze_economy *economy = bronze_hybrid_economy();
        uint64_t hash = UINT64_C(14695981039346656037);
        for (i = 0; i < count; i++)
        {
            hash = mix(hash, sites[i].x); hash = mix(hash, sites[i].y);
            hash = mix(hash, sites[i].kind); hash = mix(hash, (uint64_t)sites[i].stock);
        }
        printf("BASE bronze %u %u %lu %016llx %.0f %.0f %.0f %.0f %.0f %lu\n",
               seed, cycle, (unsigned long)count, (unsigned long long)hash,
               economy->grain, economy->copper, economy->tin, economy->charcoal,
               economy->tools, (unsigned long)economy->work_events);
    }
#endif
}
int main(int argc, char **argv)
{
    unsigned cycle, seed = argc > 1 ? (unsigned)strtoul(argv[1], NULL, 0) : 42;
    if (!sim_init(KIND_START_UP, seed, MAP_AREA, 0)) return 1;
    for (cycle = 1; cycle <= 120; cycle++)
    {
        sim_cycle();
        if (cycle == 1 || cycle == 60 || cycle == 120) sample(seed, cycle);
    }
    sim_close();
#ifndef BRONZE_SIM
    return errors ? 1 : 0;
#else
    return 0;
#endif
}
