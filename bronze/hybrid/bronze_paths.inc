/* Bounded Dijkstra search; positive costs favor sharing existing tracks. */
typedef struct {
    uint32_t cost[MAP_AREA], parent[MAP_AREA], heap[MAP_AREA], position[MAP_AREA];
    n_byte blocked[MAP_AREA], worn[MAP_AREA];
    unsigned count;
} BronzePathScratch;
static void path_swap(BronzePathScratch *s, unsigned a, unsigned b)
{
    uint32_t v=s->heap[a]; s->heap[a]=s->heap[b]; s->heap[b]=v;
    s->position[s->heap[a]]=a+1; s->position[s->heap[b]]=b+1;
}
static int path_less(BronzePathScratch *s, unsigned a, unsigned b)
{
    uint32_t x=s->heap[a], y=s->heap[b];
    return s->cost[x]<s->cost[y] || (s->cost[x]==s->cost[y] && x<y);
}
static void path_offer(BronzePathScratch *s, uint32_t cell)
{
    unsigned p=s->position[cell];
    if (!p) { p=++s->count; s->heap[p-1]=cell; s->position[cell]=p; }
    --p;
    while (p && path_less(s,p,(p-1)/2)) { path_swap(s,p,(p-1)/2); p=(p-1)/2; }
}
static uint32_t path_pop(BronzePathScratch *s)
{
    uint32_t cell=s->heap[0];
    path_swap(s,0,s->count-1); --s->count; s->position[cell]=0;
    unsigned p=0;
    while (p*2+1<s->count) {
        unsigned child=p*2+1;
        if (child+1<s->count && path_less(s,child+1,child)) ++child;
        if (!path_less(s,child,p)) break;
        path_swap(s,p,child); p=child;
    }
    return cell;
}
static int path_walkable(BronzePathScratch *s,const n_byte *height,n_uint cell)
{
    return !s->blocked[cell] && height[cell]>TIDE_MAX;
}
static void township_path(BronzeLandscape *land, BronzePathScratch *s, const n_byte *height,
                          unsigned town_index, bronze_land_cell origin, uint64_t root,
                          bronze_land_cell destination, uint64_t building, uint64_t place, int road)
{
    BronzeLayoutReport *report=&land->settlement.reports[town_index];
    n_uint start=map_cell(origin.x,origin.y), goal=map_cell(destination.x,destination.y);
    if (start==goal && root==building) return;
    if (land->settlement.path_count==BRONZE_PATH_LIMIT) { report->limited_paths++; return; }
    if (!path_walkable(s,height,start) || !path_walkable(s,height,goal)) { report->unreachable_paths++; return; }
    memset(s->cost,255,sizeof(s->cost)); memset(s->position,0,sizeof(s->position)); s->count=0;
    s->cost[start]=0; s->parent[start]=(uint32_t)start; path_offer(s,(uint32_t)start);
    unsigned visited=0;
    while (s->count && visited++<MAP_AREA) {
        n_uint cell=path_pop(s);
        if (cell==goal) break;
        for (unsigned d=0;d<4;++d) {
            n_uint next=neighbor(cell,d);
            int slope=abs((int)height[next]-height[cell]);
            if (!path_walkable(s,height,next) || slope>18) continue;
            uint32_t cost=s->cost[cell]+(s->worn[next]?2:6)+(unsigned)slope;
            if (cost>=s->cost[next]) continue;
            s->cost[next]=cost; s->parent[next]=(uint32_t)cell; path_offer(s,(uint32_t)next);
        }
    }
    if (s->cost[goal]==UINT32_MAX) { report->unreachable_paths++; return; }
    n_uint reversed[BRONZE_PATH_CELL_LIMIT], length=0, cell=goal;
    for (;;) {
        if (length==BRONZE_PATH_CELL_LIMIT) { report->limited_paths++; return; }
        reversed[length++]=cell;
        if (cell==start) break;
        cell=s->parent[cell];
    }
    unsigned ordinal=land->settlement.path_count+1;
    if (report->owner>(UINT64_MAX-ordinal)/256) { report->limited_paths++; return; }
    BronzePath *path=&land->settlement.paths[land->settlement.path_count++];
    path->id=report->owner*256+ordinal; path->owner=report->owner;
    path->from_building=root; path->to_building=building; path->to_place=place;
    path->count=(unsigned)length; path->width=1; path->surface=BRONZE_PATH_FOOT;
    int wide=road;
    for (n_uint i=0;i<length;++i) {
        cell=reversed[length-1-i];
        path->cells[i]=(bronze_land_cell){cell%MAP_DIMENSION,cell/MAP_DIMENSION};
        n_uint verge=neighbor(cell,1);
        if (!path_walkable(s,height,verge) || abs((int)height[verge]-height[cell])>6) wide=0;
        if (i) {
            n_uint previous=neighbor(reversed[length-i],1);
            if (abs((int)height[verge]-height[previous])>18) wide=0;
        }
    }
    /* Width two occupies the centerline and its +Y neighbor. Downgrade narrow
     * doorways to a footpath instead of promising an obstructed road. */
    if (wide) { path->width=2; path->surface=BRONZE_PATH_EARTH_ROAD; }
    for (n_uint i=0;i<length;++i) {
        cell=map_cell(path->cells[i].x,path->cells[i].y); s->worn[cell]=1;
        if (wide) s->worn[neighbor(cell,1)]=1;
    }
}
void bronze_townships_paths(BronzeLandscape *land,const n_byte *height)
{
    if (!land || !height) return;
    memset(land->settlement.paths,0,sizeof(land->settlement.paths)); land->settlement.path_count=0;
    BronzePathScratch *s=calloc(1,sizeof(*s));
    for (unsigned t=0;t<land->town_count;++t) {
        land->settlement.reports[t].unreachable_paths=0;
        land->settlement.reports[t].limited_paths=0;
        if (!s) land->settlement.reports[t].limited_paths=1;
    }
    if (!s) { bronze_townships_index(&land->settlement); return; }
    for (unsigned b=0;b<land->settlement.building_count;++b) {
        const BronzeBuilding *building=&land->settlement.buildings[b];
        if (building->kind != BRONZE_BUILDING_HOME && building->kind != BRONZE_BUILDING_STORE) continue;
        for (unsigned c=0;c<building->area;++c) s->blocked[map_cell(building->cells[c].x,building->cells[c].y)]=1;
    }
    for (unsigned t=0;t<land->town_count;++t) {
        uint64_t owner=land->towns[t].home, root=0; bronze_land_cell origin={0,0}; int found=0;
        for (unsigned p=0;p<land->place_count;++p) if (land->places[p].id==owner) {
            origin=land->places[p].location; found=1; break;
        }
        for (unsigned b=0;b<land->settlement.building_count;++b) {
            const BronzeBuilding *building=&land->settlement.buildings[b];
            if (building->owner==owner && building->kind==BRONZE_BUILDING_PLAZA) {
                origin=building->entrance; root=building->id; found=1; break;
            }
        }
        if (!found) { land->settlement.reports[t].unreachable_paths++; continue; }
        for (unsigned b=0;b<land->settlement.building_count;++b) {
            const BronzeBuilding *building=&land->settlement.buildings[b];
            if (building->owner!=owner) continue;
            township_path(land,s,height,t,origin,root,building->entrance,building->id,
                building->compatibility_place,building->kind==BRONZE_BUILDING_STORE);
        }
        for (unsigned p=0;p<land->place_count;++p) {
            const BronzePlace *place=&land->places[p];
            if (place->owner==owner && place->kind==BRONZE_PLACE_PRODUCTION)
                township_path(land,s,height,t,origin,root,place->location,0,place->id,1);
        }
    }
    free(s);
    bronze_townships_index(&land->settlement);
}

void bronze_townships_index(BronzeSettlementLayout *layout)
{
    memset(layout->building_at,0,sizeof(layout->building_at));
    memset(layout->entrance_at,0,sizeof(layout->entrance_at));
    memset(layout->solid,0,sizeof(layout->solid));
    memset(layout->track,0,sizeof(layout->track));
    memset(layout->track_links,0,sizeof(layout->track_links));
    for (unsigned b=0;b<layout->building_count;++b) {
        const BronzeBuilding *building=&layout->buildings[b];
        layout->entrance_at[map_cell(building->entrance.x,building->entrance.y)]=(uint16_t)(b+1);
        for (unsigned c=0;c<building->area;++c) {
            n_uint cell=map_cell(building->cells[c].x,building->cells[c].y);
            layout->building_at[cell]=(uint16_t)(b+1);
            layout->solid[cell]=building->kind==BRONZE_BUILDING_HOME || building->kind==BRONZE_BUILDING_STORE;
        }
    }
    for (unsigned p=0;p<layout->path_count;++p) {
        const BronzePath *path=&layout->paths[p];
        for (unsigned c=0;c<path->count;++c) {
            n_uint cell=map_cell(path->cells[c].x,path->cells[c].y);
            n_byte surface=path->surface==BRONZE_PATH_EARTH_ROAD ? 2 : 1;
            if (layout->track[cell]<surface) layout->track[cell]=surface;
            if (path->width==2) layout->track[neighbor(cell,1)]=2;
            if (c) {
                n_uint previous=map_cell(path->cells[c-1].x,path->cells[c-1].y);
                for (unsigned d=0;d<4;++d) if (neighbor(previous,d)==cell) {
                    layout->track_links[previous]|=(n_byte)(1u<<d);
                    layout->track_links[cell]|=(n_byte)(1u<<((d+2)%4));
                }
            }
        }
    }
}

int bronze_townships_validate_heights(const BronzeSettlementLayout *layout,const n_byte *height)
{
    if (!layout || !height) return 0;
    if (!layout->version) return 1; /* legacy layout awaits its native install */
    for (unsigned b=0;b<layout->building_count;++b) {
        const BronzeBuilding *building=&layout->buildings[b]; int low=255,high=0;
        for (unsigned c=0;c<building->area;++c) {
            n_uint cell=map_cell(building->cells[c].x,building->cells[c].y);
            if (height[cell]<=TIDE_MAX) return 0;
            if (height[cell]<low) low=height[cell]; if (height[cell]>high) high=height[cell];
        }
        if (high-low>BRONZE_LAYOUT_SLOPE || height[map_cell(building->entrance.x,building->entrance.y)]<=TIDE_MAX) return 0;
    }
    for (unsigned p=0;p<layout->path_count;++p) {
        const BronzePath *path=&layout->paths[p];
        for (unsigned c=0;c<path->count;++c) {
            n_uint cell=map_cell(path->cells[c].x,path->cells[c].y);
            if (height[cell]<=TIDE_MAX) return 0;
            if (c && abs((int)height[cell]-height[map_cell(path->cells[c-1].x,path->cells[c-1].y)])>18) return 0;
            if (path->width==2) {
                n_uint verge=neighbor(cell,1);
                if (height[verge]<=TIDE_MAX || abs((int)height[verge]-height[cell])>6) return 0;
                if (c && abs((int)height[verge]-height[neighbor(map_cell(path->cells[c-1].x,path->cells[c-1].y),1)])>18) return 0;
            }
        }
    }
    return 1;
}
