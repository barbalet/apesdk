/* Included inside shared.c's BRONZE_SIM region to activate the proper session. */
BronzeSettlementInspection shared_session_bronze_settlement_inspection(shared_session *session,
                                                                      uint64_t selection,n_int kind)
{
    BronzeSettlementInspection result={0};
    if (!shared_session_is_started(session) || !shared_core_activate(&session->core)) return result;
    const BronzeSettlementLayout *layout=bronze_hybrid_settlement();
    const BronzeLandscape *land=bronze_actors_landscape(session->core.bronze_registry);
    if (!layout || !land) return result;
    uint64_t owner=0; const BronzeBuilding *picked=NULL;
    if (kind==5) {
        uint64_t id=selection & ~(UINT64_C(1)<<63);
        for (unsigned b=0;b<layout->building_count;++b)
            if (layout->buildings[b].id==id) { picked=&layout->buildings[b]; owner=picked->owner; break; }
    } else if (selection) {
        for (unsigned p=0;p<land->place_count;++p)
            if (land->places[p].id==selection) { owner=land->places[p].owner; break; }
    } else {
        BronzeActor *actor=bronze_actors_for_being(session->core.bronze_registry,sim_group()->select);
        if (actor) {
            owner=actor->home;
            for (unsigned b=0;b<layout->building_count;++b)
                if (actor->workplace && layout->buildings[b].compatibility_place==actor->workplace &&
                    layout->buildings[b].profession) { picked=&layout->buildings[b]; break; }
        }
    }
    if (!owner) return result;
    result.township_id=owner;
    for (unsigned t=0;t<land->town_count;++t) if (land->towns[t].home==owner) {
        result.members=land->towns[t].residents;
        result.housing=layout->reports[t].placed_housing;
        result.requested_housing=layout->reports[t].requested_housing;
        result.missing_connections=layout->reports[t].unreachable_paths+layout->reports[t].limited_paths;
    }
    if (picked) {
        static const char *names[]={"Open center","Store","Home","Work yard"};
        static const char *jobs[]={"Unassigned","Metalworking","Pottery","Woodworking","Food preparation"};
        result.building_id=picked->id; result.capacity=picked->capacity;
        snprintf(result.building,sizeof(result.building),"%s",names[picked->kind]);
        snprintf(result.occupation,sizeof(result.occupation),"%s",jobs[picked->profession]);
        for (n_uint i=0;i<sim_group()->num;++i) {
            const BronzeActor *actor=bronze_actors_for_being(session->core.bronze_registry,&sim_group()->beings[i]);
            if (actor && actor->job_status==BRONZE_JOB_ASSIGNED && picked->profession &&
                (actor->workplace==picked->compatibility_place ||
                 (actor->home==picked->owner && bronze_township_premise(actor->home,actor->vocation)==picked->compatibility_place)))
                result.assigned_workers++;
        }
        for (unsigned p=0;p<layout->path_count;++p)
            if (layout->paths[p].to_building==picked->id || layout->paths[p].from_building==picked->id) result.connected=1;
        snprintf(result.access,sizeof(result.access),"%s",result.connected?"Connected to walking network":"No recorded walking connection");
    } else {
        snprintf(result.building,sizeof(result.building),"Township");
        snprintf(result.access,sizeof(result.access),"%u missing or limited connections",result.missing_connections);
    }
    return result;
}
