static void gather_delivery_test(simulated_group *group,BronzeLandscape *land)
{
    FILE *file=fopen("gather_delivery.bronze","w"); assert(file);
    fputs("kinds { resources { grain } }\n"
          "actions { action move_to 1 1 action gather 1 2 action deposit 1 2 }\n"
          "resources { resource grain { habitat field capacity 100 renew 0 nutrition 1 market_target 10 } }\n"
          "world { seed 42 town_capacity 8 patch_initial_per_cell 8 occupation_farmer_weight 1 occupation_farmer_workplace field }\n"
          "vocations { vocation farmer { task work {\nmove_to field\ngather grain 2\ndeposit grain 2\n} rule always { when true do work } } }\n",file);
    assert(!fclose(file)); BronzeScenario *scenario=bronze_scenario_create(); assert(scenario);
    assert(bronze_scenario_load(scenario,"gather_delivery.bronze","gather delivery"));
    BronzeActorRegistry *registry=bronze_actors_current();
    bronze_actors_stage_program(registry,bronze_scenario_program(scenario));
    bronze_actors_stage_professions(registry,bronze_scenario_professions(scenario));
    land->policy=bronze_scenario_land_policy(scenario); bronze_scenario_destroy(scenario);
    const BronzePlace *home=bronze_hybrid_place(land->towns[0].home); assert(home);
    n_byte2 start[]={MAPSPACE_TO_APESPACE(home->location.x)+32,MAPSPACE_TO_APESPACE(home->location.y)+32};
    group->num=1; simulated_being *being=&group->beings[0]; being->constant.date_of_birth=0;
    being_set_location(being,start); being_set_goal_none(being); being_set_state(being,BEING_STATE_AWAKE);
    being->delta.awake=FULLY_AWAKE; memset(being_drives(being),0,DRIVES);
    being_energy_delta(being,BEING_FULL-being_energy(being));
    advance_minutes((720+TIME_DAY_MINUTES-land_time())%TIME_DAY_MINUTES);
    assert(bronze_actors_world_reset(group)); bronze_hybrid_reset();
    BronzeActor *farmer=bronze_actors_for_being(registry,being); assert(farmer->job_status==BRONZE_JOB_ASSIGNED);
    BronzeTownship *town=NULL;
    for (unsigned t=0;t<land->town_count;++t) if (land->towns[t].home==farmer->home) town=&land->towns[t];
    assert(town); const bronze_site *field=bronze_task_work_site(farmer->workplace); assert(field);
    double before=field->resources[0].amount; unsigned moved=0,visited_field=0;
    for (unsigned step=0;step<4096 && town->resources[0]<2;++step) {
        land_cycle(); bronze_work_begin(group); bronze_tasks_begin(group);
        if (farmer->execution.status==BRONZE_TASK_TRAVEL) {
            n_int honor=0,x=being_location_x(being),y=being_location_y(being);
            being_set_speed(being,12); being->delta.awake=FULLY_AWAKE;
            being_tidy_loop_no_sim(being,&honor);
            moved|=x!=being_location_x(being) || y!=being_location_y(being);
        } else being_set_speed(being,0);
        bronze_work_end(group); bronze_tasks_end(group);
        if (farmer->resource_count && farmer->resources[0].amount==2) visited_field=1;
        n_uint cell=(n_uint)(APESPACE_TO_MAPSPACE(being_location_y(being))*MAP_DIMENSION+APESPACE_TO_MAPSPACE(being_location_x(being)));
        assert(!land->settlement.solid[cell]);
    }
    const BronzePlace *store=bronze_hybrid_place(town->storage);
    assert(moved && visited_field && town->resources[0]==2 && field->resources[0].amount==before-2);
    assert(farmer->resources[0].amount==0 && !field->resources[0].reserved);
    assert(APESPACE_TO_MAPSPACE(being_location_x(being))==store->location.x);
    assert(APESPACE_TO_MAPSPACE(being_location_y(being))==store->location.y);
    puts("Integrated home → field → store: native travel, solid avoidance and conserved grain delivery passed.");
}
