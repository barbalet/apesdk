#include "bronze_hybrid.h"
#include "../work/bronze_work.h"
#include "../tasks/bronze_tasks.h"
#include "../scenario/bronze_scenario.h"
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

static void task_tick(simulated_group *group)
{
    land_cycle();
    bronze_work_begin(group);
    bronze_tasks_begin(group);
    bronze_work_end(group);
    bronze_tasks_end(group);
}

static BronzeScenario *task_scenario(void)
{
    FILE *file = fopen("tasks.bronze", "w");
    assert(file);
    fputs("kinds { resources { grain copper } }\n"
          "actions { action gather 1 2 action eat 1 2 action rest 0 0 action move_to 1 1 action roam 1 1 action deposit 1 2 }\n"
          "resources { resource grain { habitat field capacity 32 renew 0.5 nutrition 1 market_target 32 } "
          "resource copper { habitat mine_copper capacity 16 renew 0 nutrition 0 market_target 16 } }\n"
          "world { seed 42 occupation_field_worker_weight 1 occupation_field_worker_workplace field "
          "occupation_skill_start 2 occupation_skill_max 3 occupation_skill_gain 2 occupation_site_capacity 256 }\n"
          "vocations { vocation field_worker {\n"
          "task work {\n"
          "move_to field\n"
          "when hunger < 0.90 { chance 100 {\n"
          "gather grain 4\n"
          "} }\n"
          "rest\n"
          "when hunger > 0.25 {\n"
          "eat grain 1\n"
          "roam field\n"
          "}\n"
          "when fatigue > 0.2 {\n"
          "deposit grain 2\n"
          "}\n"
          "}\n"
          "rule first { when prob 1 do work weight 3 }\n"
          "rule backup { when prob 1 do work weight 1 }\n"
          "rule never { when prob 0 do work weight 100 }\n"
          "} }\n", file);
    assert(!fclose(file));
    BronzeScenario *scenario = bronze_scenario_create();
    int loaded = bronze_scenario_load(scenario, "tasks.bronze", "task fixture");
    if (!loaded) fprintf(stderr, "%s\n", bronze_scenario_diagnostics(scenario));
    assert(loaded);
    return scenario;
}

static double cargo_amount(const BronzeStock *stocks, size_t count, unsigned kind)
{
    for (size_t i = 0; i < count; i++)
        if (stocks[i].kind == kind) return stocks[i].amount;
    return 0;
}

static void recipe_chains(simulated_group *group, BronzeLandscape *landscape)
{
    FILE *file = fopen("recipes.bronze", "w");
    assert(file);
    fputs("kinds { resources { grain meal clay copper tin charcoal } items { pot tool glazed_pot } }\n"
          "actions { action craft 1 2 action eat 1 2 action deposit 1 3 action fetch 1 3 }\n"
          "resources { resource meal { habitat field capacity 0 renew 0 nutrition 0.5 market_target 0 } }\n"
          "recipes {\n"
          "recipe cook { output meal 2 input grain 1 input grain 1 }\n"
          "recipe forge { output tool 1 input copper 1 input tin 1 input charcoal 1 }\n"
          "recipe pot { output pot 1 input clay 2 }\n"
          "recipe glaze { output glazed_pot 1 input pot 1 input charcoal 1 }\n"
          "recipe ration { output meal 1 input grain 1 }\n"
          "}\n"
          "world { household_capacity 4 ration_daily 1 food_spoilage 0.5 occupation_maker_weight 1 occupation_maker_workplace service recipe_forge_minutes 3 "
          "recipe_pot_minutes 2 recipe_pot_tool tool recipe_glaze_tool tool "
          "recipe_glaze_minimum_skill 2 recipe_ration_workplace workplace }\n"
          "vocations { vocation maker { task work {\n"
          "craft cook\n"
          "eat meal\n"
          "craft tool 2\n"
          "craft pot\n"
          "deposit pot\n"
          "craft glaze\n"
          "deposit tool 2\n"
          "deposit meal household 1\n"
          "fetch meal household 1\n"
          "deposit meal 1\n"
          "fetch meal 1\n"
          "} rule always { when true do work } } }\n", file);
    assert(!fclose(file));
    BronzeScenario *scenario = bronze_scenario_create();
    int loaded = bronze_scenario_load(scenario, "recipes.bronze", "recipe fixture");
    if (!loaded) fprintf(stderr, "%s\n", bronze_scenario_diagnostics(scenario));
    assert(loaded);
    BronzeActorRegistry *registry = bronze_actors_current();
    bronze_actors_stage_professions(registry, bronze_scenario_professions(scenario));
    bronze_actors_stage_program(registry, bronze_scenario_program(scenario));
    landscape->policy = bronze_scenario_land_policy(scenario);
    bronze_scenario_destroy(scenario);

    const BronzePlace *entrance = bronze_hybrid_place(landscape->towns[0].service);
    assert(entrance);
    n_byte2 location[] = {MAPSPACE_TO_APESPACE(entrance->location.x),
                          MAPSPACE_TO_APESPACE(entrance->location.y)};
    group->num = 2;
    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        being->constant.date_of_birth = 0;
        being_set_location(being, location);
        being_set_speed(being, 0);
        being_set_state(being, BEING_STATE_AWAKE);
        being_set_goal_none(being);
        being_energy_delta(being, BEING_FULL - being_energy(being));
        memset(being_drives(being), 0, DRIVES);
    }
    being_drives(&group->beings[1])[DRIVE_FATIGUE] = 255;
    landscape->policy.town_initial_grain = 0;
    advance_minutes((720 + TIME_DAY_MINUTES - land_time()) % TIME_DAY_MINUTES);
    assert(bronze_actors_world_reset(group));
    bronze_hybrid_reset();

    simulated_being *being = &group->beings[0];
    BronzeActor *worker = bronze_actors_for_being(registry, being);
    BronzeActor *competitor = bronze_actors_for_being(registry, &group->beings[1]);
    assert(worker->job_status == BRONZE_JOB_ASSIGNED && competitor->home == worker->home);
    BronzeTownship *town = NULL;
    for (unsigned i = 0; i < landscape->town_count; i++)
        if (landscape->towns[i].home == worker->home) town = &landscape->towns[i];
    assert(town && worker->workplace == town->service);
    const BronzeProgram *program = bronze_actors_program(registry);
    const RecipeDef *recipes = program->config->recipes.data;
    assert(bronze_task_craft(worker, being, town->service, &recipes[0], &program->recipes[0], 1, 0) == BRONZE_TASK_STOCK);
    assert(bronze_task_craft(worker, being, town->service, &recipes[2], &program->recipes[2], 1, 0) == BRONZE_TASK_TOOL);
    assert(bronze_task_craft(worker, being, town->service, &recipes[3], &program->recipes[3], 1, 0) == BRONZE_TASK_SKILL);
    assert(worker->item_count == 0 && !worker->execution.craft_recipe);

    const BronzeStock supplies[] = {{0, 3}, {2, 2}, {3, 2}, {4, 2}, {5, 3}};
    worker->resources = malloc(sizeof(supplies));
    assert(worker->resources);
    memcpy(worker->resources, supplies, sizeof(supplies));
    worker->resource_count = sizeof(supplies) / sizeof(supplies[0]);
    for (size_t i = 0; i < worker->resource_count; i++) {
        unsigned kind = supplies[i].kind;
        double amount = supplies[i].amount - (kind == 0 ? 1 : 0);
        assert(bronze_task_deposit(worker, being, town->storage, (int)kind, amount, 1) == BRONZE_TASK_RUNNING);
    }

    task_tick(group);
    assert(worker->remaining_ticks == 3 && town->resources[0] == 2 && town->reserved_resources[0] == 2);
    assert(bronze_task_craft(competitor, &group->beings[1], town->service,
                             &recipes[0], &program->recipes[0], 1, 0) == BRONZE_TASK_STOCK);
    assert(bronze_task_store_transfer(competitor, &group->beings[1], town->storage,
                                     0, 1, 0, 0, 1, 1) == BRONZE_TASK_STOCK);
    being_energy_delta(being, BEING_HUNGRY - being_energy(being));
    task_tick(group);
    assert(worker->remaining_ticks == 3 && town->reserved_resources[0] == 2);
    assert(bronze_actor_set_job(registry, worker, BRONZE_KIND_NONE, 0, 0, bronze_native_tick()));
    assert(town->reserved_resources[0] == 0 && town->resources[0] == 2);
    assert(cargo_amount(worker->resources, worker->resource_count, 1) == 0);
    assert(bronze_actor_set_job(registry, worker, 0, town->service, 0, bronze_native_tick()));
    being_energy_delta(being, BEING_FULL - being_energy(being));

    int native_meal = 0;
    for (unsigned i = 0; i < 40 && worker->execution.status != BRONZE_TASK_COMPLETE; i++) {
        land_cycle();
        bronze_work_begin(group);
        bronze_tasks_begin(group);
        int inject = !native_meal && worker->execution.operation &&
                     worker->execution.operation->action_code == BRZ_ACTION_EAT;
        if (inject) being_add_state(being, BEING_STATE_EATING);
        bronze_work_end(group);
        bronze_tasks_end(group);
        if (inject) {
            assert(landscape->consumed[1] == 0 && worker->remaining_ticks == 1);
            being_set_state(being, BEING_STATE_AWAKE);
            native_meal = 1;
        }
    }
    assert(native_meal && worker->execution.status == BRONZE_TASK_COMPLETE && worker->instruction == 11);
    assert(worker->ration_used == 1 && worker->household_resources[1] == 0 && town->resources[1] == 0);
    assert(town->resources[0] == 0 && town->resources[2] == 0 && town->resources[3] == 0 &&
           town->resources[4] == 0 && town->resources[5] == 0);
    assert(town->items[0] == 0 && town->items[1] == 2);
    assert(cargo_amount(worker->items, worker->item_count, 2) == 1);
    assert(cargo_amount(worker->resources, worker->resource_count, 1) == 1 && landscape->consumed[1] == 1);
    assert(bronze_actor_skill(worker, 0) == 4);
    assert(bronze_hybrid_economy().tools == 2);

    /* The workplace variant consumes personal cargo instead of the warehouse. */
    assert(bronze_task_craft(worker, being, town->service, &recipes[4], &program->recipes[4], 1, 0) == BRONZE_TASK_RUNNING);
    assert(bronze_task_deposit(worker, being, town->storage, 0, 1, 1) == BRONZE_TASK_STOCK);
    assert(bronze_task_craft(worker, being, town->service, &recipes[4], &program->recipes[4], 1, 1) == BRONZE_TASK_RUNNING);
    assert(cargo_amount(worker->resources, worker->resource_count, 0) == 0);
    assert(cargo_amount(worker->resources, worker->resource_count, 1) == 2);
    assert(bronze_task_store_transfer(worker, being, worker->home, 1, 1, 0, 1, 0, 1) == BRONZE_TASK_RUNNING);
    assert(bronze_task_store_transfer(competitor, &group->beings[1], competitor->home,
                                     1, 1, 0, 1, 1, 1) == BRONZE_TASK_STOCK);
    assert(worker->household_resources[1] == 1);
    assert(bronze_task_deposit(worker, being, town->storage, 1, 1, 1) == BRONZE_TASK_RUNNING);
    assert(bronze_task_store_transfer(worker, being, town->storage, 1, 1, 0, 0, 1, 1) == BRONZE_TASK_RATION);
    n_int energy = being_energy(being);
    n_byte2 away[] = {(n_byte2)((location[0] + MAPSPACE_TO_APESPACE(1)) & APESPACE_BOUNDS), location[1]};
    being_set_location(being, away);
    assert(bronze_task_store_transfer(worker, being, worker->home, 1, 1, 0, 1, 1, 1) == BRONZE_TASK_TRAVEL);
    assert(being_energy(being) == energy && worker->household_resources[1] == 1);
    being_set_location(being, location);
    advance_minutes(TIME_DAY_MINUTES - land_time());
    bronze_hybrid_cycle(group);
    assert(town->resources[1] == 0.5 && worker->household_resources[1] == 0.5);
    assert(landscape->spoiled[1] == 1 && landscape->consumed[1] == 1);
    assert(bronze_task_store_transfer(worker, being, town->storage, 1, 0.5, 0, 0, 1, 1) == BRONZE_TASK_RUNNING);
    assert(worker->ration_used == 0.5 && town->resources[1] == 0);
    assert(cargo_amount(worker->resources, worker->resource_count, 1) +
           worker->household_resources[1] + landscape->spoiled[1] + landscape->consumed[1] == 3);

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
    n_uint base_pixels = pixels;
    pixels = 0; dimension = HI_RES_MAP_DIMENSION;
    bronze_hybrid_draw(dimension, check_pixel, NULL);
    n_uint scale = HI_RES_MAP_DIMENSION / MAP_DIMENSION;
    assert(pixels == base_pixels * scale * scale);
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
    BronzeScenario *scenario = task_scenario();
    BronzeProfessionPolicy professions = *bronze_scenario_professions(scenario);
    bronze_actors_stage_professions(bronze_actors_current(), &professions);
    bronze_actors_stage_program(bronze_actors_current(), bronze_scenario_program(scenario));
    bronze_scenario_destroy(scenario); /* The world owns the pending program now. */
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
    uint64_t initial_rng = worker->execution.rng;
    task_tick(group);
    assert(worker->instruction == 1 && worker->remaining_ticks == 0);
    assert(!strcmp(worker->execution.action, "move_to"));
    task_tick(group);
    assert(worker->remaining_ticks == 3 && worker->execution.target == worker->workplace);
    uint64_t selected_rng = worker->execution.rng;
    assert(selected_rng == initial_rng + UINT64_C(0x9e3779b97f4a7c15) * 5);
    double before = sites[field].stock;
    assert(before == 32 && sites[field].resources[0].reserved == 4);
    BronzeActor contender = {.home = worker->home, .workplace = worker->workplace};
    landscape->policy.carry_capacity = 1;
    assert(bronze_task_gather(&contender, &group->beings[0], worker->workplace, 0, 2, 0) == BRONZE_TASK_CAPACITY);
    landscape->policy.carry_capacity = bronze_land_defaults().carry_capacity;
    assert(bronze_task_gather(&contender, &group->beings[0], worker->workplace, 0, 29, 0) == BRONZE_TASK_STOCK);
    assert(bronze_task_gather(&contender, &group->beings[0], worker->workplace, 0, 28, 0) == BRONZE_TASK_RUNNING);
    assert(sites[field].resources[0].reserved == 32);
    bronze_task_release_claim(landscape, &contender);
    assert(sites[field].resources[0].reserved == 4);
    being_energy_delta(&group->beings[0], BEING_HUNGRY - being_energy(&group->beings[0]));
    task_tick(group);
    assert(worker->remaining_ticks == 3 && sites[field].stock == before);
    assert(worker->execution.rng == selected_rng);
    being_energy_delta(&group->beings[0], BEING_FULL - being_energy(&group->beings[0]));
    for (i = 0; i < 3; i++) task_tick(group);
    assert(worker->instruction == 2 && sites[field].stock == before - 4);
    assert(worker->work.interruptions && worker->work.resumptions);
    assert(bronze_actor_skill(worker, 0) == 3);
    bronze_tasks_end(group);
    assert(bronze_hybrid_economy().grain == 0 && sites[field].stock == before - 4);
    assert(worker->resource_count == 1 && worker->resources[0].amount == 4);
    assert(sites[field].resources[0].reserved == 0);
    being_drives(&group->beings[0])[DRIVE_FATIGUE] = 4;
    n_int energy_before_rest = being_energy(&group->beings[0]);
    task_tick(group);
    assert(worker->remaining_ticks == 1);
    drives_fatigue(&group->beings[0]);
    task_tick(group);
    assert(worker->instruction == 3 && being_drive(&group->beings[0], DRIVE_FATIGUE) == 3);
    assert(being_energy(&group->beings[0]) == energy_before_rest);
    being_drives(&group->beings[0])[DRIVE_HUNGER] = 100;
    task_tick(group);
    assert(worker->instruction == 4 && worker->resources[0].amount == 3);
    assert(being_energy(&group->beings[0]) == BEING_FULL && landscape->consumed[0] == 1);
    assert(being_drive(&group->beings[0], DRIVE_HUNGER) == 100);
    task_tick(group);
    assert(worker->execution.status == BRONZE_TASK_UNSUPPORTED);
    assert(!strcmp(worker->execution.action, "roam") && worker->instruction == 4);
    task_tick(group);
    assert(worker->execution.rng == selected_rng && bronze_hybrid_economy().grain == 0);

    uint64_t storage = bronze_task_storage(worker->home);
    const BronzePlace *store = bronze_hybrid_place(storage);
    assert(store);
    assert(bronze_task_deposit(worker, &group->beings[0], storage, 0, 1, 1) == BRONZE_TASK_TRAVEL);
    n_byte2 store_location[] = {MAPSPACE_TO_APESPACE(store->location.x), MAPSPACE_TO_APESPACE(store->location.y)};
    being_set_location(&group->beings[0], store_location);
    assert(bronze_task_gather(worker, &group->beings[0], worker->workplace, 0, 1, 0) == BRONZE_TASK_TRAVEL);
    landscape->policy.storage_capacity = 2;
    assert(bronze_task_deposit(worker, &group->beings[0], storage, 0, 3, 1) == BRONZE_TASK_CAPACITY);
    assert(bronze_task_deposit(worker, &group->beings[0], storage, 0, 2, 1) == BRONZE_TASK_RUNNING);
    assert(worker->resources[0].amount == 1 && bronze_hybrid_economy().grain == 2);
    assert(sites[field].stock + worker->resources[0].amount + bronze_hybrid_economy().grain +
           landscape->consumed[0] == before);
    landscape->policy.storage_capacity = bronze_land_defaults().storage_capacity;
    being_set_location(&group->beings[0], location);
    const BronzePlace *production = bronze_hybrid_place(sites[field].place_id);
    assert(production && production->kind == BRONZE_PLACE_PRODUCTION && production->owner);
    for (unsigned i = 0; i < landscape->town_count; i++) {
        const BronzeTownship *town = &landscape->towns[i];
        assert(town->resources[0] == (town->home == production->owner ? bronze_hybrid_economy().grain : 0));
    }
    assert(sites[field].stock >= 0 && sites[field].stock < sites[field].capacity);
    assert(being_location_x(&group->beings[0]) == location[0]);
    assert(being_location_y(&group->beings[0]) == location[1]);
    /* Put the assigned footprint across the seam. Execute each accepted
       steering decision through the same native movement integrator. */
    bronze_site *patch = &landscape->sites[field];
    bronze_site saved_patch = *patch;
    n_byte *map = land_topography();
    const n_uint seam_y = 10;
    n_uint left = seam_y * MAP_DIMENSION + MAP_DIMENSION - 1;
    n_uint right = seam_y * MAP_DIMENSION;
    map[left] = map[right] = TIDE_MAX + 1;
    patch->cells[0] = (bronze_land_cell){0, seam_y};
    patch->area = 1;
    location[0] = MAPSPACE_TO_APESPACE(MAP_DIMENSION - 1) + 32;
    location[1] = MAPSPACE_TO_APESPACE(seam_y) + 32;
    being_set_location(&group->beings[0], location);
    being_set_goal_none(&group->beings[0]);
    memset(being_drives(&group->beings[0]), 0, DRIVES);
    advance_minutes(TIME_HOUR_MINUTES);
    task_tick(group);
    assert(worker->execution.status == BRONZE_TASK_TRAVEL);
    assert(worker->execution.route.length == 1 && worker->instruction == 0);
    assert(being_location_x(&group->beings[0]) == location[0]);

    being_energy_delta(&group->beings[0], BEING_HUNGRY - being_energy(&group->beings[0]));
    task_tick(group);
    assert(worker->execution.status == BRONZE_TASK_ROUTE_PAUSED);
    assert(worker->execution.route.length == 1 && worker->instruction == 0);
    being_energy_delta(&group->beings[0], BEING_FULL - being_energy(&group->beings[0]));
    being_set_goal_location(&group->beings[0], location[0], location[1]);
    task_tick(group);
    assert(worker->execution.status == BRONZE_TASK_ROUTE_PAUSED);
    assert(being_check_goal(&group->beings[0], GOAL_LOCATION));
    being_set_goal_none(&group->beings[0]);

    for (unsigned step = 0; step < 32 && !worker->instruction; step++) {
        land_cycle();
        bronze_work_begin(group);
        bronze_tasks_begin(group);
        if (worker->execution.status == BRONZE_TASK_TRAVEL) {
            n_int max_honor = 0;
            group->beings[0].delta.awake = FULLY_AWAKE;
            being_set_speed(&group->beings[0], 12);
            being_tidy_loop_no_sim(&group->beings[0], &max_honor);
        } else {
            being_set_speed(&group->beings[0], 0);
        }
        bronze_work_end(group);
        bronze_tasks_end(group);
    }
    assert(worker->execution.status == BRONZE_TASK_ARRIVED && worker->instruction == 1);
    assert(APESPACE_TO_MAPSPACE(being_location_x(&group->beings[0])) == 0);
    assert(bronze_hybrid_economy().grain == 2); /* Travel does not gather. */

    /* Isolate the dry source with a flooded edge and three steep edges. */
    being_set_location(&group->beings[0], location);
    map[right] = TIDE_MAX;
    map[left - 1] = 255;
    map[left - MAP_DIMENSION] = 255;
    map[left + MAP_DIMENSION] = 255;
    assert(bronze_actors_world_reset(group));
    worker = bronze_actors_for_being(bronze_actors_current(), &group->beings[0]);
    worker->home = production->owner;
    worker->workplace = patch->place_id;
    worker->vocation = 0;
    worker->job_status = BRONZE_JOB_ASSIGNED;
    task_tick(group);
    assert(worker->execution.status == BRONZE_TASK_ROUTE_BLOCKED && !worker->instruction);
    assert(being_location_x(&group->beings[0]) == location[0]);

    *patch = saved_patch;
    memcpy(map, original_height, sizeof(original_height));
    /* A reset with the same program, native actor ordering and needs replays
       the same stochastic selection, despite new external actor/place IDs. */
    location[0] = MAPSPACE_TO_APESPACE(sites[field].x);
    location[1] = MAPSPACE_TO_APESPACE(sites[field].y);
    being_set_location(&group->beings[0], location);
    being_energy_delta(&group->beings[0], BEING_FULL - being_energy(&group->beings[0]));
    memset(being_drives(&group->beings[0]), 0, DRIVES);
    assert(bronze_actors_world_reset(group));
    bronze_hybrid_reset();
    worker = bronze_actors_for_being(bronze_actors_current(), &group->beings[0]);
    assert(worker->execution.rng == initial_rng);
    task_tick(group);
    task_tick(group);
    assert(worker->execution.rng == selected_rng && worker->remaining_ticks == 3);
    for (i = 0; i < 5; i++) task_tick(group);
    assert(worker->instruction == 3 && worker->resources[0].amount == 4);
    const BronzePlace *destination = bronze_hybrid_place(bronze_task_storage(worker->home));
    assert(destination);
    n_byte2 destination_location[] = {MAPSPACE_TO_APESPACE(destination->location.x),
                                      MAPSPACE_TO_APESPACE(destination->location.y)};
    being_set_location(&group->beings[0], destination_location);
    being_drives(&group->beings[0])[DRIVE_FATIGUE] = 80;
    task_tick(group);
    assert(worker->instruction == 4 && !strcmp(worker->execution.action, "deposit"));
    assert(bronze_hybrid_economy().grain == 2 && worker->resources[0].amount == 2);
    task_tick(group);
    assert(worker->execution.status == BRONZE_TASK_COMPLETE);
    double field_before_renewal = sites[field].stock;
    double copper_before_renewal = sites[copper].stock;
    advance_minutes(TIME_DAY_MINUTES - land_time());
    bronze_hybrid_cycle(group);
    assert(sites[field].stock > field_before_renewal && sites[field].stock <= sites[field].capacity);
    assert(sites[copper].stock == copper_before_renewal);
    double renewed = sites[field].stock;
    bronze_hybrid_cycle(group);
    assert(sites[field].stock == renewed);
    being_set_location(&group->beings[0], location);
    assert(bronze_task_gather(worker, &group->beings[0], worker->workplace, 0, 2, 0) == BRONZE_TASK_RUNNING);
    assert(sites[field].resources[0].reserved == 2);
    bronze_actors_death(group, &group->beings[0]);
    assert(sites[field].resources[0].reserved == 0 && landscape->lost[0] == 2);
    assert(bronze_actors_world_reset(group));

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
    bronze_actors_stage_program(bronze_actors_current(), NULL);
    professions.count = 3;
    professions.site_capacity = 4;
    professions.mentor_skill = 5;
    professions.vocations[0] = (BronzeVocation){"speaker", 3, BRONZE_JOB_SERVICE};
    professions.vocations[1] = (BronzeVocation){"helper", 1, BRONZE_JOB_SERVICE};
    professions.vocations[2] = (BronzeVocation){"coastal_worker", 100, BRONZE_JOB_COAST};
    bronze_actors_stage_professions(bronze_actors_current(), &professions);
    landscape->policy.town_capacity = LARGE_SIM;
    group->beings[0].constant.date_of_birth = land_date();
    assert(bronze_actors_world_reset(group));
    bronze_hybrid_reset();
    unsigned employed[3] = {0}, qualified[3] = {0};
    n_uint vacancy = group->num, waiting = group->num;
    for (i = 0; i < group->num; i++) {
        BronzeActor *actor = bronze_actors_for_being(bronze_actors_current(), &group->beings[i]);
        if (i == 0) assert(actor->job_status == BRONZE_JOB_CHILD && !actor->workplace);
        if (actor->vocation < 3) {
            employed[actor->vocation]++;
            if (bronze_actor_skill(actor, actor->vocation) >= professions.mentor_skill)
                qualified[actor->vocation]++;
            assert(bronze_hybrid_place(actor->workplace)->kind == BRONZE_PLACE_SERVICE);
            vacancy = i;
        } else if (actor->job_status == BRONZE_JOB_FULL && waiting == group->num) waiting = i;
    }
    assert(employed[0] == 3 && employed[1] == 1 && employed[2] == 0);
    assert(qualified[0] == 1 && qualified[1] == 1 && qualified[2] == 0);
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
    recipe_chains(group, landscape);
    sim_close();
    puts("Bronze hybrid terrain, symbols, bounded economy and reset checks passed.");
    return 0;
}
