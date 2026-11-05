/* Included by bronze_actors.c so only the registry owner can install records. */
enum { SIDECAR_LIMIT = 16 * 1024 * 1024, SIDECAR_RESERVATION_LIMIT = 256 };

typedef struct {
    n_byte *output;
    const n_byte *input;
    n_uint size, position;
    int reading, valid;
    unsigned version;
} Sidecar;

static uint64_t sidecar_word(Sidecar *codec, uint64_t value, unsigned width)
{
    if (!codec->valid || width > codec->size - codec->position) {
        codec->valid = 0;
        return 0;
    }
    if (codec->reading) value = 0;
    for (unsigned i = 0; i < width; ++i) {
        n_uint position = codec->position++;
        if (codec->reading) value |= (uint64_t)codec->input[position] << (8 * i);
        else if (codec->output) codec->output[position] = (n_byte)(value >> (8 * i));
    }
    return value;
}

/* During the sizing pass output is null and position still advances. */
#define SIDECAR_FIELD(field, width) do { \
    uint64_t word = sidecar_word(codec, codec->reading ? 0 : (uint64_t)(field), width); \
    if (codec->reading) (field) = word; \
} while (0)
#define U64(field) SIDECAR_FIELD(field, 8)
#define U32(field) SIDECAR_FIELD(field, 4)
#define BOUNDED(field, limit) do { U32(field); if ((field) > (limit)) codec->valid = 0; } while (0)

static void sidecar_amount(Sidecar *codec, double *amount)
{
    uint64_t bits = 0;
    if (!codec->reading) memcpy(&bits, amount, sizeof(bits));
    bits = sidecar_word(codec, bits, 8);
    if (codec->reading) memcpy(amount, &bits, sizeof(bits));
    if (!isfinite(*amount) || *amount < 0) codec->valid = 0;
}

#define AMOUNT(field) sidecar_amount(codec, &(field))
#define AMOUNTS(array, count) for (unsigned i = 0; i < (count); ++i) AMOUNT((array)[i])

static void sidecar_cell(Sidecar *codec, bronze_land_cell *cell)
{
    U32(cell->x);
    U32(cell->y);
    if (cell->x < 0 || cell->y < 0 || cell->x >= MAP_DIMENSION || cell->y >= MAP_DIMENSION)
        codec->valid = 0;
}

static void sidecar_landscape(Sidecar *codec, BronzeLandscape *land)
{
    BronzeLandPolicy *policy = &land->policy;
    U32(policy->field_slope); U32(policy->field_coast); U32(policy->field_moisture);
    U32(policy->woodland_slope); U32(policy->woodland_moisture);
    U32(policy->ore_relief); U32(policy->tin_height); U32(policy->town_capacity);
    U32(policy->town_initial_grain); U32(policy->patch_initial_per_cell);
    U32(policy->carry_capacity); U32(policy->storage_capacity);
    U32(policy->household_capacity); U32(policy->ration_daily); AMOUNT(policy->food_spoilage);
    if (policy->food_spoilage > 1) codec->valid = 0;
    U64(land->next_place_id); U64(land->next_profession_tick);
    U32(land->last_day); U64(land->last_tick);
    BOUNDED(land->site_count, BRONZE_SITE_LIMIT);
    BOUNDED(land->place_count, BRONZE_PLACE_LIMIT);
    BOUNDED(land->town_count, BRONZE_TOWN_LIMIT);
    for (unsigned i = 0; codec->valid && i < land->site_count; ++i) {
        bronze_site *site = &land->sites[i];
        bronze_land_cell anchor = {site->x, site->y};
        sidecar_cell(codec, &anchor);
        if (codec->reading) { site->x = anchor.x; site->y = anchor.y; }
        BOUNDED(site->kind, BRONZE_WOODLAND); U64(site->place_id);
        AMOUNT(site->stock); AMOUNT(site->capacity);
        BOUNDED(site->area, BRONZE_PATCH_LIMIT);
        for (unsigned j = 0; codec->valid && j < site->area; ++j) sidecar_cell(codec, &site->cells[j]);
        for (unsigned j = 0; j < BRONZE_RESOURCE_LIMIT; ++j) {
            BronzePatchStock *stock = &site->resources[j];
            AMOUNT(stock->amount); AMOUNT(stock->capacity); AMOUNT(stock->reserved); AMOUNT(stock->daily_renewal);
            if (stock->reserved > stock->amount || stock->amount > stock->capacity)
                codec->valid = 0;
        }
    }
    for (unsigned i = 0; codec->valid && i < land->place_count; ++i) {
        BronzePlace *place = &land->places[i];
        U64(place->id); U64(place->owner); BOUNDED(place->kind, BRONZE_PLACE_PRODUCTION);
        sidecar_cell(codec, &place->location);
        if (!place->id || place->id > land->next_place_id) codec->valid = 0;
        for (unsigned j = 0; j < i; ++j)
            if (place->id == land->places[j].id) codec->valid = 0;
    }
    for (unsigned i = 0; codec->valid && i < land->town_count; ++i) {
        BronzeTownship *town = &land->towns[i];
        U64(town->home); U64(town->storage); U64(town->service);
        U32(town->residents); U32(town->capacity);
        AMOUNT(town->stores.grain); AMOUNT(town->stores.copper); AMOUNT(town->stores.tin);
        AMOUNT(town->stores.charcoal); AMOUNT(town->stores.tools); U64(town->stores.work_events);
        AMOUNTS(town->resources, BRONZE_RESOURCE_LIMIT);
        AMOUNTS(town->reserved_resources, BRONZE_RESOURCE_LIMIT);
        AMOUNTS(town->items, BRONZE_ITEM_LIMIT);
        AMOUNTS(town->reserved_items, BRONZE_ITEM_LIMIT);
        for (unsigned j = 0; j < BRONZE_RESOURCE_LIMIT; ++j)
            if (town->reserved_resources[j] > town->resources[j]) codec->valid = 0;
        for (unsigned j = 0; j < BRONZE_ITEM_LIMIT; ++j)
            if (town->reserved_items[j] > town->items[j]) codec->valid = 0;
        for (n_uint j = 0; codec->valid && j < MAP_AREA; ++j) BOUNDED(town->distance[j], MAP_AREA);
    }
    AMOUNTS(land->consumed, BRONZE_RESOURCE_LIMIT);
    AMOUNTS(land->lost, BRONZE_RESOURCE_LIMIT);
    AMOUNTS(land->spoiled, BRONZE_RESOURCE_LIMIT);
    AMOUNTS(land->lost_items, BRONZE_ITEM_LIMIT);
}

static const BrzVec *sidecar_body(const StmtDef *statement)
{
    if (statement->kind == ST_CHANCE) return &statement->as.chance.body;
    if (statement->kind == ST_WHEN) return &statement->as.when_stmt.body;
    return NULL;
}

/* Walk in source order to give operation pointers stable indexes, including
   diagnostic actions left behind after a nested frame finishes. */
static const OpDef *sidecar_find_op(const BrzVec *list, const char *action,
                                   uint32_t wanted, uint32_t *ordinal)
{
    const StmtDef *statements = list->data;
    for (size_t i = 0; i < list->len; ++i) {
        const StmtDef *statement = &statements[i];
        if (statement->kind == ST_OP) {
            ++*ordinal;
            if (action ? statement->as.op.op == action : *ordinal == wanted)
                return &statement->as.op;
        } else {
            const BrzVec *body = sidecar_body(statement);
            const OpDef *found = body ? sidecar_find_op(body, action, wanted, ordinal) : NULL;
            if (found) return found;
        }
    }
    return NULL;
}

static void sidecar_task(Sidecar *codec, BronzeActor *actor, const BronzeProgram *program)
{
    BronzeTaskState *task = &actor->execution;
    U64(task->route.tick); BOUNDED(task->route.from, MAP_AREA - 1);
    BOUNDED(task->route.length, BRONZE_ROUTE_LIMIT);
    BOUNDED(task->route.next, task->route.length);
    U32(task->route.stalled); U32(task->route.attempts);
    for (unsigned i = 0; codec->valid && i < task->route.length; ++i)
        BOUNDED(task->route.cells[i], MAP_AREA - 1);
    U64(task->claim_place); U64(task->claim_until);
    BOUNDED(task->claim_kind, BRONZE_RESOURCE_LIMIT - 1); AMOUNT(task->claim_amount);
    U64(task->craft_town); BOUNDED(task->craft_batches, 1024);
    U64(task->rng); U64(task->retry_at); U64(task->prepared_tick); U64(task->target);
    U32(task->task_index); BOUNDED(task->depth, 65); BOUNDED(task->status, BRONZE_TASK_PARTNER);
    U32(task->line);

    const ParsedConfig *config = program ? program->config : NULL;
    const RecipeDef *recipes = config ? config->recipes.data : NULL;
    uint32_t recipe = 0;
    if (!codec->reading && task->craft_recipe) {
        for (size_t i = 0; recipes && i < config->recipes.len; ++i)
            if (task->craft_recipe == &recipes[i]) recipe = (uint32_t)i + 1;
        if (!recipe) codec->valid = 0;
    }
    U32(recipe);
    if (recipe && (!recipes || recipe > config->recipes.len)) codec->valid = 0;
    if (codec->reading && codec->valid && recipe) task->craft_recipe = &recipes[recipe - 1];

    uint32_t owner = 0;
    if (!codec->reading && task->name) {
        const VocationDef *vocations = config ? config->vocations.data : NULL;
        for (size_t i = 0; vocations && i < config->vocations.len; ++i) {
            const BrzVec *tasks = &vocations[i].tasks;
            if (task->task_index < tasks->len &&
                ((const TaskDef *)tasks->data)[task->task_index].name == task->name) {
                owner = (uint32_t)i + 1;
                break;
            }
        }
        if (!owner) codec->valid = 0;
    }
    unsigned running = task->operation != NULL;
    BOUNDED(owner, BRONZE_VOCATION_LIMIT); BOUNDED(running, 1);
    const TaskDef *definition = NULL;
    if (owner && config && owner <= config->vocations.len) {
        const VocationDef *vocations = config->vocations.data;
        const BrzVec *tasks = &vocations[owner - 1].tasks;
        if (task->task_index < tasks->len)
            definition = &((const TaskDef *)tasks->data)[task->task_index];
    }
    if (!codec->valid || (!definition && (owner || running || task->depth)) ||
        (task->depth && owner != actor->vocation + 1)) {
        codec->valid = 0;
        return;
    }
    const BrzVec *list = definition ? &definition->stmts : NULL;
    if (codec->reading && definition) task->name = definition->name;
    uint32_t action = 0;
    if (!codec->reading && task->action && list &&
        !sidecar_find_op(list, task->action, 0, &action)) codec->valid = 0;
    U32(action);
    uint32_t ordinal = 0;
    const OpDef *op = action && list ? sidecar_find_op(list, NULL, action, &ordinal) : NULL;
    if ((action && !op) || (running && !op)) codec->valid = 0;
    if (codec->reading && op) {
        task->action = op->op;
        if (running) task->operation = op;
    }
    for (unsigned i = 0; codec->valid && i < task->depth; ++i) {
        BronzeTaskFrame *frame = &task->frames[i];
        uint32_t index = (uint32_t)frame->index;
        U32(index);
        if (!list || index > list->len || (!codec->reading && frame->statements != list)) {
            codec->valid = 0;
            return;
        }
        if (codec->reading) *frame = (BronzeTaskFrame){list, index};
        const StmtDef *previous = index ? &((const StmtDef *)list->data)[index - 1] : NULL;
        if (i + 1 == task->depth && running &&
            (!previous || previous->kind != ST_OP || &previous->as.op != op))
            codec->valid = 0;
        list = previous ? sidecar_body(previous) : NULL;
    }
    if (running && !task->depth) codec->valid = 0;
}

/* Counts precede arrays and are bounded before allocation. */
#define VECTOR(pointer, count, limit) \
    BOUNDED(count, limit); \
    if (!codec->valid) return; \
    if (codec->reading && (count)) (pointer) = calloc(count, sizeof(*(pointer))); \
    if ((count) && !(pointer)) { codec->valid = 0; return; }

static void sidecar_actor(Sidecar *codec, BronzeActor *actor, const BronzeProgram *program)
{
    U64(actor->id); U32(actor->vocation); U32(actor->task); U32(actor->instruction);
    if (codec->version>=3) {
        U32(actor->occupation);
        if (actor->occupation!=BRONZE_KIND_NONE && (actor->occupation>=BRONZE_VOCATION_LIMIT || (program && actor->occupation>=program->config->vocations.len))) codec->valid=0;
    } else if (codec->reading) actor->occupation=actor->vocation;
    BOUNDED(actor->job_status, BRONZE_JOB_MEMORY);
    U64(actor->home); U64(actor->workplace); U64(actor->remaining_ticks);
    U64(actor->mentor); U64(actor->apprentice);
    U64(actor->household_home); U32(actor->ration_day); AMOUNT(actor->ration_used);
    U64(actor->economic_partner); U64(actor->economic_memory_after); U64(actor->work_memory_after);
    BOUNDED(actor->economic_status, BRONZE_TASK_PARTNER); U32(actor->economic_action);
    AMOUNTS(actor->household_resources, BRONZE_RESOURCE_LIMIT);
    AMOUNTS(actor->household_items, BRONZE_ITEM_LIMIT);
    VECTOR(actor->skills, actor->skill_count, BRONZE_VOCATION_LIMIT);
    for (size_t i = 0; i < actor->skill_count; ++i) {
        BOUNDED(actor->skills[i].kind, BRONZE_VOCATION_LIMIT - 1); AMOUNT(actor->skills[i].value);
    }
    VECTOR(actor->resources, actor->resource_count, BRONZE_RESOURCE_LIMIT);
    for (size_t i = 0; i < actor->resource_count; ++i) {
        BOUNDED(actor->resources[i].kind, BRONZE_RESOURCE_LIMIT - 1); AMOUNT(actor->resources[i].amount);
    }
    VECTOR(actor->items, actor->item_count, BRONZE_ITEM_LIMIT);
    for (size_t i = 0; i < actor->item_count; ++i) {
        BOUNDED(actor->items[i].kind, BRONZE_ITEM_LIMIT - 1); AMOUNT(actor->items[i].amount);
    }
    VECTOR(actor->reservations, actor->reservation_count, SIDECAR_RESERVATION_LIMIT);
    for (size_t i = 0; i < actor->reservation_count; ++i) {
        U64(actor->reservations[i].place);
        BOUNDED(actor->reservations[i].kind, BRONZE_RESOURCE_LIMIT - 1);
        AMOUNT(actor->reservations[i].amount);
    }
    VECTOR(actor->history, actor->history_count, 32);
    for (size_t i = 0; i < actor->history_count; ++i) {
        U64(actor->history[i].tick); U32(actor->history[i].from_vocation); U32(actor->history[i].to_vocation);
    }

    BronzeWorkState *work = &actor->work;
    BOUNDED(work->reason, BRONZE_WORK_CLOCK); BOUNDED(work->initial_reason, BRONZE_WORK_CLOCK);
    BOUNDED(work->phase, BRONZE_WORK_RUNNING); BOUNDED(work->last_block_reason, BRONZE_WORK_CLOCK);
    U64(work->sampled_tick); U64(work->transition_tick); U64(work->transitions); U64(work->units);
    U64(work->interruptions); U64(work->resumptions);
    BOUNDED(work->x, APESPACE_BOUNDS); BOUNDED(work->y, APESPACE_BOUNDS);
    BOUNDED(work->sampled, 1); BOUNDED(work->eligible, 1); BOUNDED(work->consumed, 1);
    BOUNDED(work->running, 1); BOUNDED(work->ever_ran, 1); BOUNDED(work->resting, 1);
    sidecar_task(codec, actor, program);
}

static const BronzePlace *sidecar_place(const BronzeLandscape *land, uint64_t id)
{
    for (unsigned i = 0; i < land->place_count; ++i)
        if (land->places[i].id == id) return &land->places[i];
    return NULL;
}

static int sidecar_place_exists(const BronzeLandscape *land, uint64_t id)
{
    return !id || sidecar_place(land, id);
}

static int sidecar_owned_place(const BronzeLandscape *land, uint64_t id,
                               uint64_t owner, BronzePlaceKind kind)
{
    const BronzePlace *place = sidecar_place(land, id);
    return place && place->owner == owner && place->kind == kind;
}

static int sidecar_land_links(const BronzeLandscape *land)
{
    for (unsigned i = 0; i < land->town_count; ++i) {
        const BronzeTownship *town = &land->towns[i];
        if (!sidecar_owned_place(land, town->home, town->home, BRONZE_PLACE_HOME) ||
            !sidecar_owned_place(land, town->storage, town->home, BRONZE_PLACE_STORAGE) ||
            !sidecar_owned_place(land, town->service, town->home, BRONZE_PLACE_SERVICE))
            return 0;
    }
    for (unsigned i = 0; i < land->site_count; ++i) {
        const bronze_site *site = &land->sites[i];
        const BronzePlace *place = sidecar_place(land, site->place_id);
        if (!place || place->location.x != site->x || place->location.y != site->y ||
            (site->kind == BRONZE_TOWN ? place->kind != BRONZE_PLACE_HOME :
                                         place->kind != BRONZE_PLACE_PRODUCTION))
            return 0;
    }
    return 1;
}

#include "bronze_settlement_sidecar.inc"

static void sidecar_registry(Sidecar *codec, BronzeActorRegistry *registry, const n_byte *hash)
{
    uint32_t version = codec->version ? codec->version : 3;
    U32(version);
    if (version != 1 && version != 2 && version != 3) codec->valid = 0;
    codec->version=version;
    for (unsigned i = 0; i < BRONZE_SCENARIO_DIGEST_BYTES; ++i) {
        n_byte byte = hash[i];
        SIDECAR_FIELD(byte, 1);
        if (byte != hash[i]) codec->valid = 0;
    }
    U64(registry->next_id); U64(registry->generation); U64(registry->first_world_id);
    U64(registry->clock.origin); U64(registry->clock.tick); U64(registry->clock.elapsed);
    BOUNDED(registry->count, LARGE_SIM);
    if (!registry->next_id || registry->first_world_id > registry->next_id ||
        registry->clock.origin > registry->clock.tick ||
        registry->clock.elapsed != registry->clock.tick - registry->clock.origin)
        codec->valid = 0;
    if (!codec->valid) return;
    sidecar_landscape(codec, &registry->landscape);
    if (version>=2) sidecar_settlement(codec,&registry->landscape);
    else if (codec->reading) {
        registry->landscape.policy.town_house_capacity=8;
        registry->landscape.policy.town_work_yards=4;
        /* Legacy geometry is rebuilt only after its native world is installed. */
        registry->landscape.settlement.version=0;
    }
    const BronzeLandscape *land = &registry->landscape;
    if (codec->valid && !sidecar_land_links(land)) codec->valid = 0;
    for (unsigned i = 0; codec->valid && i < registry->count; ++i) {
        BronzeActor *actor = codec->reading ? &registry->records[i] : registry->slots[i];
        if (!actor) { codec->valid = 0; return; }
        sidecar_actor(codec, actor, registry->program);
        if (codec->reading) registry->slots[i] = actor;
        if (!actor->id || actor->id < registry->first_world_id || actor->id >= registry->next_id ||
            !sidecar_place_exists(land, actor->home) || !sidecar_place_exists(land, actor->workplace) ||
            actor->household_home > land->next_place_id ||
            !sidecar_place_exists(land, actor->execution.target) ||
            !sidecar_place_exists(land, actor->execution.claim_place) ||
            !sidecar_place_exists(land, actor->execution.craft_town))
            codec->valid = 0;
        for (size_t j = 0; codec->valid && j < actor->reservation_count; ++j)
            if (!sidecar_place_exists(land, actor->reservations[j].place)) codec->valid = 0;
        for (unsigned j = 0; j < i; ++j)
            if (actor->id == registry->slots[j]->id) codec->valid = 0;
    }
}

static n_file *sidecar_capture(BronzeActorRegistry *registry, const n_byte *scenario_digest,unsigned version)
{
    if (!is_current(registry) || !scenario_digest || registry->clock.open ||
        registry->count != registry->group->num) return NULL;
    if (!registry->landscape.settlement.version) bronze_townships_prepare();
    if (!registry->landscape.policy.town_house_capacity) registry->landscape.policy.town_house_capacity=8;
    if (!registry->landscape.policy.town_work_yards) registry->landscape.policy.town_work_yards=4;
    Sidecar codec = {.size = SIDECAR_LIMIT, .valid = 1, .version=version};
    sidecar_registry(&codec, registry, scenario_digest);
    if (!codec.valid) return NULL;
    n_file *file = memory_new(sizeof(*file));
    if (!file) return NULL;
    file->size = file->location = codec.position;
    file->data = memory_new(file->size);
    if (!file->data) { io_file_free(&file); return NULL; }
    codec = (Sidecar){.output = file->data, .size = file->size, .valid = 1, .version=version};
    sidecar_registry(&codec, registry, scenario_digest);
    if (!codec.valid || codec.position != file->size) io_file_free(&file);
    return file;
}

n_file *bronze_actors_capture_sidecar(BronzeActorRegistry *registry,const n_byte *digest)
{
    return sidecar_capture(registry,digest,3);
}
#ifdef BRONZE_SIDECAR_TEST
n_file *bronze_actors_capture_legacy_sidecar(BronzeActorRegistry *registry,const n_byte *digest)
{
    return sidecar_capture(registry,digest,1);
}
#endif

BronzeActorRegistry *bronze_actors_stage_sidecar(const BronzeActorRegistry *scenario,
                                                const n_file *file, const n_byte *scenario_digest)
{
    if (!scenario || !scenario_digest || !file || !file->data ||
        file->location > file->size || file->location > SIDECAR_LIMIT)
        return NULL;
    BronzeActorRegistry *registry = bronze_actors_create();
    if (!registry) return NULL;
    registry->professions = registry->pending_professions = scenario->professions;
    registry->program = bronze_program_retain(scenario->program);
    registry->pending_program = bronze_program_retain(scenario->program);
    Sidecar codec = {.input = file->data, .size = file->location, .reading = 1, .valid = 1};
    sidecar_registry(&codec, registry, scenario_digest);
    if (!codec.valid || codec.position != file->location) {
        /* Partial records can have allocations before their IDs are validated. */
        for (unsigned i = 0; i < LARGE_SIM; ++i) {
            BronzeActor *actor = &registry->records[i];
            free(actor->skills); free(actor->resources); free(actor->items);
            free(actor->reservations); free(actor->history);
            *actor = (BronzeActor){0};
        }
        bronze_actors_destroy(registry);
        return NULL;
    }
    return registry;
}

#undef VECTOR
#undef AMOUNTS
#undef AMOUNT
#undef BOUNDED
#undef U32
#undef U64
#undef SIDECAR_FIELD
