/* Bronze-specific sidecar layer. Native ApeSDK owns land, beings and movement. */
#include "bronze_hybrid.h"
#include "../../sim/sim.h"
#include <string.h>
#include <stdio.h>
#include "../../gui/gui.h"
#include "../work/bronze_work.h"

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
        bronze_actor_set_job(actor, BRONZE_KIND_NONE, 0, 0, tick);
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
        if (!bronze_actor_set_job(actor, best, landscape->places[workplace].id,
                                 policy->skill_start, tick)) {
            actor->job_status = BRONZE_JOB_MEMORY;
            continue;
        }
        actor->job_status = BRONZE_JOB_ASSIGNED;
        occupancy[workplace]++;
        counts[best]++;
    }
}

void bronze_hybrid_reset(void)
{
    BronzeLandscape *landscape = current_landscape();
    if (!landscape) return;
    memset(landscape->sites, 0, sizeof(landscape->sites));
    memset(occupied, 0, sizeof(occupied));
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
    update_membership(landscape, sim_group());
    update_professions(landscape, sim_group(), 1);
}

void bronze_hybrid_cycle(simulated_group *group)
{
    BronzeLandscape *landscape = current_landscape();
    if (!landscape) return;
    update_membership(landscape, group);
    update_professions(landscape, group, 0);
    BronzeActorRegistry *registry = bronze_actors_current();
    const BronzeProfessionPolicy *policy = bronze_actors_professions(registry);
    n_uint i, j;
    /* One economic sampling interval per simulated hour, independent of frames. */
    uint64_t tick = bronze_native_tick();
    if (land_time() % 60 != 0 || tick == landscape->last_tick) return;
    landscape->last_tick = tick;
    if (land_date() != landscape->last_day)
    {
        for (j = 0; j < landscape->site_count; j++)
            if (landscape->sites[j].kind == BRONZE_FIELD || landscape->sites[j].kind == BRONZE_WOODLAND)
            {
                landscape->sites[j].stock += 5;
                if (landscape->sites[j].stock > landscape->sites[j].capacity) landscape->sites[j].stock = landscape->sites[j].capacity;
            }
        landscape->last_day = land_date();
    }
    for (i = 0; i < group->num; i++)
    {
        simulated_being *being = &group->beings[i];
        BronzeActor *actor = bronze_actors_for_being(registry, being);
        if (!actor || actor->job_status != BRONZE_JOB_ASSIGNED || !actor->home ||
            actor->vocation >= policy->count || !being_awake(being)) continue;
        n_int x = APESPACE_TO_MAPSPACE(being_location_x(being));
        n_int y = APESPACE_TO_MAPSPACE(being_location_y(being));
        for (j = 0; j < landscape->site_count; j++)
        {
            bronze_site *site = &landscape->sites[j];
            if (site->place_id != actor->workplace || site->kind == BRONZE_TOWN || site->stock < 1) continue;
            n_uint cell = 0;
            while (cell < site->area && (site->cells[cell].x != x || site->cells[cell].y != y)) cell++;
            if (cell == site->area) continue;
            const BronzePlace *place = bronze_hybrid_place(site->place_id);
            BronzeTownship *owner = place ? town_by_home(landscape, place->owner) : NULL;
            if (!owner || owner->home != actor->home || !bronze_work_claim(being)) break;
            site->stock -= 1;
            switch (site->kind)
            {
                case BRONZE_FIELD: owner->stores.grain += 1; break;
                case BRONZE_COPPER_MINE: owner->stores.copper += 1; break;
                case BRONZE_TIN_MINE: owner->stores.tin += 1; break;
                case BRONZE_WOODLAND: owner->stores.charcoal += 1; break;
                default: break;
            }
            owner->stores.work_events++;
            bronze_actor_practice(actor, policy);
            break;
        }
    }

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
                plot(site->cells[cell].x * scale, site->cells[cell].y * scale, color, context);
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
                    plot((site->x * scale + x + dimension) % dimension,
                         (site->y * scale + y + dimension) % dimension, color, context);
            }
        /* Tin mines have a central white point; copper mines a white cross. */
        if (site->kind == BRONZE_TIN_MINE || site->kind == BRONZE_COPPER_MINE)
        {
            plot(site->x * scale, site->y * scale, COLOR_WHITE, context);
            if (site->kind == BRONZE_COPPER_MINE)
                plot((site->x * scale + 1) % dimension, site->y * scale, COLOR_WHITE, context);
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
    for (unsigned i = 0; i < landscape->town_count; i++) {
        const bronze_economy *stores = &landscape->towns[i].stores;
        total.grain += stores->grain;
        total.copper += stores->copper;
        total.tin += stores->tin;
        total.charcoal += stores->charcoal;
        total.tools += stores->tools;
        total.work_events += stores->work_events;
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
