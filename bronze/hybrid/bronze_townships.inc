/* Included only by the Bronze hybrid translation unit. All scratch storage is
 * call-owned: layout generation cannot leak another world's occupancy. */
typedef struct {
    n_byte used[MAP_AREA], blocked[MAP_AREA], seen[MAP_AREA], forbidden[MAP_AREA];
    n_uint queue[MAP_AREA];
} BronzeLayoutScratch;

static int township_delta(n_int a, n_int b)
{
    return (int)((a - b + MAP_DIMENSION * 3 / 2) % MAP_DIMENSION) - MAP_DIMENSION / 2;
}

static int township_local(n_uint cell, bronze_land_cell center)
{
    return abs(township_delta(cell % MAP_DIMENSION, center.x)) <= BRONZE_LAYOUT_RADIUS &&
           abs(township_delta(cell / MAP_DIMENSION, center.y)) <= BRONZE_LAYOUT_RADIUS;
}

static void township_reachable(BronzeLayoutScratch *scratch, const n_byte *height,
                                bronze_land_cell center)
{
    memset(scratch->seen, 0, sizeof(scratch->seen));
    n_uint head = 0, tail = 0, start = map_cell(center.x, center.y);
    if (height[start] <= TIDE_MAX || scratch->blocked[start] || scratch->forbidden[start]) return;
    scratch->seen[start] = 1;
    scratch->queue[tail++] = start;
    while (head < tail) {
        n_uint cell = scratch->queue[head++];
        for (unsigned d = 0; d < 4; ++d) {
            n_uint next = neighbor(cell, d);
            if (scratch->seen[next] || scratch->blocked[next] || scratch->forbidden[next] ||
                height[next] <= TIDE_MAX || !township_local(next, center) ||
                abs((int)height[next] - height[cell]) > 18) continue;
            scratch->seen[next] = 1;
            scratch->queue[tail++] = next;
        }
    }
}

static int township_candidate(BronzeLandscape *land, BronzeLayoutScratch *scratch,
                               const n_byte *height, bronze_land_cell center,
                               BronzeBuilding *building, n_int x, n_int y, int width,
                               unsigned door)
{
    static const int ex[] = {0, 2, 0, -1}, ey[] = {-1, 0, 2, 0};
    int low = 255, high = 0;
    building->area = 0;
    for (int dy = 0; dy < width; ++dy) for (int dx = 0; dx < width; ++dx) {
        n_uint cell = map_cell(x + dx, y + dy);
        if (!township_local(cell, center) || scratch->used[cell] || scratch->forbidden[cell] ||
            height[cell] <= TIDE_MAX) return 0;
        if (height[cell] < low) low = height[cell];
        if (height[cell] > high) high = height[cell];
        building->cells[building->area++] = (bronze_land_cell){cell % MAP_DIMENSION, cell / MAP_DIMENSION};
    }
    if (high - low > BRONZE_LAYOUT_SLOPE) return 0;
    /* Plazas and yards are open areas. Their entrance is on their footprint. */
    int solid = building->kind == BRONZE_BUILDING_HOME || building->kind == BRONZE_BUILDING_STORE;
    n_uint entry = solid ? map_cell(x + ex[door], y + ey[door]) : map_cell(x, y);
    if (solid && (scratch->used[entry] || scratch->forbidden[entry])) return 0;
    if (!township_local(entry, center) || height[entry] <= TIDE_MAX ||
        abs((int)height[entry] - height[map_cell(x, y)]) > BRONZE_LAYOUT_SLOPE) return 0;
    if (solid) for (unsigned i = 0; i < building->area; ++i)
        scratch->blocked[map_cell(building->cells[i].x, building->cells[i].y)] = 1;
    township_reachable(scratch, height, center);
    int reachable = scratch->seen[entry];
    /* Preserve access in neighboring townships too, where layout radii overlap. */
    for (unsigned t = 0; reachable && t < land->town_count; ++t) {
        uint64_t owner = land->towns[t].home;
        bronze_land_cell origin = center;
        if (owner != building->owner) {
            int found = 0;
            for (unsigned p = 0; p < land->place_count; ++p) if (land->places[p].id == owner) {
                origin = land->places[p].location; found = 1; break;
            }
            if (!found) continue;
            township_reachable(scratch, height, origin);
        } else township_reachable(scratch, height, center);
        for (unsigned i = 0; reachable && i < land->settlement.building_count; ++i) {
            const BronzeBuilding *other = &land->settlement.buildings[i];
            if (other->owner == owner && !scratch->seen[map_cell(other->entrance.x, other->entrance.y)])
                reachable = 0;
        }
    }
    if (!reachable) {
        if (solid) for (unsigned i = 0; i < building->area; ++i)
            scratch->blocked[map_cell(building->cells[i].x, building->cells[i].y)] = 0;
        return 0;
    }
    for (unsigned i = 0; i < building->area; ++i)
        scratch->used[map_cell(building->cells[i].x, building->cells[i].y)] = 1;
    scratch->used[entry] = 1; /* reserve doorway against subsequent footprints */
    building->entrance = (bronze_land_cell){entry % MAP_DIMENSION, entry / MAP_DIMENSION};
    building->work_position = building->cells[0];
    return 1;
}

typedef struct { int x,y; unsigned score; } BronzeLayoutCandidate;
static int township_candidate_order(const void *a,const void *b)
{
    const BronzeLayoutCandidate *left=a,*right=b;
    if (left->score!=right->score) return left->score<right->score?-1:1;
    if (left->y!=right->y) return left->y<right->y?-1:1;
    return left->x<right->x?-1:left->x>right->x;
}

static int township_place(BronzeLandscape *land, BronzeLayoutScratch *scratch,
                           const n_byte *height, bronze_land_cell center,
                           uint64_t owner, unsigned ordinal, BronzeBuildingKind kind,
                           uint64_t compatibility, unsigned capacity)
{
    if (land->settlement.building_count == BRONZE_BUILDING_LIMIT || owner > (UINT64_MAX - ordinal) / 128)
        return 0;
    BronzeBuilding building = {0};
    building.id = owner * 128 + ordinal;
    building.owner = owner; building.compatibility_place = compatibility;
    building.kind = kind; building.capacity = capacity;
    int width = kind == BRONZE_BUILDING_PLAZA ? 3 : 2;
    /* Rank a circular neighborhood by terrain, spacing and a stable organic
     * offset. Avoid row-by-row square rings that pack every village into rows. */
    enum { side=BRONZE_LAYOUT_RADIUS*2+1 };
    BronzeLayoutCandidate candidates[side*side]; unsigned count=0;
    for (int dy=-BRONZE_LAYOUT_RADIUS;dy<=BRONZE_LAYOUT_RADIUS;++dy)
        for (int dx=-BRONZE_LAYOUT_RADIUS;dx<=BRONZE_LAYOUT_RADIUS;++dx) {
            int distance=dx*dx+dy*dy;
            if (distance>BRONZE_LAYOUT_RADIUS*BRONZE_LAYOUT_RADIUS) continue;
            n_int x=center.x+dx,y=center.y+dy; int low=255,high=0,valid=1;
            for (int cy=0;cy<width;++cy) for (int cx=0;cx<width;++cx) {
                n_uint cell=map_cell(x+cx,y+cy);
                if (scratch->used[cell] || scratch->forbidden[cell] || height[cell]<=TIDE_MAX ||
                    !township_local(cell,center)) valid=0;
                if (height[cell]<low) low=height[cell];
                if (height[cell]>high) high=height[cell];
            }
            if (!valid || high-low>BRONZE_LAYOUT_SLOPE) continue;
            unsigned alignment=0,nearby=0;
            for (unsigned i=0;i<land->settlement.building_count;++i) {
                const BronzeBuilding *other=&land->settlement.buildings[i];
                int ax=abs(township_delta(x,other->cells[0].x));
                int ay=abs(township_delta(y,other->cells[0].y));
                if (ax<5 && ay<5) {
                    alignment+=(ax==0 || ay==0);
                    nearby+=(ax<=2 && ay<=2);
                }
            }
            uint64_t noise=(uint64_t)map_cell(x,y)*UINT64_C(374761393)+building.id*UINT64_C(668265263);
            noise=(noise^(noise>>13))*UINT64_C(1274126177); noise^=noise>>16;
            unsigned score=(unsigned)distance*12+(unsigned)(high-low)*20+
                           alignment*100+nearby*110+(unsigned)(noise%140);
            candidates[count++]=(BronzeLayoutCandidate){dx,dy,score};
        }
    qsort(candidates,count,sizeof(*candidates),township_candidate_order);
    for (unsigned i=0;i<count;++i) {
        int dx=candidates[i].x,dy=candidates[i].y;
        /* Prefer doors facing the township center; diagonally staggered homes
         * do not inherit a common street-facing cardinal direction. */
        unsigned preferred=abs(dx)>abs(dy)?(dx>0?3:1):(dy>0?0:2);
        for (unsigned d=0;d<4;++d) {
            if (township_candidate(land,scratch,height,center,&building,
                                   center.x+dx,center.y+dy,width,(preferred+d)%4)) {
                land->settlement.buildings[land->settlement.building_count++]=building;
                return 1;
            }
        }
    }
    return 0;
}

void bronze_townships_layout(BronzeLandscape *land, const n_byte *height)
{
    if (!land || !height) return;
    memset(&land->settlement, 0, sizeof(land->settlement));
    BronzeLayoutScratch *scratch = calloc(1, sizeof(*scratch));
    /* On allocation failure leave version zero so a later access can retry. */
    if (!scratch) return;
    for (n_uint s = 0; s < land->site_count; ++s) {
        const bronze_site *site = &land->sites[s];
        if (site->kind == BRONZE_TOWN) continue;
        for (n_uint c = 0; c < site->area; ++c)
            scratch->forbidden[map_cell(site->cells[c].x, site->cells[c].y)] = 1;
    }
    for (unsigned t = 0; t < land->town_count; ++t) {
        const BronzeTownship *town = &land->towns[t];
        BronzeLayoutReport *report = &land->settlement.reports[t];
        report->owner = town->home;
        report->requested_housing = town->capacity;
        bronze_land_cell center = {0, 0};
        int found = 0;
        for (unsigned p = 0; p < land->place_count; ++p) if (land->places[p].id == town->home) {
            center = land->places[p].location; found = 1; break;
        }
        unsigned ordinal = 1;
        report->missing_plaza = !found || !township_place(land, scratch, height, center, town->home,
            ordinal++, BRONZE_BUILDING_PLAZA, town->service, 0);
        report->missing_store = !found || !township_place(land, scratch, height, center, town->home,
            ordinal++, BRONZE_BUILDING_STORE, town->storage, 0);
        /* Reserve economic space before housing fills marginal terrain. */
        unsigned yards=land->policy.town_work_yards ? land->policy.town_work_yards : 4;
        for (unsigned w = 0; w < yards; ++w)
            if (!found || !township_place(land, scratch, height, center, town->home, ordinal++,
                    BRONZE_BUILDING_WORK_YARD, 0, 0)) report->missing_work_yards++;
        unsigned house_capacity=land->policy.town_house_capacity ? land->policy.town_house_capacity : BRONZE_HOUSE_CAPACITY;
        unsigned homes = (town->capacity + house_capacity - 1) / house_capacity;
        if (homes > 32) homes = 32;
        for (unsigned h = 0; h < homes; ++h) {
            unsigned capacity = town->capacity - h * house_capacity;
            if (capacity > house_capacity) capacity = house_capacity;
            if (found && township_place(land, scratch, height, center, town->home, ordinal++,
                    BRONZE_BUILDING_HOME, town->home, capacity)) report->placed_housing += capacity;
        }
        report->missing_housing = report->requested_housing - report->placed_housing;
    }
    land->settlement.version = 1;
    free(scratch);
}

const BronzeSettlementLayout *bronze_hybrid_settlement(void)
{
    BronzeLandscape *land = current_landscape();
    if (!land) return NULL;
    /* Until cycle 7 these immutable records are regenerated after disk import.
     * Existing sidecar format, place IDs and inventories stay unchanged. */
    if (!land->settlement.version) bronze_townships_prepare();
    return &land->settlement;
}
