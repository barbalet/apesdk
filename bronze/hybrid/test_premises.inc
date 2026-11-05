static void professional_premises_test(simulated_group *group, BronzeLandscape *land)
{
    FILE *file=fopen("smith.bronze","w"); assert(file);
    fputs("kinds { resources { copper } items { tool } }\n"
          "actions { action craft 1 1 action deposit 1 1 }\n"
          "recipes { recipe tool { output tool 1 input copper 2 } }\n"
          "world { seed 42 occupation_smith_weight 1 occupation_smith_workplace service "
          "occupation_site_capacity 1 recipe_tool_minutes 2 }\n"
          "vocations { vocation smith { task work {\ncraft tool\ndeposit tool\n} rule always { when true do work } } }\n",file);
    assert(!fclose(file));
    BronzeScenario *scenario=bronze_scenario_create(); assert(scenario);
    assert(bronze_scenario_load(scenario,"smith.bronze","professional premises fixture"));
    BronzeActorRegistry *registry=bronze_actors_current();
    bronze_actors_stage_professions(registry,bronze_scenario_professions(scenario));
    bronze_actors_stage_program(registry,bronze_scenario_program(scenario));
    land->policy=bronze_scenario_land_policy(scenario);
    land->policy.town_capacity=8;
    bronze_scenario_destroy(scenario);
    const BronzePlace *home=bronze_hybrid_place(land->towns[0].home); assert(home);
    n_byte2 start[]={MAPSPACE_TO_APESPACE(home->location.x)+32,MAPSPACE_TO_APESPACE(home->location.y)+32};
    group->num=3;
    for (unsigned i=0;i<3;++i) {
        simulated_being *being=&group->beings[i]; being->constant.date_of_birth=0;
        being_set_location(being,start); being_set_goal_none(being); being_set_state(being,BEING_STATE_AWAKE);
        being->delta.awake=FULLY_AWAKE; memset(being_drives(being),0,DRIVES);
        being_energy_delta(being,BEING_FULL-being_energy(being));
    }
    advance_minutes((720+TIME_DAY_MINUTES-land_time())%TIME_DAY_MINUTES);
    assert(bronze_actors_world_reset(group)); bronze_hybrid_reset();
    BronzeActor *smith=bronze_actors_for_being(registry,&group->beings[0]);
    BronzeActor *waiting=bronze_actors_for_being(registry,&group->beings[1]);
    assert(smith->job_status==BRONZE_JOB_ASSIGNED && waiting->job_status==BRONZE_JOB_FULL);
    assert(smith->workplace==bronze_township_premise(smith->home,smith->vocation));
    BronzeTownship *town=NULL;
    for (unsigned t=0;t<land->town_count;++t) if (land->towns[t].home==smith->home) town=&land->towns[t];
    assert(town && smith->workplace!=town->service);
    const BronzeBuilding *premise=NULL;
    for (unsigned b=0;b<land->settlement.building_count;++b)
        if (land->settlement.buildings[b].compatibility_place==smith->workplace) premise=&land->settlement.buildings[b];
    assert(premise && premise->profession==BRONZE_PREMISE_METAL && premise->capacity==1);
    const BronzePlace *workplace=bronze_hybrid_place(smith->workplace);
    assert(layout_cell(premise->entrance)==layout_cell(workplace->location));
    const BronzeProgram *program=bronze_actors_program(registry);
    const RecipeDef *recipe=program->config->recipes.data;
    town->resources[0]=2;
    assert(bronze_task_craft_target(smith,&program->recipes[0])==smith->workplace);
    assert(bronze_task_craft(smith,&group->beings[0],smith->workplace,recipe,&program->recipes[0],1,0)==BRONZE_TASK_TRAVEL);
    /* Integrate accepted steering through native movement, without teleporting. */
    unsigned moved=0;
    for (unsigned step=0;step<512 && (!smith->item_count || smith->items[0].amount<1);++step) {
        land_cycle(); bronze_work_begin(group); bronze_tasks_begin(group);
        if (smith->execution.status==BRONZE_TASK_TRAVEL) {
            n_int honor=0; n_int x=being_location_x(&group->beings[0]),y=being_location_y(&group->beings[0]);
            being_set_speed(&group->beings[0],12); group->beings[0].delta.awake=FULLY_AWAKE;
            being_tidy_loop_no_sim(&group->beings[0],&honor);
            moved |= x!=being_location_x(&group->beings[0]) || y!=being_location_y(&group->beings[0]);
        } else being_set_speed(&group->beings[0],0);
        bronze_work_end(group); bronze_tasks_end(group);
    }
    assert(moved && smith->item_count==1 && smith->items[0].amount==1);
    assert(town->resources[0]==0 && town->reserved_resources[0]==0);
    assert(APESPACE_TO_MAPSPACE(being_location_x(&group->beings[0]))==workplace->location.x);
    assert(APESPACE_TO_MAPSPACE(being_location_y(&group->beings[0]))==workplace->location.y);
    /* Alter stable IDs without regenerating: v2 must retain authored geometry. */
    assert(land->settlement.path_count);
    land->settlement.paths[0].id+=100000;
    uint64_t old_building=land->settlement.buildings[0].id;
    land->settlement.buildings[0].id+=1000000;
    for (unsigned p=0;p<land->settlement.path_count;++p) {
        if (land->settlement.paths[p].from_building==old_building) land->settlement.paths[p].from_building=land->settlement.buildings[0].id;
        if (land->settlement.paths[p].to_building==old_building) land->settlement.paths[p].to_building=land->settlement.buildings[0].id;
    }
    /* Dedicated place IDs and the derived network survive the existing sidecar. */
    n_byte digest[BRONZE_SCENARIO_DIGEST_BYTES]={34};
    BronzeSettlementLayout *expected=malloc(sizeof(*expected)); assert(expected);
    *expected=land->settlement;
    n_file *snapshot=bronze_actors_capture_sidecar(registry,digest); assert(snapshot);
    assert(snapshot->data[0]==3);
    n_byte *bad=malloc(snapshot->location); assert(bad); memcpy(bad,snapshot->data,snapshot->location);
    n_file malformed={snapshot->location,snapshot->location,bad};
    bad[0]=99; assert(!bronze_actors_stage_sidecar(registry,&malformed,digest)); bad[0]=3;
    /* Corrupt the owner of the distinctive persisted path, retaining file bounds. */
    int corrupted=0;
    for (n_uint i=0;i+16<malformed.location;++i) {
        uint64_t word=0; for (unsigned j=0;j<8;++j) word|=(uint64_t)bad[i+j]<<(8*j);
        if (word==land->settlement.paths[0].id) { memset(bad+i+8,0,8); corrupted=1; break; }
    }
    assert(corrupted && !bronze_actors_stage_sidecar(registry,&malformed,digest)); free(bad);
    assert(bronze_actors_current()==registry && smith->workplace);
    BronzeActorRegistry *staged=bronze_actors_stage_sidecar(registry,snapshot,digest); assert(staged);
    bronze_actors_activate(staged); assert(bronze_actors_rebind(staged,group));
    assert(!memcmp(expected,bronze_hybrid_settlement(),sizeof(*expected)));
    n_file *roundtrip=bronze_actors_capture_sidecar(staged,digest); assert(roundtrip);
    assert(snapshot->location==roundtrip->location && !memcmp(snapshot->data,roundtrip->data,snapshot->location));
    bronze_actors_activate(registry); assert(bronze_actors_rebind(registry,group));
    bronze_actors_destroy(staged); io_file_free(&snapshot); io_file_free(&roundtrip); free(expected);
    n_file *legacy=bronze_actors_capture_legacy_sidecar(registry,digest); assert(legacy && legacy->data[0]==1);
    staged=bronze_actors_stage_sidecar(registry,legacy,digest); assert(staged);
    assert(bronze_actors_landscape(staged)->settlement.version==0);
    bronze_actors_activate(staged); assert(bronze_actors_rebind(staged,group));
    assert(bronze_hybrid_settlement()->version==1);
    assert(bronze_actors_find(staged,smith->id)->workplace==smith->workplace);
    bronze_actors_activate(registry); assert(bronze_actors_rebind(registry,group));
    bronze_actors_destroy(staged); io_file_free(&legacy);
    uint64_t assigned=smith->workplace;
    advance_minutes(TIME_HOUR_MINUTES); bronze_hybrid_cycle(group);
    assert(smith->workplace==assigned); /* incumbent retains its slot */
    /* Continue the same worker from its workshop to the real store. */
    for (unsigned step=0;step<1024 && town->items[0]<1;++step) {
        land_cycle(); bronze_work_begin(group); bronze_tasks_begin(group);
        if (smith->execution.status==BRONZE_TASK_TRAVEL) {
            n_int honor=0; being_set_speed(&group->beings[0],12);
            group->beings[0].delta.awake=FULLY_AWAKE;
            being_tidy_loop_no_sim(&group->beings[0],&honor);
        } else being_set_speed(&group->beings[0],0);
        bronze_work_end(group); bronze_tasks_end(group);
    }
    const BronzePlace *store=bronze_hybrid_place(town->storage);
    assert(town->items[0]==1 && smith->items[0].amount==0 && town->resources[0]==0);
    assert(APESPACE_TO_MAPSPACE(being_location_x(&group->beings[0]))==store->location.x);
    assert(APESPACE_TO_MAPSPACE(being_location_y(&group->beings[0]))==store->location.y);
    puts("Integrated home → smith workshop → store: native travel and conserved tool delivery passed.");
    bronze_actors_death(group,&group->beings[0]);
    advance_minutes(TIME_HOUR_MINUTES); bronze_hybrid_cycle(group);
    assert(waiting->job_status==BRONZE_JOB_ASSIGNED && waiting->workplace==assigned);
    group->beings[1].constant.date_of_birth=land_date();
    advance_minutes(TIME_HOUR_MINUTES); bronze_hybrid_cycle(group);
    BronzeActor *replacement=bronze_actors_for_being(registry,&group->beings[2]);
    assert(waiting->job_status==BRONZE_JOB_CHILD && !waiting->workplace);
    assert(replacement->job_status==BRONZE_JOB_ASSIGNED && replacement->workplace==assigned);
    validate_paths(land,land_topography());
    puts("Professional smith: distinct workplace, native arrival, accounted recipe, retained assignment and death vacancy passed.");
}
