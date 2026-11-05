static void validate_paths(const BronzeLandscape *land,const n_byte *height)
{
    n_byte blocked[MAP_AREA]={0};
    const BronzeSettlementLayout *layout=&land->settlement;
    for (unsigned b=0;b<layout->building_count;++b) {
        const BronzeBuilding *building=&layout->buildings[b];
        if (building->kind==BRONZE_BUILDING_STORE || building->kind==BRONZE_BUILDING_HOME)
            for (unsigned c=0;c<building->area;++c) blocked[layout_cell(building->cells[c])]=1;
    }
    assert(layout->path_count<=BRONZE_PATH_LIMIT);
    for (unsigned p=0;p<layout->path_count;++p) {
        const BronzePath *path=&layout->paths[p];
        assert(path->id && path->owner && path->count && path->count<=BRONZE_PATH_CELL_LIMIT);
        for (unsigned q=0;q<p;++q) assert(path->id!=layout->paths[q].id);
        assert(path->width==1 || path->width==2);
        assert((path->surface==BRONZE_PATH_EARTH_ROAD)==(path->width==2));
        for (unsigned c=0;c<path->count;++c) {
            n_uint cell=layout_cell(path->cells[c]);
            assert(height[cell]>TIDE_MAX && !blocked[cell]);
            if (c) {
                int dx=abs((int)(path->cells[c].x-path->cells[c-1].x));
                int dy=abs((int)(path->cells[c].y-path->cells[c-1].y));
                if (dx==MAP_DIMENSION-1) dx=1;
                if (dy==MAP_DIMENSION-1) dy=1;
                assert(dx+dy==1);
                assert(abs((int)height[cell]-height[layout_cell(path->cells[c-1])])<=18);
            }
            if (path->width==2) {
                n_uint verge=((cell/MAP_DIMENSION+1)%MAP_DIMENSION)*MAP_DIMENSION+cell%MAP_DIMENSION;
                assert(height[verge]>TIDE_MAX && !blocked[verge]);
                assert(abs((int)height[verge]-height[cell])<=6);
                if (c) {
                    n_uint previous=layout_cell(path->cells[c-1]);
                    previous=((previous/MAP_DIMENSION+1)%MAP_DIMENSION)*MAP_DIMENSION+previous%MAP_DIMENSION;
                    assert(abs((int)height[verge]-height[previous])<=18);
                }
            }
        }
        for (unsigned b=0;b<layout->building_count;++b)
            if (layout->buildings[b].id==path->to_building)
                assert(layout_cell(layout->buildings[b].entrance)==layout_cell(path->cells[path->count-1]));
    }
}
static void township_paths_test(void)
{
    BronzeLandscape *a=calloc(1,sizeof(*a)), *b=calloc(1,sizeof(*b));
    n_byte *height=malloc(MAP_AREA); assert(a && b && height);
    memset(height,TIDE_MAX+20,MAP_AREA);
    a->town_count=1; a->place_count=1; a->towns[0].home=1; a->towns[0].capacity=128;
    a->places[0]=(BronzePlace){1,1,BRONZE_PLACE_HOME,{MAP_DIMENSION-1,MAP_DIMENSION-1}};
    bronze_townships_layout(a,height); *b=*a;
    bronze_townships_paths(a,height); validate_paths(a,height);
    assert(bronze_townships_validate_heights(&a->settlement,height));
    n_uint flooded=layout_cell(a->settlement.buildings[0].cells[0]);
    height[flooded]=TIDE_MAX;
    assert(!bronze_townships_validate_heights(&a->settlement,height));
    height[flooded]=TIDE_MAX+20;

    assert(a->settlement.path_count==a->settlement.building_count-1);
    assert(!a->settlement.reports[0].unreachable_paths && !a->settlement.reports[0].limited_paths);
    bronze_townships_paths(b,height);
    assert(!memcmp(&a->settlement,&b->settlement,sizeof(a->settlement)));
    /* Segment reuse is observable: paths share more than their single root. */
    int shared=0;
    for (unsigned p=1;p<a->settlement.path_count && !shared;++p)
        for (unsigned q=0;q<p && !shared;++q)
            for (unsigned c=1;c<a->settlement.paths[p].count && !shared;++c)
                for (unsigned d=1;d<a->settlement.paths[q].count;++d)
                    if (layout_cell(a->settlement.paths[p].cells[c])==layout_cell(a->settlement.paths[q].cells[d])) { shared=1; break; }
    assert(shared);
    memset(b,0,sizeof(*b)); b->town_count=1; b->place_count=2; b->towns[0].home=1;
    b->settlement.reports[0].owner=1;
    b->places[0]=(BronzePlace){1,1,BRONZE_PLACE_HOME,{0,0}};
    b->places[1]=(BronzePlace){2,1,BRONZE_PLACE_PRODUCTION,{10,0}};
    bronze_townships_paths(b,height); validate_paths(b,height);
    assert(b->settlement.path_count==1 && b->settlement.paths[0].width==2);
    assert(b->settlement.paths[0].to_place==2);
    height[10]=TIDE_MAX;
    bronze_townships_paths(b,height);
    assert(!b->settlement.path_count && b->settlement.reports[0].unreachable_paths==1);
    height[10]=TIDE_MAX+20;
    b->places[1].location=(bronze_land_cell){MAP_DIMENSION/2,MAP_DIMENSION/2};
    bronze_townships_paths(b,height);
    assert(!b->settlement.path_count && b->settlement.reports[0].limited_paths==1);
    assert(a->settlement.path_count); /* another world's failed search stays local */
    free(a); free(b); free(height);
    puts("Township paths: deterministic shared segments, doors, seams, roads, unreachable sites and length bounds passed.");
}
