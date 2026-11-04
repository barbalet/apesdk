/* Bronze-specific sidecar layer. Native ApeSDK owns land, beings and movement. */
#include "bronze_hybrid.h"
#include "../../sim/sim.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../gui/gui.h"
#include "../work/bronze_work.h"
#include "../tasks/bronze_tasks.h"

/* Map analysis uses low-resolution cells and native toroidal coordinates. */
typedef struct { n_byte slope, coast, moisture, relief; } TerrainCell;
static TerrainCell terrain[MAP_AREA];
static n_byte occupied[MAP_AREA], rejected[MAP_AREA];
static n_uint coast_queue[MAP_AREA];
static uint32_t preferred_distance[MAP_AREA];
static BronzeLandscape *current_landscape(void)
{
    return bronze_actors_landscape(bronze_actors_current());
}

static n_uint map_cell(n_int x, n_int y)
{
    return (n_uint)(((x + MAP_DIMENSION) % MAP_DIMENSION) +
                    ((y + MAP_DIMENSION) % MAP_DIMENSION) * MAP_DIMENSION);
}

static n_uint neighbor(n_uint cell, unsigned direction)
{
    static const int dx[] = {1, 0, -1, 0}, dy[] = {0, 1, 0, -1};
    return map_cell((n_int)(cell % MAP_DIMENSION) + dx[direction],
                    (n_int)(cell / MAP_DIMENSION) + dy[direction]);
}

static void analyze_terrain(const n_byte *height)
{
    n_uint head = 0, tail = 0;
    for (n_uint cell = 0; cell < MAP_AREA; cell++) {
        terrain[cell].coast = 255;
        if (height[cell] <= TIDE_MAX) {
            terrain[cell].coast = 0;
            coast_queue[tail++] = cell;
        }
    }
    while (head < tail) {
        n_uint cell = coast_queue[head++];
        unsigned distance = terrain[cell].coast + 1u;
        if (distance >= 255) continue;
        for (unsigned d = 0; d < 4; d++) {
            n_uint next = neighbor(cell, d);
            if (terrain[next].coast != 255) continue;
            terrain[next].coast = distance;
            coast_queue[tail++] = next;
        }
    }
    for (n_uint cell = 0; cell < MAP_AREA; cell++) {
        int low = height[cell], high = low, slope = 0;
        for (unsigned d = 0; d < 4; d++) {
            n_uint next = neighbor(cell, d);
            int delta = (int)height[next] - height[cell];
            if (delta < 0) delta = -delta;
            if (delta > slope) slope = delta;
            int outer = height[neighbor(next, d)];
            if (outer < low) low = outer;
            if (outer > high) high = outer;
        }
        int moisture = 255 - 4 * terrain[cell].coast - 5 * slope;
        terrain[cell].slope = slope;
        terrain[cell].relief = high - low;
        terrain[cell].moisture = moisture > 0 ? moisture : 0;
    }
}

static int suitability(n_uint cell, bronze_site_kind kind, const n_byte *height,
                       BronzeLandPolicy policy)
{
    const TerrainCell *t = &terrain[cell];
    if (occupied[cell] || height[cell] <= TIDE_MAX) return -1;
    switch (kind) {
        case BRONZE_TOWN:
        case BRONZE_FIELD:
            if (t->slope > policy.field_slope || t->coast > policy.field_coast ||
                t->moisture < policy.field_moisture) return -1;
            return t->moisture + 4 * (255 - t->slope);
        case BRONZE_WOODLAND:
            if (t->slope > policy.woodland_slope || t->moisture < policy.woodland_moisture)
                return -1;
            return t->moisture + t->relief;
        case BRONZE_COPPER_MINE:
        case BRONZE_TIN_MINE:
            if (height[cell] < 150 || t->slope < 2 || t->relief < policy.ore_relief ||
                (height[cell] >= policy.tin_height) != (kind == BRONZE_TIN_MINE)) return -1;
            return t->relief + t->slope;
    }
    return -1;
}

static void place_patches(BronzeLandscape *landscape, bronze_site_kind kind,
                          const n_byte *height, const uint32_t *preferred)
{
    unsigned limit = BRONZE_PATCH_LIMIT;
    unsigned minimum = kind == BRONZE_WOODLAND ? 8 : 4;
    memset(rejected, 0, sizeof(rejected));
    for (unsigned attempt = 0, placed = 0; attempt < 16 && placed < 2; attempt++) {
        int best = -1;
        n_uint anchor = 0;
        for (n_uint cell = 0; cell < MAP_AREA; cell++) {
            if (rejected[cell]) continue;
            int score = suitability(cell, kind, height, landscape->policy);
            if (score >= 0 && preferred && preferred[cell]) {
                /* Prefer sites reachable from the existing food or township network. */
                score += 1024;
                if (preferred[cell] < 128) score += 128 - preferred[cell];
            }
            if (score > best) {
                best = score;
                anchor = cell;
            }
        }
        if (best < 0) break;
        n_uint cells[BRONZE_PATCH_LIMIT] = {anchor};
        unsigned area = 1;
        occupied[anchor] = 1;
        for (unsigned head = 0; head < area && area < limit; head++) {
            for (unsigned d = 0; d < 4 && area < limit; d++) {
                n_uint next = neighbor(cells[head], d);
                if (suitability(next, kind, height, landscape->policy) < 0) continue;
                occupied[next] = 1;
                cells[area++] = next;
            }
        }
        if (area < minimum) {
            for (unsigned i = 0; i < area; i++) {
                occupied[cells[i]] = 0;
                rejected[cells[i]] = 1;
            }
            continue;
        }
        bronze_site *site = &landscape->sites[landscape->site_count++];
        site->kind = kind;
        site->area = area;
        site->capacity = 8 * area;
        site->stock = landscape->policy.patch_initial_per_cell * area;
        for (unsigned i = 0; i < area; i++) {
            site->cells[i].x = cells[i] % MAP_DIMENSION;
            site->cells[i].y = cells[i] / MAP_DIMENSION;
            /* Spread anchors of the same kind without excluding other uses. */
            for (int dy = -6; dy <= 6; dy++)
                for (int dx = -6; dx <= 6; dx++)
                    rejected[map_cell(site->cells[i].x + dx, site->cells[i].y + dy)] = 1;
        }
        site->x = site->cells[0].x;
        site->y = site->cells[0].y;
        placed++;
    }
}

/* Use the same conservative dry-land graph for siting, homes and ownership. */
static void walk_distances(uint32_t *distance, n_uint tail, const n_byte *height)
{
    for (n_uint head = 0; head < tail; head++) {
        n_uint cell = coast_queue[head];
        for (unsigned direction = 0; direction < 4; direction++) {
            n_uint next = neighbor(cell, direction);
            if (distance[next] || height[next] <= TIDE_MAX) continue;
            distance[next] = distance[cell] + 1;
            coast_queue[tail++] = next;
        }
    }
}

static void resource_distances(const BronzeLandscape *landscape, bronze_site_kind kind,
                               uint32_t *distance, const n_byte *height)
{
    memset(distance, 0, MAP_AREA * sizeof(*distance));
    n_uint tail = 0;
    for (n_uint i = 0; i < landscape->site_count; i++) {
        const bronze_site *site = &landscape->sites[i];
        if (site->kind != kind) continue;
        for (n_uint j = 0; j < site->area; j++) {
            n_uint cell = map_cell(site->cells[j].x, site->cells[j].y);
            distance[cell] = 1;
            coast_queue[tail++] = cell;
        }
    }
    walk_distances(distance, tail, height);
}

static uint64_t add_place(BronzeLandscape *landscape, BronzePlaceKind kind,
                          bronze_land_cell location, uint64_t owner)
{
    uint64_t id = ++landscape->next_place_id;
    landscape->places[landscape->place_count++] = (BronzePlace){
        id, owner ? owner : kind == BRONZE_PLACE_HOME ? id : 0, kind, location
    };
    return id;
}

static n_uint wrapped_distance(bronze_land_cell a, bronze_land_cell b)
{
    n_int dx = a.x > b.x ? a.x - b.x : b.x - a.x;
    n_int dy = a.y > b.y ? a.y - b.y : b.y - a.y;
    if (dx > MAP_DIMENSION / 2) dx = MAP_DIMENSION - dx;
    if (dy > MAP_DIMENSION / 2) dy = MAP_DIMENSION - dy;
    return (n_uint)(dx + dy);
}

static void place_townships(BronzeLandscape *landscape, const n_byte *height)
{
    static uint32_t food[MAP_AREA], woodland[MAP_AREA];
    resource_distances(landscape, BRONZE_FIELD, food, height);
    resource_distances(landscape, BRONZE_WOODLAND, woodland, height);
    bronze_land_cell first = {0};
    for (unsigned i = 0; i < BRONZE_TOWN_LIMIT; i++) {
        uint32_t best_score = UINT32_MAX;
        n_uint best = 0;
        for (n_uint cell = 0; cell < MAP_AREA; cell++) {
            const TerrainCell *t = &terrain[cell];
            if (occupied[cell] || height[cell] <= TIDE_MAX || t->slope > landscape->policy.field_slope ||
                t->coast > landscape->policy.field_coast || !food[cell] || !woodland[cell] ||
                food[cell] > 129 || woodland[cell] > 129) continue;
            bronze_land_cell location = {cell % MAP_DIMENSION, cell / MAP_DIMENSION};
            if (i && wrapped_distance(location, first) < 24) continue;
            uint32_t score = food[cell] + woodland[cell] + t->coast + 4u * t->slope;
            if (score < best_score) {
                best_score = score;
                best = cell;
            }
        }
        if (best_score == UINT32_MAX) break;
        bronze_land_cell location = {best % MAP_DIMENSION, best / MAP_DIMENSION};
        if (!i) first = location;
        BronzeTownship *town = &landscape->towns[landscape->town_count++];
        memset(town, 0, sizeof(*town));
        town->capacity = landscape->policy.town_capacity;
        town->stores.grain = landscape->policy.town_initial_grain;
        town->home = add_place(landscape, BRONZE_PLACE_HOME, location, 0);
        town->storage = add_place(landscape, BRONZE_PLACE_STORAGE, location, town->home);
        town->service = add_place(landscape, BRONZE_PLACE_SERVICE, location, town->home);
        town->distance[best] = 1;
        coast_queue[0] = best;
        walk_distances(town->distance, 1, height);
        occupied[best] = 1;
        bronze_site *site = &landscape->sites[landscape->site_count++];
        site->kind = BRONZE_TOWN;
        site->place_id = town->home;
        site->x = location.x;
        site->y = location.y;
        site->cells[0] = location;
        site->area = 1;
    }
}

static BronzeTownship *town_by_home(BronzeLandscape *landscape, uint64_t home)
{
    for (unsigned i = 0; i < landscape->town_count; i++)
        if (landscape->towns[i].home == home) return &landscape->towns[i];
    return NULL;
}

static BronzeTownship *nearest_town(BronzeLandscape *landscape, n_uint cell, int need_room)
{
    BronzeTownship *best = NULL;
    uint32_t best_distance = UINT32_MAX;
    for (unsigned i = 0; i < landscape->town_count; i++) {
        BronzeTownship *town = &landscape->towns[i];
        uint32_t distance = town->distance[cell];
        if (!distance || distance >= best_distance ||
            (need_room && town->residents >= town->capacity)) continue;
        best = town;
        best_distance = distance;
    }
    return best;
}

static void assign_workplaces(BronzeLandscape *landscape)
{
    for (n_uint i = 0; i < landscape->site_count; i++) {
        bronze_site *site = &landscape->sites[i];
        if (site->kind == BRONZE_TOWN) continue;
        BronzeTownship *owner = nearest_town(landscape, map_cell(site->x, site->y), 0);
        site->place_id = add_place(landscape, BRONZE_PLACE_PRODUCTION, site->cells[0],
                                  owner ? owner->home : 0);
    }
}

const BronzePlace *bronze_hybrid_place(uint64_t id)
{
    const BronzeLandscape *landscape = current_landscape();
    if (!landscape || !id) return NULL;
    for (unsigned i = 0; i < landscape->place_count; i++)
        if (landscape->places[i].id == id) return &landscape->places[i];
    return NULL;
}

static void bind_household(BronzeActor *actor)
{
    if (!actor->household_home) actor->household_home = actor->home;
}

static void update_membership(BronzeLandscape *landscape, simulated_group *group)
{
    BronzeActorRegistry *registry = bronze_actors_current();
    for (unsigned i = 0; i < landscape->town_count; i++) landscape->towns[i].residents = 0;
    for (n_uint i = 0; i < group->num; i++) {
        BronzeActor *actor = bronze_actors_for_being(registry, &group->beings[i]);
        if (!actor) continue;
        BronzeTownship *town = town_by_home(landscape, actor->home);
        if (town && town->residents < town->capacity) {
            town->residents++;
        } else {
            actor->home = 0;
            actor->workplace = 0;
        }
        bind_household(actor);
    }
    if (!landscape->town_count) return;
    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        BronzeActor *actor = bronze_actors_for_being(registry, being);
        if (!actor || actor->home) continue;
        n_uint cell = map_cell(APESPACE_TO_MAPSPACE(being_location_x(being)),
                               APESPACE_TO_MAPSPACE(being_location_y(being)));
        BronzeTownship *town = nearest_town(landscape, cell, 1);
        if (!town) continue;
        actor->home = town->home;
        bind_household(actor);
        town->residents++;
    }
}

static int job_matches(const BronzeLandscape *landscape, const BronzePlace *place,
                       BronzeJobPlace requirement)
{
    if (requirement == BRONZE_JOB_SERVICE) return place->kind == BRONZE_PLACE_SERVICE;
    if (place->kind != BRONZE_PLACE_PRODUCTION) return 0;
    for (n_uint i = 0; i < landscape->site_count; i++) {
        const bronze_site *site = &landscape->sites[i];
        if (site->place_id != place->id) continue;
        switch (site->kind) {
            case BRONZE_FIELD: return requirement == BRONZE_JOB_FIELD;
            case BRONZE_WOODLAND: return requirement == BRONZE_JOB_WOODLAND;
            case BRONZE_COPPER_MINE: return requirement == BRONZE_JOB_COPPER;
            case BRONZE_TIN_MINE: return requirement == BRONZE_JOB_TIN;
            case BRONZE_TOWN: return 0;
        }
    }
    return 0;
}

static int job_site(const BronzeLandscape *landscape, const BronzeTownship *town,
                    BronzeJobPlace requirement, const unsigned *occupancy,
                    unsigned capacity, int *reachable)
{
    int best = -1;
    uint32_t best_distance = UINT32_MAX;
    for (unsigned i = 0; i < landscape->place_count; i++) {
        const BronzePlace *place = &landscape->places[i];
        if (place->owner != town->home || !job_matches(landscape, place, requirement)) continue;
        uint32_t distance = town->distance[map_cell(place->location.x, place->location.y)];
        if (!distance) continue;
        *reachable = 1;
        if (occupancy[i] >= capacity || distance >= best_distance) continue;
        best = (int)i;
        best_distance = distance;
    }
    return best;
}

static void update_professions(BronzeLandscape *landscape, simulated_group *group, int reset)
{
    BronzeActorRegistry *registry = bronze_actors_current();
    const BronzeProfessionPolicy *policy = bronze_actors_professions(registry);
    if (!policy || !policy->count) return;
    uint64_t tick = bronze_native_tick();
    if (!reset && tick < landscape->next_profession_tick) return;
    landscape->next_profession_tick = tick + policy->retry_minutes;
    unsigned counts[BRONZE_VOCATION_LIMIT] = {0};
    unsigned occupancy[BRONZE_PLACE_LIMIT] = {0};
    uint64_t founding_vocations[BRONZE_PLACE_LIMIT] = {0};
    int enabled = 0;
    for (unsigned i = 0; i < policy->count; i++) enabled |= policy->vocations[i].weight != 0;

    /* Incumbents claim their slots before vacancies are filled. */
    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        BronzeActor *actor = bronze_actors_for_being(registry, being);
        if (!actor) continue;
        n_int birth = being_dob(being);
        n_int age = (n_int)land_date() - birth;
        const BronzeTownship *town = town_by_home(landscape, actor->home);
        if (!enabled) actor->job_status = BRONZE_JOB_DISABLED;
        else if (age < AGE_OF_MATURITY || age < (n_int)policy->min_age_days) actor->job_status = BRONZE_JOB_CHILD;
        else if (!town) actor->job_status = BRONZE_JOB_NO_HOME;
        else actor->job_status = BRONZE_JOB_NO_WORKPLACE;

        if (actor->job_status == BRONZE_JOB_NO_WORKPLACE && actor->vocation < policy->count &&
            policy->vocations[actor->vocation].weight) {
            const BronzePlace *place = bronze_hybrid_place(actor->workplace);
            if (place && place->owner == town->home &&
                job_matches(landscape, place, policy->vocations[actor->vocation].workplace) &&
                town->distance[map_cell(place->location.x, place->location.y)]) {
                size_t index = (size_t)(place - landscape->places);
                if (occupancy[index] < policy->site_capacity) {
                    occupancy[index]++;
                    counts[actor->vocation]++;
                    actor->job_status = BRONZE_JOB_ASSIGNED;
                    continue;
                }
            }
        }
        bronze_actor_set_job(registry, actor, BRONZE_KIND_NONE, 0, 0, tick);
    }
    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        BronzeActor *actor = bronze_actors_for_being(registry, being);
        if (!actor || actor->job_status != BRONZE_JOB_NO_WORKPLACE) continue;
        const BronzeTownship *town = town_by_home(landscape, actor->home);
        n_uint cell = map_cell(APESPACE_TO_MAPSPACE(being_location_x(being)),
                               APESPACE_TO_MAPSPACE(being_location_y(being)));
        if (!town->distance[cell]) continue;
        uint32_t best = BRONZE_KIND_NONE;
        int workplace = -1, reachable = 0;
        for (unsigned job = 0; job < policy->count; job++) {
            const BronzeVocation *vocation = &policy->vocations[job];
            if (!vocation->weight) continue;
            int place = job_site(landscape, town, vocation->workplace, occupancy,
                                 policy->site_capacity, &reachable);
            if (place < 0) continue;
            /* Minimize the resulting worker/weight ratio, with ties in DSL order. */
            if (best == BRONZE_KIND_NONE ||
                (counts[job] + 1) * policy->vocations[best].weight <
                (counts[best] + 1) * vocation->weight) {
                best = job;
                workplace = place;
            }
        }
        if (best == BRONZE_KIND_NONE) {
            if (reachable) actor->job_status = BRONZE_JOB_FULL;
            continue;
        }
        unsigned starting_skill = policy->skill_start;
        uint64_t vocation_bit = UINT64_C(1) << best;
        if (reset && !(founding_vocations[workplace] & vocation_bit) &&
            starting_skill < policy->mentor_skill)
            starting_skill = policy->mentor_skill;
        if (!bronze_actor_set_job(registry, actor, best, landscape->places[workplace].id,
                                 starting_skill, tick)) {
            actor->job_status = BRONZE_JOB_MEMORY;
            continue;
        }
        actor->job_status = BRONZE_JOB_ASSIGNED;
        founding_vocations[workplace] |= vocation_bit;
        occupancy[workplace]++;
        counts[best]++;
    }
}

static void patch_totals(bronze_site *site)
{
    site->stock = site->capacity = 0;
    for (unsigned kind = 0; kind < BRONZE_RESOURCE_LIMIT; kind++) {
        site->stock += site->resources[kind].amount;
        site->capacity += site->resources[kind].capacity;
    }
}

static const char *resource_name(const ParsedConfig *config, unsigned kind)
{
    const char *const *names = config->resource_kinds.names.data;
    return kind < config->resource_kinds.names.len ? names[kind] : "";
}

static void configure_stocks(BronzeLandscape *landscape, const ParsedConfig *config)
{
    const ResourceDef *definitions = config->resources.data;
    static const char *habitats[] = {"", "field", "mine_copper", "mine_tin", "forest"};
    for (n_uint i = 0; i < landscape->site_count; i++) {
        bronze_site *site = &landscape->sites[i];
        if (site->kind == BRONZE_TOWN) continue;
        unsigned moisture = 0;
        for (n_uint cell = 0; cell < site->area; cell++)
            moisture += terrain[map_cell(site->cells[cell].x, site->cells[cell].y)].moisture;
        double environment = 1;
        if (site->kind == BRONZE_FIELD || site->kind == BRONZE_WOODLAND)
            environment = (double)moisture / (255 * site->area);
        for (size_t r = 0; r < config->resources.len; r++) {
            const ResourceDef *definition = &definitions[r];
            if (definition->kind_id < 0 || definition->kind_id >= BRONZE_RESOURCE_LIMIT ||
                !definition->habitat || strcmp(definition->habitat, habitats[site->kind])) continue;
            BronzePatchStock *stock = &site->resources[definition->kind_id];
            stock->capacity = definition->capacity;
            double initial = landscape->policy.patch_initial_per_cell * site->area;
            stock->amount = initial < stock->capacity ? initial : stock->capacity;
            stock->daily_renewal = stock->capacity * definition->renew * environment;
        }
        patch_totals(site);
    }
    for (unsigned i = 0; i < landscape->town_count; i++) {
        BronzeTownship *town = &landscape->towns[i];
        town->stores.grain = 0;
        for (unsigned kind = 0; kind < config->resource_kinds.names.len; kind++)
            if (!strcmp(resource_name(config, kind), "grain"))
                town->resources[kind] = landscape->policy.town_initial_grain;
    }
}

void bronze_hybrid_reset(void)
{
    BronzeLandscape *landscape = current_landscape();
    if (!landscape) return;
    memset(landscape->sites, 0, sizeof(landscape->sites));
    memset(occupied, 0, sizeof(occupied));
    memset(landscape->consumed, 0, sizeof(landscape->consumed));
    memset(landscape->spoiled, 0, sizeof(landscape->spoiled));
    memset(landscape->lost, 0, sizeof(landscape->lost));
    memset(landscape->lost_items, 0, sizeof(landscape->lost_items));
    landscape->place_count = 0;
    landscape->town_count = 0;
    landscape->site_count = 0;
    if (landscape->next_place_id > UINT64_MAX - BRONZE_PLACE_LIMIT) {
        update_membership(landscape, sim_group());
        return;
    }
    landscape->last_day = land_date();
    landscape->last_tick = UINT64_MAX;
    const n_byte *height = land_topography();
    analyze_terrain(height);
    place_patches(landscape, BRONZE_FIELD, height, NULL);
    resource_distances(landscape, BRONZE_FIELD, preferred_distance, height);
    place_patches(landscape, BRONZE_WOODLAND, height, preferred_distance);
    place_townships(landscape, height);
    for (n_uint cell = 0; cell < MAP_AREA; cell++) {
        BronzeTownship *town = nearest_town(landscape, cell, 0);
        preferred_distance[cell] = town ? town->distance[cell] : 0;
    }
    place_patches(landscape, BRONZE_COPPER_MINE, height, preferred_distance);
    place_patches(landscape, BRONZE_TIN_MINE, height, preferred_distance);
    assign_workplaces(landscape);
    const BronzeProgram *program = bronze_actors_program(bronze_actors_current());
    if (program) configure_stocks(landscape, program->config);
    update_membership(landscape, sim_group());
    update_professions(landscape, sim_group(), 1);
}

static void spoil_stores(BronzeLandscape *landscape, simulated_group *group,
                         const ParsedConfig *config)
{
    if (!(landscape->policy.food_spoilage > 0)) return;
    unsigned char seen[BRONZE_RESOURCE_LIMIT] = {0};
    const ResourceDef *definitions = config->resources.data;
    for (size_t r = 0; r < config->resources.len; r++) {
        int kind = definitions[r].kind_id;
        if (kind < 0 || kind >= BRONZE_RESOURCE_LIMIT || !(definitions[r].nutrition > 0)) continue;
        if (seen[kind]) continue;
        seen[kind] = 1;
        for (unsigned i = 0; i < landscape->town_count; i++) {
            BronzeTownship *town = &landscape->towns[i];
            double available = town->resources[kind] - town->reserved_resources[kind];
            if (!(available > 0)) continue;
            double loss = available * landscape->policy.food_spoilage;
            town->resources[kind] -= loss;
            landscape->spoiled[kind] += loss;
        }
        for (n_uint i = 0; i < group->num; i++) {
            BronzeActor *actor = bronze_actors_for_being(bronze_actors_current(), &group->beings[i]);
            if (!actor) continue;
            double loss = actor->household_resources[kind] * landscape->policy.food_spoilage;
            actor->household_resources[kind] -= loss;
            landscape->spoiled[kind] += loss;
        }
    }
}

void bronze_hybrid_cycle(simulated_group *group)
{
    BronzeLandscape *landscape = current_landscape();
    if (!landscape) return;
    update_membership(landscape, group);
    update_professions(landscape, group, 0);
    uint64_t tick = bronze_native_tick();
    if (land_time() % TIME_HOUR_MINUTES != 0 || tick == landscape->last_tick) return;
    landscape->last_tick = tick;
    if (land_date() <= landscape->last_day) return;
    const BronzeProgram *program = bronze_actors_program(bronze_actors_current());
    for (n_uint i = 0; i < landscape->site_count; i++) {
        bronze_site *site = &landscape->sites[i];
        if (program) {
            for (unsigned kind = 0; kind < BRONZE_RESOURCE_LIMIT; kind++) {
                BronzePatchStock *stock = &site->resources[kind];
                stock->amount += stock->daily_renewal;
                if (stock->amount > stock->capacity) stock->amount = stock->capacity;
            }
            patch_totals(site);
            continue;
        }
        if (site->kind != BRONZE_FIELD && site->kind != BRONZE_WOODLAND) continue;
        site->stock += 5;
        if (site->stock > site->capacity) site->stock = site->capacity;
    }
    if (program) spoil_stores(landscape, group, program->config);
    landscape->last_day = land_date();
}

static bronze_site *work_site(BronzeLandscape *landscape, uint64_t id)
{
    if (!landscape || !id) return NULL;
    for (n_uint i = 0; i < landscape->site_count; i++)
        if (landscape->sites[i].place_id == id && landscape->sites[i].kind != BRONZE_TOWN)
            return &landscape->sites[i];
    return NULL;
}

const bronze_site *bronze_task_work_site(uint64_t id)
{
    return work_site(current_landscape(), id);
}

const bronze_site *bronze_task_move_site(uint64_t home, uint64_t workplace, const char *terrain)
{
    BronzeLandscape *landscape = current_landscape();
    if (!landscape || !home || !terrain) return NULL;
    const BronzeTownship *town = town_by_home(landscape, home);
    if (!town) return NULL;
    const bronze_site *best = NULL;
    uint32_t best_distance = UINT32_MAX;
    static const char *types[] = {"", "field", "mine_copper", "mine_tin", "forest"};
    for (n_uint i = 0; i < landscape->site_count; i++) {
        const bronze_site *site = &landscape->sites[i];
        if (site->kind == BRONZE_TOWN || strcmp(types[site->kind], terrain)) continue;
        const BronzePlace *place = bronze_hybrid_place(site->place_id);
        if (!place || place->owner != home) continue;
        uint32_t distance = town->distance[map_cell(site->x, site->y)];
        if (!distance) continue;
        if (site->place_id == workplace) return site;
        if (distance < best_distance) {
            best = site;
            best_distance = distance;
        }
    }
    return best;
}

static BronzeStock *cargo_slot(BronzeActor *actor, unsigned kind, int item, int create)
{
    BronzeStock **cargo = item ? &actor->items : &actor->resources;
    size_t *count = item ? &actor->item_count : &actor->resource_count;
    for (size_t i = 0; i < *count; i++)
        if ((*cargo)[i].kind == kind) return &(*cargo)[i];
    if (!create) return NULL;
    BronzeStock *next = realloc(*cargo, (*count + 1) * sizeof(*next));
    if (!next) return NULL;
    *cargo = next;
    next[*count] = (BronzeStock){kind, 0};
    return &next[(*count)++];
}

static double carried_total(const BronzeActor *actor)
{
    double total = 0;
    for (size_t i = 0; i < actor->resource_count; i++) total += actor->resources[i].amount;
    for (size_t i = 0; i < actor->item_count; i++) total += actor->items[i].amount;
    return total;
}

BronzeTaskStatus bronze_task_exchange(BronzeActor *actor, BronzeActor *partner,
                                     BronzeExchangeLeg give, BronzeExchangeLeg want, int commit)
{
    if (!actor || !partner || actor == partner ||
        give.kind < 0 || give.kind >= (give.item ? BRONZE_ITEM_LIMIT : BRONZE_RESOURCE_LIMIT) ||
        want.kind < 0 || want.kind >= (want.item ? BRONZE_ITEM_LIMIT : BRONZE_RESOURCE_LIMIT) ||
        (give.kind == want.kind && give.item == want.item) ||
        !isfinite(give.quantity) || !isfinite(want.quantity) ||
        give.quantity <= 0 || want.quantity <= 0)
        return BRONZE_TASK_STOCK;

    BronzeStock *outgoing = cargo_slot(actor, give.kind, give.item, 0);
    BronzeStock *payment = cargo_slot(partner, want.kind, want.item, 0);
    if (!outgoing || outgoing->amount < give.quantity ||
        !payment || payment->amount < want.quantity)
        return BRONZE_TASK_STOCK;

    const BronzeLandscape *landscape = current_landscape();
    if (!landscape) return BRONZE_TASK_TARGET;
    double capacity = landscape->policy.carry_capacity;
    if (carried_total(actor) - give.quantity + want.quantity > capacity ||
        carried_total(partner) - want.quantity + give.quantity > capacity)
        return BRONZE_TASK_CAPACITY;
    if (!commit) return BRONZE_TASK_RUNNING;

    /* Allocate both destinations before any debit. The first allocation may
       move the source array, so resolve both source slots again afterward. */
    size_t *count = want.item ? &actor->item_count : &actor->resource_count;
    size_t original_count = *count;
    BronzeStock *received = cargo_slot(actor, want.kind, want.item, 1);
    if (!received) return BRONZE_TASK_CAPACITY;
    BronzeStock *delivered = cargo_slot(partner, give.kind, give.item, 1);
    if (!delivered) {
        *count = original_count;
        return BRONZE_TASK_CAPACITY;
    }
    cargo_slot(actor, give.kind, give.item, 0)->amount -= give.quantity;
    cargo_slot(partner, want.kind, want.item, 0)->amount -= want.quantity;
    received->amount += want.quantity;
    delivered->amount += give.quantity;
    return BRONZE_TASK_RUNNING;
}

static double recipe_quantities(const RecipeDef *recipe, unsigned batches,
                                double *resources, double *items)
{
    const RecipeInputDef *inputs = recipe->inputs.data;
    double consumed = 0;
    for (size_t i = 0; i < recipe->inputs.len; i++) {
        const RecipeInputDef *input = &inputs[i];
        double quantity = input->amount * batches;
        if (input->item_id >= 0) items[input->item_id] += quantity;
        else resources[input->resource_id] += quantity;
        consumed += quantity;
    }
    return consumed;
}

void bronze_task_release_claim(BronzeLandscape *landscape, BronzeActor *actor)
{
    BronzeTaskState *state = &actor->execution;
    if (state->claim_place) {
        bronze_site *site = work_site(landscape, state->claim_place);
        if (site) {
            BronzePatchStock *stock = &site->resources[state->claim_kind];
            stock->reserved -= state->claim_amount;
            if (stock->reserved < 0) stock->reserved = 0;
        }
        state->claim_place = 0;
        state->claim_amount = 0;
    }
    if (state->craft_recipe && state->craft_town && landscape) {
        BronzeTownship *town = town_by_home(landscape, state->craft_town);
        if (town) {
            double resources[BRONZE_RESOURCE_LIMIT] = {0}, items[BRONZE_ITEM_LIMIT] = {0};
            recipe_quantities(state->craft_recipe, state->craft_batches, resources, items);
            for (unsigned i = 0; i < BRONZE_RESOURCE_LIMIT; i++) {
                town->reserved_resources[i] -= resources[i];
                if (town->reserved_resources[i] < 0) town->reserved_resources[i] = 0;
            }
            for (unsigned i = 0; i < BRONZE_ITEM_LIMIT; i++) {
                town->reserved_items[i] -= items[i];
                if (town->reserved_items[i] < 0) town->reserved_items[i] = 0;
            }
        }
    }
    state->craft_recipe = NULL;
    state->craft_town = 0;
    state->craft_batches = 0;
    state->claim_until = 0;
}

static int at_location(simulated_being *being, bronze_land_cell location)
{
    return APESPACE_TO_MAPSPACE(being_location_x(being)) == location.x &&
           APESPACE_TO_MAPSPACE(being_location_y(being)) == location.y;
}

BronzeTaskStatus bronze_task_gather(BronzeActor *actor, simulated_being *being,
                                   uint64_t target, int kind, double quantity, int commit)
{
    BronzeLandscape *landscape = current_landscape();
    bronze_site *site = work_site(landscape, target);
    const BronzePlace *place = bronze_hybrid_place(target);
    BronzeTownship *town = site && place && place->owner == actor->home ?
                           town_by_home(landscape, actor->home) : NULL;
    if (!town || target != actor->workplace) return BRONZE_TASK_TARGET;
    if (actor->execution.craft_recipe) return BRONZE_TASK_STOCK;
    n_uint cell = 0;
    while (cell < site->area && !at_location(being, site->cells[cell])) cell++;
    if (cell == site->area) return BRONZE_TASK_TRAVEL;
    if (kind < 0 || kind >= BRONZE_RESOURCE_LIMIT || !site->resources[kind].capacity)
        return BRONZE_TASK_UNSUPPORTED;
    if (!(quantity > 0)) return BRONZE_TASK_STOCK;
    if (quantity > landscape->policy.carry_capacity - carried_total(actor))
        return BRONZE_TASK_CAPACITY;

    BronzeTaskState *state = &actor->execution;
    BronzePatchStock *stock = &site->resources[kind];
    if (state->claim_place) {
        if (state->claim_place != target || state->claim_kind != (unsigned)kind ||
            state->claim_amount != quantity || stock->amount < quantity)
            return BRONZE_TASK_STOCK;
    } else {
        if (commit || quantity > stock->amount - stock->reserved) return BRONZE_TASK_STOCK;
        stock->reserved += quantity;
        state->claim_place = target;
        state->claim_kind = (unsigned)kind;
        state->claim_amount = quantity;
        state->claim_until = bronze_native_tick() + TIME_HOUR_MINUTES + (uint64_t)quantity + 1;
    }
    if (commit) {
        BronzeStock *cargo = cargo_slot(actor, (unsigned)kind, 0, 1);
        if (!cargo) return BRONZE_TASK_CAPACITY;
        cargo->amount += quantity;
        stock->amount -= quantity;
        bronze_task_release_claim(landscape, actor);
        patch_totals(site);
        town->stores.work_events++;
    }
    return BRONZE_TASK_RUNNING;
}

uint64_t bronze_task_storage(uint64_t home)
{
    BronzeLandscape *landscape = current_landscape();
    BronzeTownship *town = landscape ? town_by_home(landscape, home) : NULL;
    return town ? town->storage : 0;
}

static int nutritious_resource(unsigned kind)
{
    const BronzeProgram *program = bronze_actors_program(bronze_actors_current());
    if (!program) return 0;
    const ResourceDef *definitions = program->config->resources.data;
    for (size_t i = 0; i < program->config->resources.len; i++)
        if (definitions[i].kind_id == (int)kind) return definitions[i].nutrition > 0;
    return 0;
}

BronzeTaskStatus bronze_task_store_transfer(BronzeActor *actor, simulated_being *being,
                                            uint64_t target, int kind, double quantity,
                                            int item, int household, int fetch, int commit)
{
    BronzeLandscape *landscape = current_landscape();
    BronzeTownship *town = landscape ? town_by_home(landscape, actor->home) : NULL;
    const BronzePlace *place = bronze_hybrid_place(target);
    if (!town || !place || target != (household ? town->home : town->storage))
        return BRONZE_TASK_TARGET;
    if (household && !landscape->policy.household_capacity) return BRONZE_TASK_TARGET;
    if (!at_location(being, place->location)) return BRONZE_TASK_TRAVEL;
    if (kind < 0 || kind >= (item ? BRONZE_ITEM_LIMIT : BRONZE_RESOURCE_LIMIT))
        return BRONZE_TASK_UNSUPPORTED;
    if (actor->execution.craft_recipe || !(quantity > 0)) return BRONZE_TASK_STOCK;
    if (household && actor->household_home != actor->home) return BRONZE_TASK_TARGET;

    double *resources = household ? actor->household_resources : town->resources;
    double *items = household ? actor->household_items : town->items;
    double *stock = &(item ? items : resources)[kind];
    if (fetch) {
        double reserved = household ? 0 : (item ? town->reserved_items : town->reserved_resources)[kind];
        if (quantity > *stock - reserved) return BRONZE_TASK_STOCK;
        if (quantity > landscape->policy.carry_capacity - carried_total(actor)) return BRONZE_TASK_CAPACITY;
        double ration = actor->ration_day == land_date() ? actor->ration_used : 0;
        int food_ration = !household && !item && landscape->policy.ration_daily && nutritious_resource((unsigned)kind);
        if (food_ration && quantity > landscape->policy.ration_daily - ration) return BRONZE_TASK_RATION;
        if (commit) {
            BronzeStock *cargo = cargo_slot(actor, (unsigned)kind, item, 1);
            if (!cargo) return BRONZE_TASK_CAPACITY;
            cargo->amount += quantity;
            *stock -= quantity;
            if (food_ration) {
                actor->ration_day = land_date();
                actor->ration_used = ration + quantity;
            }
        }
    } else {
        BronzeStock *cargo = cargo_slot(actor, (unsigned)kind, item, 0);
        if (!cargo || quantity > cargo->amount) return BRONZE_TASK_STOCK;
        double total = 0;
        for (unsigned i = 0; i < BRONZE_RESOURCE_LIMIT; i++) total += resources[i];
        for (unsigned i = 0; i < BRONZE_ITEM_LIMIT; i++) total += items[i];
        unsigned capacity = household ? landscape->policy.household_capacity : landscape->policy.storage_capacity;
        if (quantity > capacity - total) return BRONZE_TASK_CAPACITY;
        if (commit) {
            cargo->amount -= quantity;
            *stock += quantity;
        }
    }
    return BRONZE_TASK_RUNNING;
}

BronzeTaskStatus bronze_task_deposit(BronzeActor *actor, simulated_being *being,
                                    uint64_t target, int kind, double quantity, int commit)
{
    return bronze_task_store_transfer(actor, being, target, kind, quantity, 0, 0, 0, commit);
}

BronzeTaskStatus bronze_task_deposit_item(BronzeActor *actor, simulated_being *being,
                                         uint64_t target, int kind, double quantity, int commit)
{
    return bronze_task_store_transfer(actor, being, target, kind, quantity, 1, 0, 0, commit);
}

static double available_input(BronzeActor *actor, const double *stocks, const double *reserved,
                              unsigned kind, int item, double own_reservation)
{
    if (stocks) {
        double competing = reserved[kind] - own_reservation;
        return stocks[kind] - (competing > 0 ? competing : 0);
    }
    const BronzeStock *cargo = cargo_slot(actor, kind, item, 0);
    return cargo ? cargo->amount : 0;
}

static void apply_inputs(BronzeActor *actor, double *stock, double *reserved,
                         const double *required, unsigned count, int item, int commit)
{
    for (unsigned kind = 0; kind < count; kind++) {
        if (!required[kind]) continue;
        if (!commit) {
            if (reserved) reserved[kind] += required[kind];
        } else if (stock) {
            stock[kind] -= required[kind];
        } else {
            cargo_slot(actor, kind, item, 0)->amount -= required[kind];
        }
    }
}

BronzeTaskStatus bronze_task_craft(BronzeActor *actor, simulated_being *being, uint64_t target,
                                  const RecipeDef *recipe, const BronzeRecipePolicy *policy,
                                  unsigned batches, int commit)
{
    BronzeLandscape *landscape = current_landscape();
    BronzeTownship *town = landscape ? town_by_home(landscape, actor->home) : NULL;
    const BronzePlace *place = bronze_hybrid_place(target);
    if (!town || !place || place->owner != town->home ||
        target != (policy->at_workplace ? actor->workplace : town->service))
        return BRONZE_TASK_TARGET;
    /* Production workplaces may cover several cells. */
    bronze_site *site = work_site(landscape, target);
    if (site) {
        n_uint cell = 0;
        while (cell < site->area && !at_location(being, site->cells[cell])) cell++;
        if (cell == site->area) return BRONZE_TASK_TRAVEL;
    } else if (!at_location(being, place->location)) return BRONZE_TASK_TRAVEL;
    if (!batches) return BRONZE_TASK_STOCK;
    if (bronze_actor_skill(actor, actor->vocation) < policy->minimum_skill) return BRONZE_TASK_SKILL;

    BronzeTaskState *state = &actor->execution;
    uint64_t stock_home = policy->at_workplace ? 0 : town->home;
    int owns_claim = state->craft_recipe != NULL;
    if ((commit && !owns_claim) || (owns_claim &&
        (state->craft_recipe != recipe || state->craft_batches != batches ||
         state->craft_town != stock_home))) return BRONZE_TASK_STOCK;

    double resources[BRONZE_RESOURCE_LIMIT] = {0}, items[BRONZE_ITEM_LIMIT] = {0};
    double consumed = recipe_quantities(recipe, batches, resources, items);
    if (policy->tool >= 0) {
        const BronzeStock *tool = cargo_slot(actor, (unsigned)policy->tool, 1, 0);
        double tool_and_inputs = 1 + (stock_home ? 0 : items[policy->tool]);
        if (!tool || tool->amount < tool_and_inputs) return BRONZE_TASK_TOOL;
    }
    double *resource_stock = stock_home ? town->resources : NULL;
    double *item_stock = stock_home ? town->items : NULL;
    double *resource_reserved = stock_home ? town->reserved_resources : NULL;
    double *item_reserved = stock_home ? town->reserved_items : NULL;
    for (unsigned i = 0; i < BRONZE_RESOURCE_LIMIT; i++)
        if (resources[i] && resources[i] > available_input(actor, resource_stock, resource_reserved, i, 0, owns_claim ? resources[i] : 0))
            return BRONZE_TASK_STOCK;
    for (unsigned i = 0; i < BRONZE_ITEM_LIMIT; i++)
        if (items[i] && items[i] > available_input(actor, item_stock, item_reserved, i, 1, owns_claim ? items[i] : 0))
            return BRONZE_TASK_STOCK;

    double output = recipe->output_amount * batches;
    if (carried_total(actor) - (stock_home ? 0 : consumed) + output > landscape->policy.carry_capacity)
        return BRONZE_TASK_CAPACITY;
    if (commit) {
        int item = recipe->output_item_id >= 0;
        unsigned kind = item ? (unsigned)recipe->output_item_id : (unsigned)recipe->output_resource_id;
        BronzeStock *result = cargo_slot(actor, kind, item, 1);
        if (!result) return BRONZE_TASK_CAPACITY;
        apply_inputs(actor, resource_stock, NULL, resources, BRONZE_RESOURCE_LIMIT, 0, 1);
        apply_inputs(actor, item_stock, NULL, items, BRONZE_ITEM_LIMIT, 1, 1);
        result->amount += output;
        bronze_task_release_claim(landscape, actor);
        town->stores.work_events++;
    } else if (!owns_claim) {
        apply_inputs(actor, NULL, resource_reserved, resources, BRONZE_RESOURCE_LIMIT, 0, 0);
        apply_inputs(actor, NULL, item_reserved, items, BRONZE_ITEM_LIMIT, 1, 0);
        state->craft_recipe = recipe;
        state->craft_batches = batches;
        state->craft_town = stock_home;
        state->claim_until = bronze_native_tick() + (uint64_t)policy->minutes * batches + TIME_HOUR_MINUTES;
    }
    return BRONZE_TASK_RUNNING;
}

static void draw_map_cell(bronze_plot plot, void *context, n_int x, n_int y,
                          n_int scale, n_byte color)
{
    for (n_int dy = 0; dy < scale; ++dy)
        for (n_int dx = 0; dx < scale; ++dx)
            plot(x * scale + dx, y * scale + dy, color, context);
}

void bronze_hybrid_draw(n_int dimension, bronze_plot plot, void *context)
{
    BronzeLandscape *landscape = current_landscape();
    if (!landscape) return;
    n_uint i;
    n_int scale = dimension / MAP_DIMENSION;
    if (!plot || scale < 1) return;
    for (i = 0; i < landscape->site_count; i++)
    {
        const bronze_site *site = &landscape->sites[i];
        n_int x, y, radius = site->kind == BRONZE_TOWN ? 4 : 3;
        /* White roofed town, dark mine entrance, blue field/woodland outline.
           Use the existing palette; don't change colors of native graphics. */
        n_byte color = site->kind == BRONZE_TOWN ? COLOR_WHITE :
                       (site->kind == BRONZE_COPPER_MINE || site->kind == BRONZE_TIN_MINE) ? COLOR_BLACK : COLOR_BLUE;
        if (site->kind != BRONZE_TOWN) {
            for (n_uint cell = 0; cell < site->area; cell++)
                draw_map_cell(plot, context, site->cells[cell].x, site->cells[cell].y, scale, color);
        }
        for (y = -radius; y <= radius; y++)
            for (x = -radius; x <= radius; x++)
            {
                n_int ax = x < 0 ? -x : x;
                n_int edge;
                if (site->kind == BRONZE_TOWN)
                    edge = (ax == radius && y >= -radius / 2) || y == radius || y == -radius + ax / 2;
                else if (site->kind == BRONZE_WOODLAND)
                    edge = (y <= 1 && ax == (y + radius) / 2) || (x == 0 && y >= 1);
                else
                    edge = ax == radius || y == -radius || y == radius ||
                           (site->kind == BRONZE_FIELD && y == 0);
                if (edge)
                    draw_map_cell(plot, context, (site->x + x + MAP_DIMENSION) % MAP_DIMENSION,
                                  (site->y + y + MAP_DIMENSION) % MAP_DIMENSION, scale, color);
            }
        /* Tin mines have a central white point; copper mines a white cross. */
        if (site->kind == BRONZE_TIN_MINE || site->kind == BRONZE_COPPER_MINE)
        {
            draw_map_cell(plot, context, site->x, site->y, scale, COLOR_WHITE);
            if (site->kind == BRONZE_COPPER_MINE) {
                for (n_int delta = -1; delta <= 1; delta += 2) {
                    draw_map_cell(plot, context, (site->x + delta + MAP_DIMENSION) % MAP_DIMENSION,
                                  site->y, scale, COLOR_WHITE);
                    draw_map_cell(plot, context, site->x,
                                  (site->y + delta + MAP_DIMENSION) % MAP_DIMENSION, scale, COLOR_WHITE);
                }
            }
        }
    }
}
const bronze_site *bronze_hybrid_sites(n_uint *count)
{
    const BronzeLandscape *landscape = current_landscape();
    if (count) *count = landscape ? landscape->site_count : 0;
    return landscape ? landscape->sites : NULL;
}

bronze_economy bronze_hybrid_economy(void)
{
    bronze_economy total = {0};
    const BronzeLandscape *landscape = current_landscape();
    if (!landscape) return total;
    const BronzeProgram *program = bronze_actors_program(bronze_actors_current());
    for (unsigned i = 0; i < landscape->town_count; i++) {
        const bronze_economy *stores = &landscape->towns[i].stores;
        total.grain += stores->grain;
        total.copper += stores->copper;
        total.tin += stores->tin;
        total.charcoal += stores->charcoal;
        total.tools += stores->tools;
        total.work_events += stores->work_events;
        if (program) {
            const ParsedConfig *config = program->config;
            const char *const *item_names = config->item_kinds.names.data;
            for (unsigned kind = 0; kind < config->item_kinds.names.len; kind++)
                if (!strcmp(item_names[kind], "tool")) total.tools += landscape->towns[i].items[kind];
            for (unsigned kind = 0; kind < config->resource_kinds.names.len; kind++) {
                const char *name = resource_name(config, kind);
                double stock = landscape->towns[i].resources[kind];
                if (!strcmp(name, "grain")) total.grain += stock;
                else if (!strcmp(name, "copper")) total.copper += stock;
                else if (!strcmp(name, "tin")) total.tin += stock;
                else if (!strcmp(name, "charcoal")) total.charcoal += stock;
            }
        }
    }
    return total;
}

typedef struct { n_byte4 *pixels; n_int width, height; } bronze_hud_buffer;
static n_byte bronze_hud_pixel(n_int x, n_int y, n_int dx, n_int dy, void *context)
{
    bronze_hud_buffer *buffer = context;
    (void)dx; (void)dy;
    if (x >= 0 && y >= 0 && x < buffer->width && y < buffer->height)
        buffer->pixels[y * buffer->width + x] = 0xffffffffu;
    return 0;
}
void bronze_hybrid_draw_hud(n_byte4 *rgba, n_int width, n_int height)
{
    bronze_economy economy = bronze_hybrid_economy();
    char line[80];
    n_int x, y, chars;
    bronze_hud_buffer buffer = {rgba, width, height};
    n_join draw = {bronze_hud_pixel, &buffer};
    if (!rgba || width < 80 || height < 48) return;
    chars = (width - 16) / 8;
    if (chars > 79) chars = 79;
    for (y = height - 40; y < height; y++)
        for (x = 0; x < width; x++)
        {
            /* The shared UIKit renderer consumes XRGB bytes (skip-first,
               big-endian CGImage), regardless of the host integer order. */
            n_byte *pixel = (n_byte *)&rgba[y * width + x];
            pixel[0] = 255; pixel[1] = 24; pixel[2] = 24; pixel[3] = 24;
        }
    snprintf(line, sizeof(line), "BRONZE: TOWNS / MINES / FIELDS / WOODLAND");
    line[chars] = 0;
    draw_string(line, 8, height - 34, &draw);
    snprintf(line, sizeof(line), "FOOD %.0f CU %.0f SN %.0f FUEL %.0f TOOLS %.0f",
             economy.grain, economy.copper, economy.tin, economy.charcoal, economy.tools);
    line[chars] = 0;
    draw_string(line, 8, height - 18, &draw);
}
