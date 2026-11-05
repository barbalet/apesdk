static n_uint layout_cell(bronze_land_cell cell)
{
    assert(cell.x >= 0 && cell.x < MAP_DIMENSION && cell.y >= 0 && cell.y < MAP_DIMENSION);
    return cell.y * MAP_DIMENSION + cell.x;
}

static void validate_layout(const BronzeLandscape *land, const n_byte *height)
{
    n_byte occupied_cells[MAP_AREA] = {0}, solid[MAP_AREA] = {0}, seen[MAP_AREA] = {0};
    n_uint queue[MAP_AREA];
    const BronzeSettlementLayout *layout = &land->settlement;
    assert(layout->version == 1 && layout->building_count <= BRONZE_BUILDING_LIMIT && !layout->path_count);
    for (unsigned b = 0; b < layout->building_count; ++b) {
        const BronzeBuilding *building = &layout->buildings[b];
        assert(building->id && building->owner && building->area && building->area <= BRONZE_FOOTPRINT_LIMIT);
        for (unsigned other = 0; other < b; ++other) assert(layout->buildings[other].id != building->id);
        int low = 255, high = 0;
        for (unsigned c = 0; c < building->area; ++c) {
            n_uint cell = layout_cell(building->cells[c]);
            assert(!occupied_cells[cell] && height[cell] > TIDE_MAX);
            occupied_cells[cell] = 1;
            if (building->kind == BRONZE_BUILDING_HOME || building->kind == BRONZE_BUILDING_STORE) solid[cell] = 1;
            if (height[cell] < low) low = height[cell];
            if (height[cell] > high) high = height[cell];
            for (n_uint s = 0; s < land->site_count; ++s) if (land->sites[s].kind != BRONZE_TOWN)
                for (n_uint p = 0; p < land->sites[s].area; ++p)
                    assert(cell != layout_cell(land->sites[s].cells[p]));
        }
        assert(high - low <= BRONZE_LAYOUT_SLOPE);
        assert(height[layout_cell(building->entrance)] > TIDE_MAX);
    }
    for (unsigned b = 0; b < layout->building_count; ++b)
        assert(!solid[layout_cell(layout->buildings[b].entrance)]);
    /* Independent full-map flood proves every entrance remains accessible. */
    for (unsigned t = 0; t < land->town_count; ++t) {
        memset(seen, 0, sizeof(seen));
        n_uint head = 0, tail = 0, start = 0;
        for (unsigned p = 0; p < land->place_count; ++p)
            if (land->places[p].id == land->towns[t].home) start = layout_cell(land->places[p].location);
        if (height[start] <= TIDE_MAX) continue;
        assert(!solid[start]);
        seen[start] = 1; queue[tail++] = start;
        while (head < tail) {
            n_uint cell = queue[head++];
            int x = cell % MAP_DIMENSION, y = cell / MAP_DIMENSION;
            const int dx[] = {1,-1,0,0}, dy[] = {0,0,1,-1};
            for (unsigned d = 0; d < 4; ++d) {
                n_uint next = ((y+dy[d]+MAP_DIMENSION)%MAP_DIMENSION)*MAP_DIMENSION +
                             (x+dx[d]+MAP_DIMENSION)%MAP_DIMENSION;
                if (seen[next] || solid[next] || height[next] <= TIDE_MAX ||
                    abs((int)height[next]-height[cell]) > 18) continue;
                seen[next] = 1; queue[tail++] = next;
            }
        }
        for (unsigned b = 0; b < layout->building_count; ++b)
            if (layout->buildings[b].owner == land->towns[t].home)
                assert(seen[layout_cell(layout->buildings[b].entrance)]);
        const BronzeLayoutReport *report = &layout->reports[t];
        assert(report->owner == land->towns[t].home);
        assert(report->placed_housing + report->missing_housing == report->requested_housing);
    }
}

static void spatial_townships_test(void)
{
    BronzeLandscape *a = calloc(1, sizeof(*a)), *b = calloc(1, sizeof(*b));
    n_byte *height = malloc(MAP_AREA);
    assert(a && b && height);
    memset(height, TIDE_MAX + 20, MAP_AREA);
    a->town_count = 1; a->place_count = 3;
    a->towns[0].home = 7; a->towns[0].storage = 8; a->towns[0].service = 9;
    a->towns[0].capacity = 256;
    a->places[0] = (BronzePlace){7,0,BRONZE_PLACE_HOME,{MAP_DIMENSION-1,MAP_DIMENSION-1}};
    a->places[1] = (BronzePlace){8,7,BRONZE_PLACE_STORAGE,{MAP_DIMENSION-1,MAP_DIMENSION-1}};
    a->places[2] = (BronzePlace){9,7,BRONZE_PLACE_SERVICE,{MAP_DIMENSION-1,MAP_DIMENSION-1}};
    /* Reserve an actual resource footprint inside the settlement search area. */
    a->site_count = 1; a->sites[0].kind = BRONZE_FIELD; a->sites[0].area = 1;
    a->sites[0].cells[0] = (bronze_land_cell){4,4};
    *b = *a;
    bronze_townships_layout(a, height);
    validate_layout(a, height);
    assert(a->settlement.reports[0].placed_housing == 256);
    assert(!a->settlement.reports[0].missing_store && !a->settlement.reports[0].missing_plaza);
    assert(!a->settlement.reports[0].missing_work_yards);
    bronze_townships_layout(b, height);
    assert(!memcmp(&a->settlement, &b->settlement, sizeof(a->settlement)));
    assert(!memcmp(a->places,b->places,sizeof(a->places)));
    BronzeSettlementLayout *saved = malloc(sizeof(*saved)); assert(saved); *saved = a->settlement;
    bronze_townships_layout(a, height);
    assert(!memcmp(saved, &a->settlement, sizeof(*saved))); /* stable IDs and geometry */
    /* A three-cell square island can fit its open center, not dense housing. */
    memset(height, TIDE_MAX, MAP_AREA);
    for (int y = 0; y < 3; ++y) for (int x = 0; x < 3; ++x)
        height[((MAP_DIMENSION-1+y)%MAP_DIMENSION)*MAP_DIMENSION + (MAP_DIMENSION-1+x)%MAP_DIMENSION] = TIDE_MAX+20;
    bronze_townships_layout(b, height);
    validate_layout(b,height);
    assert(b->settlement.reports[0].missing_housing == 256 && b->settlement.reports[0].missing_store);
    assert(!memcmp(saved,&a->settlement,sizeof(*saved))); /* independent worlds */
    /* Entirely wet and entirely steep terrain both fail without phantom capacity. */
    memset(height,TIDE_MAX,MAP_AREA); bronze_townships_layout(b,height);
    assert(!b->settlement.building_count && b->settlement.reports[0].missing_housing == 256);
    for (n_uint cell = 0; cell < MAP_AREA; ++cell)
        height[cell] = TIDE_MAX + 10 + ((cell % MAP_DIMENSION + cell / MAP_DIMENSION) % 2)*20;
    bronze_townships_layout(b,height); validate_layout(b,height);
    assert(!b->settlement.building_count && b->settlement.reports[0].missing_work_yards == 4);
    /* New township identity retires the old building IDs. */
    memset(height,TIDE_MAX+20,MAP_AREA); b->towns[0].home = b->places[0].id = 10;
    bronze_townships_layout(b,height);
    for (unsigned i=0;i<b->settlement.building_count;++i)
        for (unsigned j=0;j<saved->building_count;++j) assert(b->settlement.buildings[i].id != saved->buildings[j].id);
    /* Neighboring settlements share scratch occupancy but retain access. */
    b->town_count = 2; b->place_count = 4;
    b->towns[1].home = 20; b->towns[1].capacity = 256;
    b->places[3] = (BronzePlace){20,0,BRONZE_PLACE_HOME,{23,MAP_DIMENSION-1}};
    bronze_townships_layout(b,height); validate_layout(b,height);
    assert(b->settlement.reports[0].placed_housing && b->settlement.reports[1].placed_housing);
    free(saved); free(a); free(b); free(height);
    puts("Spatial township layout: dense housing, seams, dry/slope constraints, shortages, stable IDs and world isolation passed.");
}
