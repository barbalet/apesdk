#include "bronze_tasks.h"
#ifdef BRONZE_SIM
#include "../../entity/profession.h"
#endif
#include "../work/bronze_work.h"
#include <math.h>
#include <string.h>

enum { BRONZE_TASK_RETRY = TIME_HOUR_MINUTES, BRONZE_TASK_WALK_BUDGET = 32 };

static uint64_t random_word(BronzeTaskState *state)
{
    uint64_t z = (state->rng += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

static int chance(BronzeTaskState *state, double probability)
{
    return (double)(random_word(state) >> 11) * 0x1p-53 < probability;
}

static uint64_t roll_below(BronzeTaskState *state, uint64_t bound)
{
    uint64_t value, threshold = (-bound) % bound;
    do { value = random_word(state); } while (value < threshold);
    return value % bound;
}

static int condition_term(const CompiledCondition *condition, unsigned index,
                          const BronzeNeeds *needs, BronzeTaskState *state)
{
    double actual, expected = condition->value[index];
    switch (condition->kind[index]) {
        case BRZ_COND_TRUE: return 1;
        case BRZ_COND_FALSE: return 0;
        case BRZ_COND_PROB: return chance(state, expected);
        case BRZ_COND_HUNGER: actual = needs->hunger; break;
        case BRZ_COND_FATIGUE: actual = needs->fatigue; break;
        default: return 0;
    }
    switch (condition->comparison[index]) {
        case BRZ_CMP_TRUTHY: return actual != 0;
        case BRZ_CMP_GT: return actual > expected;
        case BRZ_CMP_LT: return actual < expected;
        case BRZ_CMP_GE: return actual >= expected;
        case BRZ_CMP_LE: return actual <= expected;
        case BRZ_CMP_EQ: return actual == expected;
        case BRZ_CMP_NE: return actual != expected;
    }
    return 0;
}

static int condition_met(const CompiledCondition *condition, const BronzeNeeds *needs,
                         BronzeTaskState *state)
{
    int first = condition_term(condition, 0, needs, state);
    if (condition->terms == 1) return first;
    if (condition->join_or) return first || condition_term(condition, 1, needs, state);
    return first && condition_term(condition, 1, needs, state);
}

static const RuleDef *choose_rule(const VocationDef *vocation, const BronzeNeeds *needs,
                                  BronzeTaskState *state)
{
    const RuleDef *rules = vocation->rules.data, *selected = NULL;
    uint64_t weight = 0;
    /* Weighted reservoir: eligibility and each probability are visited once. */
    for (size_t i = 0; i < vocation->rules.len; i++) {
        const RuleDef *rule = &rules[i];
        if (!condition_met(&rule->condition, needs, state)) continue;
        weight += (unsigned)rule->weight;
        if (!selected || roll_below(state, weight) < (unsigned)rule->weight) selected = rule;
    }
    return selected;
}

static void finish(BronzeActor *actor, BronzeTaskStatus status, uint64_t tick)
{
    BronzeTaskState *state = &actor->execution;
    bronze_task_release_claim(bronze_actors_landscape(bronze_actors_current()), actor);
    state->status = status;
    state->depth = 0;
    state->route = (BronzeRoute){0};
    state->operation = NULL;
    state->retry_at = tick + (status == BRONZE_TASK_COMPLETE ? 1 : BRONZE_TASK_RETRY);
    actor->task = BRONZE_KIND_NONE;
    actor->remaining_ticks = 0;
    actor->work.running = actor->work.ever_ran = actor->work.resting = 0;
}

static int start_task(BronzeActor *actor, const VocationDef *vocation, const BronzeNeeds *needs,
                      uint64_t tick)
{
    BronzeTaskState *state = &actor->execution;
    state->name = state->action = NULL;
    state->line = 0;
    state->target = 0;
    const RuleDef *rule = choose_rule(vocation, needs, state);
    if (rule) {
        const TaskDef *tasks = vocation->tasks.data;
        for (size_t i = 0; i < vocation->tasks.len; i++) {
            if (strcmp(tasks[i].name, rule->do_task)) continue;
            state->task_index = (uint32_t)i;
            state->name = tasks[i].name;
            state->frames[0] = (BronzeTaskFrame){&tasks[i].stmts, 0};
            state->depth = 1;
            state->status = BRONZE_TASK_RUNNING;
            actor->instruction = 0;
            return 1;
        }
    }
    finish(actor, BRONZE_TASK_NO_RULE, tick);
    return 0;
}

static const OpDef *next_operation(BronzeTaskState *state, const BronzeNeeds *needs)
{
    for (unsigned step = 0; step < BRONZE_TASK_WALK_BUDGET && state->depth; step++) {
        BronzeTaskFrame *frame = &state->frames[state->depth - 1];
        if (frame->index == frame->statements->len) {
            state->depth--;
            continue;
        }
        const StmtDef *statements = frame->statements->data;
        const StmtDef *stmt = &statements[frame->index++];
        if (stmt->kind == ST_OP) return &stmt->as.op;
        const BrzVec *body = NULL;
        if (stmt->kind == ST_WHEN && condition_met(&stmt->as.when_stmt.condition, needs, state))
            body = &stmt->as.when_stmt.body;
        if (stmt->kind == ST_CHANCE && chance(state, stmt->as.chance.chance_pct / 100.0))
            body = &stmt->as.chance.body;
        if (body) {
            /* Strict parsing limits nested statements to 64 levels. */
            state->frames[state->depth++] = (BronzeTaskFrame){body, 0};
        }
    }
    return NULL;
}

enum { ROUTE_SEARCH_LIMIT = 16384, ROUTE_SEARCHES_PER_TICK = 2,
       ROUTE_SLOPE_LIMIT = 18, ROUTE_SPEED_LIMIT = 12, ROUTE_STALL_LIMIT = 16,
       ROUTE_ATTEMPT_LIMIT = 2048 };

/* Scratch is reused by the serial task preparation pass. Actors own only
   their waypoints and progress, with all position reads coming from ApeSDK. */
static uint32_t route_marks[MAP_AREA], route_parent[MAP_AREA];
static uint32_t route_queue[ROUTE_SEARCH_LIMIT], route_epoch;

static uint32_t route_cell(n_int x, n_int y)
{
    return (uint32_t)(POSITIVE_LAND_COORD(x) + POSITIVE_LAND_COORD(y) * MAP_DIMENSION);
}

static uint32_t being_cell(simulated_being *being)
{
    return route_cell(APESPACE_TO_MAPSPACE(being_location_x(being)),
                      APESPACE_TO_MAPSPACE(being_location_y(being)));
}

static int target_contains(uint64_t target, uint32_t cell)
{
    const BronzePlace *place = bronze_hybrid_place(target);
    if (!place) return 0;
    const bronze_site *site = bronze_task_work_site(target);
    if (!site) return cell == route_cell(place->location.x, place->location.y);
    for (n_uint i = 0; i < site->area; i++)
        if (cell == route_cell(site->cells[i].x, site->cells[i].y)) return 1;
    return 0;
}

static int dry_edge(uint32_t from, uint32_t to)
{
    const n_byte *height = land_topography();
    int delta = (int)height[from] - height[to];
    return height[from] > TIDE_MAX && height[to] > TIDE_MAX &&
           delta >= -ROUTE_SLOPE_LIMIT && delta <= ROUTE_SLOPE_LIMIT;
}

static int build_route(BronzeRoute *route, uint32_t start, uint64_t target)
{
    if (land_topography()[start] <= TIDE_MAX) return 0;
    if (++route_epoch == 0) {
        memset(route_marks, 0, sizeof(route_marks));
        route_epoch = 1;
    }
    uint32_t goals[BRONZE_PATCH_LIMIT];
    const bronze_site *site = bronze_task_work_site(target);
    const BronzePlace *place = bronze_hybrid_place(target);
    if (!place) return 0;
    n_uint goal_count = site ? site->area : 1;
    for (n_uint i = 0; i < goal_count; i++) {
        bronze_land_cell location = site ? site->cells[i] : place->location;
        goals[i] = route_cell(location.x, location.y);
    }
    unsigned tail = 1;
    route_marks[start] = route_epoch;
    route_queue[0] = start;
    for (unsigned head = 0; head < tail; head++) {
        uint32_t cell = route_queue[head];
        n_uint goal = 0;
        while (goal < goal_count && goals[goal] != cell) goal++;
        if (goal < goal_count) {
            unsigned length = 0;
            for (uint32_t p = cell; p != start; p = route_parent[p]) {
                if (length == BRONZE_ROUTE_LIMIT) return 0;
                route->cells[length++] = p;
            }
            for (unsigned i = 0; i < length / 2; i++) {
                uint32_t swap = route->cells[i];
                route->cells[i] = route->cells[length - 1 - i];
                route->cells[length - 1 - i] = swap;
            }
            route->from = start;
            route->length = length;
            route->next = 0;
            return length != 0;
        }
        n_int x = cell % MAP_DIMENSION, y = cell / MAP_DIMENSION;
        uint32_t neighbors[] = {route_cell(x + 1, y), route_cell(x, y + 1),
                                route_cell(x - 1, y), route_cell(x, y - 1)};
        for (unsigned i = 0; i < 4 && tail < ROUTE_SEARCH_LIMIT; i++) {
            uint32_t next = neighbors[i];
            if (route_marks[next] == route_epoch || !dry_edge(cell, next)) continue;
            route_marks[next] = route_epoch;
            route_parent[next] = cell;
            route_queue[tail++] = next;
        }
    }
    return 0;
}

static void prepare_route(BronzeActor *actor, simulated_being *being, uint64_t tick,
                          unsigned *searches)
{
    BronzeTaskState *state = &actor->execution;
    BronzeRoute *route = &state->route;
    if (!being_check_goal(being, GOAL_NONE)) {
        state->status = BRONZE_TASK_ROUTE_PAUSED;
        return;
    }
    if (route->stalled >= ROUTE_STALL_LIMIT || route->attempts >= ROUTE_ATTEMPT_LIMIT) {
        finish(actor, BRONZE_TASK_ROUTE_BLOCKED, tick);
        return;
    }
    uint32_t cell = being_cell(being);
    if (route->next < route->length && cell == route->cells[route->next]) {
        route->from = cell;
        route->next++;
    }
    if (route->next == route->length || cell != route->from ||
        !dry_edge(cell, route->cells[route->next])) {
        route->length = route->next = 0;
        state->status = BRONZE_TASK_TRAVEL;
        if (*searches >= ROUTE_SEARCHES_PER_TICK) return;
        (*searches)++;
        if (!build_route(route, cell, state->target)) {
            finish(actor, BRONZE_TASK_ROUTE_BLOCKED, tick);
            return;
        }
    }
    route->tick = tick;
    state->status = BRONZE_TASK_TRAVEL;
}

static n_int wrapped_delta(n_int from, n_int to)
{
    n_int delta = to - from;
    if (delta > MAP_APE_RESOLUTION_SIZE / 2) delta -= MAP_APE_RESOLUTION_SIZE;
    if (delta < -MAP_APE_RESOLUTION_SIZE / 2) delta += MAP_APE_RESOLUTION_SIZE;
    return delta;
}

void bronze_tasks_steer(simulated_being *being)
{
    BronzeActor *actor = bronze_actors_for_being(bronze_actors_current(), being);
    if (!actor || actor->execution.status != BRONZE_TASK_TRAVEL) return;
    BronzeTaskState *state = &actor->execution;
    BronzeRoute *route = &state->route;
    if (route->tick != bronze_native_tick() || route->next >= route->length) return;
    BronzeNeeds needs = bronze_needs_read(being);
    if (actor->work.initial_reason != BRONZE_WORK_READY ||
        bronze_needs_reason(&needs) != BRONZE_WORK_READY ||
        !being_check_goal(being, GOAL_NONE) ||
        actor->work.x != being_location_x(being) || actor->work.y != being_location_y(being)) {
        state->status = BRONZE_TASK_ROUTE_PAUSED;
        return;
    }

    uint32_t current = being_cell(being), next = route->cells[route->next];
    n_vect2 origin, direction, facing;
    being_space(being, &origin);
    n_int half = MAPSPACE_TO_APESPACE(1) / 2;
    direction.x = wrapped_delta(origin.x, MAPSPACE_TO_APESPACE(next % MAP_DIMENSION) + half);
    direction.y = wrapped_delta(origin.y, MAPSPACE_TO_APESPACE(next / MAP_DIMENSION) + half);
    being_facing_towards(being, &direction);
    being_facing_vector(being, &facing, 1);
    n_int speed = being_speed(being);
    if (speed > ROUTE_SPEED_LIMIT) speed = ROUTE_SPEED_LIMIT;
    route->attempts++;
    if (dry_edge(current, next)) {
        for (; speed > 0; speed--) {
            n_vect2 end = origin, delta;
            vect2_d(&end, &facing, speed, 512);
            delta.x = end.x - origin.x;
            delta.y = end.y - origin.y;
            being_wrap(&end);
            uint32_t step = route_cell(APESPACE_TO_MAPSPACE(end.x), APESPACE_TO_MAPSPACE(end.y));
            if ((delta.x || delta.y) && (step == current || step == next) &&
                being_can_move(&origin, &delta)) {
                being_set_speed(being, (n_byte)speed);
                being_add_state(being, BEING_STATE_MOVING);
                route->stalled = 0;
                return;
            }
        }
    }
    being_set_speed(being, 0);
    route->stalled++;
}

static double amount(const OpDef *operation)
{
    return operation->has_n0 ? operation->n0 : 1;
}

static BronzeTaskStatus eat_cargo(BronzeActor *actor, simulated_being *being,
                                 const OpDef *op, int commit)
{
    const BronzeProgram *program = bronze_actors_program(bronze_actors_current());
    const ResourceDef *definitions = program->config->resources.data;
    double nutrition = 0;
    for (size_t i = 0; i < program->config->resources.len; i++)
        if (definitions[i].kind_id == op->arg0_resource_id) nutrition = definitions[i].nutrition;
    if (!(nutrition > 0)) return BRONZE_TASK_UNSUPPORTED;
    double quantity = amount(op);
    for (size_t i = 0; i < actor->resource_count; i++) {
        BronzeStock *cargo = &actor->resources[i];
        if (cargo->kind != (unsigned)op->arg0_resource_id) continue;
        n_int deficit = BEING_FULL - being_energy(being);
        double energy = nutrition * quantity * BEING_FULL;
        if (!(quantity > 0) || quantity > cargo->amount || deficit <= 0 || energy < 1)
            return BRONZE_TASK_STOCK;
        if (commit) {
            cargo->amount -= quantity;
            BronzeLandscape *landscape = bronze_actors_landscape(bronze_actors_current());
            landscape->consumed[cargo->kind] += quantity;
            being_energy_delta(being, energy > deficit ? deficit : (n_int)energy);
        }
        return BRONZE_TASK_RUNNING;
    }
    return BRONZE_TASK_STOCK;
}

static int recipe_index(const ParsedConfig *config, const OpDef *op)
{
    if (op->recipe_index >= 0) return op->recipe_index;
    if (op->arg0_item_id < 0) return -1;
    const RecipeDef *recipes = config->recipes.data;
    int match = -1;
    for (size_t i = 0; i < config->recipes.len; i++) {
        if (recipes[i].output_item_id != op->arg0_item_id) continue;
        if (match >= 0) return -1;
        match = (int)i;
    }
    return match;
}

static BronzeTaskStatus operation_status(BronzeActor *actor, simulated_being *being, int commit)
{
    BronzeTaskState *state = &actor->execution;
    const OpDef *op = state->operation;
    if (op->action_code == BRZ_ACTION_REST) return BRONZE_TASK_RUNNING;
    if (op->action_code == BRZ_ACTION_EAT) return eat_cargo(actor, being, op, commit);
    const BronzeProgram *program = bronze_actors_program(bronze_actors_current());
    int recipe = -1;
    if (op->action_code == BRZ_ACTION_CRAFT) {
        recipe = recipe_index(program->config, op);
        if (recipe < 0) return BRONZE_TASK_UNSUPPORTED;
        double batches = amount(op);
        if (!(batches >= 1) || batches > 1024 || floor(batches) != batches) return BRONZE_TASK_STOCK;
    }
    if (op->action_code != BRZ_ACTION_MOVE_TO && op->action_code != BRZ_ACTION_GATHER &&
        op->action_code != BRZ_ACTION_DEPOSIT && op->action_code != BRZ_ACTION_FETCH &&
        op->action_code != BRZ_ACTION_CRAFT)
        return BRONZE_TASK_UNSUPPORTED;
    const BronzePlace *place = bronze_hybrid_place(state->target);
    if (!place || place->owner != actor->home) return BRONZE_TASK_TARGET;
    uint32_t cell = being_cell(being);
    if (land_topography()[cell] <= TIDE_MAX || !target_contains(state->target, cell))
        return BRONZE_TASK_TRAVEL;
    if (op->action_code == BRZ_ACTION_GATHER)
        return bronze_task_gather(actor, being, state->target, op->arg0_resource_id, amount(op), commit);
    if (op->action_code == BRZ_ACTION_DEPOSIT || op->action_code == BRZ_ACTION_FETCH) {
        int item = op->arg0_resource_id < 0;
        return bronze_task_store_transfer(actor, being, state->target,
                                         item ? op->arg0_item_id : op->arg0_resource_id,
                                         amount(op), item, op->a1 != NULL,
                                         op->action_code == BRZ_ACTION_FETCH, commit);
    }
    if (recipe >= 0) {
        const RecipeDef *definitions = program->config->recipes.data;
        return bronze_task_craft(actor, being, state->target, &definitions[recipe], &program->recipes[recipe],
                                 (unsigned)amount(op), commit);
    }
    return BRONZE_TASK_RUNNING;
}

void bronze_tasks_begin(simulated_group *group)
{
    BronzeActorRegistry *registry = bronze_actors_current();
    const BronzeProgram *program = bronze_actors_program(registry);
    const BronzeClock *clock = bronze_actors_clock(registry);
    uint64_t tick = bronze_native_tick();
    if (!program || !clock || !clock->open || clock->tick != tick) return;
    const VocationDef *vocations = program->config->vocations.data;
    unsigned searches = 0;

    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        BronzeActor *actor = bronze_actors_for_being(registry, being);
        if (!actor) continue;
        BronzeTaskState *state = &actor->execution;
        if (actor->job_status != BRONZE_JOB_ASSIGNED || !actor->home || !actor->workplace ||
            actor->vocation >= program->config->vocations.len) {
            if (state->depth || state->operation) finish(actor, BRONZE_TASK_CANCELLED, tick);
            continue;
        }
        if (state->prepared_tick == tick || tick < state->retry_at) continue;
        if ((state->claim_place || state->craft_recipe) && tick >= state->claim_until) {
            finish(actor, BRONZE_TASK_STOCK, tick);
            continue;
        }
        if (actor->work.initial_reason != BRONZE_WORK_READY) {
            if (state->status == BRONZE_TASK_TRAVEL) state->status = BRONZE_TASK_ROUTE_PAUSED;
            continue;
        }
        state->prepared_tick = tick;
        if (actor->remaining_ticks && !state->operation) continue;
        BronzeNeeds needs = bronze_needs_read(being);
        if (!state->depth && !start_task(actor, &vocations[actor->vocation], &needs, tick)) continue;
        if (!state->operation) {
            const OpDef *op = next_operation(state, &needs);
            if (!op) {
                if (!state->depth) finish(actor, BRONZE_TASK_COMPLETE, tick);
                continue;
            }
            state->operation = op;
            state->action = op->op;
            state->line = op->line;
            state->target = 0;
            if (op->action_code == BRZ_ACTION_MOVE_TO) {
                if (!strcmp(op->a0, "workplace")) {
                    state->target = actor->workplace;
                } else if (!strcmp(op->a0, "service")) {
                    const BronzeLandscape *landscape = bronze_actors_landscape(registry);
                    for (unsigned place = 0; place < landscape->place_count; place++)
                        if (landscape->places[place].kind == BRONZE_PLACE_SERVICE &&
                            landscape->places[place].owner == actor->home)
                            state->target = landscape->places[place].id;
                } else {
                    const bronze_site *site = bronze_task_move_site(actor->home, actor->workplace, op->a0);
                    if (site) state->target = site->place_id;
                }
            } else if (op->action_code == BRZ_ACTION_GATHER) {
                state->target = actor->workplace;
            } else if (op->action_code == BRZ_ACTION_DEPOSIT || op->action_code == BRZ_ACTION_FETCH) {
                state->target = op->a1 ? actor->home : bronze_task_storage(actor->home);
            } else if (op->action_code == BRZ_ACTION_CRAFT) {
                int recipe = recipe_index(program->config, op);
                if (recipe >= 0 && program->recipes[recipe].at_workplace) {
                    state->target = actor->workplace;
                } else if (recipe >= 0) {
                    const BronzeLandscape *landscape = bronze_actors_landscape(registry);
                    for (unsigned i = 0; i < landscape->town_count; i++)
                        if (landscape->towns[i].home == actor->home) state->target = landscape->towns[i].service;
                }
            }
        }
        BronzeTaskStatus status = operation_status(actor, being, 0);
        if (status == BRONZE_TASK_TRAVEL) {
            bronze_task_release_claim(bronze_actors_landscape(registry), actor);
            actor->task = BRONZE_KIND_NONE;
            actor->remaining_ticks = 0;
            prepare_route(actor, being, tick, &searches);
            continue;
        }
        if (status != BRONZE_TASK_RUNNING) {
            finish(actor, status, tick);
            continue;
        }
        if (state->target && !being_check_goal(being, GOAL_NONE)) {
            bronze_task_release_claim(bronze_actors_landscape(registry), actor);
            actor->remaining_ticks = 0;
            actor->task = BRONZE_KIND_NONE;
            state->status = BRONZE_TASK_ROUTE_PAUSED;
            continue;
        }
        if (state->status == BRONZE_TASK_TRAVEL || state->status == BRONZE_TASK_ROUTE_PAUSED)
            state->route = (BronzeRoute){0};
        state->status = BRONZE_TASK_RUNNING;
        if (!actor->remaining_ticks) {
            const OpDef *op = state->operation;
            uint64_t duration = op->action_code == BRZ_ACTION_REST ? 2 : 1;
            if (op->action_code == BRZ_ACTION_GATHER) duration = (uint64_t)ceil(amount(op));
            if (op->action_code == BRZ_ACTION_CRAFT)
                duration = (uint64_t)program->recipes[recipe_index(program->config, op)].minutes * state->craft_batches;
            if (!bronze_work_request(registry, actor->id, state->task_index, duration)) {
                finish(actor, BRONZE_TASK_CANCELLED, tick);
                continue;
            }
            actor->work.resting = op->action_code == BRZ_ACTION_REST || op->action_code == BRZ_ACTION_EAT;
        }
    }
}

static simulated_being *find_mentor(BronzeActorRegistry *registry, simulated_group *group,
                        const BronzeActor *actor, const BronzeProfessionPolicy *policy,
                        uint64_t tick)
{
    simulated_being *selected = NULL;
    uint64_t selected_id = UINT64_MAX;
    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        const BronzeActor *mentor = bronze_actors_for_being(registry, being);
        if (mentor && mentor != actor && mentor->id < selected_id && mentor->job_status == BRONZE_JOB_ASSIGNED &&
            mentor->vocation == actor->vocation && mentor->workplace == actor->workplace &&
            mentor->work.sampled_tick == tick && mentor->work.eligible &&
            target_contains(actor->execution.target, being_cell(being)) &&
            bronze_actor_skill(mentor, mentor->vocation) >= policy->mentor_skill) {
            selected = being;
            selected_id = mentor->id;
        }
    }
    return selected;
}

void bronze_tasks_end(simulated_group *group)
{
    BronzeActorRegistry *registry = bronze_actors_current();
    const BronzeProgram *program = bronze_actors_program(registry);
    const BronzeClock *clock = bronze_actors_clock(registry);
    uint64_t tick = bronze_native_tick();
    if (!program || !clock || clock->open || clock->tick != tick) return;
    for (n_uint i = 0; i < group->num; i++) {
        simulated_being *being = &group->beings[i];
        BronzeActor *actor = bronze_actors_for_being(registry, being);
        if (!actor || !actor->execution.operation || actor->remaining_ticks ||
            actor->execution.prepared_tick != tick || actor->work.sampled_tick != tick || !actor->work.consumed) continue;
        BronzeTaskState *state = &actor->execution;
        BronzeTaskStatus status = operation_status(actor, being, 1);
        if (status != BRONZE_TASK_RUNNING) {
            finish(actor, status, tick);
            continue;
        }
        const BronzeProfessionPolicy *policy = bronze_actors_professions(registry);
        if (state->operation->action_code == BRZ_ACTION_GATHER ||
            state->operation->action_code == BRZ_ACTION_CRAFT) {
            double previous_skill = bronze_actor_skill(actor, actor->vocation);
            int apprentice = previous_skill < policy->mentor_skill;
            simulated_being *mentor = apprentice ? find_mentor(registry, group, actor, policy, tick) : NULL;
            if (!apprentice || mentor) {
                bronze_actor_practice(actor, policy);
#ifdef BRONZE_SIM
                double skill = bronze_actor_skill(actor, actor->vocation);
                if (mentor && skill >= policy->mentor_skill)
                    profession_training(mentor, being, (n_byte2)(skill - previous_skill));
#endif
            }
        }
        if (state->operation->action_code == BRZ_ACTION_MOVE_TO)
            state->status = BRONZE_TASK_ARRIVED;
        state->operation = NULL;
        actor->instruction++;
    }
}

const char *bronze_task_status_name(BronzeTaskStatus status)
{
    switch (status) {
        case BRONZE_TASK_IDLE: return "idle";
        case BRONZE_TASK_RUNNING: return "running";
        case BRONZE_TASK_COMPLETE: return "complete";
        case BRONZE_TASK_NO_RULE: return "no eligible rule";
        case BRONZE_TASK_TRAVEL: return "travelling";
        case BRONZE_TASK_ROUTE_BLOCKED: return "route blocked";
        case BRONZE_TASK_ROUTE_PAUSED: return "travel paused";
        case BRONZE_TASK_ARRIVED: return "arrived";
        case BRONZE_TASK_TOOL: return "required tool missing";
        case BRONZE_TASK_SKILL: return "recipe skill too low";
        case BRONZE_TASK_RATION: return "daily ration exhausted";
        case BRONZE_TASK_CAPACITY: return "inventory full or unavailable";
        case BRONZE_TASK_STOCK: return "quantity unavailable";
        case BRONZE_TASK_UNSUPPORTED: return "operation unavailable";
        case BRONZE_TASK_CANCELLED: return "cancelled";
        case BRONZE_TASK_TARGET: return "no matching site";
    }
    return "unknown";
}
