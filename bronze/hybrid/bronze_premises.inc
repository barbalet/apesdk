/* Professional premises require both an enabled occupation and a real craft
 * operation whose recipe is in the loaded scenario. No recipes are invented. */
static BronzePremiseKind premise_name(const char *name)
{
    if (strstr(name,"smith") || strstr(name,"smelt") || strstr(name,"metal")) return BRONZE_PREMISE_METAL;
    if (strstr(name,"potter") || strstr(name,"pot_")) return BRONZE_PREMISE_POTTERY;
    if (strstr(name,"carpent") || strstr(name,"woodcarv") || strstr(name,"bowyer")) return BRONZE_PREMISE_WOOD;
    if (strstr(name,"cook") || strstr(name,"baker") || strstr(name,"brewer") || strstr(name,"food")) return BRONZE_PREMISE_FOOD;
    return BRONZE_PREMISE_NONE;
}

static int premise_has_recipe(const BrzVec *body, const ParsedConfig *config)
{
    const StmtDef *statements = body->data;
    for (n_uint i=0; i<body->len; ++i) {
        const StmtDef *s = &statements[i];
        if (s->kind == ST_CHANCE && premise_has_recipe(&s->as.chance.body,config)) return 1;
        if (s->kind == ST_WHEN && premise_has_recipe(&s->as.when_stmt.body,config)) return 1;
        if (s->kind == ST_OP && s->as.op.action_code == BRZ_ACTION_CRAFT) {
            const RecipeDef *recipes = config->recipes.data;
            for (n_uint r=0; r<config->recipes.len; ++r)
                if (!strcmp(s->as.op.a0,recipes[r].name) || !strcmp(s->as.op.a0,recipes[r].output)) return 1;
        }
    }
    return 0;
}

static BronzePremiseKind premise_vocation(unsigned vocation)
{
    const BronzeActorRegistry *registry = bronze_actors_current();
    const BronzeProgram *program = bronze_actors_program(registry);
    const BronzeProfessionPolicy *policy = bronze_actors_professions(registry);
    if (!program || !policy || vocation >= policy->count || vocation >= program->config->vocations.len ||
        !policy->vocations[vocation].weight) return BRONZE_PREMISE_NONE;
    BronzePremiseKind kind = premise_name(policy->vocations[vocation].name);
    if (!kind) return kind;
    const VocationDef *vocations = program->config->vocations.data;
    const TaskDef *tasks = vocations[vocation].tasks.data;
    for (n_uint t=0; t<vocations[vocation].tasks.len; ++t)
        if (premise_has_recipe(&tasks[t].stmts,program->config)) return kind;
    return BRONZE_PREMISE_NONE;
}

uint64_t bronze_township_premise(uint64_t home, unsigned vocation)
{
    const BronzeLandscape *land = current_landscape();
    BronzePremiseKind kind = premise_vocation(vocation);
    if (!land || !kind) return 0;
    for (unsigned b=0; b<land->settlement.building_count; ++b) {
        const BronzeBuilding *building = &land->settlement.buildings[b];
        if (building->owner == home && building->profession == kind) return building->compatibility_place;
    }
    return 0;
}

static void township_bind_premises(BronzeLandscape *land)
{
    const BronzeProfessionPolicy *policy = bronze_actors_professions(bronze_actors_current());
    unsigned needed[5] = {0};
    if (policy) for (unsigned v=0; v<policy->count; ++v) needed[premise_vocation(v)] = 1;
    for (unsigned t=0; t<land->town_count; ++t) {
        BronzeTownship *town = &land->towns[t];
        /* Reuse serialized extra service places in stable kind order. */
        uint64_t existing[4] = {0}; unsigned count=0;
        for (unsigned p=0;p<land->place_count && count<4;++p)
            if (land->places[p].owner == town->home && land->places[p].kind == BRONZE_PLACE_SERVICE &&
                land->places[p].id != town->service) existing[count++] = land->places[p].id;
        unsigned slot=0;
        for (unsigned k=1;k<=4;++k) if (needed[k]) {
            BronzeBuilding *yard=NULL;
            for (unsigned b=0;b<land->settlement.building_count;++b)
                if (land->settlement.buildings[b].owner == town->home &&
                    land->settlement.buildings[b].kind == BRONZE_BUILDING_WORK_YARD &&
                    !land->settlement.buildings[b].profession) { yard=&land->settlement.buildings[b]; break; }
            if (!yard) { land->settlement.reports[t].missing_premises++; continue; }
            uint64_t place = slot<count ? existing[slot] : 0;
            if (!place && land->place_count<BRONZE_PLACE_LIMIT && land->next_place_id<UINT64_MAX)
                place = add_place(land,BRONZE_PLACE_SERVICE,yard->entrance,town->home);
            slot++;
            if (!place) { land->settlement.reports[t].missing_premises++; continue; }
            yard->profession = (BronzePremiseKind)k; yard->compatibility_place = place;
            yard->capacity = policy->site_capacity;
            yard->work_position = yard->entrance; /* open-air production station */
            for (unsigned p=0;p<land->place_count;++p)
                if (land->places[p].id == place) land->places[p].location = yard->work_position;
        }
        for (unsigned b=0;b<land->settlement.building_count;++b) {
            const BronzeBuilding *building=&land->settlement.buildings[b];
            if (building->owner != town->home || (building->kind != BRONZE_BUILDING_STORE &&
                building->kind != BRONZE_BUILDING_PLAZA)) continue;
            for (unsigned p=0;p<land->place_count;++p)
                if (land->places[p].id == building->compatibility_place) land->places[p].location=building->entrance;
        }
    }
}

void bronze_townships_prepare(void)
{
    BronzeLandscape *land=current_landscape();
    if (!land) return;
    bronze_townships_layout(land,land_topography());
    township_bind_premises(land);
    bronze_townships_paths(land,land_topography());
}
