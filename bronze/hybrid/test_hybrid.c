#include "bronze_hybrid.h"
#include "../work/bronze_work.h"
#include "../../universe/universe.h"
#include "../../gui/gui.h"
#include "../../shared.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static n_uint pixels;
static n_int dimension;
static void check_pixel(n_int x, n_int y, n_byte color, void *context)
{
    (void)color; (void)context;
    assert(x >= 0 && y >= 0 && x < dimension && y < dimension);
    pixels++;
}
static void advance_minutes(unsigned minutes)
{
    for (unsigned minute = 0; minute < minutes; minute++) land_cycle();
}

int main(void)
{
    bronze_site first[10];
    const bronze_site *sites;
    n_uint count, i;
    n_byte2 location[2];
    simulated_group *group;
    assert(draw_offscreen(sim_init(KIND_START_UP, 42u, MAP_AREA + TERRAIN_WINDOW_AREA + CONTROL_WINDOW_AREA, 0)));
    sim_cycle();
    draw_window(MAP_DIMENSION, MAP_DIMENSION);
    weather_set_lightning(0, 3, 5, 0);
    draw_cycle(0, 1 << NUM_VIEW);
    {
        n_byte before = draw_weather_grayscale()[3 + 5 * MAP_DIMENSION];
        weather_set_lightning(0, 3, 5, 255);
        draw_cycle(0, 1 << NUM_VIEW);
        assert(draw_weather_grayscale()[3 + 5 * MAP_DIMENSION] == before);
    }
    assert(weather_lightning(3, 5) == 0);
    assert(weather_seven_values(MAPSPACE_TO_APESPACE(3), MAPSPACE_TO_APESPACE(5)) < WEATHER_SEVEN_LIGHTNING_DAY);
    weather_cycle();
    assert(weather_lightning(3, 5) == 0);
    BronzeLandscape *landscape = bronze_actors_landscape(bronze_actors_current());
    sites = bronze_hybrid_sites(&count);
    for (i = 0; i < count; i++) assert(sites[i].stock == 0);
    assert(bronze_hybrid_economy().grain == 0 && bronze_hybrid_economy().tools == 0);
    BronzeLandPolicy policy = bronze_land_defaults();
    policy.patch_initial_per_cell = 8;
    landscape->policy = policy;
    bronze_hybrid_reset();
    bronze_hybrid_sites(&count);
    assert(count > 0 && count <= 10);
    n_uint first_count = count, field = count, copper = count;
    unsigned kinds[5] = {0};
    static n_byte used[MAP_AREA], original_height[MAP_AREA];
    const n_byte *height = land_topography();
    memcpy(original_height, height, sizeof(original_height));
    memcpy(first, sites, sizeof(first));
    for (i = 0; i < count; i++) {
        const bronze_site *site = &sites[i];
        assert(site->kind >= BRONZE_TOWN && site->kind <= BRONZE_WOODLAND);
        assert(++kinds[site->kind] <= 2);
        assert(site->area > 0 && site->area <= BRONZE_PATCH_LIMIT);
        assert(site->capacity == (site->kind == BRONZE_TOWN ? 0 : 8 * site->area));
        assert(site->stock == site->capacity);
        if (site->kind == BRONZE_FIELD && bronze_hybrid_place(site->place_id)->owner) field = i;
        if (site->kind == BRONZE_COPPER_MINE && bronze_hybrid_place(site->place_id)->owner) copper = i;
        if (site->kind == BRONZE_WOODLAND) assert(site->area >= 8);
        for (n_uint j = 0; j < site->area; j++) {
            n_int x = site->cells[j].x, y = site->cells[j].y;
            assert(x >= 0 && x < MAP_DIMENSION && y >= 0 && y < MAP_DIMENSION);
            n_uint index = x + y * MAP_DIMENSION;
            assert(!used[index]++ && height[index] > TIDE_MAX);
            int connected = j == 0;
            for (n_uint k = 0; k < j; k++) {
                n_int dx = labs(x - site->cells[k].x), dy = labs(y - site->cells[k].y);
                if (dx > MAP_DIMENSION / 2) dx = MAP_DIMENSION - dx;
                if (dy > MAP_DIMENSION / 2) dy = MAP_DIMENSION - dy;
                if (dx + dy == 1) connected = 1;
            }
            assert(connected);
            if (site->kind == BRONZE_FIELD) {
                assert(abs(height[index] - height[(x+1)%MAP_DIMENSION + y*MAP_DIMENSION]) <= policy.field_slope);
                assert(abs(height[index] - height[(x+MAP_DIMENSION-1)%MAP_DIMENSION + y*MAP_DIMENSION]) <= policy.field_slope);
                assert(abs(height[index] - height[x + ((y+1)%MAP_DIMENSION)*MAP_DIMENSION]) <= policy.field_slope);
                assert(abs(height[index] - height[x + ((y+MAP_DIMENSION-1)%MAP_DIMENSION)*MAP_DIMENSION]) <= policy.field_slope);
                int water = 0;
                for (int dy = -(int)policy.field_coast; dy <= (int)policy.field_coast && !water; dy++) {
                    int span = (int)policy.field_coast - abs(dy);
                    for (int dx = -span; dx <= span; dx++)
                        if (height[(x+dx+MAP_DIMENSION)%MAP_DIMENSION +
                                   ((y+dy+MAP_DIMENSION)%MAP_DIMENSION)*MAP_DIMENSION] <= TIDE_MAX)
                            water = 1;
                }
                assert(water);
            }
            if (site->kind == BRONZE_COPPER_MINE) assert(height[index] >= 150 && height[index] < policy.tin_height);
            if (site->kind == BRONZE_TIN_MINE) assert(height[index] >= policy.tin_height);
        }
    }
    for (i = 0; i < 5; i++) assert(kinds[i]);
    assert(copper < count && field < count);
    uint64_t retired_home = landscape->towns[0].home;
    policy.field_moisture = 255;
    landscape->policy = policy;
    bronze_hybrid_reset();
    bronze_hybrid_sites(&count);
    for (i = 0; i < count; i++) assert(sites[i].kind != BRONZE_FIELD);
    assert(landscape->town_count == 0 && !bronze_hybrid_place(retired_home));
    policy.field_moisture = bronze_land_defaults().field_moisture;
    landscape->policy = policy;
    bronze_hybrid_reset();
    bronze_hybrid_sites(&count);
    assert(count == first_count);
    for (i = 0; i < count; i++) {
        assert(sites[i].place_id != first[i].place_id);
        assert(sites[i].kind == first[i].kind && sites[i].area == first[i].area);
        assert(memcmp(sites[i].cells, first[i].cells, sizeof(first[i].cells)) == 0);
    }
    assert(memcmp(original_height, height, sizeof(original_height)) == 0);
    dimension = MAP_DIMENSION;
    bronze_hybrid_draw(dimension, check_pixel, NULL);
    assert(pixels > 100);
    pixels = 0; dimension = HI_RES_MAP_DIMENSION;
    bronze_hybrid_draw(dimension, check_pixel, NULL);
    assert(pixels > 100);
    group = sim_group();
    /* Use a living native being at a field, and verify bounded production
       without Bronze changing the being's position or population. */
    location[0] = MAPSPACE_TO_APESPACE(sites[field].x);
    location[1] = MAPSPACE_TO_APESPACE(sites[field].y);
    being_set_location(&group->beings[0], location);
    being_set_speed(&group->beings[0],0);
    being_set_state(&group->beings[0],BEING_STATE_AWAKE);
    being_energy_delta(&group->beings[0],BEING_FULL-being_energy(&group->beings[0]));
    memset(being_drives(&group->beings[0]),0,DRIVES);
    BronzeProfessionPolicy professions = {
        .count = 1, .min_age_days = AGE_OF_MATURITY, .site_capacity = 256,
        .retry_minutes = 60, .skill_start = 2, .skill_max = 5, .skill_gain = 1,
        .vocations = {{"field_worker", 1, BRONZE_JOB_FIELD}}
    };
    bronze_actors_stage_professions(bronze_actors_current(), &professions);
    advance_minutes(720 - land_time());
    assert(bronze_actors_world_reset(group));
    bronze_hybrid_reset();
    BronzeActor *worker = bronze_actors_for_being(bronze_actors_current(), &group->beings[0]);
    assert(worker->vocation == 0 && worker->job_status == BRONZE_JOB_ASSIGNED);
    assert(worker->history_count == 1 && bronze_actor_skill(worker, 0) == 2);
    for (field = 0; field < count && sites[field].place_id != worker->workplace; field++) {}
    assert(field < count && sites[field].kind == BRONZE_FIELD);
    location[0] = MAPSPACE_TO_APESPACE(sites[field].x);
    location[1] = MAPSPACE_TO_APESPACE(sites[field].y);
    being_set_location(&group->beings[0], location);
    assert(being_awake(&group->beings[0]));
    bronze_work_begin(group); bronze_work_end(group);
    bronze_hybrid_cycle(group);
    {
        double before = bronze_hybrid_economy().grain;
        bronze_hybrid_cycle(group);
        assert(bronze_hybrid_economy().grain == before);
    }
    for (i = 0; i < 4; i++)
    {
        advance_minutes(TIME_HOUR_MINUTES);
        bronze_work_begin(group); bronze_work_end(group);
        bronze_hybrid_cycle(group);
    }
    assert(bronze_hybrid_economy().grain > 0);
    assert(bronze_actor_skill(worker, 0) == 5);
    const BronzePlace *production = bronze_hybrid_place(sites[field].place_id);
    assert(production && production->kind == BRONZE_PLACE_PRODUCTION && production->owner);
    for (unsigned i = 0; i < landscape->town_count; i++) {
        const BronzeTownship *town = &landscape->towns[i];
        assert(town->stores.grain == (town->home == production->owner ? bronze_hybrid_economy().grain : 0));
    }
    assert(sites[field].stock >= 0 && sites[field].stock < sites[field].capacity);
    assert(being_location_x(&group->beings[0]) == location[0]);
    assert(being_location_y(&group->beings[0]) == location[1]);
    location[0] = MAPSPACE_TO_APESPACE(sites[copper].x);
    location[1] = MAPSPACE_TO_APESPACE(sites[copper].y);
    being_set_location(&group->beings[0], location);
    landscape->sites[copper].stock = 1;
    advance_minutes(TIME_HOUR_MINUTES);
    bronze_work_begin(group); bronze_work_end(group);
    bronze_hybrid_cycle(group);
    assert(sites[copper].stock == 1);
    double copper_stock = bronze_hybrid_economy().copper;
    advance_minutes(TIME_DAY_MINUTES);
    bronze_work_begin(group); bronze_work_end(group);
    bronze_hybrid_cycle(group);
    assert(sites[copper].stock == 1 && bronze_hybrid_economy().copper == copper_stock);
    bronze_hybrid_reset();
    assert(bronze_hybrid_economy().grain == 0);
    assert(bronze_hybrid_economy().tools == 0);
    /* Force a housing shortage on the same dry cell. Existing homes are
       cleared by the world rebuild and only the available slots are filled. */
    const BronzePlace *home = bronze_hybrid_place(landscape->towns[0].home);
    location[0] = MAPSPACE_TO_APESPACE(home->location.x);
    location[1] = MAPSPACE_TO_APESPACE(home->location.y);
    n_uint cell = home->location.x + home->location.y * MAP_DIMENSION;
    for (i = 0; i < group->num; i++) being_set_location(&group->beings[i], location);
    landscape->policy.town_capacity = 1;
    landscape->policy.town_initial_grain = 3;
    bronze_hybrid_reset();
    assert(bronze_hybrid_economy().grain == 3 * landscape->town_count);
    unsigned housed = 0, reachable = 0;
    for (unsigned t = 0; t < landscape->town_count; t++) {
        const BronzeTownship *town = &landscape->towns[t];
        const BronzePlace *storage = bronze_hybrid_place(town->storage);
        const BronzePlace *service = bronze_hybrid_place(town->service);
        assert(storage && storage->kind == BRONZE_PLACE_STORAGE && storage->owner == town->home);
        assert(service && service->kind == BRONZE_PLACE_SERVICE && service->owner == town->home);
        assert(town->residents <= 1);
        if (town->distance[cell]) reachable++;
    }
    for (i = 0; i < group->num; i++) {
        BronzeActor *actor = bronze_actors_for_being(bronze_actors_current(), &group->beings[i]);
        assert(actor);
        if (actor->home) {
            const BronzePlace *workplace = bronze_hybrid_place(actor->workplace);
            assert(bronze_hybrid_place(actor->home)->kind == BRONZE_PLACE_HOME);
            assert(!actor->workplace || (workplace && workplace->owner == actor->home));
            housed++;
        } else {
            assert(actor->workplace == 0);
        }
    }
    assert(housed == reachable && housed > 0 && housed < group->num);
    /* Assign four service slots in a 3:1 ratio. The first being is a child
       and an unavailable coastal vocation cannot consume a slot. */
    professions.count = 3;
    professions.site_capacity = 4;
    professions.vocations[0] = (BronzeVocation){"speaker", 3, BRONZE_JOB_SERVICE};
    professions.vocations[1] = (BronzeVocation){"helper", 1, BRONZE_JOB_SERVICE};
    professions.vocations[2] = (BronzeVocation){"coastal_worker", 100, BRONZE_JOB_COAST};
    bronze_actors_stage_professions(bronze_actors_current(), &professions);
    landscape->policy.town_capacity = LARGE_SIM;
    group->beings[0].constant.date_of_birth = land_date();
    assert(bronze_actors_world_reset(group));
    bronze_hybrid_reset();
    unsigned employed[3] = {0};
    n_uint vacancy = group->num, waiting = group->num;
    for (i = 0; i < group->num; i++) {
        BronzeActor *actor = bronze_actors_for_being(bronze_actors_current(), &group->beings[i]);
        if (i == 0) assert(actor->job_status == BRONZE_JOB_CHILD && !actor->workplace);
        if (actor->vocation < 3) {
            employed[actor->vocation]++;
            assert(bronze_hybrid_place(actor->workplace)->kind == BRONZE_PLACE_SERVICE);
            vacancy = i;
        } else if (actor->job_status == BRONZE_JOB_FULL && waiting == group->num) waiting = i;
    }
    assert(employed[0] == 3 && employed[1] == 1 && employed[2] == 0);
    assert(vacancy < group->num && waiting < group->num);
    bronze_actors_death(group, &group->beings[vacancy]);
    advance_minutes(TIME_HOUR_MINUTES);
    bronze_hybrid_cycle(group);
    BronzeActor *replacement = bronze_actors_for_being(bronze_actors_current(), &group->beings[waiting]);
    assert(replacement->job_status == BRONZE_JOB_ASSIGNED && replacement->history_count == 1);
    {
        n_byte4 *hud = calloc(512 * 512 + 2, sizeof(n_byte4));
        assert(hud);
        hud[0] = 123; hud[512 * 512 + 1] = 456;
        bronze_hybrid_draw_hud(hud + 1, 512, 512);
        assert(hud[0] == 123 && hud[512 * 512 + 1] == 456);
        assert(hud[1] == 0); /* HUD preserves map pixels above its panel. */
        assert(hud[512 * 512] != 0);
        free(hud);
    }
    sim_close();
    puts("Bronze hybrid terrain, symbols, bounded economy and reset checks passed.");
    return 0;
}
