/* Sidecar v2 stores authoritative geometry; indexes are always reconstructed. */
static int settlement_owner(const BronzeLandscape *land,uint64_t owner)
{
    for (unsigned t=0;t<land->town_count;++t) if (land->towns[t].home==owner) return 1;
    return 0;
}
static const BronzeBuilding *settlement_building(const BronzeSettlementLayout *layout,uint64_t id)
{
    for (unsigned b=0;b<layout->building_count;++b) if (layout->buildings[b].id==id) return &layout->buildings[b];
    return NULL;
}
static int settlement_adjacent(bronze_land_cell a,bronze_land_cell b)
{
    n_int dx=labs(a.x-b.x),dy=labs(a.y-b.y);
    if (dx==MAP_DIMENSION-1) dx=1;
    if (dy==MAP_DIMENSION-1) dy=1;
    return dx+dy==1;
}
static int settlement_links(BronzeLandscape *land)
{
    BronzeSettlementLayout *layout=&land->settlement;
    /* Use a separate scratch occupancy map; never trust serialized indexes. */
    n_byte *used=calloc(MAP_AREA,1); if (!used) return 0;
    int valid=1;
    for (unsigned b=0;valid && b<layout->building_count;++b) {
        const BronzeBuilding *building=&layout->buildings[b];
        if (!building->id || (building->id>>63) || !settlement_owner(land,building->owner) || !building->area) { valid=0; break; }
        for (unsigned j=0;j<b;++j) if (layout->buildings[j].id==building->id) valid=0;
        if (building->compatibility_place) {
            const BronzePlace *place=sidecar_place(land,building->compatibility_place);
            if (!place || place->owner!=building->owner) valid=0;
            if (place && (building->kind==BRONZE_BUILDING_STORE || building->profession || building->kind==BRONZE_BUILDING_PLAZA) &&
                (place->location.x!=building->entrance.x || place->location.y!=building->entrance.y)) valid=0;
        }
        if (building->profession && (building->kind!=BRONZE_BUILDING_WORK_YARD || !building->compatibility_place)) valid=0;
        int work_found=0;
        n_byte connected[BRONZE_FOOTPRINT_LIMIT]={1};
        for (unsigned pass=0;pass<building->area;++pass)
            for (unsigned c=0;c<building->area;++c)
                for (unsigned j=0;j<building->area;++j)
                    if (connected[j] && settlement_adjacent(building->cells[c],building->cells[j])) connected[c]=1;
        for (unsigned c=0;c<building->area;++c) {
            if (!connected[c]) valid=0;
            if (building->cells[c].x==building->work_position.x && building->cells[c].y==building->work_position.y) work_found=1;
            n_uint cell=(n_uint)(building->cells[c].y*MAP_DIMENSION+building->cells[c].x);
            if (used[cell]) valid=0; used[cell]=1;
        }
        if (!work_found) valid=0;
    }
    free(used);
    if (!valid) return 0;
    bronze_townships_index(layout);
    for (unsigned b=0;b<layout->building_count;++b) {
        const BronzeBuilding *building=&layout->buildings[b];
        if (layout->solid[building->entrance.y*MAP_DIMENSION+building->entrance.x]) return 0;
    }
    for (unsigned p=0;p<layout->path_count;++p) {
        const BronzePath *path=&layout->paths[p];
        if (!path->id || !path->count || !settlement_owner(land,path->owner) || path->width<1 ||
            (path->surface==BRONZE_PATH_FOOT ? path->width!=1 : path->width!=2)) return 0;
        for (unsigned j=0;j<p;++j) if (layout->paths[j].id==path->id) return 0;
        const BronzeBuilding *from=settlement_building(layout,path->from_building);
        const BronzeBuilding *to=settlement_building(layout,path->to_building);
        if (!path->from_building) {
            const BronzePlace *home=sidecar_place(land,path->owner);
            if (!home || home->location.x!=path->cells[0].x || home->location.y!=path->cells[0].y) return 0;
        }
        if (path->from_building && (!from || from->owner!=path->owner ||
            from->entrance.x!=path->cells[0].x || from->entrance.y!=path->cells[0].y)) return 0;
        bronze_land_cell end=path->cells[path->count-1];
        if (path->to_building && (!to || to->owner!=path->owner || to->entrance.x!=end.x || to->entrance.y!=end.y)) return 0;
        const BronzePlace *place=sidecar_place(land,path->to_place);
        if (path->to_place && (!place || place->owner!=path->owner)) return 0;
        if (!path->to_building && (!place || place->location.x!=end.x || place->location.y!=end.y)) return 0;
        for (unsigned c=0;c<path->count;++c) {
            n_uint cell=(n_uint)(path->cells[c].y*MAP_DIMENSION+path->cells[c].x);
            if (layout->solid[cell] || (c && !settlement_adjacent(path->cells[c-1],path->cells[c]))) return 0;
            if (path->width==2 && layout->solid[((cell/MAP_DIMENSION+1)%MAP_DIMENSION)*MAP_DIMENSION+cell%MAP_DIMENSION]) return 0;
        }
    }
    for (unsigned t=0;t<land->town_count;++t) {
        const BronzeLayoutReport *r=&layout->reports[t];
        if (r->owner!=land->towns[t].home || r->requested_housing!=land->towns[t].capacity ||
            r->placed_housing>r->requested_housing || r->missing_housing!=r->requested_housing-r->placed_housing) return 0;
    }
    return 1;
}
static void sidecar_settlement(Sidecar *codec,BronzeLandscape *land)
{
    U32(land->policy.town_house_capacity); U32(land->policy.town_work_yards);
    if (land->policy.town_house_capacity<4 || land->policy.town_house_capacity>16 ||
        land->policy.town_work_yards<1 || land->policy.town_work_yards>4) codec->valid=0;
    BronzeSettlementLayout *layout=&land->settlement;
    BOUNDED(layout->version,1); if (!layout->version) codec->valid=0;
    BOUNDED(layout->building_count,BRONZE_BUILDING_LIMIT); BOUNDED(layout->path_count,BRONZE_PATH_LIMIT);
    for (unsigned b=0;codec->valid && b<layout->building_count;++b) {
        BronzeBuilding *building=&layout->buildings[b];
        U64(building->id); U64(building->owner); U64(building->compatibility_place);
        BOUNDED(building->kind,BRONZE_BUILDING_WORK_YARD); BOUNDED(building->profession,BRONZE_PREMISE_FOOD);
        BOUNDED(building->area,BRONZE_FOOTPRINT_LIMIT); BOUNDED(building->capacity,256);
        for (unsigned c=0;codec->valid && c<building->area;++c) sidecar_cell(codec,&building->cells[c]);
        sidecar_cell(codec,&building->entrance); sidecar_cell(codec,&building->work_position);
    }
    for (unsigned p=0;codec->valid && p<layout->path_count;++p) {
        BronzePath *path=&layout->paths[p];
        U64(path->id); U64(path->owner); U64(path->from_building); U64(path->to_building); U64(path->to_place);
        BOUNDED(path->surface,BRONZE_PATH_EARTH_ROAD); BOUNDED(path->width,2); BOUNDED(path->count,BRONZE_PATH_CELL_LIMIT);
        for (unsigned c=0;codec->valid && c<path->count;++c) sidecar_cell(codec,&path->cells[c]);
    }
    for (unsigned t=0;codec->valid && t<land->town_count;++t) {
        BronzeLayoutReport *r=&layout->reports[t]; U64(r->owner);
        BOUNDED(r->requested_housing,256); BOUNDED(r->placed_housing,256); BOUNDED(r->missing_housing,256);
        BOUNDED(r->missing_store,1); BOUNDED(r->missing_plaza,1); BOUNDED(r->missing_work_yards,4);
        BOUNDED(r->missing_premises,4); BOUNDED(r->unreachable_paths,BRONZE_PATH_LIMIT);
        BOUNDED(r->limited_paths,BRONZE_PATH_LIMIT);
    }
    if (codec->valid && !settlement_links(land)) codec->valid=0;
}
