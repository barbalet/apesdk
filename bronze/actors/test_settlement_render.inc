static void settlement_render_test(void)
{
    n_byte *heights=malloc(MAP_AREA); BronzeSettlementLayout *layout=calloc(1,sizeof(*layout));
    assert(heights && layout); memset(heights,TIDE_MAX+30,MAP_AREA);
    layout->version=1; layout->building_count=1;
    BronzeBuilding *home=&layout->buildings[0]; home->id=1; home->kind=BRONZE_BUILDING_HOME;
    home->area=4; home->cells[0]=(bronze_land_cell){10,10}; home->cells[1]=(bronze_land_cell){11,10};
    home->cells[2]=(bronze_land_cell){10,11}; home->cells[3]=(bronze_land_cell){11,11};
    home->entrance=(bronze_land_cell){10,9};
    layout->path_count=1; layout->paths[0].count=3; layout->paths[0].width=1;
    layout->paths[0].cells[0]=(bronze_land_cell){10,7}; layout->paths[0].cells[1]=(bronze_land_cell){10,8};
    layout->paths[0].cells[2]=home->entrance;
    bronze_townships_index(layout);
    BronzeRenderLand land={.heights=heights,.tide=TIDE_MAX,.settlement=layout};
    BronzeRenderSample roof=bronze_render_sample(&land,11,11,1);
    assert(roof.height>heights[0]*0.155+1);
    /* Perspective ground height stays authoritative under roofs and thresholds. */
    assert(bronze_render_sample(&land,11,11,0).height==heights[0]*0.155);
    assert(bronze_render_sample(&land,10.1,9.5,0).height==heights[0]*0.155);
    assert(!bronze_render_tree_ground(&land,10,10) && !bronze_render_tree_ground(&land,10,8));
    assert(bronze_render_tree_ground(&land,20,20));
    BronzeRenderSample track=bronze_render_sample(&land,10.5,8.5,1);
    assert(track.height==heights[0]*0.155 && track.r>track.g && track.g>track.b);
    BronzeRenderSample doorway=bronze_render_sample(&land,10.5,9.5,0);
    assert(doorway.height==track.height && doorway.r<track.r);
    enum { width=96,height=72 };
    n_byte4 *pixels=calloc(width*height+2,sizeof(*pixels)); simulated_group *group=calloc(1,sizeof(*group));
    assert(pixels && group); pixels[0]=123; pixels[width*height+1]=456;
    bronze_render_map(pixels+1,width,height,&land,group,10,10,24,0);
    assert(pixels[0]==123 && pixels[width*height+1]==456);
    BronzeRenderCamera camera=bronze_render_camera(11,11,0.4,&land);
    bronze_render_terrain(pixels+1,width,height,&land,group,camera);
    assert(pixels[0]==123 && pixels[width*height+1]==456);
    /* Standalone geometry writes depth without changing ground samples. */
    float depths[width*height]; memset(depths,0,sizeof(depths));
    memset(pixels+1,0,width*height*sizeof(*pixels));
    BronzeRenderFrame objects={pixels+1,depths,width,height};
    bronze_render_structures(&objects,&land,camera,width*0.72,height*0.61);
    unsigned covered=0;
    for (unsigned i=0;i<width*height;++i) if (depths[i]>0) covered++;
    assert(covered>0);
    /* A nearer surface must hide the same object regardless of draw order. */
    for (unsigned i=0;i<width*height;++i) { depths[i]=0.5; pixels[i+1]=123; }
    bronze_render_structures(&objects,&land,camera,width*0.72,height*0.61);
    for (unsigned i=0;i<width*height;++i) assert(pixels[i+1]==123);
    /* Mine geometry is present independently of settlement footprints. */
    land.settlement=NULL; land.count=1;
    land.sites[0]=(BronzeRenderSite){11,11,3,BRONZE_COPPER_MINE,7};
    memset(depths,0,sizeof(depths));
    bronze_render_structures(&objects,&land,camera,width*0.72,height*0.61);
    covered=0; for (unsigned i=0;i<width*height;++i) if (depths[i]>0) covered++;
    assert(covered>0 && bronze_render_sample(&land,11,11,0).height==heights[0]*0.155);
    land.settlement=layout; land.count=0;
    /* Near-plane clipping and cameras inside premises keep frame bounds. */
    camera=bronze_render_camera_distance(11,11,0.4,&land,0,1);
    bronze_render_terrain(pixels+1,width,height,&land,group,camera);
    assert(pixels[0]==123 && pixels[width*height+1]==456);
    /* Cached seam footprints retain their world-space geometry in both cameras. */
    home->cells[0]=(bronze_land_cell){MAP_DIMENSION-1,10}; home->cells[1]=(bronze_land_cell){0,10};
    home->cells[2]=(bronze_land_cell){MAP_DIMENSION-1,11}; home->cells[3]=(bronze_land_cell){0,11};
    bronze_townships_index(layout);
    assert(bronze_render_sample(&land,0,11,1).height>heights[0]*0.155+1);
    /* Exercise shared world coordinates across resize, pan and actor movement. */
    group->num=1; group->select=group->beings;
    const int widths[]={96,127,320}, heights_px[]={72,96,240};
    for (unsigned frame=0;frame<3;++frame) {
        int w=widths[frame],h=heights_px[frame];
        n_byte4 *resized=calloc((n_uint)w*h+2,sizeof(*resized)); assert(resized);
        resized[0]=789; resized[w*h+1]=987;
        n_byte2 position[]={MAPSPACE_TO_APESPACE(10+frame),MAPSPACE_TO_APESPACE(9+frame)};
        being_set_location(group->beings,position);
        bronze_render_map(resized+1,w,h,&land,group,10+(int)frame,10,24,0);
        camera=bronze_render_camera_distance(11,11,0.4+frame*0.1,&land,32,8);
        bronze_render_terrain(resized+1,w,h,&land,group,camera);
        assert(resized[0]==789 && resized[w*h+1]==987);
        assert(APESPACE_TO_MAPSPACE(being_location_x(group->beings))==10+frame);
        free(resized);
    }
    free(pixels);free(group);free(heights);free(layout);
    puts("Settlement rendering: shared footprints, roofs, thresholds, path height, vegetation clearance, seams and frame bounds passed.");
}
