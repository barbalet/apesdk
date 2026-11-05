static void weighted_routes_test(void)
{
    BronzeLandscape *land=bronze_actors_landscape(bronze_actors_current());
    BronzeLandscape *saved=malloc(sizeof(*saved)); n_byte *height=land_topography(),*old=malloc(MAP_AREA);
    assert(saved && old); *saved=*land; memcpy(old,height,MAP_AREA);
    memset(land,0,sizeof(*land)); memset(height,TIDE_MAX+20,MAP_AREA);
    land->settlement.version=1; land->place_count=1;
    land->places[0]=(BronzePlace){900,1,BRONZE_PLACE_SERVICE,{16,10}};
    uint32_t start=10+10*MAP_DIMENSION;
    BronzeRoute route={0};
    for (unsigned x=10;x<=16;++x) land->settlement.track[11*MAP_DIMENSION+x]=1;
    assert(bronze_task_plan_route(&route,start,900) && route.length==8);
    int uses_path=0;
    for (unsigned i=0;i<route.length;++i) uses_path|=route.cells[i]/MAP_DIMENSION==11;
    assert(uses_path);
    memset(land->settlement.track,0,sizeof(land->settlement.track));
    for (unsigned x=10;x<=16;++x) land->settlement.track[20*MAP_DIMENSION+x]=1;
    for (unsigned y=10;y<=20;++y) {
        land->settlement.track[y*MAP_DIMENSION+10]=1;
        land->settlement.track[y*MAP_DIMENSION+16]=1;
    }
    assert(bronze_task_plan_route(&route,start,900) && route.length==6); /* reject costly detour */
    memset(land->settlement.track,0,sizeof(land->settlement.track));
    land->settlement.solid[13+10*MAP_DIMENSION]=1;
    assert(bronze_task_plan_route(&route,start,900));
    for (unsigned i=0;i<route.length;++i) assert(!land->settlement.solid[route.cells[i]]);
    /* A previously valid route becomes obstructed; a fresh plan uses another cell. */
    land->settlement.solid[route.cells[0]]=1;
    assert(bronze_task_plan_route(&route,start,900));
    for (unsigned i=0;i<route.length;++i) assert(!land->settlement.solid[route.cells[i]]);
    height[start-1]=height[start+1]=height[start-MAP_DIMENSION]=height[start+MAP_DIMENSION]=TIDE_MAX;
    assert(!bronze_task_plan_route(&route,start,900));
    assert(!bronze_task_plan_route(&route,MAP_AREA,900));
    memcpy(height,old,MAP_AREA); *land=*saved; free(saved); free(old);
    puts("Weighted agent routes: useful paths, excessive detours, solid avoidance and blocked replanning passed.");
}
