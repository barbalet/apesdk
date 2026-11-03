#include "bronze_tasks.h"
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
    state->status = status;
    state->depth = 0;
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

static int at_site(simulated_being *being, const bronze_site *site)
{
    n_int x = APESPACE_TO_MAPSPACE(being_location_x(being));
    n_int y = APESPACE_TO_MAPSPACE(being_location_y(being));
    for (n_uint i = 0; i < site->area; i++)
        if (site->cells[i].x == x && site->cells[i].y == y) return 1;
    return 0;
}

static double amount(const OpDef *operation)
{
    return operation->has_n0 ? operation->n0 : 1;
}

static BronzeTaskStatus operation_status(BronzeActor *actor, simulated_being *being, int commit)
{
    BronzeTaskState *state = &actor->execution;
    const OpDef *op = state->operation;
    if (op->action_code == BRZ_ACTION_REST) return BRONZE_TASK_RUNNING;
    if (op->action_code != BRZ_ACTION_MOVE_TO && op->action_code != BRZ_ACTION_GATHER)
        return BRONZE_TASK_UNSUPPORTED;
    const bronze_site *site = bronze_task_work_site(state->target);
    if (!site) return BRONZE_TASK_TARGET;
    if (!at_site(being, site)) return BRONZE_TASK_TRAVEL;
    if (op->action_code == BRZ_ACTION_GATHER)
        return bronze_task_gather(actor->home, state->target, op->a0, amount(op), commit);
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
        if (state->prepared_tick == tick || actor->work.initial_reason != BRONZE_WORK_READY ||
            tick < state->retry_at) continue;
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
                const bronze_site *site = bronze_task_move_site(actor->home, actor->workplace, op->a0);
                if (site) state->target = site->place_id;
            } else if (op->action_code == BRZ_ACTION_GATHER) {
                state->target = actor->workplace;
            }
        }
        BronzeTaskStatus status = operation_status(actor, being, 0);
        if (status != BRONZE_TASK_RUNNING) {
            finish(actor, status, tick);
            continue;
        }
        if (!actor->remaining_ticks) {
            const OpDef *op = state->operation;
            uint64_t duration = op->action_code == BRZ_ACTION_REST ? 2 : 1;
            if (op->action_code == BRZ_ACTION_GATHER) duration = (uint64_t)ceil(amount(op));
            if (!bronze_work_request(registry, actor->id, state->task_index, duration)) {
                finish(actor, BRONZE_TASK_CANCELLED, tick);
                continue;
            }
            actor->work.resting = op->action_code == BRZ_ACTION_REST;
        }
    }
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
        if (state->operation->action_code == BRZ_ACTION_REST)
            being_dec_drive(being, DRIVE_FATIGUE);
        if (state->operation->action_code == BRZ_ACTION_GATHER)
            bronze_actor_practice(actor, bronze_actors_professions(registry));
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
        case BRONZE_TASK_TRAVEL: return "travel required";
        case BRONZE_TASK_STOCK: return "quantity unavailable";
        case BRONZE_TASK_UNSUPPORTED: return "operation unavailable";
        case BRONZE_TASK_CANCELLED: return "cancelled";
        case BRONZE_TASK_TARGET: return "no matching site";
    }
    return "unknown";
}
