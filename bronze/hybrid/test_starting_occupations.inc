static void starting_occupations_test(simulated_group *group)
{
    BronzeActorRegistry *registry=bronze_actors_current();
    BronzeProfessionPolicy policy=*bronze_actors_professions(registry);
    policy.count=1; policy.vocations[0].weight=1; policy.vocations[0].workplace=BRONZE_JOB_COAST;
    bronze_actors_stage_professions(registry,&policy);
    group->num=5;
    for (n_uint i=0;i<group->num;++i) group->beings[i].constant.date_of_birth=land_date();
    assert(bronze_actors_world_reset(group)); bronze_hybrid_reset();
    unsigned selected=0; uint32_t labels[5];
    for (n_uint i=0;i<group->num;++i) {
        BronzeActor *actor=bronze_actors_for_being(registry,&group->beings[i]);
        labels[i]=actor->occupation; selected+=actor->occupation==0;
        assert(actor->vocation==BRONZE_KIND_NONE && !actor->workplace && !actor->skill_count);
        assert(actor->job_status==BRONZE_JOB_CHILD);
    }
    assert(selected==3);
    advance_minutes(TIME_HOUR_MINUTES); bronze_hybrid_cycle(group);
    for (n_uint i=0;i<group->num;++i)
        assert(bronze_actors_for_being(registry,&group->beings[i])->occupation==labels[i]);
    group->num=1;
    assert(bronze_actors_world_reset(group)); bronze_hybrid_reset();
    assert(bronze_actors_for_being(registry,&group->beings[0])->occupation==0); /* ceil(0.6) */
    policy.vocations[0].weight=0; bronze_actors_stage_professions(registry,&policy);
    assert(bronze_actors_world_reset(group)); bronze_hybrid_reset();
    assert(bronze_actors_for_being(registry,&group->beings[0])->occupation==BRONZE_KIND_NONE);
    puts("Starting roles: 60% coverage, small populations, stable labels, disabled policy and no child work/skills passed.");
}
