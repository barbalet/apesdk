#include <time.h>
static double township_ms(clock_t start) { return 1000.0*(clock()-start)/CLOCKS_PER_SEC; }
static void township_frame_file(const char *name,const n_byte4 *pixels,int width,int height)
{
    const char *folder=getenv("BRONZE_TOWNSHIP_EVIDENCE"); if (!folder) return;
    char path[1024]; snprintf(path,sizeof(path),"%s/%s.ppm",folder,name);
    FILE *file=fopen(path,"wb"); assert(file); fprintf(file,"P6\n%d %d\n255\n",width,height);
    for (int y=height-1;y>=0;--y) for (int x=0;x<width;++x) {
        const n_byte *rgba=(const n_byte *)(pixels+y*width+x); assert(fwrite(rgba,1,3,file)==3);
    }
    assert(!fclose(file));
}
static void township_qualification(void)
{
    const char *names[]={"example_large","first_slice","vertical_slice","bronze_age_full"};
    for (unsigned scenario_index=0;scenario_index<4;++scenario_index) {
        char path[1024],root[1024]; snprintf(root,sizeof(root),"%s",BRONZE_EXTENDED_CONTENT);
        char *slash=strrchr(root,'/'); assert(slash); *slash=0;
        snprintf(path,sizeof(path),"%s/%s.bronze",root,names[scenario_index]);
        BronzeScenario *scenario=bronze_scenario_create(); assert(scenario && bronze_scenario_load(scenario,path,names[scenario_index]));
        shared_session *session=shared_session_create(bronze_scenario_seed(scenario)); assert(session);
        shared_session_bronze_land_policy(session,bronze_scenario_land_policy(scenario));
        shared_session_bronze_professions(session,bronze_scenario_professions(scenario));
        shared_session_bronze_program(session,bronze_scenario_program(scenario));
        assert(shared_session_init(session,NUM_CONTROL,bronze_scenario_seed(scenario))==NUM_CONTROL);
        shared_session_cycle(session,1,NUM_CONTROL);
        unsigned selected_roles=0,working_roles=0;
        BronzeActorRegistry *role_registry=bronze_actors_current();
        const BronzeProfessionPolicy *roles=bronze_actors_professions(role_registry);
        for (n_uint i=0;i<sim_group()->num;++i) {
            BronzeActor *actor=bronze_actors_for_being(role_registry,&sim_group()->beings[i]);
            if (actor->occupation<roles->count) {
                assert(roles->vocations[actor->occupation].weight);
                selected_roles++;
            }
            working_roles+=actor->job_status==BRONZE_JOB_ASSIGNED;
        }
        assert(selected_roles>=(sim_group()->num*3+4)/5);
        printf("Starting occupations %s: %u/%lu selected, %u detailed workers\n",
               names[scenario_index],selected_roles,(unsigned long)sim_group()->num,working_roles);
        BronzeLandscape *land=bronze_actors_landscape(bronze_actors_current());
        clock_t begin=clock(); bronze_townships_prepare(); double generation=township_ms(begin);
        assert(bronze_townships_validate_heights(&land->settlement,land_topography()));
        n_byte4 *pixels=calloc(768*512,sizeof(*pixels)); assert(pixels);
        BronzeWorkspacePoint center=shared_session_bronze_workspace_center(session,0);
        uint64_t town=land->town_count?land->towns[0].home:0;
        begin=clock(); shared_session_bronze_workspace_draw(session,pixels,768,512,NUM_VIEW,center,48,0.7,town);
        double map_ms=township_ms(begin); char image_name[128];
        snprintf(image_name,sizeof(image_name),"%s-map",names[scenario_index]); township_frame_file(image_name,pixels,768,512);
        begin=clock(); shared_session_bronze_workspace_draw(session,pixels,768,432,NUM_TERRAIN,center,48,0.7,town);
        double terrain_ms=township_ms(begin);
        snprintf(image_name,sizeof(image_name),"%s-terrain",names[scenario_index]); township_frame_file(image_name,pixels,768,432);
        double route_ms=0;
        if (land->town_count) {
            const BronzePlace *home=bronze_hybrid_place(town); BronzeRoute route={0};
            begin=clock(); int reached=bronze_task_plan_route(&route,(uint32_t)(home->location.y*MAP_DIMENSION+home->location.x),land->towns[0].storage);
            route_ms=township_ms(begin);
            assert(reached || land->settlement.reports[0].missing_store);
        }
        if (land->settlement.building_count) {
            const BronzeBuilding *building=&land->settlement.buildings[0];
            BronzeWorkspacePoint point={building->cells[0].x,building->cells[0].y};
            BronzeWorkspaceSite selected=shared_session_bronze_workspace_select(session,point,0.5,0.5,0.6);
            assert(selected.kind==5);
            BronzeSettlementInspection inspection=shared_session_bronze_settlement_inspection(session,selected.id,selected.kind);
            assert(inspection.building_id==building->id && inspection.township_id==building->owner);
            assert(inspection.building[0] && inspection.access[0]);
        }
#ifdef BRONZE_DISK_SAVE
        n_byte digest[32]={78}; n_file *package=shared_session_bronze_save(session,digest); assert(package);
        assert(shared_session_bronze_open(session,package->data,package->location,digest));
        n_file *resumed=shared_session_bronze_save(session,digest); assert(resumed);
        assert(package->location==resumed->location && !memcmp(package->data,resumed->data,package->location));
        io_file_free(&package); io_file_free(&resumed);
#endif
        printf("Township qualification %s: layout %.3f ms, route %.3f ms, map %.3f ms, Terrain %.3f ms; buildings %u, paths %u\n",
            names[scenario_index],generation,route_ms,map_ms,terrain_ms,
            bronze_hybrid_settlement()->building_count,bronze_hybrid_settlement()->path_count);
        free(pixels); shared_session_destroy(session); bronze_scenario_destroy(scenario); shared_close();
    }
}
